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

#ifndef VIDEO_THEORAINFO_INCLUDED
#define VIDEO_THEORAINFO_INCLUDED

#include <theora/codec.h>

namespace Video {

class TheoraInfo:public th_info
	{
	/* Constructors and destructors: */
	public:
	TheoraInfo(void); // Creates a default Theora information structure
	private:
	TheoraInfo(const TheoraInfo& source); // Prohibit copy constructor
	TheoraInfo& operator=(const TheoraInfo& source); // Prohibit assignment operator
	public:
	~TheoraInfo(void); // Destroys the Theora information structure
	
	/* Methods: */
	int getGopSize(void) const // Returns the group-of-pictures size / keyframe distance
		{
		return 1<<keyframe_granule_shift;
		}
	void setImageSize(const unsigned int imageSize[2]); // Sets the size of the encoded/decoded frames; calculates appropriate padding
	void setQuality(int newQuality); // Sets the encoder's encoding quality from 0 (low) to 63 (high)
	void setGopSize(int newGopSize); // Sets the group-of-pictures size / keyframe distance for encoding
	};

}

#endif
