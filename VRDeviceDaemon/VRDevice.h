/***********************************************************************
VRDevice - Abstract base class for hardware devices delivering
position, orientation, button events and valuator values.
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

#ifndef VRDEVICE_INCLUDED
#define VRDEVICE_INCLUDED

#include <Realtime/Time.h>
#include <Threads/Thread.h>
#include <Geometry/OrthonormalTransformation.h>
#include <Vrui/Internal/VRDeviceState.h>

/* Forward declarations: */
namespace Misc {
class ConfigurationFile;
}
namespace Vrui {
class VRDeviceDescriptor;
}
template <class BaseClassParam>
class VRFactory;
class VRCalibrator;
class VRDeviceManager;

class VRDevice
	{
	/* Embedded classes: */
	public:
	typedef VRFactory<VRDevice> Factory;
	typedef Geometry::OrthonormalTransformation<float,3> TrackerPostTransformation;
	
	/* Elements: */
	private:
	Factory* factory; // Pointer to factory that created this object
	
	protected:
	int numTrackers; // Number of trackers (position reporting devices) connected to device
	int numButtons; // Number of buttons connected to device
	int numValuators; // Number of valuators (non-positional analog dials) connected to device
	private:
	int* trackerIndices; // Mapping from device tracker indices to "logical" tracker indices
	TrackerPostTransformation* trackerPostTransformations; // Array of transformations to apply to calibrated tracker measurements
	int* buttonIndices; // Mapping from device button indices to "logical" button indices
	int* valuatorIndices; // Mapping from device valuator indices to "logical" valuator indices
	float* valuatorThresholds; // Array of threshold values around zero for broken-line value mapping
	float* valuatorExponents; // Array of exponent values for non-linear value mapping
	bool active; // Flag if device is currently active
	Threads::Thread deviceThread; // Device communication thread
	VRDeviceManager* deviceManager; // Manager gathering data from VR devices
	VRCalibrator* calibrator; // Calibrator for tracker measurements
	
	/* Private methods: */
	void* deviceThreadMethodWrapper(void); // Wrapper method for the virtual device thread
	
	/* Protected methods: */
	protected:
	void setNumTrackers(int newNumTrackers,const Misc::ConfigurationFile& configFile,const std::string* trackerNames =0); // Sets number of trackers
	void setNumButtons(int newNumButtons,const Misc::ConfigurationFile& configFile,const std::string* buttonNames =0); // Sets number of buttons
	void setNumValuators(int newNumValuators,const Misc::ConfigurationFile& configFile,const std::string* valuatorNames =0); // Sets number of valuators
	void addVirtualDevice(Vrui::VRDeviceDescriptor* newDevice); // Passes the given new virtual input device to the device manager
	void calcVelocities(int deviceTrackerIndex,Vrui::VRDeviceState::TrackerState& newState); // Calculates tracker velocities based on elapsed time since last measurement
	void setTrackerState(int deviceTrackerIndex,const Vrui::VRDeviceState::TrackerState& state,Vrui::VRDeviceState::TimeStamp timeStamp); // Sets (and calibrates) a tracker (device index given)
	void setTrackerState(int deviceTrackerIndex,const Vrui::VRDeviceState::TrackerState& state) // Ditto, using current time as time stamp
		{
		Realtime::TimePointMonotonic now;
		Vrui::VRDeviceState::TimeStamp nowTs=Vrui::VRDeviceState::TimeStamp(now.tv_sec*1000000+(now.tv_nsec+500)/1000);
		setTrackerState(deviceTrackerIndex,state,nowTs);
		}
	void setButtonState(int deviceButtonIndex,Vrui::VRDeviceState::ButtonState newState); // Sets a button state (device index given)
	void setValuatorState(int deviceValuatorIndex,Vrui::VRDeviceState::ValuatorState newState); // Sets a valuator state (device index given)
	void updateState(void); // Notifies the device manager that this device's state can be sent to clients
	void startDeviceThread(void); // Starts the device communication thread
	void stopDeviceThread(bool cancel =true); // Stops the device communication thread; if flag is true, thread will be cancelled
	virtual void deviceThreadMethod(void); // Thread to communicate to device hardware
	
	/* Constructors and destructors: */
	public:
	VRDevice(Factory* sFactory,VRDeviceManager* sDeviceManager,Misc::ConfigurationFile& configFile); // Initializes device by reading current section of configuration file
	virtual ~VRDevice(void); // Deactivates and deinitializes device
	static void destroy(VRDevice* object); // Deletes an object
	
	/* Methods: */
	int getNumTrackers(void) const // Returns number of trackers
		{
		return numTrackers;
		};
	int getTrackerIndex(int deviceTrackerIndex) const // Maps device tracker index to logical tracker index
		{
		return trackerIndices[deviceTrackerIndex];
		};
	int getNumButtons(void) const // Returns number of buttons
		{
		return numButtons;
		};
	int getButtonIndex(int deviceButtonIndex) const // Maps device button index to logical button index
		{
		return buttonIndices[deviceButtonIndex];
		};
	int getNumValuators(void) const // Returns number of valuators
		{
		return numValuators;
		};
	int getValuatorIndex(int deviceValuatorIndex) const // Maps device valuator index to logical valuator index
		{
		return valuatorIndices[deviceValuatorIndex];
		};
	bool isActive(void) const // Returns true if the device is currently active, i.e., device thread is running
		{
		return active;
		}
	virtual void start(void) =0; // Starts tracking hardware and position reporting
	virtual void stop(void) =0; // Stops tracking hardware and position reporting
	};

#endif
