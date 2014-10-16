/***********************************************************************
TheoraDecoder - Wrapper class for th_dec_ctx structure handle.
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

#ifndef VIDEO_THEORADECODER_INCLUDED
#define VIDEO_THEORADECODER_INCLUDED

#include <theora/theoradec.h>

/* Forward declarations: */
namespace Video {
class TheoraInfo;
class TheoraComment;
class TheoraPacket;
class TheoraFrame;
}

namespace Video {

class TheoraDecoder
	{
	/* Embedded classes: */
	public:
	class Setup // Wrapper for Theora decoder setup structure
		{
		friend class TheoraDecoder;
		
		/* Elements: */
		private:
		th_setup_info* setup; // Pointer to the setup structure
		
		/* Constructors and destructors: */
		public:
		Setup(void) // Creates an uninitialized setup structure
			:setup(0)
			{
			}
		private:
		Setup(const Setup& source); // Prohibit copy constructor
		Setup& operator=(const Setup& source); // Prohibit assignment operator
		public:
		~Setup(void); // Destroys the setup structure
		};
	
	/* Elements: */
	private:
	th_dec_ctx* decoder; // Pointer to the Theora decoder context structure
	bool frameReady; // Flag whether the decoder has a decoded frame ready
	
	/* Constructors and destructors: */
	public:
	TheoraDecoder(void); // Creates an uninitialized decoder
	private:
	TheoraDecoder(const TheoraDecoder& source); // Prohibit copy constructor
	TheoraDecoder& operator=(const TheoraDecoder& source); // Prohibit assignment operator
	public:
	~TheoraDecoder(void); // Destroys the decoder
	
	/* Methods: */
	static bool processHeader(TheoraPacket& packet,TheoraInfo& info,TheoraComment& comments,Setup& setup); // Processes a header packet; returns false if the packet was the first data packet
	void init(const TheoraInfo& info,const Setup& setup); // Initializes the decoder with the given information and setup structures
	void release(void); // Destroys the decoder
	bool isValid(void) const // Returns true if the decoder has been successfully initialized
		{
		return decoder!=0;
		}
	int control(int control,void* parameters,size_t parametersSize); // Supplies parameters to a decoder control
	ogg_int64_t processPacket(const TheoraPacket& packet); // Processes the given data packet
	bool isFrameReady(void) const // Returns true if the decoder has a frame ready to decode
		{
		return frameReady;
		}
	void decodeFrame(TheoraFrame& frame); // Decodes a video frame
	};

}

#endif
