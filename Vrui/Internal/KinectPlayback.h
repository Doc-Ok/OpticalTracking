/***********************************************************************
KinectPlayback - Class to play 3D video of a Vrui session previously
recorded with an InputDeviceDataSaver.
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

#ifndef VRUI_INTERNAL_KINECTPLAYBACK_INCLUDED
#define VRUI_INTERNAL_KINECTPLAYBACK_INCLUDED

#include <string>
#include <vector>
#include <IO/File.h>
#include <Threads/Thread.h>
#include <Threads/MutexCond.h>
#include <Geometry/OrthogonalTransformation.h>
#include <Kinect/FrameBuffer.h>

/* Forward declarations: */
namespace Misc {
class ConfigurationFileSection;
}
class GLContextData;
class KinectProjector;

namespace Vrui {

class KinectPlayback
	{
	/* Embedded classes: */
	private:
	class KinectStreamer // Helper class to read 3D video data from a pair of time-stamped files
		{
		/* Embedded classes: */
		private:
		typedef Geometry::OrthogonalTransformation<double,3> OGTransform; // Type for facade transformations
		
		/* Elements: */
		Threads::MutexCond timeStampCond; // Condition variable to signal a change in the next time stamp value
		double readAheadTimeStamp; // Time stamp up to which to read ahead in the depth and color files
		IO::FilePtr depthFile; // Pointer to the file containing the depth stream
		Threads::Thread depthDecompressorThread; // Thread to decompress depth frames from the depth file
		IO::FilePtr colorFile; // Pointer to the file containing the color stream
		Threads::Thread colorDecompressorThread; // Thread to decompress color frames from the color file
		OGTransform projectorTransform; // Transformation from projector space to physical space
		KinectProjector* projector; // Projector to render a combined depth/color frame
		Threads::MutexCond frameUpdateCond; // Condition variable to signal arrival of a new depth or color frame
		FrameBuffer depthFrame; // The currently rendered depth frame
		double depthFrameValid; // Time at which the current depth frame becomes invalid
		double currentDepthFrame; // Time stamp of depth frame currently uploaded into projector
		FrameBuffer colorFrame; // The currently rendered color frame
		double colorFrameValid; // Time at which the current color frame becomes invalid
		double currentColorFrame; // Time stamp of color frame currently uploaded into projector
		
		/* Private methods: */
		void* depthDecompressorThreadMethod(void); // Thread method to read depth frames
		void* colorDecompressorThreadMethod(void); // Thread method to read color frames
		
		/* Constructors and destructors: */
		public:
		KinectStreamer(double firstTimeStamp,std::string saveFileNamePrefix,Misc::ConfigurationFileSection& streamerSection); // Initializes streamer by reading the given configuration file section
		~KinectStreamer(void); // Destroys the streamer
		
		/* Methods: */
		void updateFrames(double currentTimeStamp,double nextTimeStamp); // Updates the streamer's frames for display on the given time stamp
		void glRenderAction(GLContextData& contextData) const; // Renders the current frame
		};
	
	/* Elements: */
	std::vector<KinectStreamer*> streamers; // List of one streamer for each played back Kinect file pair
	
	/* Constructors and destructors: */
	public:
	KinectPlayback(double firstTimeStamp,Misc::ConfigurationFileSection& configFileSection); // Initializes playback by reading the given configuration file section
	~KinectPlayback(void); // Stops playback
	
	/* Methods: */
	void frame(double currentTimeStamp,double nextTimeStamp); // Prepares the most recent frames not later than the given timestamp for display (blocks until ready) and starts reading ahead for the next frame
	void glRenderAction(GLContextData& contextData) const; // Displays the prepared frames
	};

}

#endif
