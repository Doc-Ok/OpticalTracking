/***********************************************************************
RazerHydraDevice - Class to wrap the low-level Razer Hydra device driver
in a VRDevice.
Copyright (c) 2011-2013 Oliver Kreylos

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

#ifndef RAZERHYDRATRACKER_INCLUDED
#define RAZERHYDRATRACKER_INCLUDED

#include <Misc/Timer.h>
#include <Threads/Mutex.h>
#include <USB/Context.h>

#include <VRDeviceDaemon/VRDevice.h>

/* Forward declarations: */
class RazerHydra;

class RazerHydraDevice:public VRDevice
	{
	/* Embedded classes: */
	private:
	typedef Vrui::VRDeviceState::TrackerState TrackerState; // Type for tracker states
	typedef TrackerState::PositionOrientation PositionOrientation; // Type for tracker position/orientation
	
	/* Elements: */
	USB::Context usbContext; // A USB context for the Razer Hydra device
	RazerHydra* hydra; // Low-level driver for the Razer Hydra device
	Threads::Mutex deviceValuesMutex; // Mutex to serialize access to the device values
	bool reportEvents; // Flag if device is started (there is currently no known way to suspend the Razer Hydra device)
	Misc::Timer timer; // Free-running timer for velocity estimation
	bool notFirstMeasurement; // Flags if a measurement has already been delivered
	PositionOrientation oldPositionOrientations[2]; // Array of old tracker positions/orientations for both sensors
	bool deviceButtonStates[7*2]; // Array of button states for the device's buttons
	float deviceValuatorStates[3*2]; // Array of valuator values for the device's valuators
	TrackerState deviceTrackerStates[2]; // Array of current sensor tracker states
	volatile bool keepRunning; // Flag to shut down the device thread without lock-ups in the USB stack
	
	/* Protected methods: */
	virtual void deviceThreadMethod(void);
	
	/* Constructors and destructors: */
	public:
	RazerHydraDevice(VRDevice::Factory* sFactory,VRDeviceManager* sDeviceManager,Misc::ConfigurationFile& configFile);
	virtual ~RazerHydraDevice(void);
	
	/* Methods: */
	virtual void start(void);
	virtual void stop(void);
	};

#endif
