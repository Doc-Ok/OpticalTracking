/***********************************************************************
TheoraFrame - Wrapper class for th_img_plane structure.
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

#ifndef VIDEO_THEORAFRAME_INCLUDED
#define VIDEO_THEORAFRAME_INCLUDED

#include <theora/codec.h>

/* Forward declarations: */
namespace Video {
class TheoraInfo;
}

namespace Video {

class TheoraFrame
	{
	/* Elements: */
	public:
	bool privateData; // Flag whether the image planes were allocated by the frame itself
	th_img_plane planes[3]; // Y', Cb, and Cr image plane descriptors
	int offsets[3]; // Offsets from source frame to padded Theora frame for the three image planes
	
	/* Constructors and destructors: */
	public:
	TheoraFrame(void); // Creates uninitialized Theora frame
	private:
	TheoraFrame(const TheoraFrame& source); // Prohibit copy constructor
	TheoraFrame& operator=(const TheoraFrame& source); // Prohibit assignment operator
	public:
	~TheoraFrame(void); // Destroys the Theora frame
	
	/* Methods: */
	void copy(const TheoraFrame& source); // Copies the given frame into this frame; assumes that frame layout matches
	void init420(const TheoraInfo& info); // Initializes a frame for the given stream format with 4:2:0 pixel format (chroma planes half width and height)
	void init422(const TheoraInfo& info); // Initializes a frame for the given stream format with 4:2:2 pixel format (chroma planes half width)
	void init444(const TheoraInfo& info); // Initializes a frame for the given stream format with 4:4:4 pixel format (chroma planes full size)
	void release(void); // Releases privately allocated image planes
	};

}

#endif
