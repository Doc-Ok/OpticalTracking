/***********************************************************************
VideoDataFormat - System-independent data structure to describe the
format of video data.
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

#ifndef VIDEO_VIDEODATAFORMAT_INCLUDED
#define VIDEO_VIDEODATAFORMAT_INCLUDED

#include <stddef.h>

namespace Video {

struct VideoDataFormat // Structure to report current or available video formats
	{
	/* Elements: */
	public:
	unsigned int pixelFormat; // Pixel format identifier (a fourCC value)
	unsigned int size[2]; // Width and height of video frames in pixels
	size_t lineSize; // Size of a single line of video in bytes (undefined for compressed formats)
	size_t frameSize; // Size of an entire video frame in bytes (maximum value for compressed formats)
	unsigned int frameIntervalCounter,frameIntervalDenominator; // (Expected) video frame interval (inverse rate) as rational number
	
	/* Methods: */
	void setPixelFormat(const char* fourCC); // Sets the format's pixel format to the given fourCC code
	bool isPixelFormat(const char* fourCC) const; // Compares the video format's pixel format against the given fourCC code
	char* getFourCC(char fourCCBuffer[5]) const; // Writes format's pixel format into the given buffer as a NUL-terminated fourCC code
	};

}

#endif
