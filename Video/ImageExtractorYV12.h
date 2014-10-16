/***********************************************************************
ImageExtractorYV12 - Class to extract images from raw video frames
encoded in YpCbCr 4:2:0 format.
Copyright (c) 2013 Oliver Kreylos

This file is part of the Basic Video Library (Video).

The Basic Video Library is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as published
by the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

The Basic Video Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Basic Video Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef VIDEO_IMAGEEXTRACTORYV12_INCLUDED
#define VIDEO_IMAGEEXTRACTORYV12_INCLUDED

#include <stddef.h>
#include <Video/ImageExtractor.h>

namespace Video {

class ImageExtractorYV12:public ImageExtractor
	{
	/* Embedded classes: */
	private:
	struct Plane // Structure defining the layout of an image plane
		{
		/* Elements: */
		public:
		ptrdiff_t offset; // Frame buffer offset of plane's first (top-left) pixel
		ptrdiff_t stride; // Plane's inter-row stride in bytes
		};
	
	/* Elements: */
	private:
	unsigned int size[2]; // Frame width and height (width and height of Y' plane)
	Plane planes[3]; // Layout of the Y', Cb, and Cr planes
	
	/* Constructors and destructors: */
	public:
	ImageExtractorYV12(const unsigned int sSize[2],ptrdiff_t ypOffset,ptrdiff_t ypStride,ptrdiff_t cbOffset,ptrdiff_t cbStride,ptrdiff_t crOffset,ptrdiff_t crStride); // Constructs an extractor for the given frame size and plane layout
	
	/* Methods from ImageExtractor: */
	public:
	virtual void extractGrey(const FrameBuffer* frame,void* image);
	virtual void extractRGB(const FrameBuffer* frame,void* image);
	virtual void extractYpCbCr420(const FrameBuffer* frame,void* yp,unsigned int ypStride,void* cb,unsigned int cbStride,void* cr,unsigned int crStride);
	};

}

#endif
