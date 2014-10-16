/***********************************************************************
Joystick - VR device driver class for joysticks having arbitrary numbers
of axes and buttons using the Linux joystick driver API (thanks
Vojtech).
Copyright (c) 2004-2010 Oliver Kreylos

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

#ifndef JOYSTICK_INCLUDED
#define JOYSTICK_INCLUDED

#include <Threads/Mutex.h>

#include <VRDeviceDaemon/VRDevice.h>

class Joystick:public VRDevice
	{
	/* Elements: */
	private:
	int joystickDeviceFd; // File descriptor for the joystick device
	float* axisGains; // Array of gain values for the joystick's axes
	bool reportEvents; // Flag if device is started (joystick device cannot be disabled in hardware)
	Threads::Mutex stateMutex; // Mutex to serialize access to the joystick state
	bool* buttonStates; // Array of current button states
	float* valuatorStates; // Array of current valuator values
	
	/* Protected methods: */
	virtual void deviceThreadMethod(void);
	
	/* Constructors and destructors: */
	public:
	Joystick(VRDevice::Factory* sFactory,VRDeviceManager* sDeviceManager,Misc::ConfigurationFile& configFile);
	virtual ~Joystick(void);
	
	/* Methods: */
	virtual void start(void);
	virtual void stop(void);
	};

#endif
