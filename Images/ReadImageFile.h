/***********************************************************************
ReadImageFile - Functions to read RGB images from a variety of file
formats.
Copyright (c) 2005-2011 Oliver Kreylos

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

#ifndef IMAGES_READIMAGEFILE_INCLUDED
#define IMAGES_READIMAGEFILE_INCLUDED

#include <IO/File.h>
#include <Images/RGBImage.h>
#include <Images/RGBAImage.h>

namespace Images {

bool canReadImageFileType(const char* imageFileName); // Returns true if the image reader supports the image's file type

RGBImage readImageFile(const char* imageFileName,IO::FilePtr file); // Reads an RGB image from an already-open file; auto-detects file format
RGBImage readImageFile(const char* imageFileName); // Ditto, but opens the given file itself

RGBAImage readTransparentImageFile(const char* imageFileName,IO::FilePtr file); // Reads an RGB image with alpha layer from an already-open file; auto-detects file format
RGBAImage readTransparentImageFile(const char* imageFileName); // Ditto, but opens the given file itself

RGBAImage readCursorFile(const char* cursorFileName,IO::FilePtr file,unsigned int nominalSize,unsigned int* hotspot =0); // Reads an RGBA image from a cursor file in Xcursor format
RGBAImage readCursorFile(const char* cursorFileName,unsigned int nominalSize,unsigned int* hotspot =0); // Reads an RGBA image from a cursor file in Xcursor format

}

#endif
