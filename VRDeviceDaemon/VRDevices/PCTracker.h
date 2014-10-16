/***********************************************************************
PCTracker - Class for communicating with tracking devices on a dedicated
DOS PC.
Copyright (c) 2004-2011 Oliver Kreylos

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

#ifndef PCTRACKER_INCLUDED
#define PCTRACKER_INCLUDED

#include <Misc/Timer.h>
#include <Threads/Mutex.h>
#include <Comm/SerialPort.h>

#include <VRDeviceDaemon/VRDevice.h>

class PCTracker:public VRDevice
	{
	/* Embedded classes: */
	private:
	typedef Vrui::VRDeviceState::TrackerState TrackerState; // Type for tracker states
	typedef TrackerState::PositionOrientation PositionOrientation; // Type for tracker position/orientation
	
	/* Elements: */
	Comm::SerialPort devicePort; // Serial port the dedicated PC is connected to
	double trackerRange; // Maximum range of reported tracker coordinates
	Threads::Mutex deviceValuesMutex; // Mutex to serialize access to the device values
	bool reportEvents; // Flag if device is started (dedicated PC cannot be disabled)
	Misc::Timer* timers; // Array of free-running timers for each tracker for velocity estimation
	bool* notFirstMeasurements; // Array of flags for each tracker if a measurement has already been delivered
	PositionOrientation* oldPositionOrientations; // Array of old tracker positions/orientations
	TrackerState* deviceTrackerStates; // Array of current device tracker states
	
	/* Private methods: */
	short int extractShort(const char* lsb) const; // Extracts a 16-bit signed integer from two bytes of memory (lsb first)
	
	/* Protected methods: */
	protected:
	virtual void deviceThreadMethod(void);
	
	/* Constructors and destructors: */
	public:
	PCTracker(VRDevice::Factory* sFactory,VRDeviceManager* sDeviceManager,Misc::ConfigurationFile& configFile);
	virtual ~PCTracker(void);
	
	/* Methods: */
	virtual void start(void);
	virtual void stop(void);
	};

#endif
