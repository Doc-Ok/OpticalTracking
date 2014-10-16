/***********************************************************************
ALSAPCMDevice - Simple wrapper class around PCM devices as represented
by the Advanced Linux Sound Architecture (ALSA) library.
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

#ifndef SOUND_LINUX_ALSAPCMDEVICE_INCLUDED
#define SOUND_LINUX_ALSAPCMDEVICE_INCLUDED

#include <errno.h>
#include <alsa/asoundlib.h>
#include <stdexcept>
#include <Misc/ThrowStdErr.h>

/* Forward declarations: */
namespace Sound {
class SoundDataFormat;
}

namespace Sound {

class ALSAPCMDevice
	{
	/* Embedded classes: */
	public:
	class XrunError:public std::runtime_error // Base exception class to report overrun or underrun errors
		{
		/* Constructors and destructors: */
		public:
		XrunError(const char* error)
			:std::runtime_error(error)
			{
			}
		};
	
	class OverrunError:public XrunError // Exception class for overrun errors
		{
		/* Constructors and destructors: */
		public:
		OverrunError(void)
			:XrunError("ALSAPCMDevice::read: Overrun detected")
			{
			}
		};
	
	class UnderrunError:public XrunError // Exception class for underrun errors
		{
		/* Constructors and destructors: */
		public:
		UnderrunError(void)
			:XrunError("ALSAPCMDevice::write: Underrun detected")
			{
			}
		};
	
	/* Elements: */
	private:
	snd_pcm_t* pcmDevice; // Handle to the ALSA PCM device
	snd_pcm_hw_params_t* pcmHwParams; // Hardware parameter context for the PCM device; used to accumulate settings until prepare() is called
	
	/* Constructors and destructors: */
	public:
	ALSAPCMDevice(const char* pcmDeviceName,bool recording); // Opens the named PCM device for recording or playback
	~ALSAPCMDevice(void); // Closes the PCM device
	
	/* Methods: */
	snd_async_handler_t* registerAsyncHandler(snd_async_callback_t callback,void* privateData); // Registers an asynchronous callback with the PCM device
	void setSoundDataFormat(const SoundDataFormat& format); // Sets the PCM device's sample format
	void setBufferSize(size_t numBufferFrames,size_t numPeriodFrames); // Sets the device's buffer and period sizes
	void setStartThreshold(size_t numStartFrames); // Sets automatic PCM start threshold for playback and capture devices
	void prepare(void); // Applies cached hardware parameters to PCM device and prepares it for recording / playback
	void start(void); // Starts recording or playback on the PCM device
	bool wait(int timeout) // Waits for the PCM device to get ready for I/O; timeout is in milliseconds; negative values wait forever; returns true if device is ready
		{
		int result;
		if((result=snd_pcm_wait(pcmDevice,timeout))<0)
			{
			if(result==-EPIPE)
				throw XrunError("ALSAPCMDevice::wait: Over-/underrun detected");
			else
				Misc::throwStdErr("ALSAPCMDevice::wait: Error %s",snd_strerror(result));
			}
		
		/* Return true if PCM device is ready: */
		return result==1;
		}
	size_t read(void* buffer,size_t numFrames) // Reads from PCM device into buffer; returns number of frames read
		{
		snd_pcm_sframes_t result=snd_pcm_readi(pcmDevice,buffer,numFrames);
		if(result<0)
			{
			if(result==-EPIPE)
				throw OverrunError();
			else
				Misc::throwStdErr("ALSAPCMDevice::read: Error %s",snd_strerror(result));
			}
		return size_t(result);
		}
	size_t write(const void* buffer,size_t numFrames) // Writes from buffer to PCM device; returns number of frames written
		{
		snd_pcm_sframes_t result=snd_pcm_writei(pcmDevice,buffer,numFrames);
		if(result<0)
			{
			if(result==-EPIPE)
				throw UnderrunError();
			else
				Misc::throwStdErr("ALSAPCMDevice::write: Error %s",snd_strerror(result));
			}
		return size_t(result);
		}
	void drop(void); // Stops recording/playback and discards pending frames
	void drain(void); // Stops recording/playback but delays until all pending frames have been processed
	};

}

#endif
