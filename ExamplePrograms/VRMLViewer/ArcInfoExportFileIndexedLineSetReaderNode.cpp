/***********************************************************************
ArcInfoExportFileIndexedLineSetReaderNode - Class for nodes that read
indexed line set data from external files in e00 Arc/Info export format.
Copyright (c) 2008 Oliver Kreylos

This file is part of the Virtual Reality VRML viewer (VRMLViewer).

The Virtual Reality VRML viewer is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Virtual Reality VRML viewer is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Virtual Reality VRML viewer; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <Misc/ThrowStdErr.h>
#include <Misc/File.h>

#include "Types.h"
#include "Fields/SFBool.h"
#include "Fields/SFString.h"

#include "VRMLParser.h"
#include "CoordinateNode.h"
#include "EllipsoidNode.h"

#include "ArcInfoExportFileIndexedLineSetReaderNode.h"

namespace {

/****************
Helper functions:
****************/

void storePoint(std::vector<CoordinateNode::Point>& coords,std::vector<Int32>& coordIndices,double x,double y)
	{
	/* Convert the point to destination coordinates: */
	CoordinateNode::Point p(CoordinateNode::Point::Scalar(x),CoordinateNode::Point::Scalar(y),CoordinateNode::Point::Scalar(0));
	
	/* Store the point's index: */
	coordIndices.push_back(coords.size());
	
	/* Store the point: */
	coords.push_back(p);
	}

}

/**********************************************************
Methods of class ArcInfoExportFileIndexedLineSetReaderNode:
**********************************************************/

ArcInfoExportFileIndexedLineSetReaderNode::ArcInfoExportFileIndexedLineSetReaderNode(VRMLParser& parser)
	:radians(false),colatitude(false),depth(false),radialScale(1000.0)
	{
	/* Check for the opening brace: */
	if(!parser.isToken("{"))
		Misc::throwStdErr("ArcInfoExportFileIndexedLineSetReaderNode::ArcInfoExportFileIndexedLineSetReaderNode: Missing opening brace in node definition");
	parser.getNextToken();
	
	/* Process attributes until closing brace: */
	while(!parser.isToken("}"))
		{
		if(parser.isToken("url"))
			{
			/* Read the external Arc/Info export file's URL: */
			parser.getNextToken();
			url=SFString::parse(parser);
			}
		else if(parser.isToken("ellipsoid"))
			{
			/* Read the ellipsoid node: */
			parser.getNextToken();
			ellipsoid=parser.getNextNode();
			}
		else if(parser.isToken("radians"))
			{
			/* Read the radians flag: */
			parser.getNextToken();
			radians=SFBool::parse(parser);
			}
		else if(parser.isToken("colatitude"))
			{
			/* Read the colatitude flag: */
			parser.getNextToken();
			colatitude=SFBool::parse(parser);
			}
		else if(parser.isToken("depth"))
			{
			/* Read the depth flag: */
			parser.getNextToken();
			depth=SFBool::parse(parser);
			}
		else if(parser.isToken("radialScale"))
			{
			/* Read the radial unit scale: */
			parser.getNextToken();
			radialScale=atof(parser.getToken());
			parser.getNextToken();
			}
		else
			Misc::throwStdErr("ArcInfoExportFileIndexedLineSetReaderNode::ArcInfoExportFileIndexedLineSetReaderNode: unknown attribute \"%s\" in node definition",parser.getToken());
		}
	
	/* Skip the closing brace: */
	parser.getNextToken();
	}

bool ArcInfoExportFileIndexedLineSetReaderNode::hasColors(void) const
	{
	return false;
	}

void ArcInfoExportFileIndexedLineSetReaderNode::readIndexedLines(CoordinateNode* coordNode,std::vector<Int32>& coordIndices,ColorNode* colorNode,std::vector<Int32>& colorIndices) const
	{
	CoordinateNode::PointList& points=coordNode->getPoints();
	const EllipsoidNode* e=dynamic_cast<const EllipsoidNode*>(ellipsoid.getPointer());
	
	/* Open the input file: */
	Misc::File file(url.c_str(),"rt");
	
	/* Check the file header: */
	char line[256];
	file.gets(line,sizeof(line));
	if(strncasecmp(line,"EXP ",4)!=0)
		Misc::throwStdErr("ArcInfoExportFileIndexedLineSetReaderNode::readIndexedLines: File %s is no valid ARC/INFO export file",url.c_str());
	if(atoi(line+4)!=0)
		Misc::throwStdErr("ArcInfoExportFileIndexedLineSetReaderNode::readIndexedLines: File %s is a compressed ARC/INFO export file; not yet supported",url.c_str());
	
	/* Read embedded ARC files until the end-of-file: */
	file.gets(line,sizeof(line));
	while(strncasecmp(line,"EOS",3)!=0)
		{
		int doubleFlag=atoi(line+3);
		if(strncasecmp(line,"ARC",3)==0)
			{
			/* Read an ARC file: */
			while(true)
				{
				/* Read the polyline header: */
				file.gets(line,sizeof(line));
				int index,id,startNode,endNode,leftPolygonIndex,rightPolygonIndex,numVertices;
				sscanf(line,"%d %d %d %d %d %d %u",&index,&id,&startNode,&endNode,&leftPolygonIndex,&rightPolygonIndex,&numVertices);
				if(index==-1)
					break;
				
				/* Read the polyline vertices: */
				if(doubleFlag==2)
					{
					/* Single-precision points are two per line: */
					for(unsigned int i=0;i<numVertices/2;++i)
						{
						file.gets(line,sizeof(line));
						double p1x,p1y,p2x,p2y;
						sscanf(line,"%lf %lf %lf %lf",&p1x,&p1y,&p2x,&p2y);
						storePoint(points,coordIndices,p1x,p1y);
						storePoint(points,coordIndices,p2x,p2y);
						}
					if(numVertices%2==1)
						{
						file.gets(line,sizeof(line));
						double px,py;
						sscanf(line,"%lf %lf",&px,&py);
						storePoint(points,coordIndices,px,py);
						}
					}
				else if(doubleFlag==3)
					{
					/* Double-precision points are one per line: */
					for(unsigned int i=0;i<numVertices;++i)
						{
						file.gets(line,sizeof(line));
						double px,py;
						sscanf(line,"%lf %lf",&px,&py);
						storePoint(points,coordIndices,px,py);
						}
					}
				
				/* Terminate the current polyline: */
				coordIndices.push_back(-1);
				}
			}
		else if(strncasecmp(line,"SIN",3)==0)
			{
			/* Skip a SIN file: */
			do
				{
				file.gets(line,sizeof(line));
				}
			while(strncasecmp(line,"EOX",3)!=0);
			}
		else if(strncasecmp(line,"LOG",3)==0)
			{
			/* Skip a LOG file: */
			do
				{
				file.gets(line,sizeof(line));
				}
			while(strncasecmp(line,"EOL",3)!=0);
			}
		else if(strncasecmp(line,"PRJ",3)==0)
			{
			/* Skip a PRJ file: */
			do
				{
				file.gets(line,sizeof(line));
				}
			while(strncasecmp(line,"EOP",3)!=0);
			}
		else if(strncasecmp(line,"TX6",3)==0||strncasecmp(line,"TX7",3)==0||strncasecmp(line,"RXP",3)==0||strncasecmp(line,"RPL",3)==0)
			{
			/* Skip a text section: */
			do
				{
				file.gets(line,sizeof(line));
				}
			while(strncasecmp(line,"JABBERWOCKY",11)!=0); // Huh?
			}
		else if(strncasecmp(line,"MTD",3)==0)
			{
			/* Skip the Metadata section: */
			do
				{
				file.gets(line,sizeof(line));
				}
			while(strncasecmp(line,"EOD",3)!=0);
			}
		else if(strncasecmp(line,"IFO",3)==0)
			{
			/* Skip the INFO section: */
			do
				{
				file.gets(line,sizeof(line));
				}
			while(strncasecmp(line,"EOI",3)!=0);
			}
		else
			{
			/* Skip an unrecognized file: */
			while(true)
				{
				file.gets(line,sizeof(line));
				int in1,in2,in3,in4,in5,in6,in7;
				if(sscanf(line,"%d %d %d %d %d %d %d",&in1,&in2,&in3,&in4,&in5,&in6,&in7)==7&&in1==-1)
					break;
				}
			}
		
		/* Read the next file header: */
		file.gets(line,sizeof(line));
		}
	}
