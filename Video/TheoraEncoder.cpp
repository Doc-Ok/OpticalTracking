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

#include <Video/TheoraEncoder.h>

#include <Misc/ThrowStdErr.h>
#include <Video/OggStream.h>
#include <Video/TheoraInfo.h>
#include <Video/TheoraComment.h>
#include <Video/TheoraFrame.h>

namespace Video {

/******************************
Methods of class TheoraEncoder:
******************************/

TheoraEncoder::~TheoraEncoder(void)
	{
	/* Destroy the encoder context if it exists: */
	if(encoder!=0)
		th_encode_free(encoder);
	}

void TheoraEncoder::init(const TheoraInfo& info)
	{
	/* Destroy the current encoder context if it exists: */
	if(encoder!=0)
		th_encode_free(encoder);
	
	/* Allocate a new encoder context: */
	encoder=th_encode_alloc(&info);
	if(encoder==0)
		Misc::throwStdErr("Video::TheoraEncoder::init: Invalid encoding parameters");
	}

void TheoraEncoder::release(void)
	{
	/* Destroy the encoder context if it exists: */
	if(encoder!=0)
		th_encode_free(encoder);
	encoder=0;
	}

int TheoraEncoder::control(int control,void* parameters,size_t parametersSize)
	{
	return th_encode_ctl(encoder,control,parameters,parametersSize);
	}

int TheoraEncoder::getMaxSpeedLevel(void)
	{
	int result;
	if(th_encode_ctl(encoder,TH_ENCCTL_GET_SPLEVEL_MAX,&result,sizeof(int))<0)
		Misc::throwStdErr("Video::TheoraEncoder::getMaxSpeedLevel: Internal error");
	return result;
	}

void TheoraEncoder::setSpeedLevel(int newSpeedLevel)
	{
	if(th_encode_ctl(encoder,TH_ENCCTL_SET_SPLEVEL,&newSpeedLevel,sizeof(int))<0)
		Misc::throwStdErr("Video::TheoraEncoder::setSpeedLevel: Internal error");
	}

bool TheoraEncoder::emitHeader(TheoraComment& comments,TheoraPacket& packet)
	{
	return th_encode_flushheader(encoder,&comments,&packet)>0;
	}

void TheoraEncoder::writeHeaders(TheoraComment& comments,OggStream& oggStream)
	{
	/* Write header packets until done: */
	TheoraPacket packet;
	while(th_encode_flushheader(encoder,&comments,&packet)>0)
		{
		/* Append the packet to the Ogg stream: */
		oggStream.packetIn(packet);
		}
	}

void TheoraEncoder::encodeFrame(TheoraFrame& frame)
	{
	if(th_encode_ycbcr_in(encoder,frame.planes))
		Misc::throwStdErr("Video::TheoraEncoder::encodeFrame: Encoding error");
	}

bool TheoraEncoder::emitPacket(TheoraPacket& packet)
	{
	return th_encode_packetout(encoder,0,&packet)>0;
	}

void TheoraEncoder::writePackets(OggStream& oggStream)
	{
	/* Write data packets until done: */
	TheoraPacket packet;
	while(th_encode_packetout(encoder,0,&packet)>0)
		{
		/* Append the packet to the Ogg stream: */
		oggStream.packetIn(packet);
		}
	}

}
