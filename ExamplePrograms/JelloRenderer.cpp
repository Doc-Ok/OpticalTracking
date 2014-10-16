/***********************************************************************
JelloRenderer - Class render Jell-O crystals as translucent blocks.
Copyright (c) 2007 Oliver Kreylos

This file is part of the Virtual Jell-O interactive VR demonstration.

Virtual Jell-O is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

Virtual Jell-O is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with Virtual Jell-O; if not, write to the Free Software Foundation,
Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLVertexArrayParts.h>
#include <GL/GLContextData.h>
#include <GL/GLExtensionManager.h>
#include <GL/Extensions/GLARBVertexBufferObject.h>
#include <GL/GLGeometryWrappers.h>
#include <GL/GLTransformationWrappers.h>
#include <Vrui/Vrui.h>

#include "JelloRenderer.h"

/****************************************
Methods of class JelloRenderer::DataItem:
****************************************/

JelloRenderer::DataItem::DataItem(void)
	:hasVertexBufferObjectExtension(GLARBVertexBufferObject::isSupported()),
	 splineParameterVersion(0),
	 vertexDataVersion(0)
	{
	if(hasVertexBufferObjectExtension)
		{
		/* Initialize the vertex buffer object extension: */
		GLARBVertexBufferObject::initExtension();
		
		/* Create the vertex and index buffer objects: */
		glGenBuffersARB(6,faceVertexBufferObjectIDs);
		glGenBuffersARB(6,faceIndexBufferObjectIDs);
		}
	}

JelloRenderer::DataItem::~DataItem(void)
	{
	if(hasVertexBufferObjectExtension)
		{
		/* Destroy the vertex and index buffer objects: */
		glDeleteBuffersARB(6,faceVertexBufferObjectIDs);
		glDeleteBuffersARB(6,faceIndexBufferObjectIDs);
		}
	}

/******************************
Methods of class JelloRenderer:
******************************/

void JelloRenderer::createFacePatches(void)
	{
	/* Initialize the face spline patches: */
	for(int face=0;face<6;++face)
		{
		/* Delete the previous face spline state: */
		delete faces[face];
		delete evcs[face];
		delete[] vertices[face];
		delete[] indices[face];
		
		/* Calculate the spline patch's layout: */
		SplinePatch::Size degree(surfaceDegree,surfaceDegree);
		int majorAxis=face>>1;
		SplinePatch::Size numPoints(crystal->crystal.getSize((majorAxis+1)%3),crystal->crystal.getSize((majorAxis+2)%3));
		
		/* Calculate the spline patch's knot vectors: */
		SplinePatch::Size numKnots(numPoints[0]+degree[0]-1,numPoints[1]+degree[1]-1);
		SplinePatch::Scalar* knots[2];
		for(int i=0;i<2;++i)
			{
			knots[i]=new SplinePatch::Scalar[numKnots[i]];
			SplinePatch::Scalar* knotPtr=knots[i];
			for(int j=0;j<degree[i];++j,++knotPtr)
				*knotPtr=SplinePatch::Scalar(0);
			for(int j=1;j<numPoints[i]-degree[i];++j,++knotPtr)
				*knotPtr=SplinePatch::Scalar(j);
			for(int j=0;j<degree[i];++j,++knotPtr)
				*knotPtr=SplinePatch::Scalar(numPoints[i]-degree[i]);
			}
		
		/* Create the spline patch: */
		faces[face]=new SplinePatch(degree,numPoints,knots);
		
		/* Clean up: */
		for(int i=0;i<2;++i)
			delete[] knots[i];
		
		/* Create an evaluation cache for the spline patch: */
		evcs[face]=faces[face]->createEvaluationCache();
		
		/* Calculate the number of evaluation vertices for the face: */
		for(int i=0;i<2;++i)
			numVertices[face][i]=faces[face]->getNumSegments()[i]*numVerticesPerSegment+1;
		
		/* Allocate the index array for the face: */
		indices[face]=new GLuint[numVertices[face][0]*2*(numVertices[face][1]-1)];
		
		/* Calculate the face's quad strip indices: */
		GLuint* iPtr=indices[face];
		for(int i=0;i<numVertices[face][1]-1;++i)
			for(int j=0;j<numVertices[face][0];++j,iPtr+=2)
				{
				iPtr[0]=(i+1)*numVertices[face][0]+j;
				iPtr[1]=i*numVertices[face][0]+j;
				}
		
		/* Allocate the vertex array for the face: */
		vertices[face]=new Vertex[numVertices[face][0]*numVertices[face][1]];
		}
	
	/* Bump up the spline parameter version number: */
	++splineParameterVersion;
	}

JelloRenderer::JelloRenderer(const JelloCrystal& sCrystal)
	:crystal(&sCrystal),
	 renderDomainBox(true),
	 domainBoxColor(1.0f,1.0f,1.0f),
	 surfaceDegree(3),
	 faceMaterial(GLMaterial::Color(0.0f,1.0f,0.0f,0.75f),GLMaterial::Color(1.0f,1.0f,1.0f),25.0f),
	 active(false),
	 numVerticesPerSegment(8),
	 splineParameterVersion(0),
	 vertexDataVersion(0)
	{
	/* Clear the rendering state: */
	for(int face=0;face<6;++face)
		{
		faces[face]=0;
		evcs[face]=0;
		vertices[face]=0;
		indices[face]=0;
		}
	
	/* Create the initial face spline patches: */
	createFacePatches();
	}

JelloRenderer::~JelloRenderer(void)
	{
	/* Delete the face spline patches: */
	for(int face=0;face<6;++face)
		{
		delete faces[face];
		delete evcs[face];
		delete[] vertices[face];
		delete[] indices[face];
		}
	}

void JelloRenderer::setRenderDomainBox(bool newRenderDomainBox)
	{
	renderDomainBox=newRenderDomainBox;
	}

void JelloRenderer::setDomainBoxColor(const GLColor<GLfloat,3>& newDomainBoxColor)
	{
	domainBoxColor=newDomainBoxColor;
	}

void JelloRenderer::setSurfaceDegree(int newSurfaceDegree)
	{
	/* Update the surface degree: */
	surfaceDegree=newSurfaceDegree;
	
	/* Recreate the face spline patches: */
	createFacePatches();
	}

void JelloRenderer::setTesselation(int newNumVerticesPerSegment)
	{
	/* Update the tesselation level: */
	numVerticesPerSegment=newNumVerticesPerSegment;
	
	/* Recreate the face spline patches: */
	createFacePatches();
	}

void JelloRenderer::setFaceMaterial(const GLMaterial& newFaceMaterial)
	{
	faceMaterial=newFaceMaterial;
	}

void JelloRenderer::initContext(GLContextData& contextData) const
	{
	/* Create a context data item and store it in the GLContextData object: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	
	if(dataItem->hasVertexBufferObjectExtension)
		{
		/* Upload the (mostly invariant) index buffer data for all crystal faces: */
		for(int faceIndex=0;faceIndex<6;++faceIndex)
			{
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,dataItem->faceIndexBufferObjectIDs[faceIndex]);
			glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,numVertices[faceIndex][0]*2*(numVertices[faceIndex][1]-1)*sizeof(GLuint),indices[faceIndex],GL_STATIC_DRAW_ARB);
			}
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,0);
		
		dataItem->splineParameterVersion=splineParameterVersion;
		}
	}

void JelloRenderer::setCrystal(const JelloCrystal* newCrystal)
	{
	/* Set the associated crystal: */
	crystal=newCrystal;
	}

void JelloRenderer::setActive(bool newActive)
	{
	active=newActive;
	}

void JelloRenderer::update(void)
	{
	const JelloCrystal::Crystal& c=crystal->crystal;
	
	/* Update the face spline patches: */
	for(int face=0;face<6;++face)
		{
		SplinePatch* sp=faces[face];
		int majorAxis=face>>1;
		int dim0=(majorAxis+1)%3;
		int dim1=(majorAxis+2)%3;
		if(face&0x1)
			{
			/* Copy the atom positions in direct crystal order: */
			Index ai;
			ai[majorAxis]=c.getSize(majorAxis)-1;
			SplinePatch::Index i;
			for(i[1]=0;i[1]<sp->getNumPoints()[1];++i[1])
				for(i[0]=0;i[0]<sp->getNumPoints()[0];++i[0])
					{
					/* Calculate the crystal index of this control point: */
					ai[dim0]=i[0];
					ai[dim1]=i[1];
					sp->setPoint(i,c(ai).position);
					}
			}
		else
			{
			/* Copy the atom positions in flipped crystal order: */
			Index ai;
			ai[majorAxis]=0;
			SplinePatch::Index i;
			for(i[1]=0;i[1]<sp->getNumPoints()[1];++i[1])
				for(i[0]=0;i[0]<sp->getNumPoints()[0];++i[0])
					{
					/* Calculate the crystal index of this control point: */
					ai[dim0]=c.getSize(dim0)-1-i[0];
					ai[dim1]=i[1];
					sp->setPoint(i,c(ai).position);
					}
			}
		
		/* Evaluate the face spline patch: */
		SplinePatch::Parameter uMin=sp->getUMin();
		SplinePatch::Parameter uRange=sp->getUMax();
		for(int i=0;i<2;++i)
			uRange[i]-=uMin[i];
		SplinePatch::Parameter u;
		Vertex* vPtr=vertices[face];
		for(int i=0;i<numVertices[face][1];++i)
			{
			u[1]=uMin[1]+uRange[1]*SplinePatch::Scalar(i)/SplinePatch::Scalar(numVertices[face][1]-1);
			for(int j=0;j<numVertices[face][0];++j,++vPtr)
				{
				u[0]=uMin[0]+uRange[0]*SplinePatch::Scalar(j)/SplinePatch::Scalar(numVertices[face][0]-1);
				SplinePatch::Vector d0,d1;
				SplinePatch::Point p=sp->evaluate(u,evcs[face],d0,d1);
				SplinePatch::Vector normal=Geometry::cross(d0,d1);
				normal.normalize();
				vPtr->normal=Vertex::Normal(normal.getComponents());
				vPtr->position=Vertex::Position(p.getComponents());
				}
			}
		}
	
	/* Activate the renderer: */
	active=true;
	
	/* Bump up the vertex data version number: */
	++vertexDataVersion;
	}

void JelloRenderer::glRenderAction(GLContextData& contextData) const
	{
	if(renderDomainBox)
		{
		const Box& domain=crystal->domain;
		
		/* Render the domain box: */
		glPushAttrib(GL_ENABLE_BIT|GL_LINE_BIT);
		glDisable(GL_LIGHTING);
		glLineWidth(3.0f);
		glColor(domainBoxColor);
		glBegin(GL_LINE_LOOP);
		glVertex(domain.getVertex(0));
		glVertex(domain.getVertex(1));
		glVertex(domain.getVertex(3));
		glVertex(domain.getVertex(2));
		glVertex(domain.getVertex(0));
		glVertex(domain.getVertex(4));
		glVertex(domain.getVertex(5));
		glVertex(domain.getVertex(7));
		glVertex(domain.getVertex(6));
		glVertex(domain.getVertex(4));
		glEnd();
		glBegin(GL_LINES);
		glVertex(domain.getVertex(1));
		glVertex(domain.getVertex(5));
		glVertex(domain.getVertex(3));
		glVertex(domain.getVertex(7));
		glVertex(domain.getVertex(2));
		glVertex(domain.getVertex(6));
		glEnd();
		glPopAttrib();
		}
	
	// glRenderActionTransparent(contextData);
	}

void JelloRenderer::glRenderActionTransparent(GLContextData& contextData) const
	{
	if(!active)
		return;
	
	/* Get the data item from the OpenGL context: */
	DataItem* dataItem=contextData.retrieveDataItem<DataItem>(this);
	
	/* Save and prepare OpenGL state to render the translucent block of Jell-O: */
	glPushAttrib(GL_LIGHTING_BIT|GL_POLYGON_BIT);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);
	
	/* Go to navigation coordinates: */
	glPushMatrix();
	glMultMatrix(Vrui::getNavigationTransformation());
	
	/* Render all face spline patches: */
	glMaterial(GLMaterialEnums::FRONT_AND_BACK,faceMaterial);
	GLVertexArrayParts::enable(Vertex::getPartsMask());
	
	/* Render all back faces first: */
	glCullFace(GL_FRONT);
	for(int faceIndex=0;faceIndex<6;++faceIndex)
		{
		const GLuint* indexPtr;
		if(dataItem->hasVertexBufferObjectExtension)
			{
			/* Bind the face's index buffer object: */
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,dataItem->faceIndexBufferObjectIDs[faceIndex]);
			if(dataItem->splineParameterVersion!=splineParameterVersion)
				{
				/* Upload the new index data: */
				glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,numVertices[faceIndex][0]*2*(numVertices[faceIndex][1]-1)*sizeof(GLuint),indices[faceIndex],GL_STATIC_DRAW_ARB);
				}
			indexPtr=0;
			
			/* Bind the face's vertex buffer object: */
			glBindBufferARB(GL_ARRAY_BUFFER_ARB,dataItem->faceVertexBufferObjectIDs[faceIndex]);
			if(dataItem->vertexDataVersion!=vertexDataVersion)
				{
				/* Upload the new vertex data: */
				glBufferDataARB(GL_ARRAY_BUFFER_ARB,numVertices[faceIndex][0]*numVertices[faceIndex][1]*sizeof(Vertex),vertices[faceIndex],GL_DYNAMIC_DRAW_ARB);
				}
			glVertexPointer(static_cast<const Vertex*>(0));
			}
		else
			{
			/* Fall back to using regular vertex arrays (ouch): */
			indexPtr=indices[faceIndex];
			glVertexPointer(vertices[faceIndex]);
			}
		
		/* Render the surface as a sequence of quad strips: */
		for(int i=0;i<numVertices[faceIndex][1]-1;++i,indexPtr+=numVertices[faceIndex][0]*2)
			glDrawElements(GL_QUAD_STRIP,numVertices[faceIndex][0]*2,GL_UNSIGNED_INT,indexPtr);
		}
	
	/* Render the front faces next: */
	glCullFace(GL_BACK);
	for(int faceIndex=0;faceIndex<6;++faceIndex)
		{
		const GLuint* indexPtr;
		if(dataItem->hasVertexBufferObjectExtension)
			{
			/* Bind the face's index buffer object: */
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,dataItem->faceIndexBufferObjectIDs[faceIndex]);
			indexPtr=0;
			
			/* Bind the face's vertex buffer object: */
			glBindBufferARB(GL_ARRAY_BUFFER_ARB,dataItem->faceVertexBufferObjectIDs[faceIndex]);
			glVertexPointer(static_cast<const Vertex*>(0));
			}
		else
			{
			/* Fall back to using regular vertex arrays (ouch): */
			indexPtr=indices[faceIndex];
			glVertexPointer(vertices[faceIndex]);
			}
		
		/* Render the surface as a sequence of quad strips: */
		for(int i=0;i<numVertices[faceIndex][1]-1;++i,indexPtr+=numVertices[faceIndex][0]*2)
			glDrawElements(GL_QUAD_STRIP,numVertices[faceIndex][0]*2,GL_UNSIGNED_INT,indexPtr);
		}
	
	/* Mark the data item as up-to-date: */
	dataItem->splineParameterVersion=splineParameterVersion;
	dataItem->vertexDataVersion=vertexDataVersion;
	
	if(dataItem->hasVertexBufferObjectExtension)
		{
		/* Unbind all buffers: */
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,0);
		}
	
	GLVertexArrayParts::disable(Vertex::getPartsMask());
	
	glPopMatrix();
	glPopAttrib();
	}
