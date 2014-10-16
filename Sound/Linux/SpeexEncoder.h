/***********************************************************************
SpeexEncoder - Class encapsulating an audio encoder using the SPEEX
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

#ifndef SOUND_LINUX_SPEEXENCODER_INCLUDED
#define SOUND_LINUX_SPEEXENCODER_INCLUDED

#include <Threads/Thread.h>
#include <Threads/DropoutBuffer.h>
#include <Sound/Linux/ALSAPCMDevice.h>
#include <speex/speex.h>

namespace Sound {

class SpeexEncoder:public ALSAPCMDevice
	{
	/* Elements: */
	private:
	void* speexState; // The SPEEX speech encoding object
	size_t speexFrameSize; // Number of audio frames required to encode a SPEEX audio packet
	signed short int* recordingBuffer; // Buffer to read uncompressed audio from the recording PCM device
	SpeexBits speexBits; // SPEEX bit packing structure
	size_t speexPacketSize; // Size of encoded SPEEX audio packet in bytes
	Threads::DropoutBuffer<char> speexPacketQueue; // Queue of encoded SPEEX audio packets ready for pickup
	Threads::Thread encodingThread; // The encoding thread
	
	/* Private methods: */
	void* encodingThreadMethod(void); // The encoding thread method
	
	/* Constructors and destructors: */
	public:
	SpeexEncoder(const char* recordingPCMDeviceName,size_t sPacketQueueSize); // Creates a SPEEX encoder using the ALSA PCM device of the given name and the given delivery queue size
	~SpeexEncoder(void); // Destroys the SPEEX encoder
	
	/* Methods: */
	size_t getFrameSize(void) const // Returns number of audio frames in an encoded SPEEX packet
		{
		return speexFrameSize;
		}
	Threads::DropoutBuffer<char>& getPacketQueue(void) // Returns the packet queue to retrieve encoded SPEEX packets
		{
		return speexPacketQueue;
		} 
	};

}

#endif
