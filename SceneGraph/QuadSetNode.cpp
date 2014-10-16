/***********************************************************************
QuadSetNode - Class for sets of quadrilaterals as renderable
geometry.
Copyright (c) 2011-2013 Oliver Kreylos

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

#include <SceneGraph/QuadSetNode.h>

#include <string.h>
#include <utility>
#include <GL/gl.h>
#include <GL/GLContextData.h>
#include <GL/GLExtensionManager.h>
#include <GL/Extensions/GLARBVertexBufferObject.h>
#include <GL/GLGeometryVertex.h>
#include <SceneGraph/VRMLFile.h>
#include <SceneGraph/GLRenderState.h>

namespace SceneGraph {

/**************************************
Methods of class QuadSetNode::DataItem:
**************************************/

QuadSetNode::DataItem::DataItem(void)
	:vertexBufferObjectId(0),indexBufferObjectId(0),
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

QuadSetNode::DataItem::~DataItem(void)
	{
	/* Destroy the vertex and index buffer objects: */
	if(vertexBufferObjectId!=0)
		glDeleteBuffersARB(1,&vertexBufferObjectId);
	if(indexBufferObjectId!=0)
		glDeleteBuffersARB(1,&indexBufferObjectId);
	}

/****************************
Methods of class QuadSetNode:
****************************/

void QuadSetNode::uploadQuads(QuadSetNode::DataItem* dataItem) const
	{
	typedef GLGeometry::Vertex<Scalar,2,void,0,Scalar,Scalar,3> Vertex; // Type for vertices
	typedef GLuint Index; // Type for vertex indices
	
	/* Initialize the vertex buffer object: */
	glBufferDataARB(GL_ARRAY_BUFFER_ARB,numQuads*(subdivideY.getValue()+1)*(subdivideX.getValue()+1)*sizeof(Vertex),0,GL_STATIC_DRAW_ARB);
	Vertex* vPtr=static_cast<Vertex*>(glMapBufferARB(GL_ARRAY_BUFFER_ARB,GL_WRITE_ONLY_ARB));
	
	/* Iterate through all quads: */
	const MFPoint::ValueList& vertices=coord.getValue()->point.getValues();
	MFPoint::ValueList::const_iterator vIt=vertices.begin();
	for(unsigned int quadIndex=0;quadIndex<numQuads;++quadIndex,vIt+=4)
		{
		/* Calculate the quad's corner normal vectors: */
		Vector n0=Geometry::normalize((vIt[1]-vIt[0])^(vIt[3]-vIt[0]));
		Vector n1=Geometry::normalize((vIt[2]-vIt[1])^(vIt[0]-vIt[1]));
		Vector n2=Geometry::normalize((vIt[3]-vIt[2])^(vIt[1]-vIt[2]));
		Vector n3=Geometry::normalize((vIt[0]-vIt[3])^(vIt[2]-vIt[3]));
		
		/* Generate the quad's vertices: */
		if(subdivideX.getValue()>1||subdivideY.getValue()>1)
			{
			for(int y=0;y<=subdivideY.getValue();++y)
				{
				Scalar dy=Scalar(y)/Scalar(subdivideY.getValue());
				for(int x=0;x<=subdivideX.getValue();++x)
					{
					/* Calculate the subdivided vertex position: */
					Scalar dx=Scalar(x)/Scalar(subdivideX.getValue());
					Point v0=Geometry::affineCombination(vIt[0],vIt[1],dx);
					Point v1=Geometry::affineCombination(vIt[3],vIt[2],dx);
					Point v=Geometry::affineCombination(v0,v1,dy);
					
					/* Calculate the subdivided vertex normal: */
					Vector vn0=n0*(Scalar(1)-dx)+n1*dx;
					Vector vn1=n3*(Scalar(1)-dx)+n2*dx;
					Vector vn=vn0*(Scalar(1)-dy)+vn1*dy;
					
					if(pointTransform.getValue()!=0)
						{
						/* Transform the vertex normal and position: */
						vn=pointTransform.getValue()->transformNormal(v,vn);
						v=pointTransform.getValue()->transformPoint(v);
						}
					
					/* Upload the vertex: */
					vPtr->texCoord[0]=dx;
					vPtr->texCoord[1]=dy;
					vPtr->normal=vn;
					vPtr->position=v;
					++vPtr;
					}
				}
			}
		else
			{
			/* Upload the quad's corner vertices: */
			vPtr->texCoord[0]=Scalar(0);
			vPtr->texCoord[1]=Scalar(0);
			vPtr->normal=n0;
			vPtr->position=vIt[0];
			++vPtr;
			
			vPtr->texCoord[0]=Scalar(1);
			vPtr->texCoord[1]=Scalar(0);
			vPtr->normal=n1;
			vPtr->position=vIt[1];
			++vPtr;
			
			vPtr->texCoord[0]=Scalar(1);
			vPtr->texCoord[1]=Scalar(1);
			vPtr->normal=n2;
			vPtr->position=vIt[2];
			++vPtr;
			
			vPtr->texCoord[0]=Scalar(0);
			vPtr->texCoord[1]=Scalar(1);
			vPtr->normal=n3;
			vPtr->position=vIt[3];
			++vPtr;
			}
		}
	
	glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
	
	/* Initialize the index buffer object: */
	glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,numQuads*subdivideY.getValue()*(subdivideX.getValue()+1)*2*sizeof(Index),0,GL_STATIC_DRAW_ARB);
	Index* iPtr=static_cast<Index*>(glMapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,GL_WRITE_ONLY_ARB));
	if(subdivideX.getValue()>1||subdivideY.getValue()>1)
		{
		Index baseIndex=0;
		for(unsigned int quad=0;quad<numQuads;++quad,baseIndex+=subdivideY.getValue()*(subdivideX.getValue()+1)*2)
			{
			/* Create indices for a set of GL_QUAD_STRIPs: */
			for(int y=1;y<=subdivideY.getValue();++y)
				{
				Index stripBaseIndex=baseIndex+y*(subdivideX.getValue()+1);
				for(int x=0;x<=subdivideX.getValue();++x,iPtr+=2)
					{
					iPtr[0]=stripBaseIndex+x;
					iPtr[1]=stripBaseIndex-(subdivideX.getValue()+1)+x;
					}
				}
			}
		}
	else
		{
		/* Create indices for a set of GL_QUADS: */
		for(Index index=0;index<numQuads*4;++index,++iPtr)
			*iPtr=index;
		}
	
	glUnmapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB);
	}

QuadSetNode::QuadSetNode(void)
	:GLObject(false),
	 ccw(true),solid(true),
	 subdivideX(1),subdivideY(1),
	 inited(false),
	 version(0)
	{
	}

const char* QuadSetNode::getStaticClassName(void)
	{
	return "QuadSet";
	}

const char* QuadSetNode::getClassName(void) const
	{
	return "QuadSet";
	}

void QuadSetNode::parseField(const char* fieldName,VRMLFile& vrmlFile)
	{
	if(strcmp(fieldName,"coord")==0)
		{
		vrmlFile.parseSFNode(coord);
		}
	else if(strcmp(fieldName,"ccw")==0)
		{
		vrmlFile.parseField(ccw);
		}
	else if(strcmp(fieldName,"solid")==0)
		{
		vrmlFile.parseField(solid);
		}
	else if(strcmp(fieldName,"subdivideX")==0)
		{
		vrmlFile.parseField(subdivideX);
		}
	else if(strcmp(fieldName,"subdivideY")==0)
		{
		vrmlFile.parseField(subdivideY);
		}
	else
		GeometryNode::parseField(fieldName,vrmlFile);
	}

void QuadSetNode::update(void)
	{
	/* Determine the number of full quads: */
	numQuads=coord.getValue()->point.getNumValues()/4;
	
	/* Update the quad set version number: */
	++version;
	
	/* Register the object with all OpenGL contexts if not done already: */
	if(!inited)
		{
		GLObject::init();
		inited=true;
		}
	}

Box QuadSetNode::calcBoundingBox(void) const
	{
	if(coord.getValue()!=0)
		{
		if(pointTransform.getValue()!=0)
			{
			if(subdivideX.getValue()>1||subdivideY.getValue()>1)
				{
				/* Return the bounding box of all subdivided quad vertices (point transform is not necessarily affine): */
				Box result=Box::empty;
				
				const MFPoint::ValueList& vertices=coord.getValue()->point.getValues();
				MFPoint::ValueList::const_iterator vIt=vertices.begin();
				for(unsigned int quadIndex=0;quadIndex<numQuads;++quadIndex,vIt+=4)
					{
					/* Generate the quad's vertices: */
					for(int y=0;y<=subdivideY.getValue();++y)
						{
						Scalar dy=Scalar(y)/Scalar(subdivideY.getValue());
						for(int x=0;x<=subdivideX.getValue();++x)
							{
							/* Calculate the subdivided vertex position: */
							Scalar dx=Scalar(x)/Scalar(subdivideX.getValue());
							Point v0=Geometry::affineCombination(vIt[0],vIt[1],dx);
							Point v1=Geometry::affineCombination(vIt[3],vIt[2],dx);
							result.addPoint(pointTransform.getValue()->transformPoint(Geometry::affineCombination(v0,v1,dy)));
							}
						}
					}
				
				return result;
				}
			else
				{
				/* Return the bounding box of the transformed point coordinates: */
				return pointTransform.getValue()->calcBoundingBox(coord.getValue()->point.getValues());
				}
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

void QuadSetNode::glRenderAction(GLRenderState& renderState) const
	{
	/* Bail out if there are no complete quads: */
	if(numQuads==0)
		return;
	
	/* Set up OpenGL state: */
	if(solid.getValue())
		renderState.enableCulling(GL_BACK);
	else
		renderState.disableCulling();
	
	/* Get the context data item: */
	DataItem* dataItem=renderState.contextData.retrieveDataItem<DataItem>(this);
	
	if(dataItem->vertexBufferObjectId!=0&&dataItem->indexBufferObjectId!=0)
		{
		typedef GLGeometry::Vertex<Scalar,2,void,0,Scalar,Scalar,3> Vertex; // Type for vertices
		typedef GLuint Index; // Type for vertex indices
		
		/* Bind the quad set's vertex and index buffer objects: */
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,dataItem->vertexBufferObjectId);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,dataItem->indexBufferObjectId);
		
		/* Check if the vertex or index buffers are outdated: */
		if(dataItem->version!=version)
			{
			/* Upload the quad set's new vertices: */
			uploadQuads(dataItem);
			dataItem->version=version;
			}
		
		/* Set up the vertex arrays: */
		GLVertexArrayParts::enable(Vertex::getPartsMask());
		glVertexPointer(static_cast<Vertex*>(0));
		
		/* Render the quad set: */
		if(subdivideX.getValue()>1||subdivideY.getValue()>1)
			{
			/* Draw a set of quad strips: */
			const Index* indexPtr=0;
			for(unsigned int strip=0;strip<numQuads*subdivideY.getValue();++strip,indexPtr+=(subdivideX.getValue()+1)*2)
				glDrawElements(GL_QUAD_STRIP,(subdivideX.getValue()+1)*2,GL_UNSIGNED_INT,indexPtr);
			}
		else
			{
			/* Draw all quads in one go: */
			glDrawArrays(GL_QUADS,0,numQuads*4);
			}
		
		/* Reset the vertex arrays: */
		GLVertexArrayParts::disable(Vertex::getPartsMask());
		
		/* Protect the buffers: */
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,0);
		}
	else
		{
		/* If your OpenGL can't do vertex buffers, you're fucked anyway. */
		}
	}

void QuadSetNode::initContext(GLContextData& contextData) const
	{
	/* Create a data item and store it in the context: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	
	/* Upload the initial version of the quad set: */
	if(dataItem->vertexBufferObjectId!=0&&dataItem->indexBufferObjectId!=0&&numQuads>0)
		{
		/* Bind the quad set's vertex and index buffer objects: */
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,dataItem->vertexBufferObjectId);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,dataItem->indexBufferObjectId);
		
		/* Upload the quad set's vertices: */
		uploadQuads(dataItem);
		
		/* Protect the buffers: */
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,0);
		
		dataItem->version=version;
		}
	}

}
