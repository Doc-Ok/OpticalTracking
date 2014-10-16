/***********************************************************************
ImageExtractorYUYV - Class to extract images from raw video frames
encoded in YpCbCr 4:2:2 format.
Copyright (c) 2010 Oliver Kreylos

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

#ifndef VIDEO_IMAGEEXTRACTORYUYV_INCLUDED
#define VIDEO_IMAGEEXTRACTORYUYV_INCLUDED

#include <Video/ImageExtractor.h>

namespace Video {

class ImageExtractorYUYV:public ImageExtractor
	{
	/* Elements: */
	private:
	unsigned int size[2]; // Frame width and height
	
	/* Constructors and destructors: */
	public:
	ImageExtractorYUYV(const unsigned int sSize[2]); // Constructs an extractor for the given frame size
	
	/* Methods from ImageExtractor: */
	public:
	virtual void extractGrey(const FrameBuffer* frame,void* image);
	virtual void extractRGB(const FrameBuffer* frame,void* image);
	virtual void extractYpCbCr420(const FrameBuffer* frame,void* yp,unsigned int ypStride,void* cb,unsigned int cbStride,void* cr,unsigned int crStride);
	};

}

#endif
