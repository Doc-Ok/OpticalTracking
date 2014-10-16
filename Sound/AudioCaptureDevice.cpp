/***********************************************************************
AudioCaptureDevice - Base class for audio capture devices.
Copyright (c) 2010-2011 Oliver Kreylos

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

#include <Sound/AudioCaptureDevice.h>

#include <Misc/FunctionCalls.h>
#include <Sound/Config.h>
#if SOUND_CONFIG_HAVE_ALSA
#include <Sound/Linux/ALSAAudioCaptureDevice.h>
#endif

namespace Sound {

/***********************************
Methods of class AudioCaptureDevice:
***********************************/

AudioCaptureDevice::DeviceList AudioCaptureDevice::getDevices(void)
	{
	/* Create the result list: */
	DeviceList result;
	
	/* Add IDs for all existing devices of all supported device types: */
	#if SOUND_CONFIG_HAVE_ALSA
	ALSAAudioCaptureDevice::addDevices(result);
	#endif
	
	return result;
	}

AudioCaptureDevice::AudioCaptureDevice(void)
	:streamingCallback(0),streaming(false)
	{
	}

AudioCaptureDevice::~AudioCaptureDevice(void)
	{
	delete streamingCallback;
	}

void AudioCaptureDevice::startStreaming(void)
	{
	delete streamingCallback;
	streamingCallback=0;
	streaming=true;
	}

void AudioCaptureDevice::startStreaming(AudioCaptureDevice::StreamingCallback* newStreamingCallback)
	{
	delete streamingCallback;
	streamingCallback=newStreamingCallback;
	streaming=true;
	}

void AudioCaptureDevice::stopStreaming(void)
	{
	delete streamingCallback;
	streamingCallback=0;
	streaming=false;
	}

}
