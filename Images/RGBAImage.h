/***********************************************************************
RGBAImage - Specialized image class to represent RGBA images with 8-bit
color depth.
Copyright (c) 2007 Oliver Kreylos

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

#ifndef IMAGES_RGBAIMAGE_INCLUDED
#define IMAGES_RGBAIMAGE_INCLUDED

#include <GL/gl.h>
#include <Images/Image.h>

namespace Images {

class RGBAImage:public Image<GLubyte,4>
	{
	/* Embedded classes: */
	public:
	typedef Image<GLubyte,4> Base; // Type of base class
	
	/* Constructors and destructors: */
	public:
	RGBAImage(void) // Creates an invalid image
		{
		}
	RGBAImage(unsigned int sWidth,unsigned int sHeight) // Creates an uninitialized image of the given size
		:Base(sWidth,sHeight)
		{
		}
	RGBAImage(const RGBAImage& source) // Copies an existing image (does not copy image representation)
		:Base(source)
		{
		}
	template <class SourceScalarParam,int sourceNumComponentsParam>
	RGBAImage(const Image<SourceScalarParam,sourceNumComponentsParam>& source) // Creates an image from given source image with different number of channels and/or scalar type
		:Base(source)
		{
		}
	RGBAImage& operator=(const RGBAImage& source) // Assigns an existing image (does not copy image representation)
		{
		Base::operator=(source);
		return *this;
		}
	template <class SourceScalarParam,int sourceNumComponentsParam>
	RGBAImage& operator=(const Image<SourceScalarParam,sourceNumComponentsParam>& source) // Creates an image from given source image with different number of channels and/or scalar type
		{
		Base::operator=(source);
		return *this;
		}
	
	/* Methods: */
	static RGBAImage glReadPixels(GLint x,GLint y,GLsizei width,GLsizei height) // Creates a new image by reading from the frame buffer
		{
		RGBAImage result(width,height);
		result.readPixels(x,y,GL_RGBA,GL_UNSIGNED_BYTE);
		return result;
		}
	
	/****************************************************************************
	The following methods fail if the image does not have a valid representation!
	****************************************************************************/
	
	RGBAImage& glReadPixels(GLint x,GLint y) // Reads the frame buffer contents into the image
		{
		readPixels(x,y,GL_RGBA,GL_UNSIGNED_BYTE);
		return *this;
		}
	void glDrawPixels(void) const // Writes image to the frame buffer at the current raster position
		{
		drawPixels(GL_RGBA,GL_UNSIGNED_BYTE);
		}
	void glTexImage2D(GLenum target,GLint level,GLint internalFormat,bool padImageSize =false) const // Uploads an image as an OpenGL texture
		{
		texImage2D(target,level,internalFormat,GL_RGBA,GL_UNSIGNED_BYTE,padImageSize);
		}
	};

}

#endif
