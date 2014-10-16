/***********************************************************************
Image - Base class to represent images of arbitrary pixel formats. The
image coordinate system is such that pixel (0,0) is in the lower-left
corner.
Copyright (c) 2007-2014 Oliver Kreylos

This file is part of the Image Handling Library (Images).

The Image Handling Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Image Handling Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Image Handling Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <string.h>
#include <Math/Math.h>

#include <Images/Image.h>

namespace Images {

/*******************************************
Methods of class Image::ImageRepresentation:
*******************************************/

template <class ScalarParam,int numComponentsParam>
Image<ScalarParam,numComponentsParam>::ImageRepresentation::ImageRepresentation(
	unsigned int sWidth,
	unsigned int sHeight)
	:refCount(1),
	 image(0)
	{
	/* Set the image size: */
	size[0]=sWidth;
	size[1]=sHeight;
	
	/* Allocate the image array: */
	image=new Color[size[0]*size[1]];
	}

template <class ScalarParam,int numComponentsParam>
Image<ScalarParam,numComponentsParam>::ImageRepresentation::ImageRepresentation(
	const typename Image<ScalarParam,numComponentsParam>::ImageRepresentation& source)
	:refCount(1),
	 image(0)
	{
	/* Set the image size: */
	for(int i=0;i<2;++i)
		size[i]=source.size[i];
	
	/* Allocate the image array: */
	image=new Color[size_t(size[0])*size_t(size[1])];
	
	/* Copy the source image data: */
	memcpy(image,source.image,size_t(size[0])*size_t(size[1])*sizeof(Color));
	}

template <class ScalarParam,int numComponentsParam>
template <class SourceScalarParam,int sourceNumComponentsParam>
Image<ScalarParam,numComponentsParam>::ImageRepresentation::ImageRepresentation(
	const unsigned int sSize[2],
	const GLColor<SourceScalarParam,sourceNumComponentsParam>* sPixels)
	:refCount(1),
	 image(0)
	{
	/* Set the image size: */
	for(int i=0;i<2;++i)
		size[i]=sSize[i];
	
	/* Allocate the image array: */
	image=new Color[size_t(size[0])*size_t(size[1])];
	
	/* Copy the source image data: */
	const typename Image<SourceScalarParam,sourceNumComponentsParam>::Color* sPtr=sPixels;
	Color* dPtr=image;
	for(unsigned int y=0;y<size[1];++y)
		for(unsigned int x=0;x<size[0];++x,++sPtr,++dPtr)
			*dPtr=*sPtr;
	}

template <class ScalarParam,int numComponentsParam>
Image<ScalarParam,numComponentsParam>::ImageRepresentation::~ImageRepresentation(void)
	{
	/* Destroy the image array: */
	delete[] image;
	}

/**********************
Methods of class Image:
**********************/

template <class ScalarParam,int numComponentsParam>
void
Image<ScalarParam,numComponentsParam>::readPixels(
	GLint x,
	GLint y,
	GLenum pixelFormat,
	GLenum componentType)
	{
	/* Un-share the image representation: */
	ownRepresentation();
	
	/* Set up pixel processing pipeline: */
	glPixelStorei(GL_PACK_ALIGNMENT,1);
	glPixelStorei(GL_PACK_SKIP_PIXELS,0);
	glPixelStorei(GL_PACK_ROW_LENGTH,0);
	glPixelStorei(GL_PACK_SKIP_ROWS,0);
	
	/* Read image: */
	glReadPixels(x,y,rep->size[0],rep->size[1],pixelFormat,componentType,rep->image);
	}

template <class ScalarParam,int numComponentsParam>
void
Image<ScalarParam,numComponentsParam>::drawPixels(
	GLenum pixelFormat,
	GLenum componentType) const
	{
	/* Set up pixel processing pipeline: */
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS,0);
	glPixelStorei(GL_UNPACK_ROW_LENGTH,0);
	glPixelStorei(GL_UNPACK_SKIP_ROWS,0);
	
	/* Write image: */
	glDrawPixels(rep->size[0],rep->size[1],pixelFormat,componentType,rep->image);
	}

template <class ScalarParam,int numComponentsParam>
void
Image<ScalarParam,numComponentsParam>::texImage2D(
	GLenum target,
	GLint level,
	GLint internalFormat,
	GLenum pixelFormat,
	GLenum componentType,
	bool padImageSize) const
	{
	/* Set up pixel processing pipeline: */
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS,0);
	glPixelStorei(GL_UNPACK_ROW_LENGTH,0);
	glPixelStorei(GL_UNPACK_SKIP_ROWS,0);
	
	if(padImageSize)
		{
		/* Calculate the texture width and height as the next power of two: */
		GLsizei texSize[2];
		for(int i=0;i<2;++i)
			for(texSize[i]=1;texSize[i]<GLsizei(rep->size[i]);texSize[i]<<=1)
				;
		
		/* Create the padded texture image: */
		glTexImage2D(target,level,internalFormat,texSize[0],texSize[1],0,pixelFormat,componentType,0);
		
		/* Upload the image: */
		glTexSubImage2D(target,level,0,0,rep->size[0],rep->size[1],pixelFormat,componentType,rep->image);
		}
	else
		{
		/* Upload image: */
		glTexImage2D(target,level,internalFormat,rep->size[0],rep->size[1],0,pixelFormat,componentType,rep->image);
		}
	}

template <class ScalarParam,int numComponentsParam>
void
Image<ScalarParam,numComponentsParam>::clear(
	const typename Image<ScalarParam,numComponentsParam>::Color& c)
	{
	/* Un-share the image representation: */
	ownRepresentation();
	
	/* Set all pixel values: */
	size_t numPixels=size_t(rep->size[0])*size_t(rep->size[1]);
	Color* imgPtr=rep->image;
	for(unsigned int i=numPixels;i>0;--i,++imgPtr)
		*imgPtr=c;
	}

template <class ScalarParam,int numComponentsParam>
void
Image<ScalarParam,numComponentsParam>::resize(
	unsigned int newWidth,
	unsigned int newHeight)
	{
	unsigned int oldWidth=rep->size[0];
	unsigned int oldHeight=rep->size[1];
	
	/* Allocate an intermediate floating-point buffer: */
	typedef GLColor<GLfloat,numComponents> FColor;
	FColor* buffer=new FColor[size_t(newWidth)*size_t(oldHeight)];
	
	/* Resample pixel rows: */
	for(unsigned int x=0;x<newWidth;++x)
		{
		float sampleX=(float(x)+0.5f)*float(oldWidth)/float(newWidth)+0.5f;
		/* Note: sampleX is the x coordinate of the next pixel to the right, to get around some issues. */
		
		unsigned int sx=(unsigned int)(sampleX);
		const Color* sCol0=sx>0?&rep->image[sx-1]:&rep->image[0];
		const Color* sCol1=sx<oldWidth?&rep->image[sx]:&rep->image[oldWidth-1];
		float w1=sampleX-float(sx);
		float w0=1.0f-w1;
		FColor* dCol=&buffer[x];
		for(unsigned int y=0;y<oldHeight;++y,sCol0+=oldWidth,sCol1+=oldWidth,dCol+=newWidth)
			{
			for(int i=0;i<numComponents;++i)
				(*dCol)[i]=GLfloat((*sCol0)[i])*w0+GLfloat((*sCol1)[i])*w1;
			}
		}
	
	/* Create the new image representation: */
	rep->detach();
	rep=new ImageRepresentation(newWidth,newHeight);
	
	/* Resample pixel columns: */
	for(unsigned int y=0;y<newHeight;++y)
		{
		float sampleY=(float(y)+0.5f)*float(oldHeight)/float(newHeight)+0.5f;
		/* Note: sampleY is the y coordinate of the next pixel to the top, to get around some issues. */
		
		unsigned int sy=(unsigned int)(sampleY);
		const FColor* sRow0=sy>0?&buffer[(sy-1)*newWidth]:&buffer[0];
		const FColor* sRow1=sy<oldHeight?&buffer[sy*newWidth]:&buffer[(oldHeight-1)*newWidth];
		float w1=sampleY-float(sy);
		float w0=1.0f-w1;
		Color* dRow=&rep->image[y*newWidth];
		for(unsigned int x=0;x<newWidth;++x,++sRow0,++sRow1,++dRow)
			{
			for(int i=0;i<numComponents;++i)
				(*dRow)[i]=Scalar((*sRow0)[i]*w0+(*sRow1)[i]*w1+0.5f);
			}
		}
	delete[] buffer;
	}

/*************************************************
Force instantiation of all standard Image classes:
*************************************************/

template Image<GLubyte,3>::ImageRepresentation::ImageRepresentation(const unsigned int[2],const GLColor<GLushort,3>*);
template Image<GLubyte,3>::ImageRepresentation::ImageRepresentation(const unsigned int[2],const GLColor<GLubyte,4>*);
template Image<GLubyte,3>::ImageRepresentation::ImageRepresentation(const unsigned int[2],const GLColor<GLushort,4>*);
template Image<GLushort,3>::ImageRepresentation::ImageRepresentation(const unsigned int[2],const GLColor<GLubyte,3>*);
template Image<GLushort,3>::ImageRepresentation::ImageRepresentation(const unsigned int[2],const GLColor<GLubyte,4>*);
template Image<GLushort,3>::ImageRepresentation::ImageRepresentation(const unsigned int[2],const GLColor<GLushort,4>*);
template Image<GLubyte,4>::ImageRepresentation::ImageRepresentation(const unsigned int[2],const GLColor<GLubyte,3>*);
template Image<GLubyte,4>::ImageRepresentation::ImageRepresentation(const unsigned int[2],const GLColor<GLushort,3>*);
template Image<GLubyte,4>::ImageRepresentation::ImageRepresentation(const unsigned int[2],const GLColor<GLushort,4>*);
template Image<GLushort,4>::ImageRepresentation::ImageRepresentation(const unsigned int[2],const GLColor<GLushort,3>*);
template Image<GLushort,4>::ImageRepresentation::ImageRepresentation(const unsigned int[2],const GLColor<GLubyte,3>*);
template Image<GLushort,4>::ImageRepresentation::ImageRepresentation(const unsigned int[2],const GLColor<GLubyte,4>*);

template class Image<GLubyte,3>;
template class Image<GLubyte,4>;
template class Image<GLushort,3>;
template class Image<GLushort,4>;
template class Image<GLfloat,3>;
template class Image<GLfloat,4>;

}
