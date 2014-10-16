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

#include <Video/VideoDataFormat.h>

namespace Video {

/********************************
Methods of class VideoDataFormat:
********************************/

void VideoDataFormat::setPixelFormat(const char* fourCC)
	{
	pixelFormat=0U;
	for(int i=0;i<4;++i)
		pixelFormat=(pixelFormat<<8)|(unsigned int)fourCC[3-i];
	}

bool VideoDataFormat::isPixelFormat(const char* fourCC) const
	{
	unsigned int queryPixelFormat=0U;
	for(int i=0;i<4;++i)
		queryPixelFormat=(queryPixelFormat<<8)|(unsigned int)fourCC[3-i];
	
	return pixelFormat==queryPixelFormat;
	}

char* VideoDataFormat::getFourCC(char fourCCBuffer[5]) const
	{
	unsigned int pf=pixelFormat;
	for(int i=0;i<4;++i,pf>>=8)
		fourCCBuffer[i]=char(pf&0xff);
	fourCCBuffer[4]='\0';
	return fourCCBuffer;
	}

}
