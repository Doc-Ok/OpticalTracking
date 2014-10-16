/***********************************************************************
RazerHydraDevice - Class to wrap the low-level Razer Hydra device driver
in a VRDevice.
Copyright (c) 2011-2013 Oliver Kreylos

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

#include <VRDeviceDaemon/VRDevices/RazerHydraDevice.h>

#include <Misc/FixedArray.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/ArrayValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Vrui/Internal/VRDeviceDescriptor.h>

#include <VRDeviceDaemon/VRDeviceManager.h>
#include <VRDeviceDaemon/VRDevices/RazerHydra.h>

/*********************************
Methods of class RazerHydraDevice:
*********************************/

void RazerHydraDevice::deviceThreadMethod(void)
	{
	typedef PositionOrientation::Vector Vector;
	typedef PositionOrientation::Rotation Rotation;
	
	/* Reset first measurement flag: */
	notFirstMeasurement=false;
	
	while(keepRunning)
		{
		/* Wait for the next update message: */
		RazerHydra::SensorState sensorStates[2];
		hydra->pollSensors(sensorStates);
		
		/* Update the device tracker state structures: */
		{
		Threads::Mutex::Lock deviceValuesLock(deviceValuesMutex);
		timer.elapse();
		for(int sensor=0;sensor<2;++sensor)
			{
			/* Copy the sensor's button and valuator states: */
			for(int i=0;i<7;++i)
				deviceButtonStates[sensor*7+i]=sensorStates[sensor].buttonStates[i];
			for(int i=0;i<3;++i)
				deviceValuatorStates[sensor*3+i]=sensorStates[sensor].valuatorStates[i];
			
			/* Copy the sensor's position and orientation: */
			Vector t=Vector(sensorStates[sensor].position);
			Rotation r=sensorStates[sensor].orientation;
			deviceTrackerStates[sensor].positionOrientation=PositionOrientation(t,r);
			
			/* Calculate linear and angular velocities: */
			if(notFirstMeasurement)
				{
				/* Estimate velocities by dividing position/orientation differences by elapsed time since last measurement: */
				double time=timer.getTime();
				deviceTrackerStates[sensor].linearVelocity=(t-oldPositionOrientations[sensor].getTranslation())/TrackerState::LinearVelocity::Scalar(time);
				Rotation dR=r*Geometry::invert(oldPositionOrientations[sensor].getRotation());
				deviceTrackerStates[sensor].angularVelocity=dR.getScaledAxis()/TrackerState::AngularVelocity::Scalar(time);
				}
			else
				{
				/* Force initial velocities to zero: */
				deviceTrackerStates[sensor].linearVelocity=TrackerState::LinearVelocity::zero;
				deviceTrackerStates[sensor].angularVelocity=TrackerState::AngularVelocity::zero;
				notFirstMeasurement=true;
				}
			oldPositionOrientations[sensor]=deviceTrackerStates[sensor].positionOrientation;
			}
		
		/* Update device state in device manager: */
		if(reportEvents)
			{
			for(int i=0;i<7*2;++i)
				setButtonState(i,deviceButtonStates[i]);
			for(int i=0;i<3*2;++i)
				setValuatorState(i,deviceValuatorStates[i]);
			for(int i=0;i<2;++i)
				setTrackerState(i,deviceTrackerStates[i]);
			}
		}
		}
	}

RazerHydraDevice::RazerHydraDevice(VRDevice::Factory* sFactory,VRDeviceManager* sDeviceManager,Misc::ConfigurationFile& configFile)
	:VRDevice(sFactory,sDeviceManager,configFile),
	 hydra(0),
	 reportEvents(false),
	 notFirstMeasurement(false),
	 keepRunning(true)
	{
	/* Set device configuration: */
	setNumButtons(7*2,configFile);
	setNumValuators(3*2,configFile);
	setNumTrackers(2,configFile);
	
	/* Open the Razer Hydra device: */
	hydra=new RazerHydra(usbContext,configFile.retrieveValue<unsigned int>("./deviceIndex",0));
	
	/* Set the position unit: */
	if(configFile.retrieveValue<bool>("./unitInches",false))
		hydra->setPositionInches();
	else if(configFile.retrieveValue<bool>("./unitMMs",false))
		hydra->setPositionMMs();
	else
		hydra->setPositionConversionFactor(configFile.retrieveValue<RazerHydra::Scalar>("./unitFactor",RazerHydra::Scalar(1)));
	
	/* Set filtering parameters: */
	hydra->setApplyInterleaveFilter(configFile.retrieveValue<bool>("./applyInterleaveFilter",true));
	hydra->setApplyLowpassFilter(configFile.retrieveValue<bool>("./applyLowpassFilter",true));
	hydra->setLowpassFilterStrength(configFile.retrieveValue<RazerHydra::Scalar>("./lowpassFilterStrength",RazerHydra::Scalar(24)));
	
	/* Initialize device states: */
	for(int i=0;i<7*2;++i)
		deviceButtonStates[i]=false;
	for(int i=0;i<3*2;++i)
		deviceValuatorStates[i]=0.0f;
	for(int i=0;i<2;++i)
		{
		deviceTrackerStates[i].positionOrientation=PositionOrientation::identity;
		deviceTrackerStates[i].linearVelocity=TrackerState::LinearVelocity::zero;
		deviceTrackerStates[i].angularVelocity=TrackerState::AngularVelocity::zero;
		}
	
	/* Create a virtual device: */
	Misc::FixedArray<std::string,2> deviceNames;
	deviceNames[0]="RazerHydraLeft";
	deviceNames[1]="RazerHydraRight";
	deviceNames=configFile.retrieveValue<Misc::FixedArray<std::string,2> >("./deviceNames",deviceNames);
	for(int i=0;i<2;++i)
		{
		Vrui::VRDeviceDescriptor* vd=new Vrui::VRDeviceDescriptor(7,3);
		vd->name=deviceNames[i];
		vd->trackType=Vrui::VRDeviceDescriptor::TRACK_POS|Vrui::VRDeviceDescriptor::TRACK_DIR|Vrui::VRDeviceDescriptor::TRACK_ORIENT;
		vd->rayDirection=Vrui::VRDeviceDescriptor::Vector(0,1,0);
		vd->rayStart=0.0f;
		vd->trackerIndex=getTrackerIndex(i);
		
		vd->buttonNames[0]=i==0?"LB":"RB";
		vd->buttonNames[1]="3";
		vd->buttonNames[2]="1";
		vd->buttonNames[3]="2";
		vd->buttonNames[4]="4";
		vd->buttonNames[5]="Center";
		vd->buttonNames[6]="Stick";
		for(int j=0;j<7;++j)
			vd->buttonIndices[j]=getButtonIndex(i*7+j);
		
		vd->valuatorNames[0]="StickX";
		vd->valuatorNames[1]="StickY";
		vd->valuatorNames[2]=i==0?"LT":"RT";
		for(int j=0;j<3;++j)
			vd->valuatorIndices[j]=getValuatorIndex(i*3+j);
		
		addVirtualDevice(vd);
		}
	
	/* Start device thread (Razer Hydra device cannot be suspended and runs the entire time): */
	startDeviceThread();
	}

RazerHydraDevice::~RazerHydraDevice(void)
	{
	/* Stop device thread (Razer Hydra device cannot be suspended and runs the entire time): */
	keepRunning=false;
	stopDeviceThread(false);
	
	/* Close the Razer Hydra device: */
	delete hydra;
	}

void RazerHydraDevice::start(void)
	{
	/* Set device manager's device states to current device values: */
	Threads::Mutex::Lock deviceValuesLock(deviceValuesMutex);
	for(int i=0;i<7*2;++i)
		setButtonState(i,deviceButtonStates[i]);
	for(int i=0;i<3*2;++i)
		setValuatorState(i,deviceValuatorStates[i]);
	for(int i=0;i<2;++i)
		setTrackerState(i,deviceTrackerStates[i]);
	
	/* Start reporting events to the device manager: */
	reportEvents=true;
	}

void RazerHydraDevice::stop(void)
	{
	/* Stop reporting events to the device manager: */
	Threads::Mutex::Lock deviceValuesLock(deviceValuesMutex);
	reportEvents=false;
	}

/*************************************
Object creation/destruction functions:
*************************************/

extern "C" VRDevice* createObjectRazerHydraDevice(VRFactory<VRDevice>* factory,VRFactoryManager<VRDevice>* factoryManager,Misc::ConfigurationFile& configFile)
	{
	VRDeviceManager* deviceManager=static_cast<VRDeviceManager::DeviceFactoryManager*>(factoryManager)->getDeviceManager();
	return new RazerHydraDevice(factory,deviceManager,configFile);
	}

extern "C" void destroyObjectRazerHydraDevice(VRDevice* device,VRFactory<VRDevice>* factory,VRFactoryManager<VRDevice>* factoryManager)
	{
	delete device;
	}
