/***********************************************************************
FormattedPointSetReaderNode - Point set reader class for formatted
(fixed-width) ASCII files.
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
#include <Misc/File.h>

#include "Types.h"
#include "Fields/SFInt32.h"
#include "Fields/MFInt32.h"
#include "Fields/SFString.h"

#include "VRMLParser.h"
#include "CoordinateNode.h"
#include "ColorNode.h"
#include "ColorInterpolatorNode.h"
#include "EllipsoidNode.h"

#include "FormattedPointSetReaderNode.h"

/********************************************
Methods of class FormattedPointSetReaderNode:
********************************************/

FormattedPointSetReaderNode::FormattedPointSetReaderNode(VRMLParser& parser)
	:numHeaderLines(0)
	{
	for(int i=0;i<4;++i)
		columnIndices[i]=-1;
	
	/* Check for the opening brace: */
	if(!parser.isToken("{"))
		Misc::throwStdErr("FormattedPointSetReaderNode::FormattedPointSetReaderNode: Missing opening brace in node definition");
	parser.getNextToken();
	
	/* Process attributes until closing brace: */
	while(!parser.isToken("}"))
		{
		if(parser.isToken("url"))
			{
			/* Read the external point file's URL: */
			parser.getNextToken();
			url=SFString::parse(parser);
			}
		else if(parser.isToken("columnStarts"))
			{
			/* Read the array of column starts: */
			parser.getNextToken();
			columnStarts=MFInt32::parse(parser);
			}
		else if(parser.isToken("columnWidths"))
			{
			/* Read the array of column widths: */
			parser.getNextToken();
			columnWidths=MFInt32::parse(parser);
			}
		else if(parser.isToken("ellipsoid"))
			{
			/* Read the ellipsoid node: */
			parser.getNextToken();
			ellipsoid=parser.getNextNode();
			}
		else if(parser.isToken("colorMap"))
			{
			/* Read the color map node: */
			parser.getNextToken();
			colorMap=parser.getNextNode();
			}
		else if(parser.isToken("coordColumnIndices"))
			{
			/* Read the array of coordinate column indices: */
			parser.getNextToken();
			for(int i=0;i<3;++i)
				columnIndices[i]=SFInt32::parse(parser);
			}
		else if(parser.isToken("valueColumnIndex"))
			{
			/* Read the value column index: */
			parser.getNextToken();
			columnIndices[3]=SFInt32::parse(parser);
			}
		else if(parser.isToken("numHeaderLines"))
			{
			/* Read the number of header lines to skip: */
			parser.getNextToken();
			numHeaderLines=SFInt32::parse(parser);
			}
		else
			Misc::throwStdErr("FormattedPointSetReaderNode::FormattedPointSetReaderNode: unknown attribute \"%s\" in node definition",parser.getToken());
		}
	
	/* Skip the closing brace: */
	parser.getNextToken();
	
	/* Compute required information: */
	if(columnStarts.empty())
		{
		int start=0;
		for(std::vector<Int32>::const_iterator cwIt=columnWidths.begin();cwIt!=columnWidths.end();++cwIt)
			{
			columnStarts.push_back(start);
			start+=*cwIt;
			}
		}
	}

bool FormattedPointSetReaderNode::hasColors(void) const
	{
	return columnIndices[3]>=0&&dynamic_cast<const ColorInterpolatorNode*>(colorMap.getPointer())!=0;
	}

void FormattedPointSetReaderNode::readPoints(CoordinateNode* coordNode,ColorNode* colorNode) const
	{
	CoordinateNode::PointList& points=coordNode->getPoints();
	ColorNode::ColorList* colors=colorNode!=0?&colorNode->getColors():0;
	const EllipsoidNode* e=dynamic_cast<const EllipsoidNode*>(ellipsoid.getPointer());
	const ColorInterpolatorNode* c=dynamic_cast<const ColorInterpolatorNode*>(colorMap.getPointer());
	
	/* Open the input file: */
	Misc::File pointFile(url.c_str(),"rt");
	
	/* Skip the header lines: */
	char line[256];
	for(int i=0;i<numHeaderLines;++i)
		pointFile.gets(line,sizeof(line));
	
	/* Read all lines in the point file: */
	while(!pointFile.eof())
		{
		/* Read the next line from the file: */
		pointFile.gets(line,sizeof(line));
		
		/* Extract the relevant information: */
		double values[4];
		for(int i=0;i<4;++i)
			if(columnIndices[i]>=0)
				{
				/* Add a temporary separator into the string and extract the value: */
				char savedChar=line[columnStarts[columnIndices[i]]+columnWidths[columnIndices[i]]];
				line[columnStarts[columnIndices[i]]+columnWidths[columnIndices[i]]]='\0';
				values[i]=atof(line+columnStarts[columnIndices[i]]);
				line[columnStarts[columnIndices[i]]+columnWidths[columnIndices[i]]]=savedChar;
				}
		
		if(e!=0)
			{
			/* Convert the point to Cartesian coordinates and store it: */
			for(int i=0;i<2;++i)
				values[i]=Math::rad(values[i]);
			values[2]*=1000.0;
			points.push_back(e->sphericalToCartesian(values));
			}
		else
			{
			/* Store the point in the given coordinates: */
			points.push_back(CoordinateNode::Point(values));
			}
		
		if(columnIndices[3]>=0&&c!=0)
			{
			/* Convert the point value to a color and store it: */
			colors->push_back(c->interpolate(float(values[3])));
			}
		}
	}
