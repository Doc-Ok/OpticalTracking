/***********************************************************************
OggStream - Wrapper class for ogg_stream_state structure from Ogg API.
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

#ifndef VIDEO_OGGSTREAM_INCLUDED
#define VIDEO_OGGSTREAM_INCLUDED

#include <ogg/ogg.h>

/* Forward declarations: */
namespace Video {
class OggPage;
}

namespace Video {

class OggStream:public ogg_stream_state
	{
	/* Constructors and destructors: */
	public:
	OggStream(int serialNumber); // Creates a stream object with the given stream serial number
	~OggStream(void);
	
	/* Methods: */
	void packetIn(ogg_packet& packet); // Submits an Ogg packet to the Ogg stream during encoding
	bool pageOut(OggPage& page); // Requests a page of data from the Ogg stream; returns true if page structure was filled
	bool flush(OggPage& page); // Writes any remaining data in the Ogg stream into a page; returns true if any data was written
	
	void pageIn(OggPage& page); // Submits a page of data to the Ogg stream during decoding
	bool packetOut(ogg_packet& packet) // Requests an Ogg packet from the Ogg stream; returns true if packet structure was filled
		{
		/* Retrieve a packet from the ogg_stream_state: */
		return ogg_stream_packetout(this,&packet)==1;
		}
	bool isEos(void) const // Returns true if the end of the stream has been reached
		{
		return ogg_stream_eos(const_cast<OggStream*>(this))!=0;
		}
	};

}

#endif
