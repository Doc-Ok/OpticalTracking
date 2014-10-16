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

#include <Video/OggStream.h>

#include <Misc/ThrowStdErr.h>
#include <Video/OggPage.h>

namespace Video {

/**************************
Methods of class OggStream:
**************************/

OggStream::OggStream(int serialNumber)
	{
	/* Initialize the ogg_stream_state: */
	if(ogg_stream_init(this,serialNumber)!=0)
		Misc::throwStdErr("Video::OggStream::OggStream: Error in ogg_stream_init");
	}

OggStream::~OggStream(void)
	{
	/* Clear the ogg_stream_state: */
	ogg_stream_clear(this);
	}

void OggStream::packetIn(ogg_packet& packet)
	{
	/* Append the Ogg packet to the Ogg stream: */
	if(ogg_stream_packetin(this,&packet)!=0)
		Misc::throwStdErr("Video::OggStream::packetIn: Error in ogg_stream_packetin");
	}

bool OggStream::flush(OggPage& page)
	{
	/* Forcefully retrieve a page from the Ogg stream: */
	return ogg_stream_flush(this,&page)!=0;
	}

bool OggStream::pageOut(OggPage& page)
	{
	/* Retrieve a page from the Ogg stream: */
	return ogg_stream_pageout(this,&page)!=0;
	}

void OggStream::pageIn(OggPage& page)
	{
	/* Append the Ogg page to the Ogg stream: */
	if(ogg_stream_pagein(this,&page)!=0)
		Misc::throwStdErr("Video::OggStream::pageIn: Error in ogg_stream_pagein");
	}

}
