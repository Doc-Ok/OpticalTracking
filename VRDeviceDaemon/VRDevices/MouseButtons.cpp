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

#include <VRDeviceDaemon/VRDevices/MouseButtons.h>

#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>

#include <VRDeviceDaemon/VRDeviceManager.h>

/*****************************
Methods of class MouseButtons:
*****************************/

void MouseButtons::deviceThreadMethod(void)
	{
	while(true)
		{
		/* Read next mouse packet: */
		signed char packet[3];
		mouseDeviceFile.read(packet,3);
		
		/* Parse mouse packet: */
		{
		Threads::Mutex::Lock buttonStateLock(buttonStateMutex);
		for(int i=0;i<3;++i)
			{
			bool newButtonState=(packet[0]&(1<<i))!=0;
			if(newButtonState!=buttonStates[i]&&reportEvents)
				setButtonState(i,newButtonState);
			buttonStates[i]=newButtonState;
			}
		}
		}
	}

MouseButtons::MouseButtons(VRDevice::Factory* sFactory,VRDeviceManager* sDeviceManager,Misc::ConfigurationFile& configFile)
	:VRDevice(sFactory,sDeviceManager,configFile),
	 mouseDeviceFile(configFile.retrieveString("./mouseDeviceFile").c_str(),"rb",Misc::File::DontCare),
	 reportEvents(false)
	{
	/* Set device's layout: */
	setNumButtons(3,configFile);
	
	/* Initialize button states: */
	for(int i=0;i<3;++i)
		buttonStates[i]=false;
	
	/* Start device thread (mouse button device cannot be disabled): */
	startDeviceThread();
	}

MouseButtons::~MouseButtons(void)
	{
	/* Stop device thread (mouse button device cannot be disabled): */
	{
	Threads::Mutex::Lock buttonStateLock(buttonStateMutex);
	stopDeviceThread();
	}
	}

void MouseButtons::start(void)
	{
	/* Set device manager's button states to current button states: */
	{
	Threads::Mutex::Lock buttonStateLock(buttonStateMutex);
	for(int i=0;i<3;++i)
		setButtonState(i,buttonStates[i]);
	
	/* Start reporting events to the device manager: */
	reportEvents=true;
	}
	}

void MouseButtons::stop(void)
	{
	/* Stop reporting events to the device manager: */
	{
	Threads::Mutex::Lock buttonStateLock(buttonStateMutex);
	reportEvents=false;
	}
	}

/*************************************
Object creation/destruction functions:
*************************************/

extern "C" VRDevice* createObjectMouseButtons(VRFactory<VRDevice>* factory,VRFactoryManager<VRDevice>* factoryManager,Misc::ConfigurationFile& configFile)
	{
	VRDeviceManager* deviceManager=static_cast<VRDeviceManager::DeviceFactoryManager*>(factoryManager)->getDeviceManager();
	return new MouseButtons(factory,deviceManager,configFile);
	}

extern "C" void destroyObjectMouseButtons(VRDevice* device,VRFactory<VRDevice>* factory,VRFactoryManager<VRDevice>* factoryManager)
	{
	delete device;
	}
