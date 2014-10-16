/***********************************************************************
SpeexDecoder - Class encapsulating an audio decoder using the SPEEX
speech codec.
Copyright (c) 2009-2010 Oliver Kreylos

This file is part of the Basic Sound Library (Sound).

The Basic Sound Library is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as published
by the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

The Basic Sound Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Basic Sound Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <Sound/Linux/SpeexDecoder.h>

namespace Sound {

/*****************************
Methods of class SpeexDecoder:
*****************************/

void* SpeexDecoder::decodingThreadMethod(void)
	{
	Threads::Thread::setCancelState(Threads::Thread::CANCEL_ENABLE);
	// Threads::Thread::setCancelType(Threads::Thread::CANCEL_ASYNCHRONOUS);
	
	while(true)
		{
		/* Get an encoded SPEEX packet from the queue: */
		const char* speexPacket=speexPacketQueue.popSegment();
		
		/* Unpack the packet: */
		speex_bits_read_from(&speexBits,const_cast<char*>(speexPacket),speexPacketQueue.getSegmentSize()); // const_cast due to API failure!
		
		/* Decode the sound data: */
		signed short int* decodedSegment=decodedPacketQueue.getWriteSegment();
		if(speex_decode_int(speexState,&speexBits,decodedSegment)>=0)
			{
			/* Push the decoded packet to the output queue: */
			decodedPacketQueue.pushSegment();
			}
		
		/* Reset the encoder for the next packet: */
		speex_bits_reset(&speexBits);
		}
	
	return 0;
	}

SpeexDecoder::SpeexDecoder(size_t sSpeexFrameSize,Threads::DropoutBuffer<char>& sSpeexPacketQueue)
	:speexState(0),
	 speexPacketQueue(sSpeexPacketQueue),
	 speexFrameSize(sSpeexFrameSize),
	 decodedPacketQueue(speexFrameSize,speexPacketQueue.getMaxQueueSize())
	{
	bool speexBitsInitialized=false;
	try
		{
		/* Initialize the SPEEX decoder: */
		speexState=speex_decoder_init(&speex_wb_mode);
		spx_int32_t enhancement=0;
		speex_decoder_ctl(speexState,SPEEX_SET_ENH,&enhancement);
		spx_int32_t speexSamplingRate=16000;
		speex_decoder_ctl(speexState,SPEEX_SET_SAMPLING_RATE,&speexSamplingRate);
		
		/* Initialize the SPEEX bit unpacker: */
		speex_bits_init(&speexBits);
		speexBitsInitialized=true;
		
		/* Start the audio decoding thread: */
		decodingThread.start(this,&SpeexDecoder::decodingThreadMethod);
		}
	catch(...)
		{
		/* Clean up and re-throw: */
		if(speexBitsInitialized)
			speex_bits_destroy(&speexBits);
		if(speexState!=0)
			speex_decoder_destroy(speexState);
		throw;
		}
	}

SpeexDecoder::~SpeexDecoder(void)
	{
	/* Stop the audio decoding thread: */
	decodingThread.cancel();
	decodingThread.join();
	
	/* Release all allocated resources: */
	speex_bits_destroy(&speexBits);
	speex_decoder_destroy(speexState);
	}

}
