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

#include <Video/TheoraPacket.h>

#include <string.h>

namespace Video {

/*****************************
Methods of class TheoraPacket:
*****************************/

TheoraPacket& TheoraPacket::operator=(const ogg_packet& source)
	{
	/* Check for aliasing: */
	if(this!=&source)
		{
		/* Copy the packet's flags: */
		b_o_s=source.b_o_s;
		e_o_s=source.e_o_s;
		
		/* Copy the packet's stream position: */
		granulepos=source.granulepos;
		packetno=source.packetno;
		
		/* Copy the packet's size: */
		bytes=source.bytes;
		
		/* Check if the packet needs to allocate a private buffer: */
		if(allocSize<size_t(bytes))
			{
			/* Re-allocate the packet buffer: */
			if(allocSize>0)
				delete[] packet;
			allocSize=source.bytes;
			packet=new unsigned char[allocSize];
			}
		
		/* Copy the packet's data: */
		memcpy(packet,source.packet,bytes);
		}
	
	return *this;
	}

}
