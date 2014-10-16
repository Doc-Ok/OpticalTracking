/***********************************************************************
ImageReaderPNM - Class to read images from files in Portable aNyMap
format.
Copyright (c) 2013 Oliver Kreylos

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

#ifndef IMAGES_IMAGEREADERPNM_INCLUDED
#define IMAGES_IMAGEREADERPNM_INCLUDED

#include <Images/ImageReader.h>

namespace Images {

class ImageReaderPNM:public ImageReader
	{
	/* Elements: */
	private:
	unsigned int imageType; // Type of the image stored in the PNM file
	unsigned int maxValue; // Maximum channel value used by the image
	bool done; // Flag set after the only image in the image file has been read
	
	/* Constructors and destructors: */
	public:
	ImageReaderPNM(IO::FilePtr sFile); // Creates a PNM image reader for the given file
	
	/* Methods from ImageReader: */
	virtual bool eof(void) const;
	virtual void readNative(ImageReader::ImagePlane imagePlanes[]);
	};

}

#endif
