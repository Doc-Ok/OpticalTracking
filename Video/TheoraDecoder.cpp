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

#include <Video/TheoraDecoder.h>

#include <Misc/ThrowStdErr.h>
#include <Video/TheoraInfo.h>
#include <Video/TheoraComment.h>
#include <Video/TheoraPacket.h>
#include <Video/TheoraFrame.h>

namespace Video {

/*************************************
Methods of class TheoraDecoder::Setup:
*************************************/

TheoraDecoder::Setup::~Setup(void)
	{
	th_setup_free(setup);
	}

/******************************
Methods of class TheoraDecoder:
******************************/

TheoraDecoder::TheoraDecoder(void)
	:decoder(0),frameReady(false)
	{
	}

TheoraDecoder::~TheoraDecoder(void)
	{
	if(decoder!=0)
		th_decode_free(decoder);
	}

bool TheoraDecoder::processHeader(TheoraPacket& packet,TheoraInfo& info,TheoraComment& comments,TheoraDecoder::Setup& setup)
	{
	int result=th_decode_headerin(&info,&comments,&setup.setup,&packet);
	if(result<0)
		Misc::throwStdErr("Video::TheoraDecoder::processHeader: Invalid header packet");
	
	return result>0;
	}

void TheoraDecoder::init(const TheoraInfo& info,const TheoraDecoder::Setup& setup)
	{
	/* Destroy the current decoder context if it exists: */
	if(decoder!=0)
		th_decode_free(decoder);
	
	/* Allocate a new decoder context: */
	decoder=th_decode_alloc(&info,setup.setup);
	if(decoder==0)
		Misc::throwStdErr("Video::TheoraDecoder::init: Invalid decoding parameters");
	
	/* Reset the decoder's frame state: */
	frameReady=false;
	}

void TheoraDecoder::release(void)
	{
	/* Destroy the decoder context if it exists: */
	if(decoder!=0)
		th_decode_free(decoder);
	decoder=0;
	frameReady=false;
	}

int TheoraDecoder::control(int control,void* parameters,size_t parametersSize)
	{
	return th_decode_ctl(decoder,control,parameters,parametersSize);
	}

ogg_int64_t TheoraDecoder::processPacket(const TheoraPacket& packet)
	{
	ogg_int64_t granulePos;
	int result=th_decode_packetin(decoder,&packet,&granulePos);
	frameReady=result==0;
	if(result<0&&result!=TH_DUPFRAME)
		Misc::throwStdErr("Video::TheoraDecoder::processPacket: Invalid data packet");
	
	return granulePos;
	}

void TheoraDecoder::decodeFrame(TheoraFrame& frame)
	{
	/* Copy the decoded frame into the frame buffer: */
	th_decode_ycbcr_out(decoder,frame.planes);
	
	/* Reset the ready flag: */
	frameReady=false;
	}

}
