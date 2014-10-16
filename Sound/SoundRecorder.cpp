/***********************************************************************
SoundRecorder - Simple class to record sound from a capture device to a
sound file on the local file system. Uses ALSA under Linux, and the Core
Audio frameworks under Mac OS X.
Copyright (c) 2008-2014 Oliver Kreylos

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

#include <Sound/SoundRecorder.h>

#include <unistd.h>
#include <Misc/FileNameExtensions.h>
#include <IO/OpenFile.h>
#include <Sound/Config.h>

#ifdef __APPLE__
#include <math.h>
#endif
#include <string.h>
#include <iostream>
#include <stdexcept>
#include <Misc/ThrowStdErr.h>
#ifdef __APPLE__
#include <CoreFoundation/CFURL.h>
#endif

namespace Sound {

/******************************
Methods of class SoundRecorder:
******************************/

#ifdef __APPLE__

/*********************************
Mac OS X version of SoundRecorder:
*********************************/

void SoundRecorder::setAudioFileMagicCookie(void)
	{
	/* Query the size of the magic cookie: */
	UInt32 magicCookieSize;
	if(AudioQueueGetPropertySize(queue,kAudioQueueProperty_MagicCookie,&magicCookieSize)==noErr)
		{
		/* Allocate a buffer for the magic cookie: */
		char* magicCookie=new char[magicCookieSize];
		
		/* Copy the magic cookie from the audio queue into the audio file: */
		if(AudioQueueGetProperty(queue,kAudioQueueProperty_MagicCookie,magicCookie,&magicCookieSize)==noErr)
			AudioFileSetProperty(audioFile,kAudioFilePropertyMagicCookieData,magicCookieSize,magicCookie);
		
		/* Delete the cookie buffer: */
		delete[] magicCookie;
		}
	}

void SoundRecorder::handleInputBuffer(AudioQueueRef inAQ,AudioQueueBufferRef inBuffer,const AudioTimeStamp* inStartTime,UInt32 inNumPackets,const AudioStreamPacketDescription* inPacketDesc)
	{
	/* Calculate the number of packets in the buffer if not given: */
	if(inNumPackets==0&&format.mBytesPerPacket!=0)
		inNumPackets=inBuffer->mAudioDataByteSize/format.mBytesPerPacket;
	
	/* Write the just-filled buffer to the audio file: */
	if(AudioFileWritePackets(audioFile,false,inBuffer->mAudioDataByteSize,inPacketDesc,numRecordedPackets,&inNumPackets,inBuffer->mAudioData)==noErr)
		{
		/* Update the number of written packets: */
		numRecordedPackets+=inNumPackets;
		}
	
	/* Put the just-filled buffer back into the audio queue if we're still recording: */
	if(active)
		AudioQueueEnqueueBuffer(queue,inBuffer,0,0);
	}

void SoundRecorder::init(const char* audioSource,const SoundDataFormat& sFormat,const char* outputFileName)
	{
	/* Store and sanify the sound data format: */
	format.mSampleRate=double(sFormat.framesPerSecond);
	format.mFormatID=kAudioFormatLinearPCM;
	format.mFormatFlags=0x0;
	format.mBitsPerChannel=sFormat.bitsPerSample>8?(sFormat.bitsPerSample+7)&~0x7:8;
	format.mChannelsPerFrame=sFormat.samplesPerFrame>=1?sFormat.samplesPerFrame:1;
	format.mBytesPerFrame=format.mChannelsPerFrame*(format.mBitsPerChannel/8);
	format.mFramesPerPacket=1;
	format.mBytesPerPacket=format.mFramesPerPacket*format.mBytesPerFrame;
	
	/* Determine the output file format from the file name extension: */
	AudioFileTypeID audioFileType=kAudioFileWAVEType; // Not really a default; just to make compiler happy
	const char* ext=Misc::getExtension(outputFileName);
	if(*ext=='\0'||strcasecmp(ext,".aiff")==0)
		{
		/* Adjust the sound data format for AIFF files: */
		audioFileType=kAudioFileAIFFType;
		format.mFormatFlags=kLinearPCMFormatFlagIsBigEndian|kLinearPCMFormatFlagIsSignedInteger|kLinearPCMFormatFlagIsPacked;
		}
	else if(strcasecmp(ext,".wav")==0)
		{
		/* Adjust the sound data format for WAV files: */
		audioFileType=kAudioFileWAVEType;
		format.mFormatFlags=kLinearPCMFormatFlagIsPacked;
		if(format.mBitsPerChannel>8)
			format.mFormatFlags|=kLinearPCMFormatFlagIsSignedInteger;
		}
	else
		Misc::throwStdErr("SoundRecorder::SoundRecorder: Output file name %s has unrecognized extension",outputFileName);
	
	/* Create the recording audio queue: */
	if(AudioQueueNewInput(&format,handleInputBufferWrapper,this,0,kCFRunLoopCommonModes,0,&queue)!=noErr)
		Misc::throwStdErr("SoundRecorder::SoundRecorder: Error while creating audio queue");
	
	/* Retrieve the fully specified audio data format from the audio queue: */
	UInt32 formatSize=sizeof(format);
	if(AudioQueueGetProperty(queue,kAudioConverterCurrentOutputStreamDescription,&format,&formatSize)!=noErr)
		{
		AudioQueueDispose(queue,true);
		Misc::throwStdErr("SoundRecorder::SoundRecorder: Error while retrieving audio queue sound format");
		}
	
	/* Open the target audio file: */
	CFURLRef audioFileURL=CFURLCreateFromFileSystemRepresentation(0,reinterpret_cast<const UInt8*>(outputFileName),strlen(outputFileName),false);
	if(AudioFileCreateWithURL(audioFileURL,audioFileType,&format,kAudioFileFlags_EraseFile,&audioFile)!=noErr)
		{
		AudioQueueDispose(queue,true);
		CFRelease(audioFileURL);
		Misc::throwStdErr("SoundRecorder::SoundRecorder: Error while opening output file %s",outputFileName);
		}
	CFRelease(audioFileURL);
	
	/* Calculate an appropriate buffer size and allocate the sound buffers: */
	int maxPacketSize=format.mBytesPerPacket;
	if(maxPacketSize==0) // Must be a variable bit rate sound format
		{
		/* Query the expected maximum packet size from the audio queue: */
		UInt32 maxVBRPacketSize=sizeof(maxPacketSize);
		if(AudioQueueGetProperty(queue,kAudioConverterPropertyMaximumOutputPacketSize,&maxPacketSize,&maxVBRPacketSize)!=noErr)
			{
			AudioQueueDispose(queue,true);
			AudioFileClose(audioFile);
			Misc::throwStdErr("SoundRecorder::SoundRecorder: Error while calcuating sample buffer size");
			}
		}
	
	/* Calculate an appropriate buffer size based on the given duration: */
	int numPackets=int(floor(double(format.mSampleRate)*0.25+0.5));
	bufferSize=UInt32(numPackets*maxPacketSize);
	
	/* Create the sample buffers: */
	for(int i=0;i<2;++i)
		{
		/* Create the sound buffer: */
		if(AudioQueueAllocateBuffer(queue,bufferSize,&buffers[i])!=noErr)
			{
			AudioQueueDispose(queue,true);
			AudioFileClose(audioFile);
			Misc::throwStdErr("SoundRecorder::SoundRecorder: Error while allocating sample buffer %d",i);
			}
		
		/* Add the buffer to the queue: */
		if(AudioQueueEnqueueBuffer(queue,buffers[i],0,0)!=noErr)
			{
			AudioQueueDispose(queue,true);
			AudioFileClose(audioFile);
			Misc::throwStdErr("SoundRecorder::SoundRecorder: Error while enqueuing sample buffer %d",i);
			}
		}
	}

SoundRecorder::SoundRecorder(const SoundDataFormat& sFormat,const char* outputFileName)
	:bufferSize(0),
	 numRecordedPackets(0),
	 active(false)
	{
	/* Initialize the sound recorder: */
	init(0,sFormat,outputFileName);
	}

SoundRecorder::SoundRecorder(const char* audioSource,const SoundDataFormat& sFormat,const char* outputFileName)
	:bufferSize(0),
	 numRecordedPackets(0),
	 active(false)
	{
	/* Initialize the sound recorder: */
	init(audioSource,sFormat,outputFileName);
	}

SoundRecorder::~SoundRecorder(void)
	{
	/* Destroy the audio queue: */
	if(active)
		{
		/* Stop the audio queue: */
		AudioQueueStop(queue,true);
		
		/* Set the audio file's magic cookie: */
		setAudioFileMagicCookie();
		}
	AudioQueueDispose(queue,true);
	
	/* Destroy the audio file: */
	AudioFileClose(audioFile);
	}

SoundDataFormat SoundRecorder::getSoundDataFormat(void) const
	{
	/* Extract relevant information from the format data structure: */
	SoundDataFormat result;
	result.bitsPerSample=format.mBitsPerChannel;
	result.bytesPerSample=format.mBitsPerChannel/8;
	result.signedSamples=(format.mFormatFlags&kLinearPCMFormatFlagIsSignedInteger)!=0;
	result.sampleEndianness=format.mFormatFlags&kLinearPCMFormatFlagIsBigEndian?SoundDataFormat::BigEndian:SoundDataFormat::LittleEndian;
	result.samplesPerFrame=format.mChannelsPerFrame;
	result.framesPerSecond=int(floor(format.mSampleRate+0.5));
	
	return result;
	}

void SoundRecorder::start(void)
	{
	/* Do nothing if already started: */
	if(active)
		return;
	
	/* Reset the packet counter: */
	numRecordedPackets=0;
	
	/* Set the audio file's magic cookie: */
	setAudioFileMagicCookie();
	
	/* Start recording: */
	if(AudioQueueStart(queue,0)==noErr)
		active=true;
	else
		Misc::throwStdErr("SoundRecorder::start: Unable to start recording");
	}

void SoundRecorder::stop(void)
	{
	/* Do nothing if not started: */
	if(!active)
		return;
	
	/* Stop recording: */
	AudioQueueStop(queue,true);
	active=false;
	
	/* Set the audio file's magic cookie (might have been updated during recording): */
	setAudioFileMagicCookie();
	}

#else

/***************************************
OS-independent version of SoundRecorder:
***************************************/

#if SOUND_CONFIG_HAVE_ALSA

void SoundRecorder::writeWAVHeader(void)
	{
	/* Rewind the file: */
	outputFile->setWritePosAbs(0);
	
	/* Calculate the total file size: */
	size_t dataChunkSize=numRecordedFrames*size_t(format.samplesPerFrame)*size_t(format.bytesPerSample);
	size_t dataHeaderSize=2*sizeof(int);
	size_t fmtChunkSize=2*sizeof(int)+4*sizeof(short int);
	size_t fmtHeaderSize=2*sizeof(int);
	size_t riffChunkSize=sizeof(int)+fmtHeaderSize+fmtChunkSize+dataHeaderSize+dataChunkSize;
	
	/* Write the RIFF chunk: */
	outputFile->write<char>("RIFF",4);
	outputFile->write<unsigned int>(riffChunkSize);
	outputFile->write<char>("WAVE",4);
	
	/* Write the fmt chunk: */
	outputFile->write<char>("fmt ",4);
	outputFile->write<unsigned int>(fmtChunkSize);
	outputFile->write<unsigned short>(1); // PCM
	outputFile->write<unsigned short>(format.samplesPerFrame);
	outputFile->write<unsigned int>(format.framesPerSecond);
	outputFile->write<unsigned int>(format.framesPerSecond*format.samplesPerFrame*format.bytesPerSample);
	outputFile->write<unsigned short>(format.samplesPerFrame*format.bytesPerSample);
	outputFile->write<unsigned short>(format.bitsPerSample);
	
	/* Write the data chunk header: */
	outputFile->write<char>("data",4);
	outputFile->write<unsigned int>(dataChunkSize);
	}

void* SoundRecorder::recordingThreadMethod(void)
	{
	Threads::Thread::setCancelState(Threads::Thread::CANCEL_ENABLE);
	// Threads::Thread::setCancelType(Threads::Thread::CANCEL_ASYNCHRONOUS);
	
	/* Read buffers worth of sound data from the PCM device until interrupted: */
	while(keepReading)
		{
		/* Read pending sound data, up to the buffer size: */
		size_t numFramesRead=pcmDevice.read(sampleBuffer,sampleBufferSize);
		
		/* Write the buffer to the file: */
		outputFile->write(sampleBuffer,numFramesRead*bytesPerFrame);
		numRecordedFrames+=numFramesRead;
		}
	
	return 0;
	}

#endif

void SoundRecorder::init(const char* audioSource,const SoundDataFormat& sFormat,const char* outputFileName)
	{
	#if SOUND_CONFIG_HAVE_ALSA
	
	/* Sanify the sound data format: */
	if(format.bitsPerSample<1)
		format.bitsPerSample=1;
	format.bitsPerSample=(format.bitsPerSample+7)&~0x7; // Round the number of bits to the next multiple of 8
	if(format.bitsPerSample==24)
		format.bytesPerSample=4; // 24 bit sound data padded into 32 bit words
	else
		format.bytesPerSample=format.bitsPerSample/8;
	if(format.samplesPerFrame<1)
		format.samplesPerFrame=1;
	
	/* Determine the output file format from the file name extension: */
	if(Misc::hasCaseExtension(outputFileName,".wav"))
		{
		/* It's a WAV file: */
		outputFileFormat=WAV;
		
		/* Adjust the sound data format for WAV files: */
		format.signedSamples=format.bitsPerSample>8;
		format.sampleEndianness=SoundDataFormat::LittleEndian;
		outputFile->setEndianness(Misc::LittleEndian);
		}
	else if(Misc::hasCaseExtension(outputFileName,""))
		{
		/* It's a raw file: */
		outputFileFormat=RAW;
		}
	else
		Misc::throwStdErr("SoundRecorder::SoundRecorder: Output file %s has unrecognized extension",outputFileName);
	
	/* Calculate the number of bytes per frame: */
	bytesPerFrame=format.bytesPerSample*format.samplesPerFrame;
	
	/* Set the PCM device's parameters according to the sound data format: */
	pcmDevice.setSoundDataFormat(format);
	
	/* Create a sample buffer holding a quarter second of sound: */
	sampleBufferSize=(size_t(format.framesPerSecond)*250+500)/1000;
	sampleBuffer=new char[sampleBufferSize*bytesPerFrame];
	
	#endif
	}

SoundRecorder::SoundRecorder(const SoundDataFormat& sFormat,const char* outputFileName)
	:format(sFormat),
	#if SOUND_CONFIG_HAVE_ALSA
	 outputFileFormat(RAW),
	 bytesPerFrame(0),
	 pcmDevice("default",true),
	 outputFile(IO::openSeekableFile(outputFileName,IO::File::WriteOnly)),
	 sampleBufferSize(0),sampleBuffer(0),
	 numRecordedFrames(0),
	 keepReading(true),
	#endif
	 active(false)
	{
	/* Initialize the sound recorder: */
	init("default",sFormat,outputFileName);
	}

SoundRecorder::SoundRecorder(const char* audioSource,const SoundDataFormat& sFormat,const char* outputFileName)
	:format(sFormat),
	#if SOUND_CONFIG_HAVE_ALSA
	 outputFileFormat(RAW),
	 bytesPerFrame(0),
	 pcmDevice(audioSource,true),
	 outputFile(IO::openSeekableFile(outputFileName,IO::File::WriteOnly)),
	 sampleBufferSize(0),sampleBuffer(0),
	 numRecordedFrames(0),
	 keepReading(true),
	#endif
	 active(false)
	{
	/* Initialize the sound recorder: */
	init(audioSource,sFormat,outputFileName);
	}

SoundRecorder::~SoundRecorder(void)
	{
	#if SOUND_CONFIG_HAVE_ALSA
	
	/* Stop the recording thread if still active: */
	if(active)
		{
		/* Stop the recording thread at the next opportunity: */
		keepReading=false;
		recordingThread.join();
		
		/* Write the final audio file header if necessary: */
		if(outputFileFormat==WAV)
			writeWAVHeader();
		}
	
	/* Delete the sample buffer: */
	delete[] sampleBuffer;
	
	#endif
	}

SoundDataFormat SoundRecorder::getSoundDataFormat(void) const
	{
	return format;
	}

void SoundRecorder::start(void)
	{
	/* Do nothing if already started: */
	if(active)
		return;
	
	#if SOUND_CONFIG_HAVE_ALSA
	
	/* Reset the number of recorded frames: */
	numRecordedFrames=0;
	
	/* Write an empty audio file header if necessary: */
	if(outputFileFormat==WAV)
		writeWAVHeader();
	
	/* Prepare the device for recording: */
	pcmDevice.prepare();
	
	/* Start the PCM device: */
	pcmDevice.start();
	
	/* Start the background recording thread: */
	recordingThread.start(this,&SoundRecorder::recordingThreadMethod);
	
	#endif
	
	active=true;
	}

void SoundRecorder::stop(void)
	{
	/* Do nothing if not started: */
	if(!active)
		return;
	
	#if SOUND_CONFIG_HAVE_ALSA
	
	/* Stop the PCM device: */
	pcmDevice.drain();
	usleep(10000);
	
	/* Kill the background recording thread: */
	recordingThread.cancel();
	recordingThread.join();
	
	/* Write the final audio file header if necessary: */
	if(outputFileFormat==WAV)
		writeWAVHeader();
	
	#endif
	
	active=false;
	}

#endif

}
