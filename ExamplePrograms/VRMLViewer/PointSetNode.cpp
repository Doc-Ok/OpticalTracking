/***********************************************************************
PointSetNode - Class for shapes represented as sets of points.
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
#include "Fields/SFFloat.h"

#include "VRMLParser.h"
#include "VRMLRenderState.h"
#include "ColorNode.h"
#include "CoordinateNode.h"
#include "PointSetReaderNode.h"

#include "PointSetNode.h"

/***************************************
Methods of class PointSetNode::DataItem:
***************************************/

PointSetNode::DataItem::DataItem(void)
	:vertexBufferObjectId(0)
	{
	if(GLARBVertexBufferObject::isSupported())
		{
		/* Initialize the vertex buffer object extension: */
		GLARBVertexBufferObject::initExtension();
		
		/* Create the vertex buffer object: */
		glGenBuffersARB(1,&vertexBufferObjectId);
		}
	}

PointSetNode::DataItem::~DataItem(void)
	{
	if(vertexBufferObjectId!=0)
		{
		/* Destroy the buffer object: */
		glDeleteBuffersARB(1,&vertexBufferObjectId);
		}
	}

/*****************************
Methods of class PointSetNode:
*****************************/

PointSetNode::PointSetNode(VRMLParser& parser)
	:pointSize(1.0f)
	{
	/* Check for the opening brace: */
	if(!parser.isToken("{"))
		Misc::throwStdErr("PointSetNode::PointSetNode: Missing opening brace in node definition");
	parser.getNextToken();
	
	/* Process attributes until closing brace: */
	while(!parser.isToken("}"))
		{
		if(parser.isToken("pointSize"))
			{
			/* Parse the point size: */
			parser.getNextToken();
			pointSize=SFFloat::parse(parser);
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
		else if(parser.isToken("pointSetReader"))
			{
			/* Parse the point set reader node: */
			parser.getNextToken();
			VRMLNodePointer pointSetReader=parser.getNextNode();
			PointSetReaderNode* psrn=dynamic_cast<PointSetReaderNode*>(pointSetReader.getPointer());
			if(psrn!=0)
				{
				coord=new CoordinateNode;
				if(psrn->hasColors())
					color=new ColorNode;
				psrn->readPoints(dynamic_cast<CoordinateNode*>(coord.getPointer()),dynamic_cast<ColorNode*>(color.getPointer()));
				}
			}
		else
			Misc::throwStdErr("PointSetNode::PointSetNode: unknown attribute \"%s\" in node definition",parser.getToken());
		}
	
	/* Skip the closing brace: */
	parser.getNextToken();
	}

PointSetNode::~PointSetNode(void)
	{
	}

void PointSetNode::initContext(GLContextData& contextData) const
	{
	/* Create a data item and store it in the context: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	
	/* Do nothing if the vertex buffer object extension is not supported: */
	if(dataItem->vertexBufferObjectId==0)
		return;
	
	const ColorNode* colorNode=dynamic_cast<const ColorNode*>(color.getPointer());
	const CoordinateNode* coordNode=dynamic_cast<const CoordinateNode*>(coord.getPointer());
	
	/* Do nothing if there is no coordinate node: */
	if(coordNode==0)
		return;
	
	/* Upload all points into the vertex buffer: */
	glBindBufferARB(GL_ARRAY_BUFFER_ARB,dataItem->vertexBufferObjectId);
	size_t numPoints=coordNode->getNumPoints();
	glBufferDataARB(GL_ARRAY_BUFFER_ARB,numPoints*sizeof(Vertex),0,GL_STATIC_DRAW_ARB);
	Vertex* vPtr=static_cast<Vertex*>(glMapBufferARB(GL_ARRAY_BUFFER_ARB,GL_WRITE_ONLY_ARB));
	if(colorNode!=0)
		{
		for(size_t i=0;i<numPoints;++i,++vPtr)
			{
			vPtr->color=colorNode->getColor(i);
			vPtr->position=Vertex::Position(coordNode->getPoint(i).getComponents());
			}
		}
	else
		{
		for(size_t i=0;i<numPoints;++i,++vPtr)
			{
			vPtr->color=Vertex::Color(255,255,255);
			vPtr->position=Vertex::Position(coordNode->getPoint(i).getComponents());
			}
		}
	
	/* Unmap and protect the vertex buffer: */
	glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
	}

VRMLNode::Box PointSetNode::calcBoundingBox(void) const
	{
	/* Get a pointer to the coord node: */
	CoordinateNode* coordNode=dynamic_cast<CoordinateNode*>(coord.getPointer());
	
	/* Calculate the bounding box of all used points: */
	Box result=Box::empty;
	if(coordNode!=0)
		{
		for(size_t i=0;i<coordNode->getNumPoints();++i)
			result.addPoint(coordNode->getPoint(i));
		}
	return result;
	}

void PointSetNode::glRenderAction(VRMLRenderState& renderState) const
	{
	/* Retrieve the data item from the context: */
	DataItem* dataItem=renderState.contextData.retrieveDataItem<DataItem>(this);
	
	const ColorNode* colorNode=dynamic_cast<const ColorNode*>(color.getPointer());
	const CoordinateNode* coordNode=dynamic_cast<const CoordinateNode*>(coord.getPointer());
	
	/* Set up OpenGL: */
	bool lightingEnabled=glIsEnabled(GL_LIGHTING);
	if(lightingEnabled)
		glDisable(GL_LIGHTING);
	GLfloat storedPointSize;
	glGetFloatv(GL_POINT_SIZE,&storedPointSize);
	glPointSize(pointSize);
	
	/* Check if the vertex buffer object extension is supported: */
	if(dataItem->vertexBufferObjectId!=0)
		{
		GLVertexArrayParts::enable(Vertex::getPartsMask());
		
		/* Bind the point set's vertex buffer object: */
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,dataItem->vertexBufferObjectId);
		
		/* Render the point set from the vertex buffer object: */
		glVertexPointer(static_cast<const Vertex*>(0));
		glDrawArrays(GL_POINTS,0,coordNode->getNumPoints());
		
		/* Protect the vertex buffer object: */
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
		
		GLVertexArrayParts::disable(Vertex::getPartsMask());
		}
	else
		{
		/* Render the point set in immediate mode: */
		glBegin(GL_POINTS);
		for(size_t i=0;i<coordNode->getNumPoints();++i)
			{
			if(colorNode!=0)
				glColor(colorNode->getColor(i));
			glVertex(coordNode->getPoint(i));
			}
		}
	
	/* Restore OpenGL state: */
	glPointSize(storedPointSize);
	if(lightingEnabled)
		glEnable(GL_LIGHTING);
	}
