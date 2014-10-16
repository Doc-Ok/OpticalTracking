/***********************************************************************
SphereRenderer - Class to efficiently render transparent spheres used
as interaction brushes.
Copyright (c) 2006 Oliver Kreylos
***********************************************************************/

#include <GL/gl.h>
#include <GL/GLContextData.h>
#include <GL/GLModels.h>

#include "SphereRenderer.h"

/*****************************************
Methods of class SphereRenderer::DataItem:
*****************************************/

SphereRenderer::DataItem::DataItem(void)
	{
	sphereDisplayListId=glGenLists(1);
	}

SphereRenderer::DataItem::~DataItem(void)
	{
	glDeleteLists(sphereDisplayListId,1);
	}

/*******************************
Methods of class SphereRenderer:
*******************************/

SphereRenderer::SphereRenderer(GLfloat sRadius,int sNumStrips)
	:radius(sRadius),numStrips(sNumStrips)
	{
	}

void SphereRenderer::initContext(GLContextData& contextData) const
	{
	/* Create a context data item and add it to the context data: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	
	/* Render the sphere model: */
	glNewList(dataItem->sphereDisplayListId,GL_COMPILE);
	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_LINE_BIT|GL_POLYGON_BIT);
	
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);
	glLineWidth(1.0f);
	glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	glColor4f(0.0f,1.0f,0.0f,0.33f);
	glDrawSphereIcosahedron(1.0f,5);
	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	glColor4f(0.1f,0.5f,0.1f,0.33f);
	glDrawSphereIcosahedron(1.0f,5);
	
	glPopAttrib();
	glEndList();
	}

void SphereRenderer::glRenderAction(GLContextData& contextData) const
	{
	/* Get a pointer to the context data item: */
	DataItem* dataItem=contextData.retrieveDataItem<DataItem>(this);
	
	/* Call the display list: */
	glCallList(dataItem->sphereDisplayListId);
	}
