/***********************************************************************
InputDeviceAdapterTrackd - Class to connect a trackd tracking daemon to
a Vrui application.
Copyright (c) 2013 Oliver Kreylos

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

#ifndef VRUI_INTERNAL_INPUTDEVICEADAPTERTRACKD_INCLUDED
#define VRUI_INTERNAL_INPUTDEVICEADAPTERTRACKD_INCLUDED

#include <string>
#include <vector>
#include <Misc/SizedTypes.h>
#include <Threads/Thread.h>
#include <Geometry/OrthogonalTransformation.h>
#include <Vrui/Geometry.h>
#include <Vrui/Internal/InputDeviceAdapterIndexMap.h>

/* Forward declarations: */
namespace Misc {
class ConfigurationFileSection;
}

namespace Vrui {

class InputDeviceAdapterTrackd:public InputDeviceAdapterIndexMap
	{
	/* Embedded classes: */
	private:
	struct SensorHeader // Header structure for trackd's sensor data shared memory segment
		{
		/* Elements: */
		public:
		Misc::UInt32 version; // Version number of CAVElib/trackd daemon
		Misc::UInt32 numSensors; // Number of sensors reported by the trackd daemon
		Misc::UInt32 sensorsOffset; // Offset from beginning of shared memory segment to start of sensor data array in bytes
		Misc::UInt32 sensorDataSize; // Size of a sensor data structure in bytes
		Misc::UInt32 dataTimeStamp[2]; // Time stamp of sensor data currently reflected in sensor data array
		Misc::UInt32 daemonCommand; // A field to issue commands to the trackd daemon (0 when no commands are pending)
		};
	
	struct SensorData // Structure for sensor data in trackd's sensor data shared memory segment
		{
		/* Elements: */
		Misc::Float32 position[3]; // Position of the sensor in trackd's own coordinate system
		Misc::Float32 angles[3]; // Euler angle triplet defining the sensor's orientation in trackd's own coordinate system
		Misc::UInt32 dataTimeStamp[2]; // Time stamp of sensor data currently reflected in this structure
		Misc::SInt32 calibrated; // Flag whether the sensor's position and orientation can be considered calibrated
		Misc::SInt32 frame; // ?
		};
	
	struct ControllerHeader // Header structure for trackd's controller data shared memory segment
		{
		/* Elements: */
		public:
		Misc::UInt32 version; // Version number of CAVElib/trackd daemon
		Misc::UInt32 buttonsOffset; // Offset from beginning of shared memory segment to start of button data array in bytes
		Misc::UInt32 valuatorsOffset; // Offset from beginning of shared memory segment to start of valuator data array in bytes
		Misc::UInt32 numButtons; // Number of buttons reported by the trackd daemon
		Misc::UInt32 numValuators; // Number of valuators reported by the trackd daemon
		Misc::UInt32 dataTimeStamp[2]; // Time stamp of button/valuator data currently reflected in respective data arrays
		Misc::UInt32 daemonCommand; // A field to issue commands to the trackd daemon (0 when no commands are pending)
		};
	
	/* Elements: */
	private:
	SensorHeader* volatile sensorHeader; // Pointer to the header structure of trackd's sensor data shared memory segment
	ControllerHeader* volatile controllerHeader; // Pointer to the header structure of trackd's controller data shared memory segment
	SensorData** volatile sensors; // Array of pointers to the entries of the sensor data array
	Misc::SInt32* volatile buttons; // Pointer to the beginning of the button data array
	Misc::Float32* volatile valuators; // Pointer to the beginning of the valuator data array
	std::vector<std::string> buttonNames; // Array of button names for all defined input devices
	std::vector<std::string> valuatorNames; // Array of valuator names for all defined input devices
	OGTransform calibrationTransformation; // A calibration transformation from the trackd daemon's coordinate space to Vrui's physical coordinate space
	double updateInterval; // Maximal time interval between tracker updates in seconds, to account for trackd's lack of event notification
	Threads::Thread spinPollThread; // Background thread to poll the trackd daemon's shared memory segment for device updates to simulate event notification at the cost of 100% CPU utilization
	volatile bool runSpinPollThread; // Flag to shut down the spin polling thread
	
	/* Protected methods from InputDeviceAdapter: */
	protected:
	virtual void createInputDevice(int deviceIndex,const Misc::ConfigurationFileSection& configFileSection);
	
	/* Private methods: */
	void* spinPollThreadMethod(void); // Thread method for the spin polling thread
	
	/* Constructors and destructors: */
	public:
	InputDeviceAdapterTrackd(InputDeviceManager* sInputDeviceManager,const Misc::ConfigurationFileSection& configFileSection);
	virtual ~InputDeviceAdapterTrackd(void);
	
	/* Methods: */
	virtual std::string getFeatureName(const InputDeviceFeature& feature) const;
	virtual int getFeatureIndex(InputDevice* device,const char* featureName) const;
	virtual void updateInputDevices(void);
	};

}

#endif
