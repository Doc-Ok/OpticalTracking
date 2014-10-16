/***********************************************************************
BayerPattern - Enumerated type for Bayer color filter pattern types.
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

#ifndef VIDEO_BAYERPATTERN_INCLUDED
#define VIDEO_BAYERPATTERN_INCLUDED

namespace Video {

enum BayerPattern
	{
	BAYER_INVALID,BAYER_RGGB,BAYER_GBRG,BAYER_GRBG,BAYER_BGGR
	};

}

#endif
