/***********************************************************************
ImageExtractorMJPG - Class to extract images from raw video frames
encoded in Motion JPEG format.
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

#ifndef VIDEO_IMAGEEXTRACTORMJPG_INCLUDED
#define VIDEO_IMAGEEXTRACTORMJPG_INCLUDED

#include <Video/ImageExtractor.h>

/* Forward declarations: */
struct jpeg_error_mgr;
struct jpeg_decompress_struct;

namespace Video {

class ImageExtractorMJPG:public ImageExtractor
	{
	/* Elements: */
	private:
	unsigned int size[2]; // Frame width and height
	jpeg_error_mgr* jpegErrorManager; // JPEG error manager object
	jpeg_decompress_struct* jpegStruct; // JPEG decompression object
	static const unsigned char huffmanBits[4][17]; // Huffman bit tables (DC luminance/chrominance, AC luminance/chrominance)
	static const unsigned char huffmanValues[4][256]; // Huffman value tables (DC luminance/chrominance, AC luminance/chrominance)
	unsigned char** imageRows; // Array of image row pointers needed by the JPEG decompressor
	
	/* Constructors and destructors: */
	public:
	ImageExtractorMJPG(const unsigned int sSize[2]); // Constructs an extractor for the given frame size
	virtual ~ImageExtractorMJPG(void);
	
	/* Methods from ImageExtractor: */
	public:
	virtual void extractGrey(const FrameBuffer* frame,void* image);
	virtual void extractRGB(const FrameBuffer* frame,void* image);
	virtual void extractYpCbCr420(const FrameBuffer* frame,void* yp,unsigned int ypStride,void* cb,unsigned int cbStride,void* cr,unsigned int crStride);
	};

}

#endif
