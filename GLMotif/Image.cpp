/***********************************************************************
Image - Class for widgets displaying image as textures.
Copyright (c) 2011-2014 Oliver Kreylos

This file is part of the GLMotif Widget Library (GLMotif).

The GLMotif Widget Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GLMotif Widget Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the GLMotif Widget Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <GLMotif/Image.h>

#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLVertexTemplates.h>
#include <GL/GLContextData.h>
#include <GL/Extensions/GLARBTextureNonPowerOfTwo.h>
#include <Images/ReadImageFile.h>

namespace GLMotif {

/********************************
Methods of class Image::DataItem:
********************************/

Image::DataItem::DataItem(void)
	:npotdtSupported(GLARBTextureNonPowerOfTwo::isSupported()),
	 textureObjectId(0),version(0),
	 regionVersion(0),settingsVersion(0)
	{
	if(npotdtSupported)
		GLARBTextureNonPowerOfTwo::initExtension();
	
	glGenTextures(1,&textureObjectId);
	}

Image::DataItem::~DataItem(void)
	{
	glDeleteTextures(1,&textureObjectId);
	}

/**********************
Methods of class Image:
**********************/

Image::Image(const char* sName,Container* sParent,const Images::RGBImage& sImage,const GLfloat sResolution[2],bool sManageChild)
	:Widget(sName,sParent,false),
	 image(sImage),version(1),
	 regionVersion(1),
	 interpolationMode(GL_NEAREST),settingsVersion(1),
	 illuminated(false)
	{
	/* Copy the image resolution: */
	for(int i=0;i<2;++i)
		resolution[i]=sResolution[i];
	
	/* Initialize the region to display the entire image: */
	region[0]=0.0f;
	region[1]=0.0f;
	region[2]=GLfloat(image.getWidth());
	region[3]=GLfloat(image.getHeight());
	
	/* Manage me: */
	if(sManageChild)
		manageChild();
	}

Image::Image(const char* sName,Container* sParent,const char* imageFileName,const GLfloat sResolution[2],bool sManageChild)
	:Widget(sName,sParent,false),
	 version(1),
	 regionVersion(1),
	 interpolationMode(GL_NEAREST),settingsVersion(1),
	 illuminated(false)
	{
	/* Load the image file: */
	image=Images::readImageFile(imageFileName);
	
	/* Copy the image resolution: */
	for(int i=0;i<2;++i)
		resolution[i]=sResolution[i];
	
	/* Initialize the region to display the entire image: */
	region[0]=0.0f;
	region[1]=0.0f;
	region[2]=GLfloat(image.getWidth());
	region[3]=GLfloat(image.getHeight());
	
	/* Manage me: */
	if(sManageChild)
		manageChild();
	}

Vector Image::calcNaturalSize(void) const
	{
	/* Calculate the widget's natural interior size based on the image resolution and display region: */
	Vector size;
	for(int i=0;i<2;++i)
		{
		GLfloat regionSize=region[2+i]-region[i];
		if(regionSize>GLfloat(image.getSize(i)))
			regionSize=GLfloat(image.getSize(i));
		size[i]=regionSize/resolution[i];
		}
	size[2]=0.0f;
	
	/* Return the widget's exterior size: */
	return calcExteriorSize(size);
	}

void Image::resize(const Box& newExterior)
	{
	/* Resize the parent class widget: */
	Widget::resize(newExterior);
	
	/* Adjust the displayed image region: */
	setRegion(region);
	}

void Image::draw(GLContextData& contextData) const
	{
	/* Draw parent class decorations: */
	Widget::draw(contextData);
	
	/* Draw the image frame, in case there is one: */
	glBegin(GL_QUAD_STRIP);
	glColor(getBackgroundColor());
	glNormal3f(0.0f,0.0f,1.0f);
	glVertex(imageBox.getCorner(0));
	glVertex(getInterior().getCorner(0));
	glVertex(imageBox.getCorner(1));
	glVertex(getInterior().getCorner(1));
	glVertex(imageBox.getCorner(3));
	glVertex(getInterior().getCorner(3));
	glVertex(imageBox.getCorner(2));
	glVertex(getInterior().getCorner(2));
	glVertex(imageBox.getCorner(0));
	glVertex(getInterior().getCorner(0));
	glEnd();
	
	/* Get the context data item: */
	DataItem* dataItem=contextData.retrieveDataItem<DataItem>(this);
	
	/* Set up OpenGL state: */
	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_TEXTURE_2D);
	if(illuminated)
		{
		glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
		glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL,GL_SEPARATE_SPECULAR_COLOR);
		}
	else
		glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
	
	/* Bind the texture object: */
	glBindTexture(GL_TEXTURE_2D,dataItem->textureObjectId);
	
	/* Check if the texture object is outdated: */
	if(dataItem->version!=version)
		{
		/* Upload the new texture image: */
		image.glTexImage2D(GL_TEXTURE_2D,0,GL_RGB8,!dataItem->npotdtSupported);
		
		/* Update the texture image's size: */
		if(dataItem->npotdtSupported)
			{
			/* Copy the image's size: */
			for(int i=0;i<2;++i)
				dataItem->textureSize[i]=image.getSize(i);
			}
		else
			{
			/* Power-of-two pad the image's size: */
			for(int i=0;i<2;++i)
				for(dataItem->textureSize[i]=1;dataItem->textureSize[i]<image.getSize(i);dataItem->textureSize[i]<<=1)
					;
			}
		
		/* Mark the texture object as up-to-date: */
		dataItem->version=version;
		}
	
	/* Check if the texture coordinate cache is outdated: */
	if(dataItem->regionVersion!=regionVersion)
		{
		/* Calculate the new image region's texture coordinates: */
		dataItem->regionTex[0]=GLfloat(region[0])/GLfloat(dataItem->textureSize[0]);
		if(dataItem->regionTex[0]<0.0f)
			dataItem->regionTex[0]=0.0f;
		dataItem->regionTex[1]=GLfloat(region[1])/GLfloat(dataItem->textureSize[1]);
		if(dataItem->regionTex[1]<0.0f)
			dataItem->regionTex[1]=0.0f;
		dataItem->regionTex[2]=GLfloat(region[2])/GLfloat(dataItem->textureSize[0]);
		if(dataItem->regionTex[2]>GLfloat(image.getWidth())/dataItem->textureSize[0])
			dataItem->regionTex[2]=GLfloat(image.getWidth())/dataItem->textureSize[0];
		dataItem->regionTex[3]=GLfloat(region[3])/GLfloat(dataItem->textureSize[1]);
		if(dataItem->regionTex[3]>GLfloat(image.getHeight())/dataItem->textureSize[1])
			dataItem->regionTex[3]=GLfloat(image.getHeight())/dataItem->textureSize[1];
		
		/* Mark the texture coordinate cache as up-to-date: */
		dataItem->regionVersion=regionVersion;
		}
	
	/* Check if the texture display mode is outdated: */
	if(dataItem->settingsVersion!=settingsVersion)
		{
		/* Set the texture interpolation mode: */
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,interpolationMode);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,interpolationMode);
		
		/* Mark the texture display mode as up-to-date: */
		dataItem->settingsVersion=settingsVersion;
		}
	
	/* Draw the image: */
	glBegin(GL_QUADS);
	glTexCoord2f(dataItem->regionTex[0],dataItem->regionTex[1]);
	glVertex(imageBox.getCorner(0));
	glTexCoord2f(dataItem->regionTex[2],dataItem->regionTex[1]);
	glVertex(imageBox.getCorner(1));
	glTexCoord2f(dataItem->regionTex[2],dataItem->regionTex[3]);
	glVertex(imageBox.getCorner(3));
	glTexCoord2f(dataItem->regionTex[0],dataItem->regionTex[3]);
	glVertex(imageBox.getCorner(2));
	glEnd();
	
	/* Protect the texture object: */
	glBindTexture(GL_TEXTURE_2D,0);
	
	/* Restore OpenGL state: */
	if(illuminated)
		glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL,GL_SINGLE_COLOR);
	glPopAttrib();
	}

void Image::initContext(GLContextData& contextData) const
	{
	/* Create and register the context data item: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	
	/* Bind the texture object: */
	glBindTexture(GL_TEXTURE_2D,dataItem->textureObjectId);
	
	/* Initialize basic texture settings: */
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_BASE_LEVEL,0);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAX_LEVEL,0);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
	
	/* Protect the texture object: */
	glBindTexture(GL_TEXTURE_2D,0);
	}

void Image::updateImage(void)
	{
	++version;
	}

void Image::setRegion(const GLfloat newRegion[4])
	{
	/* Copy the new region extents: */
	for(int i=0;i<4;++i)
		region[i]=newRegion[i];
	
	/* Calculate the image box: */
	imageBox=getInterior();
	
	/* Adjust the image box to fill the widget's interior at constant aspect ratio: */
	GLfloat ww=getInterior().size[0];
	GLfloat wh=getInterior().size[1];
	GLfloat rw=(region[2]-region[0])/resolution[0];
	GLfloat rh=(region[3]-region[1])/resolution[1];
	if(ww*rh>rw*wh) // Interior is wider than region
		{
		/* Shrink the image box horizontally: */
		GLfloat delta=(ww-rw*wh/rh);
		imageBox.origin[0]+=delta*0.5f;
		imageBox.size[0]-=delta;
		}
	else // Interior is narrower than region
		{
		/* Shrink the image box vertically: */
		GLfloat delta=(wh-rh*ww/rw);
		imageBox.origin[1]+=delta*0.5f;
		imageBox.size[1]-=delta;
		}
	
	/* Adjust the image box to account for regions larger than the image: */
	if(region[0]<0.0f)
		{
		GLfloat delta=(0.0f-region[0])/(region[2]-region[0])*ww;
		imageBox.origin[0]+=delta;
		imageBox.size[0]-=delta;
		}
	if(region[2]>GLfloat(image.getWidth()))
		{
		GLfloat delta=(region[2]-GLfloat(image.getWidth()))/(region[2]-region[0])*ww;
		imageBox.size[0]-=delta;
		}
	if(region[1]<0.0f)
		{
		GLfloat delta=(0.0f-region[1])/(region[3]-region[1])*wh;
		imageBox.origin[1]+=delta;
		imageBox.size[1]-=delta;
		}
	if(region[3]>GLfloat(image.getHeight()))
		{
		GLfloat delta=(region[3]-GLfloat(image.getHeight()))/(region[3]-region[1])*wh;
		imageBox.size[1]-=delta;
		}
	
	/* Invalidate the cached region: */
	++regionVersion;
	}

void Image::setInterpolationMode(GLenum newInterpolationMode)
	{
	interpolationMode=newInterpolationMode;
	++settingsVersion;
	}

void Image::setIlluminated(bool newIlluminated)
	{
	illuminated=newIlluminated;
	}

}
