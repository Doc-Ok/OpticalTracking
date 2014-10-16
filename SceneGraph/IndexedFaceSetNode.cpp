/***********************************************************************
IndexedFaceSetNode - Class for sets of polygonal faces as renderable
geometry.
Copyright (c) 2009-2013 Oliver Kreylos

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

#include <SceneGraph/IndexedFaceSetNode.h>

#include <string.h>
#include <GL/gl.h>
#include <GL/GLContextData.h>
#include <GL/GLExtensionManager.h>
#include <GL/Extensions/GLARBVertexBufferObject.h>
#include <GL/GLGeometryVertex.h>
#include <SceneGraph/VRMLFile.h>
#include <SceneGraph/GLRenderState.h>

namespace SceneGraph {

/*********************************************
Methods of class IndexedFaceSetNode::DataItem:
*********************************************/

IndexedFaceSetNode::DataItem::DataItem(void)
	:vertexBufferObjectId(0),indexBufferObjectId(0),
	 numVertexIndices(0),
	 version(0)
	{
	if(GLARBVertexBufferObject::isSupported())
		{
		/* Initialize the vertex buffer object extension: */
		GLARBVertexBufferObject::initExtension();
		
		/* Create the vertex buffer object: */
		glGenBuffersARB(1,&vertexBufferObjectId);
		
		/* Create the index buffer object: */
		glGenBuffersARB(1,&indexBufferObjectId);
		}
	}

IndexedFaceSetNode::DataItem::~DataItem(void)
	{
	/* Destroy the vertex and index buffer objects: */
	if(vertexBufferObjectId!=0)
		glDeleteBuffersARB(1,&vertexBufferObjectId);
	if(indexBufferObjectId!=0)
		glDeleteBuffersARB(1,&indexBufferObjectId);
	}

/***********************************
Methods of class IndexedFaceSetNode:
***********************************/

void IndexedFaceSetNode::uploadColoredFaceSet(DataItem* dataItem) const
	{
	/* Define the vertex type used in the vertex array: */
	// typedef GLGeometry::Vertex<Scalar,2,Scalar,4,Scalar,Scalar,3> Vertex;
	
	/* This should maybe be implemented at some point... */
	}

void IndexedFaceSetNode::uploadFaceSet(DataItem* dataItem) const
	{
	/* Define the vertex type used in the vertex array: */
	// typedef GLGeometry::Vertex<Scalar,2,void,0,Scalar,Scalar,3> Vertex;
	
	/* This should maybe be implemented at some point... */
	}

IndexedFaceSetNode::IndexedFaceSetNode(void)
	:GLObject(false),
	 colorPerVertex(true),normalPerVertex(true),
	 ccw(true),convex(true),solid(true),
	 inited(false),
	 version(0)
	{
	}

const char* IndexedFaceSetNode::getStaticClassName(void)
	{
	return "IndexedFaceSet";
	}

const char* IndexedFaceSetNode::getClassName(void) const
	{
	return "IndexedFaceSet";
	}

void IndexedFaceSetNode::parseField(const char* fieldName,VRMLFile& vrmlFile)
	{
	if(strcmp(fieldName,"texCoord")==0)
		{
		vrmlFile.parseSFNode(texCoord);
		}
	else if(strcmp(fieldName,"color")==0)
		{
		vrmlFile.parseSFNode(color);
		}
	else if(strcmp(fieldName,"normal")==0)
		{
		vrmlFile.parseSFNode(normal);
		}
	else if(strcmp(fieldName,"coord")==0)
		{
		vrmlFile.parseSFNode(coord);
		}
	else if(strcmp(fieldName,"texCoordIndex")==0)
		{
		vrmlFile.parseField(texCoordIndex);
		}
	else if(strcmp(fieldName,"colorIndex")==0)
		{
		vrmlFile.parseField(colorIndex);
		}
	else if(strcmp(fieldName,"colorPerVertex")==0)
		{
		vrmlFile.parseField(colorPerVertex);
		}
	else if(strcmp(fieldName,"normalIndex")==0)
		{
		vrmlFile.parseField(normalIndex);
		}
	else if(strcmp(fieldName,"normalPerVertex")==0)
		{
		vrmlFile.parseField(normalPerVertex);
		}
	else if(strcmp(fieldName,"coordIndex")==0)
		{
		vrmlFile.parseField(coordIndex);
		}
	else if(strcmp(fieldName,"ccw")==0)
		{
		vrmlFile.parseField(ccw);
		}
	else if(strcmp(fieldName,"convex")==0)
		{
		vrmlFile.parseField(convex);
		}
	else if(strcmp(fieldName,"solid")==0)
		{
		vrmlFile.parseField(solid);
		}
	else if(strcmp(fieldName,"creaseAngle")==0)
		{
		vrmlFile.parseField(creaseAngle);
		}
	else
		GeometryNode::parseField(fieldName,vrmlFile);
	}

void IndexedFaceSetNode::update(void)
	{
	/* Bump up the indexed face set's version number: */
	++version;
	
	/* Register the object with all OpenGL contexts if not done already: */
	if(!inited)
		{
		GLObject::init();
		inited=true;
		}
	}

Box IndexedFaceSetNode::calcBoundingBox(void) const
	{
	if(coord.getValue()!=0)
		{
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

void IndexedFaceSetNode::glRenderAction(GLRenderState& renderState) const
	{
	/* Get the context data item: */
	DataItem* dataItem=renderState.contextData.retrieveDataItem<DataItem>(this);
	
	if(dataItem->vertexBufferObjectId!=0&&dataItem->indexBufferObjectId!=0)
		{
		/*******************************************************************
		Render the indexed face set from the vertex and index buffers:
		*******************************************************************/
		
		typedef GLGeometry::Vertex<Scalar,2,Scalar,4,Scalar,Scalar,3> ColorVertex;
		typedef GLGeometry::Vertex<Scalar,2,void,0,Scalar,Scalar,3> Vertex;
		
		/* Bind the face set's vertex and index buffer objects: */
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,dataItem->vertexBufferObjectId);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,dataItem->indexBufferObjectId);
		
		if(dataItem->version!=version)
			{
			/* Upload the new face set: */
			if(color.getValue()!=0)
				uploadColoredFaceSet(dataItem);
			else
				uploadFaceSet(dataItem);
			dataItem->version=version;
			
			/* Mark the vertex and index buffer objects as up-to-date: */
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
		
		/* Draw the indexed face set: */
		glDrawElements(GL_TRIANGLES,dataItem->numVertexIndices,GL_UNSIGNED_INT,static_cast<const GLuint*>(0));
		
		/* Disable the vertex arrays: */
		if(color.getValue()!=0)
			GLVertexArrayParts::disable(ColorVertex::getPartsMask());
		else
			GLVertexArrayParts::disable(Vertex::getPartsMask());
		
		/* Protect the buffers: */
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,0);
		}
	else
		{
		/*******************************************************************
		Render the indexed face set directly:
		*******************************************************************/
		}
	}

void IndexedFaceSetNode::initContext(GLContextData& contextData) const
	{
	/* Create a data item and store it in the context: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	
	/* Upload the initial version of the indexed face set: */
	if(dataItem->vertexBufferObjectId!=0&&dataItem->indexBufferObjectId!=0)
		{
		/* Bind the face set's vertex and index buffer objects: */
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,dataItem->vertexBufferObjectId);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,dataItem->indexBufferObjectId);
		
		/* Upload the face set: */
		if(color.getValue()!=0)
			uploadColoredFaceSet(dataItem);
		else
			uploadFaceSet(dataItem);
		dataItem->version=version;
		
		/* Protect the buffers: */
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,0);
		}
	}

}
