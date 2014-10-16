/***********************************************************************
GLPolylineTube - Class to render a polyline as a cylindrical tube.
Copyright (c) 2006-2013 Oliver Kreylos

This file is part of the OpenGL Support Library (GLSupport).

The OpenGL Support Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The OpenGL Support Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the OpenGL Support Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <Math/Math.h>
#include <Math/Constants.h>
#include <Geometry/Vector.h>
#include <GL/gl.h>
#include <GL/GLVertexArrayParts.h>
#include <GL/GLContextData.h>
#include <GL/GLExtensionManager.h>
#include <GL/Extensions/GLARBVertexBufferObject.h>

#include <GL/GLPolylineTube.h>

/*****************************************
Methods of class GLPolylineTube::DataItem:
*****************************************/

GLPolylineTube::DataItem::DataItem(void)
	:vertexBufferId(0),indexBufferId(0),
	 vertices(0),indices(0),
	 vertexVersion(0),indexVersion(0)
	{
	if(GLARBVertexBufferObject::isSupported())
		{
		/* Initialize the vertex buffer object extension: */
		GLARBVertexBufferObject::initExtension();
		
		/* Create a vertex and an index buffer object: */
		glGenBuffersARB(1,&vertexBufferId);
		glGenBuffersARB(1,&indexBufferId);
		}
	}

GLPolylineTube::DataItem::~DataItem(void)
	{
	if(vertexBufferId!=0)
		glDeleteBuffersARB(1,&vertexBufferId);
	else
		delete[] vertices;
	if(indexBufferId!=0)
		glDeleteBuffersARB(1,&indexBufferId);
	else
		delete[] indices;
	}

/*******************************
Methods of class GLPolylineTube:
*******************************/

void GLPolylineTube::updateTubeVertices(GLPolylineTube::DataItem* dataItem) const
	{
	typedef Geometry::Vector<Scalar,3> Vector;
	
	size_t numTubeVertices=vertices.size()*size_t(numTubeSegments);
	Vertex* vertexPtr;
	if(dataItem->vertexBufferId!=0)
		{
		/* Bind, resize and map the vertex buffer object: */
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,dataItem->vertexBufferId);
		glBufferDataARB(GL_ARRAY_BUFFER_ARB,numTubeVertices*sizeof(Vertex),0,GL_STATIC_DRAW_ARB);
		vertexPtr=static_cast<Vertex*>(glMapBufferARB(GL_ARRAY_BUFFER_ARB,GL_WRITE_ONLY_ARB));
		}
	else
		{
		/* Reallocate the vertex array: */
		delete[] dataItem->vertices;
		dataItem->vertices=new Vertex[numTubeVertices];
		vertexPtr=dataItem->vertices;
		}
	
	/* Pre-compute the tube circle coefficients: */
	Scalar* sincos=new Scalar[numTubeSegments*2];
	for(int i=0;i<numTubeSegments;++i)
		{
		Scalar angle=Scalar(i)*Scalar(2)*Math::Constants<Scalar>::pi/Scalar(numTubeSegments);
		sincos[2*i+0]=Math::sin(angle);
		sincos[2*i+1]=Math::cos(angle);
		}
	
	/* Create the tube geometry: */
	Vector tangent=vertices[1]-vertices[0];
	tangent.normalize();
	Vector x=Geometry::normal(tangent);
	x.normalize();
	Vector y=tangent^x;
	y.normalize();
	for(int i=0;i<numTubeSegments;++i)
		{
		Vector radius=x*sincos[2*i+0]+y*sincos[2*i+1];
		vertexPtr->normal=Vertex::Normal(radius.getComponents());
		vertexPtr->position=Vertex::Position((vertices[0]+radius*tubeRadius).getComponents());
		++vertexPtr;
		}
	for(size_t vi=1;vi<vertices.size()-1;++vi)
		{
		tangent=vertices[vi+1]-vertices[vi-1];
		tangent.normalize();
		x-=tangent*(x*tangent);
		x.normalize();
		y=tangent^x;
		y.normalize();
		for(int i=0;i<numTubeSegments;++i)
			{
			Vector radius=x*sincos[2*i+0]+y*sincos[2*i+1];
			vertexPtr->normal=Vertex::Normal(radius.getComponents());
			vertexPtr->position=Vertex::Position((vertices[vi]+radius*tubeRadius).getComponents());
			++vertexPtr;
			}
		}
	tangent=vertices[vertices.size()-1]-vertices[vertices.size()-2];
	tangent.normalize();
	x-=tangent*(x*tangent);
	x.normalize();
	y=tangent^x;
	y.normalize();
	for(int i=0;i<numTubeSegments;++i)
		{
		Vector radius=x*sincos[2*i+0]+y*sincos[2*i+1];
		vertexPtr->normal=Vertex::Normal(radius.getComponents());
		vertexPtr->position=Vertex::Position((vertices[vertices.size()-1]+radius*tubeRadius).getComponents());
		++vertexPtr;
		}
	
	/* Delete the tube circle coefficients: */
	delete[] sincos;
	
	if(dataItem->vertexBufferId!=0)
		{
		/* Unmap and unbind the vertex buffer object: */
		glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
		}
	dataItem->vertexVersion=vertexVersion;
	}

void GLPolylineTube::updateTubeIndices(GLPolylineTube::DataItem* dataItem) const
	{
	size_t numTubeIndices=(size_t(numTubeSegments)+1)*2*(vertices.size()-1);
	GLuint* indexPtr;
	if(dataItem->indexBufferId!=0)
		{
		/* Bind, resize and map the index buffer object: */
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,dataItem->indexBufferId);
		glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,numTubeIndices*sizeof(GLuint),0,GL_STATIC_DRAW_ARB);
		indexPtr=static_cast<GLuint*>(glMapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,GL_WRITE_ONLY_ARB));
		}
	else
		{
		/* Reallocate the index array: */
		delete[] dataItem->indices;
		dataItem->indices=new GLuint[numTubeIndices];
		indexPtr=dataItem->indices;
		}
	
	/* Create the tube index array: */
	for(size_t vi=1;vi<vertices.size();++vi)
		{
		GLuint index0=GLuint((vi-1)*numTubeSegments);
		GLuint index1=GLuint(vi*numTubeSegments);
		for(int i=0;i<numTubeSegments;++i)
			{
			*indexPtr=index0+i;
			++indexPtr;
			*indexPtr=index1+i;
			++indexPtr;
			}
		*indexPtr=index0;
		++indexPtr;
		*indexPtr=index1;
		++indexPtr;
		}
	
	if(dataItem->indexBufferId!=0)
		{
		/* Unmap and unbind the index buffer object: */
		glUnmapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,0);
		}
	dataItem->indexVersion=indexVersion;
	}

GLPolylineTube::GLPolylineTube(GLPolylineTube::Scalar sTubeRadius,size_t sNumVertices,const GLPolylineTube::Point* sVertices)
	:GLObject(false),
	 tubeRadius(sTubeRadius),numTubeSegments(8),
	 vertexVersion(1),indexVersion(1)
	{
	vertices.reserve(sNumVertices);
	if(sVertices!=0)
		{
		/* Copy the given polyline vertices: */
		for(size_t i=0;i<sNumVertices;++i)
			vertices[i]=sVertices[i];
		}
	
	GLObject::init();
	}

GLPolylineTube::~GLPolylineTube(void)
	{
	/* Nothing to do, incidentally */
	}

void GLPolylineTube::initContext(GLContextData& contextData) const
	{
	/* Create a data item and store it in the context data: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	
	/* Upload the current version of the tube geometry to OpenGL: */
	updateTubeVertices(dataItem);
	updateTubeIndices(dataItem);
	}

void GLPolylineTube::setVertex(size_t vertexIndex,const GLPolylineTube::Point& newVertex)
	{
	/* Store the vertex and up the version number: */
	vertices[vertexIndex]=newVertex;
	++vertexVersion;
	}

void GLPolylineTube::addVertex(const GLPolylineTube::Point& newVertex)
	{
	/* Store the vertex and up the version numbers: */
	vertices.push_back(newVertex);
	++vertexVersion;
	++indexVersion;
	}

void GLPolylineTube::setTubeRadius(GLPolylineTube::Scalar newTubeRadius)
	{
	/* Set the tube radius and up the version number: */
	tubeRadius=newTubeRadius;
	++vertexVersion;
	}

void GLPolylineTube::setNumTubeSegments(int newNumTubeSegments)
	{
	/* Set the number of tube segments and up the version numbers: */
	numTubeSegments=newNumTubeSegments;
	++vertexVersion;
	++indexVersion;
	}

void GLPolylineTube::glRenderAction(GLContextData& contextData) const
	{
	/* Get pointer to the data item: */
	DataItem* dataItem=contextData.retrieveDataItem<DataItem>(this);
	
	/* Update the vertex and/or index buffers: */
	if(dataItem->vertexVersion!=vertexVersion)
		updateTubeVertices(dataItem);
	if(dataItem->indexVersion!=indexVersion)
		updateTubeIndices(dataItem);
	
	/* Render the tube geometry: */
	GLVertexArrayParts::enable(Vertex::getPartsMask());
	const Vertex* vertexPtr;
	if(dataItem->vertexBufferId!=0)
		{
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,dataItem->vertexBufferId);
		vertexPtr=0;
		}
	else
		vertexPtr=dataItem->vertices;
	glVertexPointer(vertexPtr);
	
	const GLuint* indexPtr;
	if(dataItem->indexBufferId!=0)
		{
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,dataItem->indexBufferId);
		indexPtr=0;
		}
	else
		indexPtr=dataItem->indices;
	
	GLsizei numStripIndices=(numTubeSegments+1)*2;
	for(size_t i=1;i<vertices.size();++i)
		{
		// glDrawElements(GL_QUAD_STRIP,numStripIndices,GL_UNSIGNED_INT,indexPtr);
		glDrawRangeElements(GL_QUAD_STRIP,(i-1)*numTubeSegments,(i+1)*numTubeSegments-1,numStripIndices,GL_UNSIGNED_INT,indexPtr);
		indexPtr+=numStripIndices;
		}
	
	if(dataItem->vertexBufferId!=0)
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
	if(dataItem->indexBufferId!=0)
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,0);
	GLVertexArrayParts::disable(Vertex::getPartsMask());
	}
