/***********************************************************************
PNMImageFileReader - Class to read images in PNM format from a stream
data source.
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

#ifndef IMAGES_PNMIMAGEFILEREADER_INCLUDED
#define IMAGES_PNMIMAGEFILEREADER_INCLUDED

namespace Images {

template <class DataSourceParam>
class PNMImageFileReader
	{
	/* Embedded classes: */
	public:
	typedef DataSourceParam DataSource; // Type of image data source
	
	/* Elements: */
	private:
	DataSource& dataSource; // Source of image data
	int fileFormat; // Type of the PNM file (number read from first header line)
	unsigned int size[2]; // Width and height of image
	unsigned int maxVal; // Maximum pixel value in image
	bool imageAlreadyRead; // Flag if the image has already been read (assumes stream data source and cannot revert)
	
	/* Constructors and destructors: */
	public:
	PNMImageFileReader(DataSource& sDataSource); // Creates a PNM image reader for the given data source
	
	/* Methods: */
	const unsigned int* getSize(void) const // Returns the size of the image stored in the PNM image file
		{
		return size;
		}
	unsigned int getSize(int dimension) const // Returns the width or height of the image stored in the PNM image file
		{
		return size[dimension];
		}
	
	/* Image reading functions, can be called only once for each file reader: */
	template <class ImageParam>
	ImageParam readImage(void); // Reads the contents of the PNM image file into an image of template class Image
	};

}

#ifndef IMAGES_PNMIMAGEFILEREADER_IMPLEMENTATION
#include <Images/PNMImageFileReader.icpp>
#endif

#endif
