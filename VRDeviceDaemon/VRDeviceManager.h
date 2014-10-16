/***********************************************************************
VRDeviceManager - Class to gather position, button and valuator data
from one or several VR devices and associate them with logical input
devices.
Copyright (c) 2002-2014 Oliver Kreylos

This file is part of the Vrui VR Device Driver Daemon (VRDeviceDaemon).

The Vrui VR Device Driver Daemon is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Vrui VR Device Driver Daemon is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Vrui VR Device Driver Daemon; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#ifndef VRDEVICEMANAGER_INCLUDED
#define VRDEVICEMANAGER_INCLUDED

#include <string>
#include <Threads/Mutex.h>
#include <Threads/MutexCond.h>
#include <Vrui/Internal/VRDeviceState.h>

#include <VRDeviceDaemon/VRFactoryManager.h>

/* Forward declarations: */
namespace Misc {
class ConfigurationFile;
}
namespace Vrui {
class VRDeviceDescriptor;
}
class VRDevice;
class VRCalibrator;

class VRDeviceManager
	{
	/* Embedded classes: */
	public:
	class DeviceFactoryManager:public VRFactoryManager<VRDevice>
		{
		/* Elements: */
		private:
		VRDeviceManager* deviceManager; // Pointer to device manager "owning" this factory manager
		
		/* Constructors and destructors: */
		public:
		DeviceFactoryManager(const std::string& sDsoPath,VRDeviceManager* sDeviceManager)
			:VRFactoryManager<VRDevice>(sDsoPath),
			 deviceManager(sDeviceManager)
			{
			};
		
		/* Methods: */
		VRDeviceManager* getDeviceManager(void) const // Returns pointer to device manager
			{
			return deviceManager;
			};
		};
	
	typedef VRFactoryManager<VRCalibrator> CalibratorFactoryManager;
	
	/* Elements: */
	private:
	DeviceFactoryManager deviceFactories; // Factory manager to load VR device classes
	CalibratorFactoryManager calibratorFactories; // Factory manager to load VR calibrator classes
	int numDevices; // Number of managed devices
	VRDevice** devices; // Array of pointers to VR devices
	int* trackerIndexBases; // Array of base tracker indices for each VR device
	int* buttonIndexBases; // Array of base button indices for each VR device
	int* valuatorIndexBases; // Array of base valuator indices for each VR device
	int currentDeviceIndex; // Index of currently constructed device during initialization
	std::vector<std::string> trackerNames; // List of tracker names
	std::vector<std::string> buttonNames; // List of button names
	std::vector<std::string> valuatorNames; // List of valuator names
	Threads::Mutex stateMutex; // Mutex serializing access to all state elements
	Vrui::VRDeviceState state; // Current state of all managed devices
	std::vector<Vrui::VRDeviceDescriptor*> virtualDevices; // List of virtual devices combining selected trackers, buttons, and valuators
	unsigned int fullTrackerReportMask; // Bitmask containing 1-bits for all used logical tracker indices
	unsigned int trackerReportMask; // Bitmask of logical tracker indices that have reported state
	bool trackerUpdateNotificationEnabled; // Flag if update notification is enabled
	Threads::MutexCond* trackerUpdateCompleteCond; // Condition variable to notify client threads that all tracker states has been updated
	
	/* Constructors and destructors: */
	public:
	VRDeviceManager(Misc::ConfigurationFile& configFile); // Creates device manager by reading current section of configuration file
	~VRDeviceManager(void);
	
	/* Methods to communicate with device driver modules during initialization: */
	int getTrackerIndexBase(void) const // Returns the tracker index base for the currently constructed device
		{
		return trackerIndexBases[currentDeviceIndex];
		}
	int getButtonIndexBase(void) const // Returns the button index base for the currently constructed device
		{
		return buttonIndexBases[currentDeviceIndex];
		}
	int getValuatorIndexBase(void) const // Returns the valuator index base for the currently constructed device
		{
		return valuatorIndexBases[currentDeviceIndex];
		}
	int addTracker(const char* name =0); // Adds a new tracker to the manager's namespace; returns tracker index
	int addButton(const char* name =0); // Adds a new button to the manager's namespace; returns button index
	int addValuator(const char* name =0); // Adds a new valuator to the manager's namespace; returns valuator index
	VRCalibrator* createCalibrator(const std::string& calibratorType,Misc::ConfigurationFile& configFile); // Loads calibrator of given type from current section in configuration file
	void addVirtualDevice(Vrui::VRDeviceDescriptor* newVirtualDevice); // Adds a virtual device; is adopted by device manager
	
	/* Methods to communicate with device driver modules during operation: */
	void setTrackerState(int trackerIndex,const Vrui::VRDeviceState::TrackerState& newTrackerState,Vrui::VRDeviceState::TimeStamp newTimeStamp); // Updates state of single tracker
	void setButtonState(int buttonIndex,Vrui::VRDeviceState::ButtonState newButtonState); // Updates state of single button
	void setValuatorState(int valuatorIndex,Vrui::VRDeviceState::ValuatorState newValuatorState); // Updates state of single valuator
	void updateState(void); // Tells device manager that the current state should be considered "complete"
	
	/* Methods to communicate with device server: */
	int getNumVirtualDevices(void) const // Returns the number of managed virtual input devices
		{
		return int(virtualDevices.size());
		}
	const Vrui::VRDeviceDescriptor& getVirtualDevice(int deviceIndex) const // Returns the virtual input device of the given index
		{
		return *(virtualDevices[deviceIndex]);
		}
	void lockState(void) // Locks current device states
		{
		stateMutex.lock();
		};
	void unlockState(void) // Unlocks current device states
		{
		stateMutex.unlock();
		};
	Vrui::VRDeviceState& getState(void) // Returns current state of all managed devices (state must be locked while being used)
		{
		return state;
		};
	void enableTrackerUpdateNotification(Threads::MutexCond* sTrackerUpdateCompleteCond); // Sets a condition variable to be signalled when all trackers have updated
	void disableTrackerUpdateNotification(void); // Disables tracker update notification
	void start(void); // Starts device processing
	void stop(void); // Stops device processing
	};

#endif
