/***********************************************************************
MouseButtons - Class to feed mouse button presses into the device daemon
architecture to support using wireless USB mice as VR input devices.
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

#ifndef MOUSEBUTTONS_INCLUDED
#define MOUSEBUTTONS_INCLUDED

#include <Misc/File.h>
#include <Threads/Mutex.h>

#include <VRDeviceDaemon/VRDevice.h>

class MouseButtons:public VRDevice
	{
	/* Elements: */
	private:
	Misc::File mouseDeviceFile; // Object representing the mouse device file
	bool reportEvents; // Flag if device is started (mouse button device cannot be disabled in hardware)
	Threads::Mutex buttonStateMutex; // Mutex to serialize access to the button states
	bool buttonStates[3]; // Current button states
	
	/* Protected methods: */
	protected:
	virtual void deviceThreadMethod(void);
	
	/* Constructors and destructors: */
	public:
	MouseButtons(VRDevice::Factory* sFactory,VRDeviceManager* sDeviceManager,Misc::ConfigurationFile& configFile);
	virtual ~MouseButtons(void);
	
	/* Methods: */
	virtual void start(void);
	virtual void stop(void);
	};

#endif
