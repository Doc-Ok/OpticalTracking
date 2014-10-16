/***********************************************************************
TheoraInfo - Wrapper class for th_info structure from Theora v1.1 API.
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

#include <Video/TheoraInfo.h>

namespace Video {

/***************************
Methods of class TheoraInfo:
***************************/

TheoraInfo::TheoraInfo(void)
	{
	th_info_init(this);
	}

TheoraInfo::~TheoraInfo(void)
	{
	th_info_clear(this);
	}

void TheoraInfo::setImageSize(const unsigned int imageSize[2])
	{
	/* Pad the image width and height to multiples of 16: */
	frame_width=(imageSize[0]+0xfU)&~0xfU;
	pic_width=imageSize[0];
	pic_x=((frame_width-pic_width)/2U)&~0x1U; // Use only even offsets to prevent chroma shifts
	frame_height=(imageSize[1]+0xfU)&~0xfU;
	pic_height=imageSize[1];
	pic_y=((frame_height-pic_height)/2U)&~0x1U; // Use only even offsets to prevent chroma shifts
	}

void TheoraInfo::setQuality(int newQuality)
	{
	/* Limit the quality to the legal range: */
	if(newQuality<0)
		newQuality=0;
	if(newQuality>63)
		newQuality=63;
	quality=newQuality;
	}

void TheoraInfo::setGopSize(int newGopSize)
	{
	/* Find the keyframe granule shift that results in a gop size not smaller than the requested value: */
	for(keyframe_granule_shift=0;(1<<keyframe_granule_shift)<newGopSize;++keyframe_granule_shift)
		;
	}

}
