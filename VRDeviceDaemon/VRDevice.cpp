/***********************************************************************
VRDevice - Abstract base class for hardware devices delivering
position, orientation, button events and valuator values.
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

#include <VRDeviceDaemon/VRDevice.h>

#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Math/Math.h>
#include <Geometry/GeometryValueCoders.h>

#include <VRDeviceDaemon/VRFactory.h>
#include <VRDeviceDaemon/VRCalibrator.h>
#include <VRDeviceDaemon/VRDeviceManager.h>

/*************************
Methods of class VRDevice:
*************************/

void* VRDevice::deviceThreadMethodWrapper(void)
	{
	/* Enable immediate cancellation of this thread: */
	Threads::Thread::setCancelState(Threads::Thread::CANCEL_ENABLE);
	Threads::Thread::setCancelType(Threads::Thread::CANCEL_ASYNCHRONOUS);

	/* Call device thread method: */
	deviceThreadMethod();
	
	return 0;
	}

void VRDevice::setNumTrackers(int newNumTrackers,const Misc::ConfigurationFile& configFile,const std::string* trackerNames)
	{
	/* Reallocate tracker index mapping and post transformation arrays: */
	if(numTrackers!=newNumTrackers)
		{
		delete[] trackerIndices;
		delete[] trackerPostTransformations;
		numTrackers=newNumTrackers;
		trackerIndices=new int[numTrackers];
		trackerPostTransformations=new TrackerPostTransformation[numTrackers];
		}
	
	/* Initialize tracker post transformations: */
	for(int i=0;i<numTrackers;++i)
		{
		/* Read post transformation: */
		char transformationTagName[40];
		snprintf(transformationTagName,sizeof(transformationTagName),"./trackerPostTransformation%d",i);
		trackerPostTransformations[i]=configFile.retrieveValue<TrackerPostTransformation>(transformationTagName,TrackerPostTransformation::identity);
		}
	
	if(calibrator!=0)
		{
		/* Set the number of trackers in the calibrator: */
		calibrator->setNumTrackers(numTrackers);
		}
	
	/* Add the trackers to the device daemon's namespace: */
	for(int i=0;i<numTrackers;++i)
		trackerIndices[i]=deviceManager->addTracker(trackerNames!=0?trackerNames[i].c_str():0);
	}

void VRDevice::setNumButtons(int newNumButtons,const Misc::ConfigurationFile& configFile,const std::string* buttonNames)
	{
	/* Reallocate button index mapping array: */
	if(numButtons!=newNumButtons)
		{
		delete[] buttonIndices;
		numButtons=newNumButtons;
		buttonIndices=new int[numButtons];
		}
	
	/* Add the buttons to the device daemon's namespace: */
	for(int i=0;i<numButtons;++i)
		buttonIndices[i]=deviceManager->addButton(buttonNames!=0?buttonNames[i].c_str():0);
	}

void VRDevice::setNumValuators(int newNumValuators,const Misc::ConfigurationFile& configFile,const std::string* valuatorNames)
	{
	/* Reallocate valuator index mapping and value mapping arrays: */
	if(numValuators!=newNumValuators)
		{
		delete[] valuatorIndices;
		delete[] valuatorThresholds;
		delete[] valuatorExponents;
		numValuators=newNumValuators;
		valuatorIndices=new int[numValuators];
		valuatorThresholds=new float[numValuators];
		valuatorExponents=new float[numValuators];
		}
	
	/* Set number of valuators: */
	numValuators=newNumValuators;
	
	/* Read default valuator threshold and exponent: */
	float valuatorThreshold=configFile.retrieveValue<float>("./valuatorThreshold",0.0f);
	float valuatorExponent=configFile.retrieveValue<float>("./valuatorExponent",1.0f);
	
	/* Read per-valuator thresholds and exponents: */
	for(int i=0;i<numValuators;++i)
		{
		/* Read valuator threshold: */
		char thresholdTagName[40];
		snprintf(thresholdTagName,sizeof(thresholdTagName),"./valuatorThreshold%d",i);
		valuatorThresholds[i]=configFile.retrieveValue<float>(thresholdTagName,valuatorThreshold);
		
		/* Read valuator exponent: */
		char exponentTagName[40];
		snprintf(exponentTagName,sizeof(exponentTagName),"./valuatorExponent%d",i);
		valuatorExponents[i]=configFile.retrieveValue<float>(exponentTagName,valuatorExponent);
		}
	
	/* Add the valuators to the device daemon's namespace: */
	for(int i=0;i<numValuators;++i)
		valuatorIndices[i]=deviceManager->addValuator(valuatorNames!=0?valuatorNames[i].c_str():0);
	}

void VRDevice::addVirtualDevice(Vrui::VRDeviceDescriptor* newDevice)
	{
	deviceManager->addVirtualDevice(newDevice);
	}

void VRDevice::setTrackerState(int deviceTrackerIndex,const Vrui::VRDeviceState::TrackerState& state,Vrui::VRDeviceState::TimeStamp timeStamp)
	{
	Vrui::VRDeviceState::TrackerState calibratedState=state;
	if(calibrator!=0)
		calibrator->calibrate(deviceTrackerIndex,calibratedState);
	calibratedState.positionOrientation*=trackerPostTransformations[deviceTrackerIndex];
	deviceManager->setTrackerState(trackerIndices[deviceTrackerIndex],calibratedState,timeStamp);
	}

void VRDevice::setButtonState(int deviceButtonIndex,Vrui::VRDeviceState::ButtonState newState)
	{
	deviceManager->setButtonState(buttonIndices[deviceButtonIndex],newState);
	}

void VRDevice::setValuatorState(int deviceValuatorIndex,Vrui::VRDeviceState::ValuatorState newState)
	{
	Vrui::VRDeviceState::ValuatorState calibratedState=newState;
	float th=valuatorThresholds[deviceValuatorIndex];
	if(calibratedState<-th)
		calibratedState=-Math::pow(-(calibratedState+th)/(1.0f-th),valuatorExponents[deviceValuatorIndex]);
	else if(calibratedState>th)
		calibratedState=Math::pow((calibratedState-th)/(1.0f-th),valuatorExponents[deviceValuatorIndex]);
	else
		calibratedState=0.0f;
	deviceManager->setValuatorState(valuatorIndices[deviceValuatorIndex],calibratedState);
	}

void VRDevice::updateState(void)
	{
	deviceManager->updateState();
	}

void VRDevice::startDeviceThread(void)
	{
	if(!active)
		{
		/* Create device communication thread: */
		deviceThread.start(this,&VRDevice::deviceThreadMethodWrapper);
		active=true;
		}
	}

void VRDevice::stopDeviceThread(bool cancel)
	{
	if(active)
		{
		/* Destroy device communication thread: */
		if(cancel)
			deviceThread.cancel();
		deviceThread.join();
		active=false;
		}
	}

void VRDevice::deviceThreadMethod(void)
	{
	}

VRDevice::VRDevice(VRDevice::Factory* sFactory,VRDeviceManager* sDeviceManager,Misc::ConfigurationFile& configFile)
	:factory(sFactory),
	 numTrackers(0),numButtons(0),numValuators(0),
	 trackerIndices(0),trackerPostTransformations(0),
	 buttonIndices(0),
	 valuatorIndices(0),valuatorThresholds(0),valuatorExponents(0),
	 active(false),
	 deviceManager(sDeviceManager),
	 calibrator(0)
	{
	/* Check if the device has an attached calibrator: */
	if(configFile.hasTag("./calibratorName"))
		{
		/* Go to the calibrator's section and read the calibrator type: */
		configFile.setCurrentSection(configFile.retrieveString("./calibratorName").c_str());
		std::string calibratorType=configFile.retrieveString("./type");
		
		/* Create the calibrator: */
		calibrator=deviceManager->createCalibrator(calibratorType,configFile);
		configFile.setCurrentSection("..");
		}
	}

VRDevice::~VRDevice(void)
	{
	/* Delete calibrator: */
	if(calibrator!=0)
		VRCalibrator::destroy(calibrator);
	
	/* Delete tracker post transformations: */
	delete[] trackerPostTransformations;
	
	/* Delete valuator thresholds and exponents: */
	delete[] valuatorThresholds;
	delete[] valuatorExponents;
	
	/* Delete index mappings: */
	delete[] trackerIndices;
	delete[] buttonIndices;
	delete[] valuatorIndices;
	}

void VRDevice::destroy(VRDevice* object)
	{
	object->factory->destroyObject(object);
	}
