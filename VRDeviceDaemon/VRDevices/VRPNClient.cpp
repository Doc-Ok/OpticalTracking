/***********************************************************************
VRPNClient - Class to receive input device data from a remote VRPN
server.
Copyright (c) 2008-2010 Oliver Kreylos

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

#include <VRDeviceDaemon/VRDevices/VRPNClient.h>

#include <stdio.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/CompoundValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Geometry/GeometryValueCoders.h>

#include <VRDeviceDaemon/VRDeviceManager.h>

/***************************
Methods of class VRPNClient:
***************************/

void VRPNClient::deviceThreadMethod(void)
	{
	/* Read messages from the VRPN server until interrupted: */
	while(true)
		readNextMessages();
	}

void VRPNClient::updateTrackerPosition(int trackerIndex,const PositionOrientation& positionOrientation)
	{
	/* Check if the current position/orientation is the "fallback position:" */
	bool fallbackPosition=true;
	for(int i=0;i<3&&fallbackPosition;++i)
		fallbackPosition=positionOrientation.getTranslation()[i]==0.0;
	for(int i=0;i<3&&fallbackPosition;++i)
		fallbackPosition=positionOrientation.getRotation().getQuaternion()[i]==0.0;
	if(fallbackPosition)
		fallbackPosition=positionOrientation.getRotation().getQuaternion()[3]==1.0;
	
	/* Only update the current position if the new position is not the fallback: */
	if(!fallbackPosition)
		trackerStates[trackerIndex].positionOrientation=positionOrientation;
	
	/* Notify the device manager that an update has arrived (even if it was ignored): */
	trackerFlags[trackerIndex]|=0x1;
	// if(trackerFlags[trackerIndex]==0x3)
		{
		if(reportEvents)
			setTrackerState(trackerIndex,trackerStates[trackerIndex]);
		trackerFlags[trackerIndex]=0x0;
		}
	}

void VRPNClient::updateTrackerVelocity(int trackerIndex,const LinearVelocity& linearVelocity,const AngularVelocity& angularVelocity)
	{
	trackerStates[trackerIndex].linearVelocity=linearVelocity;
	trackerStates[trackerIndex].angularVelocity=angularVelocity;
	trackerFlags[trackerIndex]|=0x2;
	// if(trackerFlags[trackerIndex]==0x3)
		{
		if(reportEvents)
			setTrackerState(trackerIndex,trackerStates[trackerIndex]);
		trackerFlags[trackerIndex]=0x0;
		}
	}

void VRPNClient::updateButtonState(int buttonIndex,ButtonState newState)
	{
	buttonStates[buttonIndex]=newState;
	if(reportEvents)
		setButtonState(buttonIndex,newState);
	}

void VRPNClient::updateValuatorState(int valuatorIndex,ValuatorState newState)
	{
	valuatorStates[valuatorIndex]=newState;
	if(reportEvents)
		setValuatorState(valuatorIndex,newState);
	}

void VRPNClient::finalizePacket(void)
	{
	/* Tell the VR device manager that the current state has updated completely: */
	updateState();
	}

VRPNClient::VRPNClient(VRDevice::Factory* sFactory,VRDeviceManager* sDeviceManager,Misc::ConfigurationFile& configFile)
	:VRDevice(sFactory,sDeviceManager,configFile),
	 VRPNConnection(configFile.retrieveString("./serverName").c_str(),configFile.retrieveValue<int>("./serverPort",3883)),
	 reportEvents(false),
	 trackerStates(0),trackerFlags(0),buttonStates(0),valuatorStates(0)
	{
	#ifdef VERBOSE
	printf("VRPNClient: Initializing senders...");
	fflush(stdout);
	#endif
	
	/* Check if the z axis if incoming position data needs to be flipped: */
	setFlipZAxis(configFile.retrieveValue<bool>("./flipZAxis",false));
	
	/* Retrieve list of sender names: */
	typedef std::vector<std::string> StringList;
	StringList senderNames=configFile.retrieveValue<StringList>("./senderNames");
	
	/* Process all senders: */
	int totalNumTrackers=0;
	int totalNumButtons=0;
	int totalNumValuators=0;
	for(StringList::const_iterator snIt=senderNames.begin();snIt!=senderNames.end();++snIt)
		{
		/* Go to the sender's section: */
		configFile.setCurrentSection(snIt->c_str());
		
		/* Read the number of trackers, buttons, and valuators for this sender: */
		int numTrackers=configFile.retrieveValue<int>("./numTrackers",0);
		if(numTrackers>0)
			{
			requestTrackers(snIt->c_str(),totalNumTrackers,numTrackers);
			totalNumTrackers+=numTrackers;
			}
		int numButtons=configFile.retrieveValue<int>("./numButtons",0);
		if(numButtons>0)
			{
			requestButtons(snIt->c_str(),totalNumButtons,numButtons);
			totalNumButtons+=numButtons;
			}
		int numValuators=configFile.retrieveValue<int>("./numValuators",0);
		if(numValuators>0)
			{
			requestValuators(snIt->c_str(),totalNumValuators,numValuators);
			totalNumValuators+=numValuators;
			}
		
		/* Go back to device's section: */
		configFile.setCurrentSection("..");
		}
	
	#ifdef VERBOSE
	printf(" done\n");
	fflush(stdout);
	#endif
	
	/* Set number of trackers, buttons, and valuators: */
	setNumTrackers(totalNumTrackers,configFile);
	setNumButtons(totalNumButtons,configFile);
	setNumValuators(totalNumValuators,configFile);
	
	/* Read the initial position/orientation for all trackers: */
	PositionOrientation defaultPosition=configFile.retrieveValue<PositionOrientation>("./defaultPosition",PositionOrientation::identity);
	
	/* Initialize the local state arrays: */
	trackerStates=new TrackerState[getNumTrackers()];
	trackerFlags=new int[getNumTrackers()];
	for(int i=0;i<getNumTrackers();++i)
		{
		trackerStates[i].positionOrientation=defaultPosition;
		trackerStates[i].linearVelocity=LinearVelocity::zero;
		trackerStates[i].angularVelocity=AngularVelocity::zero;
		trackerFlags[i]=0x0;
		}
	buttonStates=new ButtonState[getNumButtons()];
	for(int i=0;i<getNumButtons();++i)
		buttonStates[i]=false;
	valuatorStates=new ValuatorState[getNumValuators()];
	for(int i=0;i<getNumValuators();++i)
		valuatorStates[i]=ValuatorState(0);
	
	/* Start device communication thread: */
	startDeviceThread();
	}

VRPNClient::~VRPNClient(void)
	{
	/* Stop device communication thread: */
	{
	Threads::Mutex::Lock stateLock(stateMutex);
	stopDeviceThread();
	}
	
	delete[] trackerStates;
	delete[] buttonStates;
	delete[] valuatorStates;
	}

void VRPNClient::start(void)
	{
	/* Set device manager's states to current states: */
	{
	Threads::Mutex::Lock stateLock(stateMutex);
	for(int i=0;i<getNumTrackers();++i)
		setTrackerState(i,trackerStates[i]);
	for(int i=0;i<getNumButtons();++i)
		setButtonState(i,buttonStates[i]);
	for(int i=0;i<getNumValuators();++i)
		setValuatorState(i,valuatorStates[i]);
	
	/* Start reporting events to the device manager: */
	reportEvents=true;
	}
	}

void VRPNClient::stop(void)
	{
	/* Stop reporting events to the device manager: */
	{
	Threads::Mutex::Lock stateLock(stateMutex);
	reportEvents=false;
	}
	}

/*************************************
Object creation/destruction functions:
*************************************/

extern "C" VRDevice* createObjectVRPNClient(VRFactory<VRDevice>* factory,VRFactoryManager<VRDevice>* factoryManager,Misc::ConfigurationFile& configFile)
	{
	VRDeviceManager* deviceManager=static_cast<VRDeviceManager::DeviceFactoryManager*>(factoryManager)->getDeviceManager();
	return new VRPNClient(factory,deviceManager,configFile);
	}

extern "C" void destroyObjectVRPNClient(VRDevice* device,VRFactory<VRDevice>* factory,VRFactoryManager<VRDevice>* factoryManager)
	{
	delete device;
	}
