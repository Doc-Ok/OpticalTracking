/***********************************************************************
IFFImageFileReader - Class to read images in IFF format from a stream
data source.
Copyright (c) 2008-2010 Oliver Kreylos

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

#ifndef IMAGES_IFFIMAGEFILEREADER_INCLUDED
#define IMAGES_IFFIMAGEFILEREADER_INCLUDED

#include <GL/gl.h>
#include <GL/GLColor.h>

namespace Images {

template <class DataSourceParam>
class IFFImageFileReader
	{
	/* Embedded classes: */
	public:
	typedef DataSourceParam DataSource; // Type of image data source
	private:
	enum Masking // Type for image masking types
		{
		OPAQUE=0,INTERLEAVED=1,TRANSPARENT_COLOR=2,LASSOED=3
		};
	typedef GLColor<GLubyte,3> ColorMapEntry; // Type of color map data
	
	/* Elements: */
	private:
	DataSource& dataSource; // Source of image data
	unsigned int offset[2]; // Coordinates of lower-left image pixel
	unsigned int size[2]; // Width and height of image
	unsigned int numBitPlanes; // Number of bits per pixel
	int masking; // Masking type for the image
	bool compress; // Flag whether image data is compressed
	unsigned int transparentColorIndex; // Index of transparent color
	ColorMapEntry* colorMap; // Color map
	unsigned int imageDataSize; // Size of image data in bytes
	
	/* Constructors and destructors: */
	public:
	IFFImageFileReader(DataSource& sDataSource); // Creates an IFF image reader for the given data source
	~IFFImageFileReader(void); // Destroys the IFF image reader
	
	/* Methods: */
	const unsigned int* getSize(void) const // Returns the size of the image stored in the IFF image file
		{
		return size;
		}
	unsigned int getSize(int dimension) const // Returns the width or height of the image stored in the IFF image file
		{
		return size[dimension];
		}
	
	/* Image reading functions, can be called only once for each file reader: */
	template <class ImageParam>
	ImageParam readImage(void); // Reads the contents of the IFF image file into an image of template class Image
	};

}

#ifndef IMAGES_IFFIMAGEFILEREADER_IMPLEMENTATION
#include <Images/IFFImageFileReader.icpp>
#endif

#endif
