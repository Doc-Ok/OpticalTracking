/***********************************************************************
TheoraEncoder - Wrapper class for th_enc_ctx structure handle.
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

#ifndef VIDEO_THEORAENCODER_INCLUDED
#define VIDEO_THEORAENCODER_INCLUDED

#include <theora/theoraenc.h>
#include <Video/TheoraPacket.h>

/* Forward declarations: */
namespace Video {
class OggStream;
class TheoraInfo;
class TheoraComment;
class TheoraFrame;
}

namespace Video {

class TheoraEncoder
	{
	/* Elements: */
	private:
	th_enc_ctx* encoder; // Pointer to the Theora encoder context structure
	
	/* Constructors and destructors: */
	public:
	TheoraEncoder(void) // Creates an uninitialized encoder
		:encoder(0)
		{
		}
	private:
	TheoraEncoder(const TheoraEncoder& source); // Prohibit copy constructor
	TheoraEncoder& operator=(const TheoraEncoder& source); // Prohibit assignment operator
	public:
	~TheoraEncoder(void); // Destroys the encoder
	
	/* Methods: */
	void init(const TheoraInfo& info); // Initializes the encoder with the given information structure
	void release(void); // Destroys the encoder
	bool isValid(void) const // Returns true if the encoder has been successfully initialized
		{
		return encoder!=0;
		}
	int control(int control,void* parameters,size_t parametersSize); // Supplies parameters to or reads from an encoder control
	int getMaxSpeedLevel(void); // Returns the encoder's maximum speed level for current encoder settings
	void setSpeedLevel(int speedLevel); // Sets the encoder's speed level
	bool emitHeader(TheoraComment& comments,TheoraPacket& packet); // Emits a header packet into the given packet buffer; returns false if no packet was written and headers are complete
	void writeHeaders(TheoraComment& comments,OggStream& oggStream); // Writes all header packets to the given Ogg stream
	template <class PipeParam>
	void writeHeaders(TheoraComment& comments,PipeParam& pipe) // Writes all header packets to the given pipe
		{
		/* Write header packets until done: */
		TheoraPacket packet;
		while(emitHeader(comments,packet))
			{
			/* Write the packet to the pipe: */
			packet.write(pipe);
			}
		}
	void encodeFrame(TheoraFrame& frame); // Encodes a video frame
	bool emitPacket(TheoraPacket& packet); // Emits a data packet into the given packet buffer; returns false if no packet was written and data is complete
	void writePackets(OggStream& oggStream); // Writes all data packets to the given Ogg stream
	template <class PipeParam>
	void writePackets(PipeParam& pipe) // Writes all data packets to the given pipe
		{
		/* Write data packets until done: */
		TheoraPacket packet;
		while(emitPacket(packet))
			{
			/* Write the packet to the pipe: */
			packet.write(pipe);
			}
		}
	};

}

#endif
