/***********************************************************************
PointSetNode - Class for sets of points as renderable geometry.
Copyright (c) 2009 Oliver Kreylos

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

#include <SceneGraph/PointSetNode.h>

#include <string.h>
#include <Geometry/Box.h>
#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLVertexTemplates.h>
#include <GL/GLContextData.h>
#include <GL/GLExtensionManager.h>
#include <GL/Extensions/GLARBVertexBufferObject.h>
#include <GL/GLGeometryWrappers.h>
#include <GL/GLGeometryVertex.h>
#include <SceneGraph/VRMLFile.h>
#include <SceneGraph/GLRenderState.h>

namespace SceneGraph {

/***************************************
Methods of class PointSetNode::DataItem:
***************************************/

PointSetNode::DataItem::DataItem(void)
	:vertexBufferObjectId(0),
	 version(0)
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

PointSetNode::PointSetNode(void)
	:pointSize(Scalar(1)),
	 version(0)
	{
	}

const char* PointSetNode::getStaticClassName(void)
	{
	return "PointSet";
	}

const char* PointSetNode::getClassName(void) const
	{
	return "PointSet";
	}

void PointSetNode::parseField(const char* fieldName,VRMLFile& vrmlFile)
	{
	if(strcmp(fieldName,"color")==0)
		{
		vrmlFile.parseSFNode(color);
		}
	else if(strcmp(fieldName,"coord")==0)
		{
		vrmlFile.parseSFNode(coord);
		}
	else if(strcmp(fieldName,"pointSize")==0)
		{
		vrmlFile.parseField(pointSize);
		}
	else
		GeometryNode::parseField(fieldName,vrmlFile);
	}

void PointSetNode::update(void)
	{
	/* Bump up the point set's version number: */
	++version;
	}

Box PointSetNode::calcBoundingBox(void) const
	{
	if(coord.getValue()!=0)
		{
		/* Return the bounding box of the point coordinates: */
		if(pointTransform.getValue()!=0)
			{
			/* Return the bounding box of the transformed point coordinates: */
			return pointTransform.getValue()->calcBoundingBox(coord.getValue()->point.getValues());
			}
		else
			{
			/* Return the bounding box of the untransformed point coordinates: */
			return coord.getValue()->calcBoundingBox();
			}
		}
	else
		return Box::empty;
	}

void PointSetNode::glRenderAction(GLRenderState& renderState) const
	{
	if(coord.getValue()!=0&&coord.getValue()->point.getNumValues()>0)
		{
		/* Get the context data item: */
		DataItem* dataItem=renderState.contextData.retrieveDataItem<DataItem>(this);
		
		/* Set up OpenGL state: */
		renderState.disableMaterials();
		renderState.disableTextures();
		glPointSize(pointSize.getValue());
		
		if(dataItem->vertexBufferObjectId!=0)
			{
			typedef GLGeometry::Vertex<void,0,GLubyte,4,void,Scalar,3> ColorVertex;
			typedef GLGeometry::Vertex<void,0,void,0,void,Scalar,3> Vertex;
			
			/* Bind the point set's vertex buffer object: */
			glBindBufferARB(GL_ARRAY_BUFFER_ARB,dataItem->vertexBufferObjectId);
			
			/* Check if the vertex buffer object is outdated: */
			if(dataItem->version!=version)
				{
				const std::vector<Point>& points=coord.getValue()->point.getValues();
				size_t numPoints=points.size();
				
				/* Prepare a vertex buffer: */
				glBufferDataARB(GL_ARRAY_BUFFER_ARB,numPoints*(color.getValue()!=0?sizeof(ColorVertex):sizeof(Vertex)),0,GL_STATIC_DRAW_ARB);
				if(color.getValue()!=0)
					{
					const std::vector<Color>& colors=color.getValue()->color.getValues();
					ColorVertex* vPtr=static_cast<ColorVertex*>(glMapBufferARB(GL_ARRAY_BUFFER_ARB,GL_WRITE_ONLY_ARB));
					if(pointTransform.getValue()!=0)
						{
						for(size_t i=0;i<numPoints;++i,++vPtr)
							{
							vPtr->color=colors[i];
							vPtr->position=pointTransform.getValue()->transformPoint(points[i]);
							}
						}
					else
						{
						for(size_t i=0;i<numPoints;++i,++vPtr)
							{
							vPtr->color=colors[i];
							vPtr->position=points[i];
							}
						}
					}
				else
					{
					Vertex* vPtr=static_cast<Vertex*>(glMapBufferARB(GL_ARRAY_BUFFER_ARB,GL_WRITE_ONLY_ARB));
					if(pointTransform.getValue()!=0)
						{
						for(size_t i=0;i<numPoints;++i,++vPtr)
							vPtr->position=pointTransform.getValue()->transformPoint(points[i]);
						}
					else
						{
						for(size_t i=0;i<numPoints;++i,++vPtr)
							vPtr->position=points[i];
						}
					}
				
				glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
				
				/* Mark the vertex buffer object as up-to-date: */
				dataItem->version=version;
				}
			
			/* Set up the vertex arrays: */
			if(color.getValue()!=0)
				{
				GLVertexArrayParts::enable(ColorVertex::getPartsMask());
				glVertexPointer(static_cast<ColorVertex*>(0));
				}
			else
				{
				GLVertexArrayParts::enable(Vertex::getPartsMask());
				glVertexPointer(static_cast<Vertex*>(0));
				}
			
			/* Draw the point set: */
			glDrawArrays(GL_POINTS,0,coord.getValue()->point.getNumValues());
			
			/* Disable the vertex arrays: */
			if(color.getValue()!=0)
				GLVertexArrayParts::disable(ColorVertex::getPartsMask());
			else
				GLVertexArrayParts::disable(Vertex::getPartsMask());
			
			/* Protect the vertex buffer object: */
			glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
			}
		else
			{
			/* Render the point set in immediate mode: */
			glBegin(GL_POINTS);
			const std::vector<Point>& points=coord.getValue()->point.getValues();
			size_t numPoints=points.size();
			if(color.getValue()!=0)
				{
				/* Color each point: */
				const std::vector<Color>& colors=color.getValue()->color.getValues();
				size_t numColors=colors.size();
				for(size_t i=0;i<numPoints;++i)
					{
					if(i<numColors)
						glColor(colors[i]);
					if(pointTransform.getValue()!=0)
						glVertex(pointTransform.getValue()->transformPoint(points[i]));
					else
						glVertex(points[i]);
					}
				}
			else
				{
				/* Use the current emissive color: */
				glColor(renderState.emissiveColor);
				for(size_t i=0;i<numPoints;++i)
					{
					if(pointTransform.getValue()!=0)
						glVertex(pointTransform.getValue()->transformPoint(points[i]));
					else
						glVertex(points[i]);
					}
				}
			glEnd();
			}
		}
	}

void PointSetNode::initContext(GLContextData& contextData) const
	{
	/* Create a data item and store it in the context: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	}

}
