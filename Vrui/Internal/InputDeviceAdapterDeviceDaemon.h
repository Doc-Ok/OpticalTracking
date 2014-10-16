/***********************************************************************
InputDeviceAdapterDeviceDaemon - Class to convert from Vrui's own
distributed device driver architecture to Vrui's internal device
representation.
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

#ifndef VRUI_INTERNAL_INPUTDEVICEADAPTERDEVICEDAEMON_INCLUDED
#define VRUI_INTERNAL_INPUTDEVICEADAPTERDEVICEDAEMON_INCLUDED

#include <string>
#include <vector>
#include <Threads/Spinlock.h>
#include <Vrui/Internal/VRDeviceClient.h>
#include <Vrui/Internal/InputDeviceAdapterIndexMap.h>

/* Forward declarations: */
namespace Misc {
class ConfigurationFileSection;
}

namespace Vrui {

class InputDeviceAdapterDeviceDaemon:public InputDeviceAdapterIndexMap
	{
	/* Elements: */
	private:
	VRDeviceClient deviceClient; // Device client delivering "raw" device state
	float motionPredictionDelta; // Motion prediction time interval to apply to tracked devices in seconds
	std::vector<std::string> buttonNames; // Array of button names for all defined input devices
	std::vector<std::string> valuatorNames; // Array of valuator names for all defined input devices
	Threads::Spinlock errorMessageMutex; // Mutex protecting the error message log
	std::vector<std::string> errorMessages; // Log of error messages received from the device client
	
	/* Private methods: */
	static void packetNotificationCallback(VRDeviceClient* client);
	void errorCallback(const VRDeviceClient::ProtocolError& error);
	
	/* Protected methods from InputDeviceAdapter: */
	protected:
	virtual void createInputDevice(int deviceIndex,const Misc::ConfigurationFileSection& configFileSection);
	
	/* Constructors and destructors: */
	public:
	InputDeviceAdapterDeviceDaemon(InputDeviceManager* sInputDeviceManager,const Misc::ConfigurationFileSection& configFileSection); // Creates adapter by connecting to server and initializing Vrui input devices
	virtual ~InputDeviceAdapterDeviceDaemon(void);
	
	/* Methods from InputDeviceAdapter: */
	virtual std::string getFeatureName(const InputDeviceFeature& feature) const;
	virtual int getFeatureIndex(InputDevice* device,const char* featureName) const;
	virtual void updateInputDevices(void);
	};

}

#endif
