/***********************************************************************
PCWand - Class for communicating with button/valuator devices on a
dedicated DOS PC.
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

#ifndef PCWAND_INCLUDED
#define PCWAND_INCLUDED

#include <Threads/Mutex.h>
#include <Comm/SerialPort.h>

#include <VRDeviceDaemon/VRDevice.h>

class PCWand:public VRDevice
	{
	/* Elements: */
	private:
	Comm::SerialPort devicePort; // Serial port the dedicated PC is connected to
	Threads::Mutex deviceValuesMutex; // Mutex to serialize access to the device values
	bool reportEvents; // Flag if device is started (dedicated PC cannot be disabled)
	bool deviceButtonStates[3]; // Array of button states for the device's buttons
	float deviceValuatorStates[2]; // Array of valuator values for the device's valuators
	
	/* Protected methods: */
	protected:
	virtual void deviceThreadMethod(void);
	
	/* Constructors and destructors: */
	public:
	PCWand(VRDevice::Factory* sFactory,VRDeviceManager* sDeviceManager,Misc::ConfigurationFile& configFile);
	virtual ~PCWand(void);
	
	/* Methods: */
	virtual void start(void);
	virtual void stop(void);
	};

#endif
