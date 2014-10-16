/***********************************************************************
Doom3Model - Class to represent static models using Doom3 materials and
shaders.
Copyright (c) 2007-2013 Oliver Kreylos

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

#include <SceneGraph/Internal/Doom3Model.h>

#include <algorithm>
#include <Geometry/ComponentArray.h>
#include <Geometry/Matrix.h>
#include <GL/gl.h>
#include <GL/GLVertexArrayTemplates.h>
#include <GL/GLContextData.h>
#include <GL/GLExtensionManager.h>
#include <GL/Extensions/GLARBVertexBufferObject.h>
#include <GL/Extensions/GLARBVertexShader.h>
#include <GL/GLGeometryWrappers.h>

namespace SceneGraph {

/*************************************
Methods of class Doom3Model::DataItem:
*************************************/

Doom3Model::DataItem::DataItem(void)
	:hasVertexBufferExtension(GLARBVertexBufferObject::isSupported()),
	 vertexBufferId(0),indexBufferId(0)
	{
	if(hasVertexBufferExtension)
		{
		/* Initialize the vertex buffer object extension: */
		GLARBVertexBufferObject::initExtension();
		
		/* Create a vertex buffer object: */
		glGenBuffersARB(1,&vertexBufferId);
		
		/* Create an index buffer object: */
		glGenBuffersARB(1,&indexBufferId);
		}
	
	if(GLARBVertexShader::isSupported())
		{
		/* Initialize the vertex shader extension: */
		GLARBVertexShader::initExtension();
		}
	}

Doom3Model::DataItem::~DataItem(void)
	{
	if(hasVertexBufferExtension)
		{
		/* Delete the vertex buffer object: */
		glDeleteBuffersARB(1,&vertexBufferId);
		
		/* Delete the index buffer object: */
		glDeleteBuffersARB(1,&indexBufferId);
		}
	}

/***************************
Methods of class Doom3Model:
***************************/

void Doom3Model::calcNormalVectors(void)
	{
	/* Initialize all normal vectors to zero: */
	for(std::vector<Vertex>::iterator vIt=vertices.begin();vIt!=vertices.end();++vIt)
		vIt->normal=Vector::zero;
	
	/* Calculate normal vectors for all triangles and accumulate them in the vertices: */
	for(std::vector<GLuint>::iterator tIt=vertexIndices.begin();tIt!=vertexIndices.end();tIt+=3)
		{
		Vertex* vs[3];
		for(int i=0;i<3;++i)
			vs[i]=&vertices[tIt[i]];
		Vector d1=vs[1]->position-vs[0]->position;
		Vector d2=vs[2]->position-vs[0]->position;
		
		/* Calculate the triangle's normal vector: */
		Vector triangleNormal=d2^d1; // Model faces have clockwise orientation!
		triangleNormal.normalize();
		
		/* Accumulate the normal vector in the triangle's vertices: */
		for(int i=0;i<3;++i)
			{
			/* Calculate the vertex' accumulation weight, proportional to triangle angle at vertex: */
			Vector dp=vs[(i+1)%3]->position-vs[i]->position;
			Vector dm=vs[(i+2)%3]->position-vs[i]->position;
			Scalar angle=Math::acos((dp*dm)/(Geometry::mag(dp)*Geometry::mag(dm)));
			
			/* Accumulate the vertex' normal: */
			vs[i]->normal+=triangleNormal*angle;
			}
		}
	
	/* Normalize all vertex normal vectors: */
	for(std::vector<Vertex>::iterator vIt=vertices.begin();vIt!=vertices.end();++vIt)
		vIt->normal.normalize();
	}

void Doom3Model::calcTangentVectors(void)
	{
	/* Bump mapping requires unique tangent vectors per face vertex, so vertices shared between triangles must be broken up: */
	std::vector<Vertex> newVertices; // New vertex vector; contains all vertices in the order they are referenced by triangles
	std::vector<GLuint> newVertexIndices; // New vertex index vector; strictly speaking, it's unnecessary
	
	/* Calculate tangent vectors and copy vertices for all triangles: */
	GLuint newVertexIndex=0;
	for(std::vector<GLuint>::iterator tIt=vertexIndices.begin();tIt!=vertexIndices.end();tIt+=3)
		{
		Vertex vs[3];
		for(int i=0;i<3;++i)
			vs[i]=vertices[tIt[i]];
		Vector d1=vs[1].position-vs[0].position;
		Vector d2=vs[2].position-vs[0].position;
		
		/* Calculate the triangle's normal vector: */
		Vector triangleNormal=d2^d1; // Model faces have clockwise orientation!
		triangleNormal.normalize();
		
		/* Calculate the triangle's tangent vectors: */
		Geometry::Matrix<Scalar,2,2> T;
		for(int i=0;i<2;++i)
			for(int j=0;j<2;++j)
				T(i,j)=vs[j+1].texCoord[i]-vs[0].texCoord[i];
		T=Geometry::invert(T);
		Vector triangleTangents[2];
		for(int i=0;i<2;++i)
			triangleTangents[i]=d1*T(0,i)+d2*T(1,i);
		
		/* Store the vertices and vertex indices: */
		for(int i=0;i<3;++i)
			{
			/* Orthonormalize the tangents with the vertex normal: */
			for(int j=0;j<2;++j)
				{
				// vs[i].normal=triangleNormal;
				vs[i].tangents[j]=triangleTangents[j];
				vs[i].tangents[j]-=vs[i].normal*(vs[i].tangents[j]*vs[i].normal);
				vs[i].tangents[j].normalize();
				}
			
			/* Store the vertex and vertex index: */
			newVertices.push_back(vs[i]);
			newVertexIndices.push_back(newVertexIndex);
			++newVertexIndex;
			}
		}
	
	/* Store the updated vertex data: */
	std::swap(vertices,newVertices);
	std::swap(vertexIndices,newVertexIndices);
	}

Doom3Model::Doom3Model(Doom3MaterialManager& sMaterialManager,std::string sName)
	:materialManager(sMaterialManager),
	 name(sName)
	{
	}

void Doom3Model::initContext(GLContextData& contextData) const
	{
	/* Create a new context data item: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	
	if(dataItem->hasVertexBufferExtension)
		{
		/* Upload all vertices into the vertex buffer: */
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,dataItem->vertexBufferId);
		glBufferDataARB(GL_ARRAY_BUFFER_ARB,vertices.size()*sizeof(Vertex),&vertices[0],GL_STATIC_DRAW_ARB);
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
		
		/* Upload all vertex indices into the index buffer: */
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,dataItem->indexBufferId);
		glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,vertexIndices.size()*sizeof(GLuint),&vertexIndices[0],GL_STATIC_DRAW_ARB);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,0);
		}
	}

void Doom3Model::finalizeVertices(bool calcNormals,bool calcTangents)
	{
	/* Calculate the bounding box: */
	boundingBox=Box::empty;
	for(std::vector<Vertex>::iterator vIt=vertices.begin();vIt!=vertices.end();++vIt)
		boundingBox.addPoint(vIt->position);
	
	if(calcNormals)
		{
		/* Calculate normal vectors: */
		calcNormalVectors();
		}
	
	if(calcTangents)
		{
		/* Calculate tangent vectors: */
		calcTangentVectors();
		}
	}

void Doom3Model::glRenderAction(GLContextData& contextData,Doom3MaterialManager::RenderContext& mmRc) const
	{
	/* Get the context data item: */
	DataItem* dataItem=contextData.retrieveDataItem<DataItem>(this);
	
	/* Get the index of the tangent vector vertex attribute: */
	GLint tangentAttributeIndexS=materialManager.getTangentAttributeIndex(mmRc,0);
	GLint tangentAttributeIndexT=materialManager.getTangentAttributeIndex(mmRc,1);
	
	/* Enable the appropriate vertex arrays: */
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	if(tangentAttributeIndexS>=0)
		glEnableVertexAttribArrayARB(tangentAttributeIndexS);
	if(tangentAttributeIndexT>=0)
		glEnableVertexAttribArrayARB(tangentAttributeIndexT);
	glEnableClientState(GL_VERTEX_ARRAY);
	
	/* Install the vertex and index arrays: */
	const Vertex* vertexPtr;
	const GLuint* indexPtr;
	if(dataItem->hasVertexBufferExtension)
		{
		/* Bind the model's vertex buffer: */
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,dataItem->vertexBufferId);
		vertexPtr=0;
		
		/* Bind the mesh's index buffer: */
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,dataItem->indexBufferId);
		indexPtr=0;
		}
	else
		{
		vertexPtr=&vertices[0];
		indexPtr=&vertexIndices[0];
		}
	glTexCoordPointer(2,sizeof(Vertex),vertexPtr->texCoord.getComponents());
	glNormalPointer(sizeof(Vertex),vertexPtr->normal.getComponents());
	if(tangentAttributeIndexS>=0)
		glVertexAttribPointerARB(tangentAttributeIndexS,3,GL_FALSE,sizeof(Vertex),vertexPtr->tangents[0].getComponents());
	if(tangentAttributeIndexT>=0)
		glVertexAttribPointerARB(tangentAttributeIndexT,3,GL_FALSE,sizeof(Vertex),vertexPtr->tangents[1].getComponents());
	glVertexPointer(3,sizeof(Vertex),vertexPtr->position.getComponents());
	
	/* Render all surfaces: */
	for(std::vector<Surface>::const_iterator sIt=surfaces.begin();sIt!=surfaces.end();++sIt)
		{
		/* Install the mesh's material and check whether to render this mesh: */
		if(materialManager.setMaterial(mmRc,sIt->material))
			{
			/* Render the surface: */
			glDrawElements(GL_TRIANGLES,sIt->numVertexIndices,GL_UNSIGNED_INT,indexPtr);
			}
		
		/* Go to the next surface's vertex indices: */
		indexPtr+=sIt->numVertexIndices;
		}
	
	if(dataItem->hasVertexBufferExtension)
		{
		/* Unbind all buffers: */
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,0);
		}
	
	/* Disable the appropriate vertex arrays: */
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	if(tangentAttributeIndexS>=0)
		glDisableVertexAttribArrayARB(tangentAttributeIndexS);
	if(tangentAttributeIndexT>=0)
		glDisableVertexAttribArrayARB(tangentAttributeIndexT);
	glDisableClientState(GL_VERTEX_ARRAY);
	}

void Doom3Model::drawNormals(GLContextData& contextData,Scalar scale) const
	{
	glBegin(GL_LINES);
	for(std::vector<Vertex>::const_iterator vIt=vertices.begin();vIt!=vertices.end();++vIt)
		{
		/* Render the coordinate frame: */
		glColor3f(0.0f,0.0f,1.0f);
		glVertex(vIt->position);
		glVertex(vIt->position+vIt->normal*scale);
		glColor3f(1.0f,0.0f,0.0f);
		glVertex(vIt->position);
		glVertex(vIt->position+vIt->tangents[0]*scale);
		glColor3f(0.0f,1.0f,0.0f);
		glVertex(vIt->position);
		glVertex(vIt->position+vIt->tangents[1]*scale);
		}
	glEnd();
	}

}
