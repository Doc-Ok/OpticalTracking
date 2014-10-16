/***********************************************************************
DummyDevice - Class for devices reporting constant states.
Copyright (c) 2002-2010 Oliver Kreylos

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

#ifndef DUMMYDEVICE_INCLUDED
#define DUMMYDEVICE_INCLUDED

#include <Vrui/Internal/VRDeviceState.h>

#include <VRDeviceDaemon/VRDevice.h>

class DummyDevice:public VRDevice
	{
	/* Elements: */
	private:
	Vrui::VRDeviceState state; // State of all simulated devices
	unsigned long sleepTime; // Time between "state updates" in microseconds
	
	/* Protected methods: */
	virtual void deviceThreadMethod(void);
	
	/* Constructors and destructors: */
	public:
	DummyDevice(VRDevice::Factory* sFactory,VRDeviceManager* sDeviceManager,Misc::ConfigurationFile& configFile);
	
	/* Methods: */
	virtual void start(void);
	virtual void stop(void);
	};

#endif
