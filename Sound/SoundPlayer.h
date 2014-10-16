/***********************************************************************
SoundPlayer - Simple class to play sound from a sound file on the local
file system to a playback device. Uses ALSA under Linux, and the Core
Audio frameworks under Mac OS X.
Copyright (c) 2008-2011 Oliver Kreylos

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

#ifndef SOUND_SOUNDPLAYER_INCLUDED
#define SOUND_SOUNDPLAYER_INCLUDED

#include <Sound/Config.h>

#include <Threads/MutexCond.h>
#if SOUND_CONFIG_HAVE_ALSA
#include <IO/SeekableFile.h>
#include <Threads/Thread.h>
#endif
#ifdef __APPLE__
#include <CoreAudio/CoreAudioTypes.h>
#include <AudioToolbox/AudioQueue.h>
#include <AudioToolbox/AudioConverter.h>
#include <AudioToolbox/AudioFile.h>
#endif

#if SOUND_CONFIG_HAVE_ALSA
#include <Sound/Linux/ALSAPCMDevice.h>
#endif
#include <Sound/SoundDataFormat.h>

namespace Sound {

class SoundPlayer
	{
	/* Elements: */
	private:
	#ifdef __APPLE__
	AudioFileID inputFile; // Handle of the audio file
	AudioStreamBasicDescription format; // Audio data format description
	AudioQueueRef queue; // Handle of the audio playback queue
	UInt32 bufferSize; // Buffer size in bytes
	UInt32 numPacketsPerBuffer; // Number of packets that fit into a single audio buffer
	AudioQueueBufferRef buffers[2]; // Array of audio buffers
	AudioStreamPacketDescription* packetDescriptors; // Array of packet descriptors for the packets in one buffer, or 0 for CBR formats
	SInt64 numPlayedPackets; // Total number of packets read from the output file
	#else
	#if SOUND_CONFIG_HAVE_ALSA
	IO::SeekableFilePtr inputFile; // File from which to read the sound data
	#endif
	SoundDataFormat format; // Format of the sound data in the input file
	#if SOUND_CONFIG_HAVE_ALSA
	size_t bytesPerFrame; // Number of bytes per frame of sound data
	ALSAPCMDevice pcmDevice; // Pointer to the ALSA PCM device used for playback
	size_t sampleBufferSize; // Size of the sample buffer in bytes
	char* sampleBuffer; // A buffer to write sound data to the PCM device
	Threads::Thread playingThread; // Thread ID of the background playing thread
	#endif
	#endif
	bool active; // Flag whether the sound player is currently playing
	Threads::MutexCond finishedPlayingCond; // Signal to notify the main thread when playing thread is finished
	
	/* Private methods: */
	#if SOUND_CONFIG_HAVE_ALSA
	bool readWAVHeader(void); // Reads a WAV file's header and extracts the sound data format; returns true if input file is compatible WAV file
	void* playingThreadMethod(void); // The background playing thread's method
	#endif
	#ifdef __APPLE__
	static void handleOutputBufferWrapper(void* aqData,AudioQueueRef inAQ,AudioQueueBufferRef inBuffer)
		{
		/* Get a pointer to the SoundPlayer structure: */
		SoundPlayer* thisPtr=static_cast<SoundPlayer*>(aqData);
		
		/* Call the actual handler method: */
		thisPtr->handleOutputBuffer(inAQ,inBuffer);
		}
	void handleOutputBuffer(AudioQueueRef inAQ,AudioQueueBufferRef inBuffer);
	#endif
	
	/* Constructors and destructors: */
	public:
	SoundPlayer(const char* inputFileName); // Creates a sound player for the given input file
	private:
	SoundPlayer(const SoundPlayer& source); // Prohibit copy constructor
	SoundPlayer& operator=(const SoundPlayer& source); // Prohibit assignment operator
	public:
	~SoundPlayer(void); // Destroys the sound player
	
	/* Methods: */
	SoundDataFormat getSoundDataFormat(void) const; // Returns the input file's sound data format
	void start(void); // Starts playback from the input file
	void stop(void); // Stops playback from the input file
	bool isPlaying(void) const // Returns true if the sound player is currently playing
		{
		return active;
		}
	void wait(void); // Blocks the caller until the player has finished playing
	};

}

#endif
