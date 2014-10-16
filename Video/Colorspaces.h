/***********************************************************************
Colorspaces - Helper functions to convert color values between several
common color spaces.
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

#ifndef VIDEO_COLORSPACES_INCLUDED
#define VIDEO_COLORSPACES_INCLUDED

namespace Video {

namespace {

inline unsigned char clampFixed16(int fixed16)
	{
	if(fixed16<32768)
		return 0;
	else if(fixed16>=16678912)
		return 255;
	else
		return (unsigned char)((fixed16+32768)>>16);
	}

}

inline void rgbToYpcbcr(const unsigned char rgb[3],unsigned char ypcbcr[3])
	{
	/* Convert RGB to YpCbCr directly using 16-bit fixed-comma arithmetic: */
	ypcbcr[0]=clampFixed16(1048576+int(rgb[0])*16829+int(rgb[1])*33039+int(rgb[2])*6416);
	ypcbcr[1]=clampFixed16(8388608-int(rgb[0])*9714-int(rgb[1])*19071+int(rgb[2])*28784);
	ypcbcr[2]=clampFixed16(8388608+int(rgb[0])*28784-int(rgb[1])*24103-int(rgb[2])*4681);
	}

inline void ypcbcrToRgb(const unsigned char ypcbcr[3],unsigned char rgb[3])
	{
	/* Convert Y'CbCr to YUV first: */
	int y=(int(ypcbcr[0])-16); // *255/219;
	int u=(int(ypcbcr[1])-128); // *255/224;
	int v=(int(ypcbcr[2])-128); // *255/224;
	
	/* Convert YUV to RGB, using 16-bit fixed-comma arithmetic: */
	rgb[0]=clampFixed16(y*76309+v*104597);
	rgb[1]=clampFixed16(y*76309-u*25675-v*53279);
	rgb[2]=clampFixed16(y*76309+u*132202);
	}

}

#endif
