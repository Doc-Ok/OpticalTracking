/***********************************************************************
IndexedLineSetNode - Class for shapes represented as sets of polylines.
Copyright (c) 2006-2008 Oliver Kreylos

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

#include <vector>
#include <Misc/OrderedTuple.h>
#include <Misc/HashTable.h>
#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLVertexTemplates.h>
#define NONSTANDARD_GLVERTEX_TEMPLATES
#include <GL/GLVertex.h>
#include <GL/GLContextData.h>
#include <GL/GLExtensionManager.h>
#include <GL/Extensions/GLARBVertexBufferObject.h>
#include <GL/GLGeometryWrappers.h>

#include "Types.h"
#include "Fields/SFBool.h"
#include "Fields/MFInt32.h"

#include "VRMLParser.h"
#include "VRMLRenderState.h"
#include "ColorNode.h"
#include "CoordinateNode.h"
#include "IndexedLineSetReaderNode.h"

#include "IndexedLineSetNode.h"

/*********************************************
Methods of class IndexedLineSetNode::DataItem:
*********************************************/

IndexedLineSetNode::DataItem::DataItem(void)
	:vertexBufferObjectId(0),
	 indexBufferObjectId(0)
	{
	if(GLARBVertexBufferObject::isSupported())
		{
		/* Initialize the vertex buffer object extension: */
		GLARBVertexBufferObject::initExtension();
		
		/* Create vertex and index buffer objects: */
		glGenBuffersARB(1,&vertexBufferObjectId);
		glGenBuffersARB(1,&indexBufferObjectId);
		}
	}

IndexedLineSetNode::DataItem::~DataItem(void)
	{
	if(vertexBufferObjectId!=0||indexBufferObjectId!=0)
		{
		/* Destroy the buffer objects: */
		glDeleteBuffersARB(1,&vertexBufferObjectId);
		glDeleteBuffersARB(1,&indexBufferObjectId);
		}
	}

/***********************************
Methods of class IndexedLineSetNode:
***********************************/

IndexedLineSetNode::IndexedLineSetNode(VRMLParser& parser)
	:colorPerVertex(true)
	{
	/* Check for the opening brace: */
	if(!parser.isToken("{"))
		Misc::throwStdErr("IndexedLineSetNode::IndexedLineSetNode: Missing opening brace in node definition");
	parser.getNextToken();
	
	/* Process attributes until closing brace: */
	while(!parser.isToken("}"))
		{
		if(parser.isToken("colorPerVertex"))
			{
			parser.getNextToken();
			colorPerVertex=SFBool::parse(parser);
			}
		else if(parser.isToken("color"))
			{
			/* Parse the color node: */
			parser.getNextToken();
			color=parser.getNextNode();
			}
		else if(parser.isToken("coord"))
			{
			/* Parse the coordinate node: */
			parser.getNextToken();
			coord=parser.getNextNode();
			}
		else if(parser.isToken("colorIndex"))
			{
			/* Parse the color index array: */
			parser.getNextToken();
			colorIndices=MFInt32::parse(parser);
			}
		else if(parser.isToken("coordIndex"))
			{
			/* Parse the coordinate index array: */
			parser.getNextToken();
			coordIndices=MFInt32::parse(parser);
			
			/* Terminate the coordinate index array: */
			if(coordIndices.back()>=0)
				coordIndices.push_back(-1);
			}
		else if(parser.isToken("indexedLineSetReader"))
			{
			/* Parse the indexed line set reader node: */
			parser.getNextToken();
			VRMLNodePointer indexedLineSetReader=parser.getNextNode();
			IndexedLineSetReaderNode* ilsrn=dynamic_cast<IndexedLineSetReaderNode*>(indexedLineSetReader.getPointer());
			if(ilsrn!=0)
				{
				coord=new CoordinateNode;
				coordIndices.clear();
				if(ilsrn->hasColors())
					color=new ColorNode;
				colorIndices.clear();
				ilsrn->readIndexedLines(dynamic_cast<CoordinateNode*>(coord.getPointer()),coordIndices,dynamic_cast<ColorNode*>(color.getPointer()),colorIndices);
				}
			}
		else
			Misc::throwStdErr("IndexedLineSetNode::IndexedLineSetNode: unknown attribute \"%s\" in node definition",parser.getToken());
		}
	
	/* Skip the closing brace: */
	parser.getNextToken();
	}

IndexedLineSetNode::~IndexedLineSetNode(void)
	{
	}

void IndexedLineSetNode::initContext(GLContextData& contextData) const
	{
	/* Create a data item and store it in the context: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	
	/* Do nothing if the vertex buffer object extension is not supported: */
	if(dataItem->vertexBufferObjectId==0||dataItem->indexBufferObjectId==0)
		return;
	
	const ColorNode* colorNode=dynamic_cast<const ColorNode*>(color.getPointer());
	const CoordinateNode* coordNode=dynamic_cast<const CoordinateNode*>(coord.getPointer());
	
	/*********************************************************************
	The problem with indexed line sets in VRML is that the format supports
	component-wise vertex indices, i.e., a vertex used in a face can have
	different indices for color and position. OpenGL, on the other hand,
	only supports a single index for all vertex components. This method
	tries to reuse vertices as much as possible, by mapping tuples of
	per-component vertex indices to complete OpenGL vertex indices using a
	hash table.
	*********************************************************************/
	
	/* Create a hash table to map compound vertex indices to complete vertices: */
	typedef Misc::OrderedTuple<int,2> VertexIndices; // (colorIndex, vertexIndex)
	typedef Misc::HashTable<VertexIndices,GLuint,VertexIndices> VertexHasher;
	VertexHasher vertexHasher(101);
	
	std::vector<int>::const_iterator colorIt=colorIndices.empty()?coordIndices.begin():colorIndices.begin();
	int colorCounter=0;
	std::vector<int>::const_iterator coordIt=coordIndices.begin();
	VertexIndices currentVertex(0,0);
	std::vector<VertexIndices> vertexIndices;
	std::vector<GLuint> lineVertexIndices;
	while(coordIt!=coordIndices.end())
		{
		/* Process the vertices of this line strip: */
		GLsizei numVertices=0;
		while(*coordIt>=0)
			{
			/* Create the current compound vertex: */
			if(colorNode!=0)
				{
				if(!colorPerVertex&&colorIndices.empty())
					currentVertex.set(0,colorCounter);
				else
					currentVertex.set(0,*colorIt);
				}
			currentVertex.set(1,*coordIt);
			
			if(currentVertex[0]<0||currentVertex[1]<0)
				Misc::throwStdErr("Bad index in vertex!");
			
			/* Find the index of the complete vertex: */
			int vertexIndex;
			VertexHasher::Iterator vhIt=vertexHasher.findEntry(currentVertex);
			if(vhIt.isFinished())
				{
				/* Create a new vertex and store its index: */
				lineVertexIndices.push_back(vertexIndices.size());
				vertexHasher.setEntry(VertexHasher::Entry(currentVertex,vertexIndices.size()));
				vertexIndices.push_back(currentVertex);
				}
			else
				{
				/* Store the existing vertex index: */
				lineVertexIndices.push_back(vhIt->getDest());
				}
			
			/* Go to the next vertex in the same polyline: */
			if(colorPerVertex)
				++colorIt;
			++coordIt;
			++numVertices;
			}
		
		/* Generate a line strip for this polyline: */
		dataItem->numLineStripVertices.push_back(numVertices);
		
		/* Go to the next polyline: */
		if(!colorPerVertex&&colorIndices.empty())
			++colorCounter;
		else
			++colorIt;
		++coordIt;
		}
	
	/* Upload all vertices into the vertex buffer: */
	glBindBufferARB(GL_ARRAY_BUFFER_ARB,dataItem->vertexBufferObjectId);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB,vertexIndices.size()*sizeof(Vertex),0,GL_STATIC_DRAW_ARB);
	Vertex* vertices=static_cast<Vertex*>(glMapBufferARB(GL_ARRAY_BUFFER_ARB,GL_WRITE_ONLY_ARB));
	for(std::vector<VertexIndices>::const_iterator viIt=vertexIndices.begin();viIt!=vertexIndices.end();++viIt,++vertices)
		{
		/* Assemble the vertex from its components: */
		if(colorNode!=0)
			vertices->color=colorNode->getColor((*viIt)[0]);
		vertices->position=Vertex::Position(coordNode->getPoint((*viIt)[1]).getComponents());
		}
	
	/* Unmap and protect the vertex buffer: */
	glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
	
	/* Upload all vertex indices into the index buffers: */
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,dataItem->indexBufferObjectId);
	glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,lineVertexIndices.size()*sizeof(GLuint),&lineVertexIndices[0],GL_STATIC_DRAW_ARB);
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,0);
	}

VRMLNode::Box IndexedLineSetNode::calcBoundingBox(void) const
	{
	/* Get a pointer to the coord node: */
	CoordinateNode* coordNode=dynamic_cast<CoordinateNode*>(coord.getPointer());
	
	/* Calculate the bounding box of all used vertex coordinates: */
	Box result=Box::empty;
	for(std::vector<int>::const_iterator ciIt=coordIndices.begin();ciIt!=coordIndices.end();++ciIt)
		if(*ciIt>=0)
			result.addPoint(coordNode->getPoint(*ciIt));
	return result;
	}

void IndexedLineSetNode::glRenderAction(VRMLRenderState& renderState) const
	{
	/* Retrieve the data item from the context: */
	DataItem* dataItem=renderState.contextData.retrieveDataItem<DataItem>(this);
	
	const ColorNode* colorNode=dynamic_cast<const ColorNode*>(color.getPointer());
	const CoordinateNode* coordNode=dynamic_cast<const CoordinateNode*>(coord.getPointer());
	
	/* Set up OpenGL: */
	glDisable(GL_LIGHTING);
	
	if(dataItem->vertexBufferObjectId!=0&&dataItem->indexBufferObjectId!=0)
		{
		/* Determine which parts of the vertex array to enable: */
		int vertexPartsMask=0;
		if(colorNode!=0)
			vertexPartsMask|=GLVertexArrayParts::Color;
		vertexPartsMask|=GLVertexArrayParts::Position;
		
		/* Draw the indexed line set: */
		GLVertexArrayParts::enable(vertexPartsMask);
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,dataItem->vertexBufferObjectId);
		glVertexPointer(vertexPartsMask,static_cast<const Vertex*>(0));
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,dataItem->indexBufferObjectId);
		const GLuint* indexPtr=0;
		for(std::vector<GLsizei>::const_iterator nlsvIt=dataItem->numLineStripVertices.begin();nlsvIt!=dataItem->numLineStripVertices.end();++nlsvIt)
			{
			glDrawElements(GL_LINE_STRIP,*nlsvIt,GL_UNSIGNED_INT,indexPtr);
			indexPtr+=*nlsvIt;
			}
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,0);
		GLVertexArrayParts::disable(vertexPartsMask);
		}
	else
		{
		/* Process all line strips: */
		std::vector<int>::const_iterator colorIt=colorIndices.empty()?coordIndices.begin():colorIndices.begin();
		int colorCounter=0;
		std::vector<int>::const_iterator coordIt=coordIndices.begin();
		while(coordIt!=coordIndices.end())
			{
			glBegin(GL_LINE_STRIP);
			while(*coordIt>=0)
				{
				if(colorNode!=0)
					{
					if(!colorPerVertex&&colorIndices.empty())
						glColor(colorNode->getColor(colorCounter));
					else
						glColor(colorNode->getColor(*colorIt));
					}
				glVertex(coordNode->getPoint(*coordIt));
				if(colorPerVertex)
					++colorIt;
				++coordIt;
				}
			glEnd();
			
			if(!colorPerVertex&&colorIndices.empty())
				++colorCounter;
			else
				++colorIt;
			++coordIt;
			}
		}
	
	/* Reset OpenGL state: */
	glEnable(GL_LIGHTING);
	}
