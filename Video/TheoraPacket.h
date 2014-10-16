/***********************************************************************
TheoraPacket - Wrapper class for Ogg packets containing Theora video
streams.
Copyright (c) 2010-2014 Oliver Kreylos

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

#ifndef VIDEO_THEORAPACKET_INCLUDED
#define VIDEO_THEORAPACKET_INCLUDED

#include <ogg/ogg.h>
#include <theora/codec.h>

namespace Video {

class TheoraPacket:public ogg_packet
	{
	/* Elements: */
	private:
	size_t allocSize; // If !=0, size of allocated private packet buffer
	
	/* Constructors and destructors: */
	public:
	TheoraPacket(void) // Creates an empty Ogg/Theora packet
		:allocSize(0)
		{
		packet=0;
		bytes=0;
		}
	private:
	TheoraPacket(const TheoraPacket& source); // Prohibit copy constructor
	public:
	TheoraPacket& operator=(const ogg_packet& source); // Clones another Ogg packet (avoid if possible)
	~TheoraPacket(void) // Destroys the Ogg/Theora packet
		{
		if(allocSize>0)
			delete[] packet;
		}
	
	/* Methods: */
	bool isHeader(void) // Returns true if the packet is a Theora header packet
		{
		return th_packet_isheader(this);
		}
	bool isKeyframe(void) // Returns true if the packet is a Theora keyframe packet
		{
		return th_packet_iskeyframe(this)>0;
		}
	size_t getWireSize(void) const // Returns the marshalled size of the Theora packet
		{
		return sizeof(char)+2*sizeof(ogg_int64_t)+sizeof(unsigned int)+bytes*sizeof(unsigned char);
		}
	template <class PipeParam>
	void read(PipeParam& pipe) // Reads a packet from a pipe
		{
		/* Read the packet flags: */
		b_o_s=pipe.template read<char>();
		e_o_s=0;
		
		/* Read the packet stream position: */
		granulepos=pipe.template read<ogg_int64_t>();
		packetno=pipe.template read<ogg_int64_t>();
		
		/* Read the packet size: */
		bytes=pipe.template read<unsigned int>();
		
		/* Check if the packet needs to allocate a private buffer: */
		if(allocSize<size_t(bytes))
			{
			/* Re-allocate the packet buffer: */
			if(allocSize>0)
				delete[] packet;
			allocSize=bytes;
			packet=new unsigned char[allocSize];
			}
		
		/* Read the packet data: */
		pipe.template read<unsigned char>(packet,bytes);
		}
	template <class PipeParam>
	void write(PipeParam& pipe) const
		{
		pipe.template write<char>(b_o_s);
		pipe.template write<ogg_int64_t>(granulepos);
		pipe.template write<ogg_int64_t>(packetno);
		pipe.template write<unsigned int>(bytes);
		pipe.template write<unsigned char>(packet,bytes);
		}
	};

}

#endif
