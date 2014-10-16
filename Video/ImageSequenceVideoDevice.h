/***********************************************************************
ImageSequenceVideoDevice - Class for "fake" video capture devices
showing a set of image files.
Copyright (c) 2014 Oliver Kreylos

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

#ifndef VIDEO_IMAGESEQUENCEVIDEODEVICE_INCLUDED
#define VIDEO_IMAGESEQUENCEVIDEODEVICE_INCLUDED

#include <string>
#include <IO/Directory.h>
#include <GLMotif/TextFieldSlider.h>
#include <Images/RGBImage.h>
#include <Video/FrameBuffer.h>
#include <Video/VideoDevice.h>

namespace Video {

class ImageSequenceVideoDevice:public VideoDevice
	{
	/* Embedded classes: */
	public:
	class DeviceId:public VideoDevice::DeviceId
		{
		friend class ImageSequenceVideoDevice;
		
		/* Elements: */
		private:
		std::string frameNameTemplate; // The name template for all frame image files
		
		/* Constructors and destructors: */
		public:
		DeviceId(std::string sName)
			:VideoDevice::DeviceId(sName)
			{
			}
		
		/* Methods from VideoDevice::DeviceId: */
		virtual VideoDevice* createDevice(void) const;
		};
	
	/* Elements: */
	private:
	IO::DirectoryPtr frameDir; // Directory containing all frame image files
	std::string frameNameTemplate; // The name template for all frame image files
	unsigned int frameSize[2]; // Frame size of all frame images
	int firstIndex; // The index of the first frame
	int lastIndex; // The index one past the last frame
	int currentIndex; // The index of the most recently pushed-out frame
	Images::RGBImage currentFrame; // The current frame
	FrameBuffer currentBuffer; // A frame buffer representing the current frame
	
	/* Private methods: */
	void loadFrame(int frameIndex); // Loads the frame of the given index
	void frameIndexSliderCallback(GLMotif::TextFieldSlider::ValueChangedCallbackData* cbData);
	
	/* Constructors and destructors: */
	public:
	ImageSequenceVideoDevice(const char* sFrameNameTemplate); // Creates a video device from the given frame name template, containing exactly one %d converter
	private:
	ImageSequenceVideoDevice(const ImageSequenceVideoDevice& source); // Prohibit copy constructor
	ImageSequenceVideoDevice& operator=(const ImageSequenceVideoDevice& source); // Prohibit assignment operator
	public:
	virtual ~ImageSequenceVideoDevice(void);
	
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
	static void enumerateDevices(std::vector<DeviceIdPtr>& devices); // Appends device ID objects for a *hard-coded* video device to the given list
	};

}

#endif
