/***********************************************************************
V4L2VideoDevice - Wrapper class around video devices as represented by
the Video for Linux version 2 (V4L2) library.
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

#ifndef VIDEO_LINUX_V4L2VIDEODEVICE_INCLUDED
#define VIDEO_LINUX_V4L2VIDEODEVICE_INCLUDED

#include <Threads/Thread.h>
#include <Video/FrameBuffer.h>
#include <Video/VideoDevice.h>

/* Forward declarations: */
namespace Misc {
class CallbackData;
}

namespace Video {

class V4L2VideoDevice:public VideoDevice
	{
	/* Embedded classes: */
	public:
	class DeviceId:public VideoDevice::DeviceId
		{
		friend class V4L2VideoDevice;
		
		/* Elements: */
		private:
		std::string deviceFileName; // Name of the video device's device file
		
		/* Constructors and destructors: */
		public:
		DeviceId(std::string sName)
			:VideoDevice::DeviceId(sName)
			{
			}
		
		/* Methods from VideoDevice::DeviceId: */
		virtual VideoDevice* createDevice(void) const;
		};
	
	struct V4L2FrameBuffer:public FrameBuffer // Structure to retain state of allocated frame buffers
		{
		/* Elements: */
		public:
		unsigned int index; // Index to identify memory-mapped buffers
		unsigned int sequence; // Sequence number of frame
		
		/* Constructors and destructors: */
		V4L2FrameBuffer(void) // Creates an empty, unallocated frame buffer
			:index(~0U)
			{
			}
		};
	
	/* Elements: */
	protected:
	int videoFd; // File handle of the V4L2 video device
	private:
	bool canRead; // Flag if video device supports read() I/O
	bool canStream; // Flag if video device supports streaming (buffer passing) I/O
	bool frameBuffersMemoryMapped; // Flag if the frame buffer set is memory-mapped from device space
	unsigned int numFrameBuffers; // Number of currently allocated frame buffers
	V4L2FrameBuffer* frameBuffers; // Array of currently allocated frame buffers
	Threads::Thread streamingThread; // Background streaming capture thread
	
	/* Private methods: */
	void enumFrameIntervals(VideoDataFormat& format,std::vector<VideoDataFormat>& formatList) const; // Appends video data formats for each available frame interval in the given pixel format and frame size to the format list
	void setControl(unsigned int controlId,const char* controlTag,const Misc::ConfigurationFileSection& cfg); // Sets a video device control according to the given tag in the given configuration file section
	void integerControlChangedCallback(Misc::CallbackData* cbData,const unsigned int& controlId);
	void booleanControlChangedCallback(Misc::CallbackData* cbData,const unsigned int& controlId);
	void menuControlChangedCallback(Misc::CallbackData* cbData,const unsigned int& controlId);
	void* streamingThreadMethod(void); // The background streaming capture thread method
	
	/* Constructors and destructors: */
	public:
	V4L2VideoDevice(const char* videoDeviceName); // Opens the given V4L2 video device (/dev/videoXX) as a video source
	private:
	V4L2VideoDevice(const V4L2VideoDevice& source); // Prohibit copy constructor
	V4L2VideoDevice& operator=(const V4L2VideoDevice& source); // Prohibit assignment operator
	public:
	virtual ~V4L2VideoDevice(void); // Closes the video device
	
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
	static void enumerateDevices(std::vector<DeviceIdPtr>& devices); // Appends device ID objects for all available V4L2 video devices to the given list
	};

}

#endif
