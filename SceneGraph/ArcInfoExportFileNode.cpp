/***********************************************************************
ArcInfoExportFileNode - Class to represent an ARC/INFO export file as a
collection of line sets, point sets, and face sets.
Copyright (c) 2009-2011 Oliver Kreylos

This file is part of the Simple Scene Graph Renderer (SceneGraph).

The Simple Scene Graph Renderer is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Simple Scene Graph Renderer is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Simple Scene Graph Renderer; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <SceneGraph/ArcInfoExportFileNode.h>

#include <string.h>
#include <IO/ValueSource.h>
#include <Cluster/OpenFile.h>
#include <SceneGraph/EventTypes.h>
#include <SceneGraph/VRMLFile.h>
#include <SceneGraph/ShapeNode.h>
#include <SceneGraph/ColorNode.h>
#include <SceneGraph/CoordinateNode.h>
#include <SceneGraph/IndexedLineSetNode.h>

namespace SceneGraph {

namespace {

/****************
Helper functions:
****************/

bool readBlockHeader(IO::ValueSource& exportFile,int values[7])
	{
	int numValues=0;
	try
		{
		while(numValues<7)
			{
			values[numValues]=exportFile.readInteger();
			++numValues;
			}
		}
	catch(IO::ValueSource::NumberError err)
		{
		/* Ignore the error: */
		}
	
	/* Check if all seven values were read: */
	return numValues==7;
	}

}

/**************************************
Methods of class ArcInfoExportFileNode:
**************************************/

ArcInfoExportFileNode::ArcInfoExportFileNode(void)
	:multiplexer(0)
	{
	}

const char* ArcInfoExportFileNode::getStaticClassName(void)
	{
	return "ArcInfoExportFile";
	}

const char* ArcInfoExportFileNode::getClassName(void) const
	{
	return "ArcInfoExportFile";
	}

void ArcInfoExportFileNode::parseField(const char* fieldName,VRMLFile& vrmlFile)
	{
	if(strcmp(fieldName,"url")==0)
		{
		vrmlFile.parseField(url);
		
		/* Fully qualify all URLs: */
		for(size_t i=0;i<url.getNumValues();++i)
			url.setValue(i,vrmlFile.getFullUrl(url.getValue(i)));
		
		multiplexer=vrmlFile.getMultiplexer();
		}
	else
		GroupNode::parseField(fieldName,vrmlFile);
	}

EventOut* ArcInfoExportFileNode::getEventOut(const char* fieldName) const
	{
	if(strcmp(fieldName,"url")==0)
		return makeEventOut(this,url);
	else
		return GroupNode::getEventOut(fieldName);
	}

EventIn* ArcInfoExportFileNode::getEventIn(const char* fieldName)
	{
	if(strcmp(fieldName,"url")==0)
		return makeEventIn(this,url);
	else
		return GroupNode::getEventIn(fieldName);
	}

void ArcInfoExportFileNode::update(void)
	{
	/* Do nothing if there is no export file name: */
	if(url.getNumValues()==0)
		return;
	
	/* Open the ARC/INFO export file: */
	IO::ValueSource exportFile(Cluster::openFile(multiplexer,url.getValue(0).c_str()));
	
	/* Check the file's format: */
	if(exportFile.readString()!="EXP"||exportFile.readInteger()!=0)
		return;
	
	/* Skip the rest of the header line: */
	exportFile.skipLine();
	exportFile.skipWs();
	
	/* Create a shape node to contain the polylines read from embedded ARC files: */
	ShapeNode* arcShape=new ShapeNode;
	IndexedLineSetNode* arcLineSet=new IndexedLineSetNode;
	arcShape->geometry.setValue(arcLineSet);
	ColorNode* arcLineSetColors=new ColorNode;
	arcLineSet->color.setValue(arcLineSetColors);
	CoordinateNode* arcLineSetCoords=new CoordinateNode;
	arcLineSet->coord.setValue(arcLineSetCoords);
	arcLineSet->colorPerVertex.setValue(false);
	
	/* Read embedded files until end-of-file: */
	while(!exportFile.eof())
		{
		/* Read the next embedded file's header: */
		std::string fileType=exportFile.readString();
		if(fileType=="EOS")
			{
			/* Marks the end of the file; ignore everything after here: */
			break;
			}
		else if(fileType=="ARC")
			{
			/* Skip the rest of the line: */
			exportFile.skipLine();
			exportFile.skipWs();
			
			/* Read polylines from the ARC file: */
			while(true)
				{
				/* Read the next block header: */
				int blockHeader[7];
				if(!readBlockHeader(exportFile,blockHeader)||blockHeader[0]==-1)
					break;
				
				/* Read all vertices in this polyline: */
				for(int i=0;i<blockHeader[6];++i)
					{
					/* Read the next vertex's 2D position: */
					double px=exportFile.readNumber();
					double py=exportFile.readNumber();
					
					/* Add the vertex to the current polyline: */
					arcLineSet->coordIndex.appendValue(arcLineSetCoords->point.getNumValues());
					arcLineSetCoords->point.appendValue(Point(px,py,0));
					}
				
				/* Finalize the current polyline: */
				arcLineSet->coordIndex.appendValue(-1);
				arcLineSetColors->color.appendValue(Color(1.0f,1.0f,1.0f));
				}
			}
		else if(fileType=="SIN")
			{
			/* Skip a SIN file: */
			while(!exportFile.eof()&&exportFile.readString()!="EOX")
				;
			}
		else if(fileType=="LOG")
			{
			/* Skip a LOG file: */
			while(!exportFile.eof()&&exportFile.readString()!="EOL")
				;
			}
		else if(fileType=="PRJ")
			{
			/* Skip a PRJ file: */
			while(!exportFile.eof()&&exportFile.readString()!="EOP")
				;
			}
		else if(fileType=="TX6"||fileType=="TX7"||fileType=="RXP"||fileType=="RPL")
			{
			/* Skip a text file: */
			while(!exportFile.eof()&&exportFile.readString()!="JABBERWOCKY") // What the hell?
				;
			}
		else if(fileType=="MTD")
			{
			/* Skip a MTD file: */
			while(!exportFile.eof()&&exportFile.readString()!="EOD")
				;
			}
		else if(fileType=="IFO")
			{
			/* Skip a IFO file: */
			while(!exportFile.eof()&&exportFile.readString()!="EOI")
				;
			}
		else
			{
			/* Temporarily mark newline characters as punctuation: */
			exportFile.setPunctuation('\n',true);
			
			/* Skip an unrecognized file: */
			while(!exportFile.eof())
				{
				/* Try to read a block header: */
				int values[7];
				if(readBlockHeader(exportFile,values)&&values[0]==-1)
					break;
				
				/* Skip the rest of the line: */
				exportFile.skipLine();
				exportFile.skipWs();
				}
			
			/* Mark newline characters as whitespace again: */
			exportFile.setWhitespace('\n',true);
			}
		}
	
	/* Finalize the generated nodes: */
	arcLineSetColors->update();
	arcLineSetCoords->update();
	arcLineSet->update();
	arcShape->update();
	
	/* Store all generated nodes as children: */
	children.appendValue(arcShape);
	GroupNode::update();
	}

}
