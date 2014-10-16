/***********************************************************************
TSurfFileNode - Class for triangle meshes read from GoCAD TSurf files.
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

#include <SceneGraph/TSurfFileNode.h>

#include <string.h>
#include <Misc/ThrowStdErr.h>
#include <IO/ValueSource.h>
#include <Cluster/OpenFile.h>
#include <Geometry/Box.h>
#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLVertexTemplates.h>
#include <GL/GLContextData.h>
#include <GL/GLExtensionManager.h>
#include <GL/Extensions/GLARBVertexBufferObject.h>
#include <GL/GLGeometryWrappers.h>
#include <SceneGraph/VRMLFile.h>
#include <SceneGraph/GLRenderState.h>

namespace SceneGraph {

/****************************************
Methods of class TSurfFileNode::DataItem:
****************************************/

TSurfFileNode::DataItem::DataItem(void)
	:vertexBufferObjectId(0),indexBufferObjectId(0),
	 version(0)
	{
	if(GLARBVertexBufferObject::isSupported())
		{
		/* Initialize the vertex buffer object extension: */
		GLARBVertexBufferObject::initExtension();
		
		/* Create the vertex and index buffer objects: */
		glGenBuffersARB(1,&vertexBufferObjectId);
		glGenBuffersARB(1,&indexBufferObjectId);
		}
	}

TSurfFileNode::DataItem::~DataItem(void)
	{
	/* Destroy the vertex and index buffer objects: */
	if(vertexBufferObjectId!=0)
		glDeleteBuffersARB(1,&vertexBufferObjectId);
	if(indexBufferObjectId!=0)
		glDeleteBuffersARB(1,&indexBufferObjectId);
	}

/******************************
Methods of class TSurfFileNode:
******************************/

TSurfFileNode::TSurfFileNode(void)
	:multiplexer(0),version(0)
	{
	}

const char* TSurfFileNode::getStaticClassName(void)
	{
	return "TSurfFile";
	}

const char* TSurfFileNode::getClassName(void) const
	{
	return "TSurfFile";
	}

void TSurfFileNode::parseField(const char* fieldName,VRMLFile& vrmlFile)
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
		GeometryNode::parseField(fieldName,vrmlFile);
	}

void TSurfFileNode::update(void)
	{
	vertices.clear();
	indices.clear();
	
	/* Do nothing if there is no export file name: */
	if(url.getNumValues()==0)
		return;
	
	/* Read the TSurf file: */
	IO::ValueSource tSurf(Cluster::openFile(multiplexer,url.getValue(0).c_str()));
	tSurf.setPunctuation("{}");
	tSurf.skipWs();
	
	/* Check and skip the TSurf file header: */
	if(tSurf.readString()!="GOCAD"||tSurf.readString()!="TSurf"||tSurf.readString()!="0.01")
		Misc::throwStdErr("TSurfFileNode::update: File %s is not a valid TSurf file",url.getValue(0).c_str());
	if(tSurf.readString()!="HEADER"||tSurf.readString()!="{")
		Misc::throwStdErr("TSurfFileNode::update: File %s is not a valid TSurf file",url.getValue(0).c_str());
	unsigned int braceLevel=1;
	while(!tSurf.eof()&&braceLevel>0)
		{
		if(tSurf.peekc()=='{')
			++braceLevel;
		else if(tSurf.peekc()=='}')
			--braceLevel;
		tSurf.skipString();
		}
	
	while(!tSurf.eof())
		{
		/* Read the next keyword: */
		std::string keyword=tSurf.readString();
		if(keyword=="VRTX")
			{
			/* Read the next vertex index and adjust the vertex list: */
			unsigned int index=tSurf.readUnsignedInteger();
			while(vertices.size()<index)
				vertices.push_back(Vertex());
			
			/* Read a vertex: */
			Vertex v;
			v.normal=Vertex::Normal(0,0,1);
			for(int i=0;i<3;++i)
				v.position[i]=Vertex::Position::Scalar(tSurf.readNumber());
			
			/* Transform the vertex if asked: */
			if(pointTransform.getValue()!=0)
				v.position=pointTransform.getValue()->transformPoint(v.position);
			
			vertices.push_back(v);
			}
		else if(keyword=="TRGL")
			{
			/* Read a triangle: */
			for(int i=0;i<3;++i)
				indices.push_back(Card(tSurf.readUnsignedInteger()));
			}
		else if(keyword=="END")
			break;
		}
	
	/* Bump up the mesh version number: */
	++version;
	}

Box TSurfFileNode::calcBoundingBox(void) const
	{
	Box result=Box::empty;
	
	/* Return the bounding box of all point coordinates: */
	for(std::vector<Vertex>::const_iterator vIt=vertices.begin();vIt!=vertices.end();++vIt)
		result.addPoint(vIt->position);
	
	return result;
	}

void TSurfFileNode::glRenderAction(GLRenderState& renderState) const
	{
	/* Get the context data item: */
	DataItem* dataItem=renderState.contextData.retrieveDataItem<DataItem>(this);
	
	const Vertex* vertexPtr=0;
	const Card* indexPtr=0;
	if(dataItem->vertexBufferObjectId!=0&&dataItem->indexBufferObjectId!=0)
		{
		/* Bind the vertex and index buffer objects: */
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,dataItem->vertexBufferObjectId);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,dataItem->indexBufferObjectId);
		
		/* Check if the buffers need to be updated: */
		if(dataItem->version!=version)
			{
			/* Upload the vertex array: */
			glBufferDataARB(GL_ARRAY_BUFFER_ARB,vertices.size()*sizeof(Vertex),&vertices[0],GL_STATIC_DRAW_ARB);
			
			/* Upload the index array: */
			glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,indices.size()*sizeof(Card),&indices[0],GL_STATIC_DRAW_ARB);
			
			dataItem->version=version;
			}
		}
	else
		{
		/* Get the addresses of the vertex and index arrays: */
		vertexPtr=&vertices[0];
		indexPtr=&indices[0];
		}
	
	/* Set up the vertex arrays: */
	GLVertexArrayParts::enable(Vertex::getPartsMask());
	glVertexPointer(vertexPtr);
	
	/* Draw all triangles: */
	glDrawElements(GL_TRIANGLES,indices.size(),GL_UNSIGNED_INT,indexPtr);
	
	/* Reset the vertex arrays: */
	GLVertexArrayParts::disable(Vertex::getPartsMask());
	
	if(dataItem->vertexBufferObjectId!=0&&dataItem->indexBufferObjectId!=0)
		{
		/* Protect the buffer objects: */
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,0);
		}
	}

void TSurfFileNode::initContext(GLContextData& contextData) const
	{
	/* Create a data item and store it in the context: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	}

}
