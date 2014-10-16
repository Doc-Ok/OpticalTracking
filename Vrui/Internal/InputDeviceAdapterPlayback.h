/***********************************************************************
InputDeviceAdapterPlayback - Class to read input device states from a
pre-recorded file for playback and/or movie generation.
Copyright (c) 2004-2014 Oliver Kreylos

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

#ifndef VRUI_INTERNAL_INPUTDEVICEADAPTERPLAYBACK_INCLUDED
#define VRUI_INTERNAL_INPUTDEVICEADAPTERPLAYBACK_INCLUDED

#include <string>
#include <vector>
#include <IO/SeekableFile.h>
#include <Geometry/Vector.h>
#include <Vrui/Geometry.h>
#include <Vrui/Internal/InputDeviceAdapter.h>

/* Forward declarations: */
namespace Misc {
template <class ValueParam>
void swapEndianness(ValueParam& value);
class ConfigurationFileSection;
}
namespace Sound {
class SoundPlayer;
}
namespace Vrui {
class MouseCursorFaker;
class VRWindow;
#ifdef VRUI_INPUTDEVICEADAPTERPLAYBACK_USE_KINECT
class KinectPlayback;
#endif
}

namespace Vrui {

class InputDeviceAdapterPlayback:public InputDeviceAdapter
	{
	/* Elements: */
	private:
	IO::SeekableFilePtr inputDeviceDataFile; // File containing the input device data
	unsigned int fileVersion; // Version of the input device data file
	int* deviceFeatureBaseIndices; // Array of base indices in feature name array for each input device
	std::vector<std::string> deviceFeatureNames; // Array of input device feature names
	MouseCursorFaker* mouseCursorFaker; // Pointer to object used to render a fake mouse cursor
	bool synchronizePlayback; // Flag whether to force the Vrui mainloop to run at the speed of the recording; by default, mainloop runs as fast as it can
	bool quitWhenDone; // Flag whether to quit the Vrui application when all saved data has been played back
	Sound::SoundPlayer* soundPlayer; // Pointer to a sound player object used to play back synchronized commentary tracks
	#ifdef VRUI_INPUTDEVICEADAPTERPLAYBACK_USE_KINECT
	KinectPlayback* kinectPlayer; // Pointer to a 3D video player object to play back a recorded user
	#endif
	bool saveMovie; // Flag whether to create a movie by writing screenshots at regular intervals
	std::string movieFileNameTemplate; // Template for creating image file names; must contain exactly one %d placeholder
	int movieWindowIndex; // Index of the master node window from which to save screenshots; awkward, but windows have no names
	VRWindow* movieWindow; // Pointer to the window from which to save screenshots; determined from name on first frame
	double movieFrameTimeInterval; // Time between adjacent frames in the saved movie; == 1.0/movieFrameRate
	int movieFrameStart; // Number of movie frames to skip at the beginning of playback. First frame will always be written with index 0
	int movieFrameOffset; // Index to assign to the first saved movie frame (after initial frames have been skipped)
	unsigned int firstFrameCountdown; // Counter to indicate the first frame of the Vrui application
	double timeStamp; // Current time stamp of input device data
	double timeStampOffset; // Offset from system's wall clock time to input data's time stamp sequence
	double nextTimeStamp; // Time stamp of next frame of input device data
	double nextMovieFrameTime; // Time at which to save the next movie frame
	int nextMovieFrameCounter; // Frame index for the next movie frame
	bool done; // Flag if input file is at end
	
	/* Constructors and destructors: */
	public:
	InputDeviceAdapterPlayback(InputDeviceManager* sInputDeviceManager,const Misc::ConfigurationFileSection& configFileSection); // Creates adapter by opening and reading pre-recorded device data file
	virtual ~InputDeviceAdapterPlayback(void);
	
	/* Methods from InputDeviceAdapter: */
	virtual std::string getFeatureName(const InputDeviceFeature& feature) const;
	virtual int getFeatureIndex(InputDevice* device,const char* featureName) const;
	virtual void updateInputDevices(void);
	#ifdef VRUI_INPUTDEVICEADAPTERPLAYBACK_USE_KINECT
	virtual void glRenderAction(GLContextData& contextData) const;
	#endif
	
	/* New methods: */
	bool isDone(void) const // Returns true if file has been entirely read
		{
		return done;
		}
	double getCurrentTime(void) const // Returns current data frame's time stamp
		{
		return timeStamp;
		}
	double getNextTime(void) const // Returns next data frame's time stamp
		{
		return nextTimeStamp;
		}
	};

}

#endif
