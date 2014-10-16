/***********************************************************************
DC1394VideoDevice - Wrapper class around video devices as represented by
the dc1394 IEEE 1394 (Firewire) DCAM video library.
Copyright (c) 2009-2014 Oliver Kreylos

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

#ifndef VIDEO_LINUX_DC1394VIDEODEVICE_INCLUDED
#define VIDEO_LINUX_DC1394VIDEODEVICE_INCLUDED

#include <dc1394/types.h>
#include <dc1394/video.h>
#include <Threads/Thread.h>
#include <Video/FrameBuffer.h>
#include <Video/BayerPattern.h>
#include <Video/VideoDevice.h>

/* Forward declarations: */
namespace Misc {
class CallbackData;
}

namespace Video {

class DC1394VideoDevice:public VideoDevice
	{
	/* Embedded classes: */
	public:
	class DeviceId:public VideoDevice::DeviceId
		{
		friend class DC1394VideoDevice;
		
		/* Elements: */
		private:
		uint64_t guid; // Camera's globally unique identifier
		unsigned int unitIndex; // Index of sub-device inside camera device
		
		/* Constructors and destructors: */
		public:
		DeviceId(std::string sName)
			:VideoDevice::DeviceId(sName)
			{
			}
		
		/* Methods from VideoDevice::DeviceId: */
		virtual VideoDevice* createDevice(void) const;
		};
	
	struct DC1394FrameBuffer:public FrameBuffer
		{
		friend class DC1394VideoDevice;
		
		/* Elements: */
		private:
		dc1394video_frame_t* frame; // Pointer to the original DC1394 frame structure
		
		/* Constructors and destructors: */
		public:
		DC1394FrameBuffer(void) // Creates an empty, unallocated frame buffer
			:frame(0)
			{
			}
		};
	
	/* Elements: */
	private:
	dc1394_t* context; // Device context for dc1394 cameras
	dc1394camera_t* camera; // Pointer to selected camera device
	BayerPattern bayerPattern; // The camera's Bayer filter pattern, or BAYER_INVALID if the camera does not have a Bayer filter
	Threads::Thread streamingThread; // Background streaming capture thread
	
	/* Private methods: */
	bool parseVideoMode(dc1394video_mode_t videoMode,VideoDataFormat& format) const; // Updates part of the given format structure from the given video mode identifier
	void triggerModeCallback(Misc::CallbackData* cbData);
	void triggerSourceCallback(Misc::CallbackData* cbData);
	void triggerPolarityCallback(Misc::CallbackData* cbData);
	void featureModeCallback(Misc::CallbackData* cbData,const unsigned int& featureId);
	void whiteBalanceBUValueCallback(Misc::CallbackData* cbData);
	void whiteBalanceRVValueCallback(Misc::CallbackData* cbData);
	void featureValueCallback(Misc::CallbackData* cbData,const unsigned int& featureId);
	void* streamingThreadMethod(void); // The background streaming capture thread method
	
	/* Constructors and destructors: */
	public:
	DC1394VideoDevice(uint64_t guid,unsigned int unitIndex =0); // Opens the DC1394 video device of the given GUID and unit index as a video source; uses first device if guid is zero
	private:
	DC1394VideoDevice(const DC1394VideoDevice& source); // Prohibit copy constructor
	DC1394VideoDevice& operator=(const DC1394VideoDevice& source); // Prohibit assignment operator
	public:
	virtual ~DC1394VideoDevice(void); // Closes the video device
	
	/* Methods from VideoDevice: */
	virtual std::vector<VideoDataFormat> getVideoFormatList(void) const;
	virtual VideoDataFormat getVideoFormat(void) const;
	virtual VideoDataFormat& setVideoFormat(VideoDataFormat& newFormat);
	virtual void configure(const Misc::ConfigurationFileSection& cfg);
	virtual ImageExtractor* createImageExtractor(void) const;
	virtual GLMotif::Widget* createControlPanel(GLMotif::WidgetManager* widgetManager);
	
	/* Streaming capture interface methods: */
	virtual unsigned int allocateFrameBuffers(unsigned int requestedNumFrameBuffers);
	virtual void startStreaming(void);
	virtual void startStreaming(StreamingCallback* newStreamingCallback);
	virtual FrameBuffer* dequeueFrame(void);
	virtual void enqueueFrame(FrameBuffer* frame);
	virtual void stopStreaming(void);
	virtual void releaseFrameBuffers(void);
	
	/* New methods: */
	static void enumerateDevices(std::vector<DeviceIdPtr>& devices); // Appends device ID objects for all available DC1394 video devices to the given list
	BayerPattern getBayerPattern(void) const // Returns the camera's Bayer pattern
		{
		return bayerPattern;
		}
	};

}

#endif
