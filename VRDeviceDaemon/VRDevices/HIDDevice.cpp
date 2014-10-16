/***********************************************************************
HIDDevice - VR device driver class for generic input devices supported
by the Linux or MacOS X HID event interface. Reports buttons and
absolute axes.
Copyright (c) 2004-2010 Oliver Kreylos
MacOS X additions copyright (c) 2006 Braden Pellett

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

#include <VRDeviceDaemon/VRDevices/HIDDevice.h>

#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Misc/ThrowStdErr.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/CompoundValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Math/Math.h>
#include <Math/MathValueCoders.h>

#include <VRDeviceDaemon/VRDeviceManager.h>

/*************************
System specific functions:
*************************/

#ifdef __linux__
#include <VRDeviceDaemon/VRDevices/Linux/HIDDevice.cpp>
#endif
#ifdef __APPLE__
#include <VRDeviceDaemon/VRDevices/MacOSX/HIDDevice.cpp>
#endif

/*************************************
Object creation/destruction functions:
*************************************/

extern "C" VRDevice* createObjectHIDDevice(VRFactory<VRDevice>* factory,VRFactoryManager<VRDevice>* factoryManager,Misc::ConfigurationFile& configFile)
	{
	VRDeviceManager* deviceManager=static_cast<VRDeviceManager::DeviceFactoryManager*>(factoryManager)->getDeviceManager();
	return new HIDDevice(factory,deviceManager,configFile);
	}

extern "C" void destroyObjectHIDDevice(VRDevice* device,VRFactory<VRDevice>* factory,VRFactoryManager<VRDevice>* factoryManager)
	{
	delete device;
	}
