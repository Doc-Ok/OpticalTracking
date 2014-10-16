/***********************************************************************
VideoDevice - Base class for video capture devices.
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

#ifndef VIDEO_VIDEODEVICE_INCLUDED
#define VIDEO_VIDEODEVICE_INCLUDED

#include <string>
#include <vector>
#include <Misc/RefCounted.h>
#include <Misc/Autopointer.h>
#include <Video/VideoDataFormat.h>

/* Forward declarations: */
namespace Misc {
template <class ParameterParam>
class FunctionCall;
class ConfigurationFileSection;
}
namespace GLMotif {
class WidgetManager;
class Widget;
}
namespace Video {
class FrameBuffer;
class ImageExtractor;
}

namespace Video {

class VideoDevice
	{
	/* Embedded classes: */
	public:
	class DeviceId:public Misc::RefCounted // Class to uniquely identify video devices across different device classes
		{
		/* Elements: */
		protected:
		std::string name; // Human-readable device name
		
		/* Constructors and destructors: */
		public:
		DeviceId(std::string sName)
			:name(sName)
			{
			}
		virtual ~DeviceId(void)
			{
			}
		
		/* Methods: */
		std::string getName(void) const // Returns the human-readable device name
			{
			return name;
			}
		virtual VideoDevice* createDevice(void) const =0; // Creates a video device based on this device ID
		};
	
	typedef Misc::Autopointer<DeviceId> DeviceIdPtr; // Type for smart pointers to device ID objects
	typedef void (*EnumerateVideoDevicesFunc)(std::vector<DeviceIdPtr>& devices); // Type for functions to enumerate connected video devices of a certain class
	typedef Misc::FunctionCall<const FrameBuffer*> StreamingCallback; // Function call type for streaming capture callback
	
	private:
	struct DeviceClass // Structure to represent additional video device classes
		{
		/* Elements: */
		public:
		EnumerateVideoDevicesFunc enumerateVideoDevices; // The video device enumeration function for the represented device class
		DeviceClass* succ; // Pointer to the next device class in the list
		};
	
	/* Elements: */
	private:
	static DeviceClass* deviceClasses; // List of additional registered video device classes
	protected:
	StreamingCallback* streamingCallback; // Function called when a frame buffer becomes ready in streaming capture mode
	
	/* Constructors and destructors: */
	public:
	VideoDevice(void); // Creates video device
	virtual ~VideoDevice(void); // Closes the video device
	
	/* Device enumeration and creation methods: */
	static void registerDeviceClass(EnumerateVideoDevicesFunc enumerateVideoDevices); // Registers a new device class for the given enumeration function
	static void unregisterDeviceClass(EnumerateVideoDevicesFunc enumerateVideoDevices); // Unregisters the device class with the given enumeration function
	static std::vector<DeviceIdPtr> getVideoDevices(void); // Returns a list of device IDs for all video devices currently available on the system
	static VideoDevice* createVideoDevice(DeviceIdPtr deviceId); // Creates a video device for the given device ID
	
	/* Methods: */
	virtual std::vector<VideoDataFormat> getVideoFormatList(void) const =0; // Returns a list of video formats supported by the device
	virtual VideoDataFormat getVideoFormat(void) const =0; // Returns the video device's current video format
	virtual VideoDataFormat& setVideoFormat(VideoDataFormat& newFormat) =0; // Sets the video device's video format to the most closely matching format; changes the given format structure to the actually set format
	virtual void configure(const Misc::ConfigurationFileSection& cfg); // Configures the video device from the given configuration file section
	virtual ImageExtractor* createImageExtractor(void) const =0; // Creates an image extractor for the video source's current video format
	virtual GLMotif::Widget* createControlPanel(GLMotif::WidgetManager* widgetManager) =0; // Creates a GLMotif control panel to adjust all exposed video device controls
	
	/* Streaming capture interface methods: */
	virtual unsigned int allocateFrameBuffers(unsigned int requestedNumFrameBuffers) =0; // Allocates the given number of streaming frame buffers; returns actual number of buffers allocated by device
	virtual void startStreaming(void); // Starts streaming video capture using a previously allocated set of frame buffers
	virtual void startStreaming(StreamingCallback* newStreamingCallback); // Ditto; calls callback from separate thread whenever a new frame buffer becomes ready
	virtual FrameBuffer* dequeueFrame(void) =0; // Returns the next frame buffer captured from the video device; blocks if no frames are ready
	virtual void enqueueFrame(FrameBuffer* frame) =0; // Returns the given frame buffer to the capturing queue after the caller is done with it
	virtual void stopStreaming(void); // Stops streaming video capture
	virtual void releaseFrameBuffers(void) =0; // Releases all previously allocated frame buffers
	};

}

#endif
