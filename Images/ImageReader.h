/***********************************************************************
ImageReader - Abstract base class to read images from files in a variety
of image file formats.
Copyright (c) 2012-2013 Oliver Kreylos

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

#ifndef IMAGES_IMAGEREADER_INCLUDED
#define IMAGES_IMAGEREADER_INCLUDED

#include <stddef.h>
#include <IO/File.h>
#include <Images/Image.h>
#include <Images/RGBImage.h>
#include <Images/RGBAImage.h>

namespace Images {

class ImageReader
	{
	/* Embedded classes: */
	public:
	enum ColorSpace // Enumerated type for image color spaces
		{
		Grayscale, // Image is grayscale
		RGB // Image is in RGB color space
		};
	
	enum ChannelValueType // Enumerated type for image channel data types
		{
		UnsignedInt, // Channel values are unsigned integers
		SignedInt, // Channel values are two's complement signed integers
		Float // Channel values are IEEE floating-point numbers
		};
	
	struct ChannelSpec // Specification for an image channel
		{
		/* Elements: */
		public:
		ChannelValueType valueType; // Data type for values in this channel
		unsigned int numFieldBits; // Number of bits for values in this channel, usually a multiple of 8 for byte-aligned values
		unsigned int numValueBits; // Number of used bits LSB-aligned inside each value field; <= numFieldBits
		};
	
	struct ImageSpec // Specification for a sub-image inside the image file
		{
		/* Elements: */
		public:
		unsigned int offset[2]; // Position of image inside the canvas
		unsigned int size[2]; // Width and height of image in pixels
		ColorSpace colorSpace; // Color space of the image
		bool hasAlpha; // Flag if the image has an alpha channel
		unsigned int numChannels; // Number of channels in image; typically 1 (grayscale), 2 (grayscale+alpha), 3 (RGB), or 4 (RGB+alpha)
		ChannelSpec* channelSpecs; // Array of channel specifications
		};
	
	struct ImagePlane // Structure describing the memory layout of an image component plane
		{
		/* Elements: */
		public:
		void* basePtr; // Pointer to lower-left pixel
		ptrdiff_t pixelStride; // Pointer stride within an image row in bytes
		ptrdiff_t rowStride; // Pointer stride between image rows in bytes
		};
	
	/* Elements: */
	protected:
	IO::FilePtr file; // Handle of the image file
	unsigned int canvasSize[2]; // Size of the image canvas, i.e., the bounding box of all sub-images
	ImageSpec imageSpec; // Specification structure for the next image to be read from the file
	
	/* Protected methods: */
	virtual ImagePlane* readSubImagePlanes(void) =0; // Reads an image and returns an array of one image plane descriptor per image component
	virtual void releaseSubImagePlanes(ImagePlane* planes) =0; // Releases the image's planes after copying
	
	/* Constructors and destructors: */
	public:
	ImageReader(IO::FilePtr sFile); // Creates an image reader for the given file
	virtual ~ImageReader(void);
	
	/* Methods: */
	const unsigned int* getCanvasSize(void) const // Returns the size of the image canvas
		{
		return canvasSize;
		}
	unsigned int getCanvasSize(int dimension) const // Ditto, for single dimension
		{
		return canvasSize[dimension];
		}
	virtual bool eof(void) const =0; // Returns true if there are no more images to read in the image file
	
	/* The following methods must not be called if eof() returns true: */
	const ImageSpec& getImageSpec(void) const // Returns the specification of the next image to be returned by the image reader methods
		{
		return imageSpec;
		}
	virtual void readNative(const ImagePlane imagePlanes[]) =0; // Reads an image in its native format into caller-allocated image planes; planes must be of types and sizes compatible with native image format
	RGBImage readRGB8(void); // Reads the image as an 8-bit RGB image
	RGBAImage readRGBA8(void); // Reads the image as an 8-bit RGB with alpha image
	};

}

#endif
