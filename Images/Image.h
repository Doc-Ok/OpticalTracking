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

#ifndef IMAGES_IMAGE_INCLUDED
#define IMAGES_IMAGE_INCLUDED

#include <stddef.h>
#include <GL/gl.h>
#include <GL/GLColor.h>

namespace Images {

template <class ScalarParam,int numComponentsParam>
class Image
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar; // Data type for pixel color components
	static const int numComponents=numComponentsParam; // Number of pixel components
	typedef GLColor<Scalar,numComponentsParam> Color; // Data type for pixel colors
	
	private:
	struct ImageRepresentation // Structure to represent an image
		{
		/* Elements: */
		public:
		size_t refCount; // Number of RGBImage objects referencing this representation
		unsigned int size[2]; // Image size (width, height)
		Color* image; // Pointer to the image array
		
		/* Constructors and destructors: */
		ImageRepresentation(unsigned int sWidth,unsigned int sHeight); // Creates a new image representation with uninitialized data
		ImageRepresentation(const ImageRepresentation& source); // Copies an existing image representation
		template <class SourceScalarParam,int sourceNumComponentsParam>
		ImageRepresentation(const unsigned int sSize[2],const GLColor<SourceScalarParam,sourceNumComponentsParam>* sPixels); // Copies an existing image representation with scalar type conversion and channel number adaptation
		~ImageRepresentation(void); // Destroys an image representation
		
		/* Methods: */
		ImageRepresentation* attach(void)
			{
			++refCount;
			return this;
			}
		void detach(void)
			{
			--refCount;
			if(refCount==0)
				delete this;
			}
		};
	
	/* Elements: */
	private:
	ImageRepresentation* rep; // Pointer to a (shared) image representation
	
	/* Private methods: */
	void ownRepresentation(void) // Ensures that image representation is not shared
		{
		/* Copy the image representation if it is shared: */
		if(rep->refCount>1)
			{
			ImageRepresentation* newRep=new ImageRepresentation(*rep);
			rep->detach();
			rep=newRep;
			}
		}
	
	/* Protected methods: */
	protected:
	void readPixels(GLint x,GLint y,GLenum pixelFormat,GLenum componentType);
	void drawPixels(GLenum pixelFormat,GLenum componentType) const;
	void texImage2D(GLenum target,GLint level,GLint internalFormat,GLenum pixelFormat,GLenum componentType,bool padImageSize) const;
	
	/* Constructors and destructors: */
	public:
	Image(void) // Creates an invalid image
		:rep(0)
		{
		}
	private:
	Image(ImageRepresentation* sRep) // Creates an image with a pre-attached representation
		:rep(sRep)
		{
		}
	public:
	Image(unsigned int sWidth,unsigned int sHeight) // Creates an uninitialized image of the given size
		:rep(new ImageRepresentation(sWidth,sHeight))
		{
		}
	Image(const Image& source) // Copies an existing image (does not copy image representation)
		:rep(source.rep!=0?source.rep->attach():0)
		{
		}
	template <class SourceScalarParam,int sourceNumComponentsParam>
	Image(const Image<SourceScalarParam,sourceNumComponentsParam>& source) // Creates an image from given source image with different number of channels and/or scalar type
		:rep(source.isValid()?new ImageRepresentation(source.getSize(),source.getPixels()):0)
		{
		}
	Image& operator=(const Image& source) // Assigns an existing image (does not copy image representation)
		{
		if(rep!=source.rep)
			{
			if(rep!=0)
				rep->detach();
			rep=source.rep!=0?source.rep->attach():0;
			}
		return *this;
		}
	template <class SourceScalarParam,int sourceNumComponentsParam>
	Image& operator=(const Image<SourceScalarParam,sourceNumComponentsParam>& source) // Creates an image from given source image with different number of channels and/or scalar type
		{
		if(rep!=0)
			rep->detach();
		rep=source.isValid()?new ImageRepresentation(source.getSize(),source.getPixels()):0;
		return *this;
		}
	~Image(void) // Destroys the image
		{
		if(rep!=0)
			rep->detach();
		}
	
	/* Methods: */
	bool isValid(void) const // Returns if the image has a valid representation
		{
		return rep!=0;
		}
	void invalidate(void) // Invalidates the image, i.e., detaches from any shared pixel buffers
		{
		if(rep!=0)
			rep->detach();
		rep=0;
		}
	
	/****************************************************************************
	The following methods fail if the image does not have a valid representation!
	****************************************************************************/
	
	const unsigned int* getSize(void) const // Returns image size
		{
		return rep->size;
		}
	unsigned int getSize(int dimension) const // Returns one dimension of image size
		{
		return rep->size[dimension];
		}
	unsigned int getWidth(void) const // Returns image width
		{
		return rep->size[0];
		}
	unsigned int getHeight(void) const // Returns image height
		{
		return rep->size[1];
		}
	const Color& getPixel(unsigned int x,unsigned int y) const // Returns color of an image pixel
		{
		/* Return the pixel value: */
		return rep->image[size_t(y)*size_t(rep->size[0])+size_t(x)];
		}
	void setPixel(unsigned int x,unsigned int y,const Color& c) // Sets an image pixel to the given color
		{
		/* Un-share the image representation: */
		ownRepresentation();
		
		/* Set the pixel value: */
		rep->image[size_t(y)*size_t(rep->size[0])+size_t(x)]=c;
		}
	void clear(const Color& c); // Sets all image pixels to the given color
	const Color* getPixels(void) const // Returns pointer to the entire image data for reading
		{
		/* Return the base pointer: */
		return rep->image;
		};
	Color* modifyPixels(void) // Returns pointer to the entire image for writing
		{
		/* Un-share the image representation: */
		ownRepresentation();
		
		/* Return the base pointer: */
		return rep->image;
		};
	const Color* getPixelRow(unsigned int y) const // Returns pointer to an image row for reading
		{
		/* Return the row pointer: */
		return &rep->image[size_t(y)*size_t(rep->size[0])];
		}
	Color* modifyPixelRow(unsigned int y) // Returns pointer to an image row for writing
		{
		/* Un-share the image representation: */
		ownRepresentation();
		
		/* Return the row pointer: */
		return &rep->image[size_t(y)*size_t(rep->size[0])];
		}
	void resize(unsigned int newWidth,unsigned int newHeight); // Resamples the image to the given size
	};

}

#endif
