/***********************************************************************
KinectRecorder - Class to save 3D video of a Vrui session recorded with
an InputDeviceDataSaver.
Copyright (c) 2011 Oliver Kreylos

This file is part of the Virtual Reality User Interface Library (Vrui).

The Virtual Reality User Interface Library is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The Virtual Reality User Interface Library is distributed in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Virtual Reality User Interface Library; if not, write to the
Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#ifndef VRUI_INTERNAL_KINECTRECORDER_INCLUDED
#define VRUI_INTERNAL_KINECTRECORDER_INCLUDED

#include <string>
#include <vector>
#include <Kinect/USBContext.h>
#include <Kinect/KinectCamera.h>

/* Forward declarations: */
namespace Misc {
class ConfigurationFileSection;
}
class FrameBuffer;
class KinectFrameSaver;

namespace Vrui {

class KinectRecorder
	{
	/* Embedded classes: */
	private:
	class KinectStreamer // Helper class to stream 3D video data from a Kinect camera to a pair of time-stamped files
		{
		/* Elements: */
		public:
		KinectCamera camera; // The Kinect camera from which to receive depth and color streams
		KinectFrameSaver* frameSaver; // Pointer to helper object saving depth and color frames received from the Kinect
		
		/* Private methods: */
		void depthStreamingCallback(const FrameBuffer& frameBuffer); // Callback receiving depth frames from the Kinect camera
		void colorStreamingCallback(const FrameBuffer& frameBuffer); // Callback receiving color frames from the Kinect camera
		
		/* Constructors and destructors: */
		public:
		KinectStreamer(libusb_device* sDevice,std::string calibrationFilesPath,std::string saveFileNamePrefix,Misc::ConfigurationFileSection& cameraSection); // Creates a streamer for the Kinect camera of the given index in the given USB context
		~KinectStreamer(void); // Destroys the streamer
		
		/* Methods: */
		KinectCamera& getCamera(void) // Returns a reference to the streamer's camera
			{
			return camera;
			}
		void startStreaming(void); // Begins streaming from the Kinect camera
		};
	
	/* Elements: */
	USBContext usbContext; // USB device context
	std::vector<KinectStreamer*> streamers; // List of Kinect streamers, each connected to one Kinect camera
	
	/* Constructors and destructors: */
	public:
	KinectRecorder(Misc::ConfigurationFileSection& configFileSection); // Creates a Kinect recorder based on the given configuration file section
	~KinectRecorder(void); // Destroys the Kinect recorder
	
	/* Methods: */
	void start(double currentTimeStamp); // Starts recording
	};

}

#endif
