/***********************************************************************
ImageExtractor - Abstract base class for processors that can extract
image data in a variety of formats from raw video streams.
Copyright (c) 2009-2010 Oliver Kreylos

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

#ifndef VIDEO_IMAGEEXTRACTOR_INCLUDED
#define VIDEO_IMAGEEXTRACTOR_INCLUDED

/* Forward declarations: */
namespace Video {
class FrameBuffer;
}

namespace Video {

class ImageExtractor
	{
	/* Constructors and destructors: */
	public:
	virtual ~ImageExtractor(void)
		{
		}
	
	/* Methods: */
	virtual void extractGrey(const FrameBuffer* frame,void* image) =0; // Extracts an 8-bit greyscale image from the given video buffer
	virtual void extractRGB(const FrameBuffer* frame,void* image) =0; // Extracts an 8-bit RGB image from the given video buffer
	virtual void extractYpCbCr420(const FrameBuffer* frame,void* yp,unsigned int ypStride,void* cb,unsigned int cbStride,void* cr,unsigned int crStride) =0; // Extracts a Y'CbCr image using 4:2:0 downsampling from the given video buffer
	};

}

#endif
