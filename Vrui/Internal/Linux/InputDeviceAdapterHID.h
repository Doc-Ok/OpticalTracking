/***********************************************************************
InputDeviceAdapterHID - Linux-specific version of HID input device
adapter.
Copyright (c) 2009-2014 Oliver Kreylos

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

#ifndef VRUI_INTERNAL_LINUX_INPUTDEVICEADAPTERHID_INCLUDED
#define VRUI_INTERNAL_LINUX_INPUTDEVICEADAPTERHID_INCLUDED

#include <string>
#include <vector>
#include <Threads/Mutex.h>
#include <Threads/Thread.h>
#include <Math/BrokenLine.h>
#include <Vrui/Internal/InputDeviceAdapter.h>

/* Forward declarations: */
namespace Vrui {
class InputDevice;
}

namespace Vrui {

class InputDeviceAdapterHID:public InputDeviceAdapter
	{
	/* Embedded classes: */
	private:
	struct Device // Structure describing a human interface device
		{
		/* Embedded classes: */
		public:
		typedef Math::BrokenLine<double> AxisValueMapper; // Type for axis value mappers
		
		/* Elements: */
		int deviceFd; // HID's device file handle
		int firstButtonIndex; // Index of HID's first button in device state array
		int numButtons; // Number of HID's buttons
		std::vector<int> keyMap; // Vector mapping key features to device button indices
		int firstValuatorIndex; // Index of HID's first axis in device state array
		int numValuators; // Number of HID's axes
		std::vector<int> absAxisMap; // Vector mapping absolute axis features to device valuator indices
		std::vector<int> relAxisMap; // Vector mapping relative axis features to device valuator indices
		std::vector<AxisValueMapper> axisValueMappers; // Vector of axis value mappers converting from raw HID axis values to [-1, 1]
		Vrui::InputDevice* trackingDevice; // Pointer to Vrui input device from which this device gets its tracking data
		Vrui::InputDevice* device; // Pointer to Vrui input device associated with the HID
		std::vector<std::string> buttonNames; // Array of button feature names
		std::vector<std::string> valuatorNames; // Array of valuator feature names
		};
	
	/* Elements: */
	private:
	std::vector<Device> devices; // List of human interface devices
	Threads::Mutex deviceStateMutex; // Mutex protecting the device state array
	bool* buttonStates; // Button state array
	double* valuatorStates; // Valuator state array
	Threads::Thread devicePollingThread; // Thread polling the event files of all HIDs
	
	/* Protected methods from InputDeviceAdapter: */
	protected:
	virtual void createInputDevice(int deviceIndex,const Misc::ConfigurationFileSection& configFileSection);
	
	/* New private methods: */
	private:
	void* devicePollingThreadMethod(void); // Method polling the event files of all HIDs
	
	/* Constructors and destructors: */
	public:
	InputDeviceAdapterHID(InputDeviceManager* sInputDeviceManager,const Misc::ConfigurationFileSection& configFileSection); // Creates adapter connected to a set of human interface devices
	virtual ~InputDeviceAdapterHID(void);
	
	/* Methods from InputDeviceAdapter: */
	virtual std::string getFeatureName(const InputDeviceFeature& feature) const;
	virtual int getFeatureIndex(InputDevice* device,const char* featureName) const;
	virtual void updateInputDevices(void);
	};

}

#endif
