/***********************************************************************
RemoteDevice - Class to daisy-chain device servers on remote machines.
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

#include <VRDeviceDaemon/VRDevices/RemoteDevice.h>

#include <Misc/ThrowStdErr.h>
#include <Misc/Time.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>

#include <VRDeviceDaemon/VRCalibrator.h>
#include <VRDeviceDaemon/VRDeviceManager.h>

/*****************************
Methods of class RemoteDevice:
*****************************/

void RemoteDevice::deviceThreadMethod(void)
	{
	while(true)
		{
		/* Wait for next message: */
		if(pipe.readMessage()==Vrui::VRDevicePipe::PACKET_REPLY) // Just ignore any other messages
			{
			/* Read current server state: */
			state.read(pipe,false);
			
			/* Copy new state into device manager: */
			for(int i=0;i<state.getNumValuators();++i)
				setValuatorState(i,state.getValuatorState(i));
			for(int i=0;i<state.getNumButtons();++i)
				setButtonState(i,state.getButtonState(i));
			for(int i=0;i<state.getNumTrackers();++i)
				setTrackerState(i,state.getTrackerState(i));
			}
		}
	}

RemoteDevice::RemoteDevice(VRDevice::Factory* sFactory,VRDeviceManager* sDeviceManager,Misc::ConfigurationFile& configFile)
	:VRDevice(sFactory,sDeviceManager,configFile),
	 pipe(configFile.retrieveString("./serverName").c_str(),configFile.retrieveValue<int>("./serverPort"))
	{
	/* Initiate connection: */
	#ifdef VERBOSE
	printf("RemoteDevice: Connecting to device server\n");
	fflush(stdout);
	#endif
	pipe.writeMessage(Vrui::VRDevicePipe::CONNECT_REQUEST);
	
	/* Wait for server's reply: */
	if(!pipe.waitForData(Misc::Time(10,0))) // Throw exception if reply does not arrive in time
		Misc::throwStdErr("RemoteDevice: Timeout while waiting for CONNECT_REPLY");
	if(pipe.readMessage()!=Vrui::VRDevicePipe::CONNECT_REPLY)
		Misc::throwStdErr("RemoteDevice: Mismatching message while waiting for CONNECT_REPLY");
	
	/* Read server's layout and initialize current state: */
	state.readLayout(pipe);
	#ifdef VERBOSE
	printf("RemoteDevice: Serving %d trackers, %d buttons, %d valuators\n",state.getNumTrackers(),state.getNumButtons(),state.getNumValuators());
	fflush(stdout);
	#endif
	setNumTrackers(state.getNumTrackers(),configFile);
	setNumButtons(state.getNumButtons(),configFile);
	setNumValuators(state.getNumValuators(),configFile);
	}

RemoteDevice::~RemoteDevice(void)
	{
	/* Disconnect from device server: */
	pipe.writeMessage(Vrui::VRDevicePipe::DISCONNECT_REQUEST);
	}

void RemoteDevice::start(void)
	{
	/* Start device communication thread: */
	startDeviceThread();
	
	/* Activate device server: */
	pipe.writeMessage(Vrui::VRDevicePipe::ACTIVATE_REQUEST);
	pipe.writeMessage(Vrui::VRDevicePipe::STARTSTREAM_REQUEST);
	}

void RemoteDevice::stop(void)
	{
	/* Deactivate device server: */
	pipe.writeMessage(Vrui::VRDevicePipe::STOPSTREAM_REQUEST);
	pipe.writeMessage(Vrui::VRDevicePipe::DEACTIVATE_REQUEST);
	
	/* Stop device communication thread: */
	stopDeviceThread();
	}

/*************************************
Object creation/destruction functions:
*************************************/

extern "C" VRDevice* createObjectRemoteDevice(VRFactory<VRDevice>* factory,VRFactoryManager<VRDevice>* factoryManager,Misc::ConfigurationFile& configFile)
	{
	VRDeviceManager* deviceManager=static_cast<VRDeviceManager::DeviceFactoryManager*>(factoryManager)->getDeviceManager();
	return new RemoteDevice(factory,deviceManager,configFile);
	}

extern "C" void destroyObjectRemoteDevice(VRDevice* device,VRFactory<VRDevice>* factory,VRFactoryManager<VRDevice>* factoryManager)
	{
	delete device;
	}
