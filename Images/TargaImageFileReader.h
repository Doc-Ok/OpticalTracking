/***********************************************************************
TargaImageFileReader - Class to read images in TARGA format from a
stream data source.
Copyright (c) 2007-2010 Oliver Kreylos

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

#ifndef IMAGES_TARGAIMAGEFILEREADER_INCLUDED
#define IMAGES_TARGAIMAGEFILEREADER_INCLUDED

#include <GL/gl.h>
#include <GL/GLColor.h>

namespace Images {

template <class DataSourceParam>
class TargaImageFileReader
	{
	/* Embedded classes: */
	public:
	typedef DataSourceParam DataSource; // Type of image data source
	private:
	typedef GLColor<GLubyte,4> ColorMapEntry; // Type of color map data
	
	/* Elements: */
	private:
	DataSource& dataSource; // Source of image data
	int imageType; // Type of image stored in the TARGA file
	unsigned int firstColorMapEntry; // Index of first entry stored in color map
	unsigned int numColorMapEntries; // Number of entries stored in color map
	unsigned int offset[2]; // Coordinates of lower-left image pixel
	unsigned int size[2]; // Width and height of image
	unsigned int pixelSize; // Size of a pixel in bits
	unsigned int imageDescriptor; // Bit fields defining the pixel order of the image and the size of the alpha channel
	ColorMapEntry* colorMap; // Color map
	bool imageAlreadyRead; // Flag if the image has already been read (assumes stream data source and cannot revert)
	
	/* Constructors and destructors: */
	public:
	TargaImageFileReader(DataSource& sDataSource); // Creates a Targa image reader for the given data source
	~TargaImageFileReader(void); // Destroys the Targa image reader
	
	/* Methods: */
	const unsigned int* getSize(void) const // Returns the size of the image stored in the Targa image file
		{
		return size;
		}
	unsigned int getSize(int dimension) const // Returns the width or height of the image stored in the Targa image file
		{
		return size[dimension];
		}
	
	/* Image reading functions, can be called only once for each file reader: */
	template <class ImageParam>
	ImageParam readImage(void); // Reads the contents of the Targa image file into an image of template class Image
	};

}

#ifndef IMAGES_TARGAIMAGEFILEREADER_IMPLEMENTATION
#include <Images/TargaImageFileReader.icpp>
#endif

#endif
