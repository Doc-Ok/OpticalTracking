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

#include <VRDeviceDaemon/VRDevices/DummyDevice.h>

#include <unistd.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Geometry/GeometryValueCoders.h>

#include <VRDeviceDaemon/VRDeviceManager.h>

/****************************
Methods of class DummyDevice:
****************************/

void DummyDevice::deviceThreadMethod(void)
	{
	while(true)
		{
		/* Wait for the next status update: */
		usleep(sleepTime);
		
		/* 'Update' the device manager's state: */
		for(int i=0;i<state.getNumButtons();++i)
			setButtonState(i,state.getButtonState(i));
		for(int i=0;i<state.getNumValuators();++i)
			setValuatorState(i,state.getValuatorState(i));
		for(int i=0;i<state.getNumTrackers();++i)
			setTrackerState(i,state.getTrackerState(i));
		}
	}

DummyDevice::DummyDevice(VRDevice::Factory* sFactory,VRDeviceManager* sDeviceManager,Misc::ConfigurationFile& configFile)
	:VRDevice(sFactory,sDeviceManager,configFile),
	 sleepTime(configFile.retrieveValue<int>("./sleepTime"))
	{
	/* Read device layout: */
	int numTrackers=configFile.retrieveValue<int>("./numTrackers",0);
	int numButtons=configFile.retrieveValue<int>("./numButtons",0);
	int numValuators=configFile.retrieveValue<int>("./numValuators",0);
	setNumTrackers(numTrackers,configFile);
	setNumButtons(numButtons,configFile);
	setNumValuators(numValuators,configFile);
	state.setLayout(numTrackers,numButtons,numValuators);
	
	/* Read fake state data: */
	for(int i=0;i<numTrackers;++i)
		{
		char trackerStateName[40];
		sprintf(trackerStateName,"./trackerState%d",i);
		Vrui::VRDeviceState::TrackerState ts;
		ts.positionOrientation=configFile.retrieveValue<Vrui::VRDeviceState::TrackerState::PositionOrientation>(trackerStateName,Vrui::VRDeviceState::TrackerState::PositionOrientation::identity);
		ts.linearVelocity=Vrui::VRDeviceState::TrackerState::LinearVelocity::zero;
		ts.angularVelocity=Vrui::VRDeviceState::TrackerState::AngularVelocity::zero;
		state.setTrackerState(i,ts);
		}
	for(int i=0;i<numButtons;++i)
		{
		char buttonStateName[40];
		sprintf(buttonStateName,"./buttonState%d",i);
		state.setButtonState(i,configFile.retrieveValue<Vrui::VRDeviceState::ButtonState>(buttonStateName,Vrui::VRDeviceState::ButtonState(false)));
		}
	for(int i=0;i<numValuators;++i)
		{
		char valuatorStateName[40];
		sprintf(valuatorStateName,"./valuatorState%d",i);
		state.setValuatorState(i,configFile.retrieveValue<Vrui::VRDeviceState::ValuatorState>(valuatorStateName,Vrui::VRDeviceState::ValuatorState(0)));
		}
	}

void DummyDevice::start(void)
	{
	/* Start device update thread: */
	startDeviceThread();
	}

void DummyDevice::stop(void)
	{
	/* Stop device update thread: */
	stopDeviceThread();
	}

/*************************************
Object creation/destruction functions:
*************************************/

extern "C" VRDevice* createObjectDummyDevice(VRFactory<VRDevice>* factory,VRFactoryManager<VRDevice>* factoryManager,Misc::ConfigurationFile& configFile)
	{
	VRDeviceManager* deviceManager=static_cast<VRDeviceManager::DeviceFactoryManager*>(factoryManager)->getDeviceManager();
	return new DummyDevice(factory,deviceManager,configFile);
	}

extern "C" void destroyObjectDummyDevice(VRDevice* device,VRFactory<VRDevice>* factory,VRFactoryManager<VRDevice>* factoryManager)
	{
	delete device;
	}
