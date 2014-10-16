/***********************************************************************
OculusRiftDK2VideoDevice - Custom subclass of V4L2VideoDevice to correct
the DK2's tracking cameras quirks, and allow for LED strobe
synchronization.
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

#ifndef VIDEO_LINUX_OCULUSRIFTDK2VIDEODEVICE_INCLUDED
#define VIDEO_LINUX_OCULUSRIFTDK2VIDEODEVICE_INCLUDED

#include <Video/Linux/V4L2VideoDevice.h>

/* Forward declarations: */
namespace Misc {
class CallbackData;
}

namespace Video {

class OculusRiftDK2VideoDevice:public V4L2VideoDevice
	{
	/* Embedded classes: */
	public:
	class DeviceId:public VideoDevice::DeviceId
		{
		friend class OculusRiftDK2VideoDevice;
		
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
	
	/* Elements: */
	private:
	bool colorMode; // Flag whether the camera's currently selected video data format is a color format
	
	/* Camera's current control values: */
	unsigned int horizontalBlanking;
	unsigned int verticalBlanking;
	bool synched;
	unsigned int coarseShutterWidth;
	bool flipColumns;
	bool flipRows;
	unsigned int analogGain;
	bool automaticBlacklevelCalibration;
	int blacklevelCalibrationValue;
	bool automaticExposureGain;
	unsigned int fineShutterWidth;
	
	/* Private methods: */
	void getControlValues(void); // Downloads the current set of control values from the camera's imaging sensor
	void setControlValues(void); // Uploads the current set of control values to the camera's imaging sensor
	void toggleButtonCallback(Misc::CallbackData* cbData,const int& index);
	void textFieldSliderCallback(Misc::CallbackData* cbData,const int& index);
	
	/* Constructors and destructors: */
	public:
	OculusRiftDK2VideoDevice(const char* videoDeviceName); // Opens the given V4L2 video device (/dev/videoXX) as a video source
	private:
	OculusRiftDK2VideoDevice(const OculusRiftDK2VideoDevice& source); // Prohibit copy constructor
	OculusRiftDK2VideoDevice& operator=(const OculusRiftDK2VideoDevice& source); // Prohibit assignment operator
	public:
	virtual ~OculusRiftDK2VideoDevice(void); // Closes the video device
	
	/* Methods from VideoDevice: */
	virtual std::vector<VideoDataFormat> getVideoFormatList(void) const;
	virtual VideoDataFormat getVideoFormat(void) const;
	virtual VideoDataFormat& setVideoFormat(VideoDataFormat& newFormat);
	virtual ImageExtractor* createImageExtractor(void) const;
	virtual GLMotif::Widget* createControlPanel(GLMotif::WidgetManager* widgetManager);
	
	/* Streaming capture interface methods: */
	virtual void startStreaming(void);
	virtual void startStreaming(StreamingCallback* newStreamingCallback);
	virtual void stopStreaming(void);
	
	/* New methods: */
	static void enumerateDevices(std::vector<VideoDevice::DeviceIdPtr>& devices); // Appends device ID objects for all available Oculus Rift DK2 video devices to the given list
	void setTrackingMode(bool enabled); // Enables or disables Oculus Rift DK2 infrared LED tracking mode
	};

}

#endif
