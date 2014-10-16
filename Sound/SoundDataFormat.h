/***********************************************************************
SoundDataFormat - System-independent data structure to describe the
format of sound data.
Copyright (c) 2008-2010 Oliver Kreylos

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

#ifndef SOUND_SOUNDDATAFORMAT_INCLUDED
#define SOUND_SOUNDDATAFORMAT_INCLUDED

#include <Sound/Config.h>

#if SOUND_CONFIG_HAVE_ALSA
#include <alsa/asoundlib.h>
#endif

namespace Sound {

struct SoundDataFormat
	{
	/* Embedded classes: */
	public:
	enum Endianness // Enumerated type for sound data endianness
		{
		DontCare,LittleEndian,BigEndian
		};
	
	/* Elements: */
	int bitsPerSample; // Number of bits per sample; usually a multiple of 8
	int bytesPerSample; // Number of bytes per sample; some formats use internal padding
	bool signedSamples; // Flag whether to use signed or unsigned samples
	Endianness sampleEndianness; // Endianness of the samples
	int samplesPerFrame; // Number of samples per frame; in other words, number of channels (1: mono, 2: stereo, etc.)
	int framesPerSecond; // Number of frames to sample per second; in other words, sample rate in Hertz
	
	/* Constructors and destructors: */
	SoundDataFormat(void) // Creates a default sound data format: 8bit unsigned samples, 8kHz mono
		:bitsPerSample(8),bytesPerSample(1),signedSamples(false),sampleEndianness(DontCare),
		 samplesPerFrame(1),framesPerSecond(8000)
		{
		}
	
	/* Methods: */
	void setStandardSampleFormat(int newBitsPerSample,bool newSignedSamples,Endianness newSampleEndianness =DontCare); // Sets a standard (sanitized) sample format
	#if SOUND_CONFIG_HAVE_ALSA
	snd_pcm_format_t getPCMFormat(void) const; // Constructs an ALSA PCM format from a sound data format structure
	#endif
	};

}

#endif
