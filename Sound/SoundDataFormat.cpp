/***********************************************************************
SoundDataFormat - System-independent data structure to describe the
format of sound data.
Copyright (c) 2008-2009 Oliver Kreylos

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

#include <Sound/SoundDataFormat.h>

#include <Sound/Config.h>

namespace Sound {

/********************************
Methods of class SoundDataFormat:
********************************/

void SoundDataFormat::setStandardSampleFormat(int newBitsPerSample,bool newSignedSamples,SoundDataFormat::Endianness newSampleEndianness)
	{
	/* Update and sanitize the sample format: */
	bitsPerSample=newBitsPerSample;
	
	/* Limit sample resolution to standard range: */
	if(bitsPerSample<1)
		bitsPerSample=1;
	else if(bitsPerSample>32)
		bitsPerSample=32;
	
	/* Quantize sample resolution to multiples of 8: */
	bitsPerSample=(bitsPerSample+7)&~0x07;
	
	/* Calculate number of bytes per sample: */
	if(bitsPerSample==24)
		bytesPerSample=4; // 24 bit sound data padded into 32 bit words
	else
		bytesPerSample=bitsPerSample/8;
	
	/* Assign the signed flag: */
	signedSamples=newSignedSamples;
	
	/* Set the sample endianness: */
	sampleEndianness=bitsPerSample>8?newSampleEndianness:DontCare;
	}

#if SOUND_CONFIG_HAVE_ALSA

snd_pcm_format_t SoundDataFormat::getPCMFormat(void) const
	{
	snd_pcm_format_t pcmFormat;
	if(bitsPerSample==8)
		pcmFormat=signedSamples?SND_PCM_FORMAT_S8:SND_PCM_FORMAT_U8;
	else if(bitsPerSample==16)
		{
		if(sampleEndianness==SoundDataFormat::LittleEndian)
			pcmFormat=signedSamples?SND_PCM_FORMAT_S16_LE:SND_PCM_FORMAT_U16_LE;
		else if(sampleEndianness==SoundDataFormat::BigEndian)
			pcmFormat=signedSamples?SND_PCM_FORMAT_S16_BE:SND_PCM_FORMAT_U16_BE;
		else
			pcmFormat=signedSamples?SND_PCM_FORMAT_S16:SND_PCM_FORMAT_U16;
		}
	else if(bitsPerSample==24)
		{
		if(sampleEndianness==SoundDataFormat::LittleEndian)
			pcmFormat=signedSamples?SND_PCM_FORMAT_S24_LE:SND_PCM_FORMAT_U24_LE;
		else if(sampleEndianness==SoundDataFormat::BigEndian)
			pcmFormat=signedSamples?SND_PCM_FORMAT_S24_BE:SND_PCM_FORMAT_U24_BE;
		else
			pcmFormat=signedSamples?SND_PCM_FORMAT_S24:SND_PCM_FORMAT_U24;
		}
	else if(bitsPerSample==32)
		{
		if(sampleEndianness==SoundDataFormat::LittleEndian)
			pcmFormat=signedSamples?SND_PCM_FORMAT_S32_LE:SND_PCM_FORMAT_U32_LE;
		else if(sampleEndianness==SoundDataFormat::BigEndian)
			pcmFormat=signedSamples?SND_PCM_FORMAT_S32_BE:SND_PCM_FORMAT_U32_BE;
		else
			pcmFormat=signedSamples?SND_PCM_FORMAT_S32:SND_PCM_FORMAT_U32;
		}
	else
		pcmFormat=SND_PCM_FORMAT_UNKNOWN;
	
	return pcmFormat;
	}

#endif

}
