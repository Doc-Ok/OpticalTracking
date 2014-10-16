/***********************************************************************
MouseCursorFaker - Helper class to render a fake mouse cursor for cases
where a hardware cursor is not available, or not appropriate.
Copyright (c) 2009-2014 Oliver Kreylos

This file is part of the Virtual Reality User Interface Library (Vrui).

The Virtual Reality User Interface Library is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The Virtual Reality User Interface Library is distributed in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Virtual Reality User Interface Library; if not, write to the
Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#include <Vrui/Internal/MouseCursorFaker.h>

#include <Geometry/Ray.h>
#include <GL/gl.h>
#include <GL/GLContextData.h>
#include <GL/GLGeometryWrappers.h>
#include <Images/ReadImageFile.h>
#include <Vrui/InputDevice.h>
#include <Vrui/VRScreen.h>
#include <Vrui/Vrui.h>

namespace Vrui {

/*******************************************
Methods of class MouseCursorFaker::DataItem:
*******************************************/

MouseCursorFaker::DataItem::DataItem(void)
	:textureObjectId(0)
	{
	glGenTextures(1,&textureObjectId);
	}

MouseCursorFaker::DataItem::~DataItem(void)
	{
	glDeleteTextures(1,&textureObjectId);
	}

/*********************************
Methods of class MouseCursorFaker:
*********************************/

MouseCursorFaker::MouseCursorFaker(const InputDevice* sDevice,const char* cursorImageFileName,unsigned int cursorNominalSize)
	:GLObject(false),
	 device(sDevice),
	 cursorImage(Images::readCursorFile(cursorImageFileName,cursorNominalSize)),
	 cursorSize(getUiSize(),getUiSize(),0),
	 cursorHotspot(0,1,0)
	{
	/* Calculate the cursor texture coordinate box: */
	Geometry::Point<float,2> tcMin,tcMax;
	for(int i=0;i<2;++i)
		{
		unsigned int texSize;
		for(texSize=1;texSize<cursorImage.getSize(i);texSize<<=1)
			;
		tcMin[i]=0.5f/float(texSize);
		tcMax[i]=(float(cursorImage.getSize(i))-0.5f)/float(texSize);
		}
	cursorTexCoordBox=Geometry::Box<float,2>(tcMin,tcMax);
	
	GLObject::init();
	}

void MouseCursorFaker::initContext(GLContextData& contextData) const
	{
	/* Create and store a data item: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	
	/* Upload the cursor image as a 2D texture: */
	glBindTexture(GL_TEXTURE_2D,dataItem->textureObjectId);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_BASE_LEVEL,0);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAX_LEVEL,0);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
	cursorImage.glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,true);
	glBindTexture(GL_TEXTURE_2D,0);
	}

void MouseCursorFaker::glRenderActionTransparent(GLContextData& contextData) const
	{
	/* Get the data item: */
	DataItem* dataItem=contextData.retrieveDataItem<DataItem>(this);
	
	/* Calculate the device's ray equation: */
	Ray deviceRay=device->getRay();
	
	/* Find the nearest screen intersected by the device ray: */
	std::pair<VRScreen*,Scalar> fsResult=findScreen(deviceRay);
	if(fsResult.first!=0)
		{
		/* Calculate the cursor position on the screen: */
		Point cursorPos=deviceRay(fsResult.second);
		Vector x=fsResult.first->getScreenTransformation().getDirection(0)*cursorSize[0];
		Vector y=fsResult.first->getScreenTransformation().getDirection(1)*cursorSize[1];
		cursorPos-=x*cursorHotspot[0];
		cursorPos-=y*cursorHotspot[1];
		
		/* Draw the mouse cursor: */
		glPushAttrib(GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT);
		GLdouble depthRange[2];
		glGetDoublev(GL_DEPTH_RANGE,depthRange);
		glDepthRange(0.0,0.0);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,dataItem->textureObjectId);
		glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GEQUAL,0.5f);
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
		glBegin(GL_QUADS);
		glTexCoord(cursorTexCoordBox.getVertex(0));
		glVertex(cursorPos);
		glTexCoord(cursorTexCoordBox.getVertex(1));
		cursorPos+=x;
		glVertex(cursorPos);
		glTexCoord(cursorTexCoordBox.getVertex(3));
		cursorPos+=y;
		glVertex(cursorPos);
		glTexCoord(cursorTexCoordBox.getVertex(2));
		cursorPos-=x;
		glVertex(cursorPos);
		glEnd();
		glBindTexture(GL_TEXTURE_2D,0);
		glDepthRange(depthRange[0],depthRange[1]);
		glPopAttrib();
		}
	}

void MouseCursorFaker::setCursorSize(const Size& newCursorSize)
	{
	cursorSize=newCursorSize;
	}

void MouseCursorFaker::setCursorHotspot(const Vector& newCursorHotspot)
	{
	cursorHotspot=newCursorHotspot;
	}

}
