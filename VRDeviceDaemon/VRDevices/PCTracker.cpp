/***********************************************************************
PCTracker - Class for communicating with tracking devices on a dedicated
DOS PC.
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

#include <VRDeviceDaemon/VRDevices/PCTracker.h>

#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Math/Math.h>
#include <Math/Constants.h>

#include <VRDeviceDaemon/VRDeviceManager.h>

/**************************
Methods of class PCTracker:
**************************/

short int PCTracker::extractShort(const char* lsb) const
	{
	return ((unsigned char)lsb[1]<<8)|(unsigned char)lsb[0];
	}

void PCTracker::deviceThreadMethod(void)
	{
	/* Reset first measurement flags: */
	for(int i=0;i<numTrackers;++i)
		notFirstMeasurements[i]=false;
	
	/* Process packets until killed: */
	bool synchronize=true;
	while(true)
		{
		char buffer[15];
		
		if(synchronize)
			{
			/* Wait for the start of the next message: */
			do
				{
				buffer[0]=devicePort.getChar();
				}
			while(buffer[0]!='S');
			
			/* Read the rest of the message: */
			devicePort.readRaw(buffer+1,14);
			}
		else
			{
			/* Read the next message: */
			devicePort.readRaw(buffer,15);
			}
		
		/* Check for sync: */
		synchronize=buffer[0]!='S'||buffer[13]!=0||buffer[14]<1||buffer[14]>numTrackers;
		if(synchronize)
			{
			/* Lost sync - discard the read data and wait for the next record start: */
			#ifdef VERBOSE
			printf("PCTracker: Re-synchronizing with data stream\n");
			fflush(stdout);
			#endif
			}
		else
			{
			/* Extract data from the buffer: */
			int deviceTrackerIndex=int(buffer[14])-1;
			
			/* Calculate raw position and orientation: */
			typedef PositionOrientation::Vector Vector;
			typedef Vector::Scalar VScalar;
			VScalar vFactor=VScalar(trackerRange)/VScalar(32767);
			Vector v;
			v[0]=VScalar(extractShort(buffer+1))*vFactor;
			v[1]=VScalar(extractShort(buffer+3))*vFactor;
			v[2]=VScalar(extractShort(buffer+5))*vFactor;
			
			typedef PositionOrientation::Rotation Rotation;
			typedef Rotation::Scalar RScalar;
			RScalar rFactor=Math::Constants<RScalar>::pi/RScalar(32767);
			RScalar angles[3];
			angles[2]=RScalar(extractShort(buffer+7))*rFactor;
			angles[1]=RScalar(extractShort(buffer+9))*rFactor;
			angles[0]=RScalar(extractShort(buffer+11))*rFactor;
			Rotation o=Rotation::identity;
			o*=Rotation::rotateZ(angles[2]);
			o*=Rotation::rotateY(angles[1]);
			o*=Rotation::rotateX(angles[0]);
			
			/* Set new position and orientation: */
			deviceTrackerStates[deviceTrackerIndex].positionOrientation=PositionOrientation(v,o);
			
			/* Calculate linear and angular velocities: */
			timers[deviceTrackerIndex].elapse();
			if(notFirstMeasurements[deviceTrackerIndex])
				{
				/* Estimate velocities by dividing position/orientation differences by elapsed time since last measurement: */
				double time=timers[deviceTrackerIndex].getTime();
				deviceTrackerStates[deviceTrackerIndex].linearVelocity=(v-oldPositionOrientations[deviceTrackerIndex].getTranslation())/TrackerState::LinearVelocity::Scalar(time);
				Rotation dO=o*Geometry::invert(oldPositionOrientations[deviceTrackerIndex].getRotation());
				deviceTrackerStates[deviceTrackerIndex].angularVelocity=dO.getScaledAxis()/TrackerState::AngularVelocity::Scalar(time);
				}
			else
				{
				/* Force initial velocities to zero: */
				deviceTrackerStates[deviceTrackerIndex].linearVelocity=TrackerState::LinearVelocity::zero;
				deviceTrackerStates[deviceTrackerIndex].angularVelocity=TrackerState::AngularVelocity::zero;
				notFirstMeasurements[deviceTrackerIndex]=true;
				}
			oldPositionOrientations[deviceTrackerIndex]=deviceTrackerStates[deviceTrackerIndex].positionOrientation;
			
			/* Update tracker state in device manager: */
			{
			Threads::Mutex::Lock deviceValuesLock(deviceValuesMutex);
			if(reportEvents)
				setTrackerState(deviceTrackerIndex,deviceTrackerStates[deviceTrackerIndex]);
			}
			}
		}
	}

PCTracker::PCTracker(VRDevice::Factory* sFactory,VRDeviceManager* sDeviceManager,Misc::ConfigurationFile& configFile)
	:VRDevice(sFactory,sDeviceManager,configFile),
	 devicePort(configFile.retrieveString("./devicePort").c_str()),
	 trackerRange(configFile.retrieveValue<double>("./trackerRange",36.0)),
	 reportEvents(false),
	 timers(0),notFirstMeasurements(0),
	 oldPositionOrientations(0),deviceTrackerStates(0)
	{
	/* Set device configuration: */
	setNumTrackers(configFile.retrieveValue<int>("./numTrackers"),configFile);
	
	/* Set device port parameters: */
	devicePort.ref();
	int deviceBaudRate=configFile.retrieveValue<int>("./deviceBaudRate");
	devicePort.setSerialSettings(deviceBaudRate,8,Comm::SerialPort::NoParity,1,false);
	devicePort.setRawMode(1,0);
	
	/* Initialize device states: */
	timers=new Misc::Timer[numTrackers];
	notFirstMeasurements=new bool[numTrackers];
	oldPositionOrientations=new PositionOrientation[numTrackers];
	deviceTrackerStates=new TrackerState[numTrackers];
	for(int i=0;i<numTrackers;++i)
		{
		deviceTrackerStates[i].positionOrientation=PositionOrientation::identity;
		deviceTrackerStates[i].linearVelocity=TrackerState::LinearVelocity::zero;
		deviceTrackerStates[i].angularVelocity=TrackerState::AngularVelocity::zero;
		}
	
	/* Start device thread (dedicated PC cannot be disabled): */
	startDeviceThread();
	}

PCTracker::~PCTracker(void)
	{
	/* Stop device thread (dedicated PC cannot be disabled): */
	{
	Threads::Mutex::Lock deviceValuesLock(deviceValuesMutex);
	stopDeviceThread();
	}
	
	/* Delete device states: */
	delete[] timers;
	delete[] notFirstMeasurements;
	delete[] oldPositionOrientations;
	delete[] deviceTrackerStates;
	}

void PCTracker::start(void)
	{
	/* Set device manager's tracker states to current device values: */
	{
	Threads::Mutex::Lock deviceValuesLock(deviceValuesMutex);
	for(int i=0;i<numTrackers;++i)
		setTrackerState(i,deviceTrackerStates[i]);
	
	/* Start reporting events to the device manager: */
	reportEvents=true;
	}
	}

void PCTracker::stop(void)
	{
	/* Stop reporting events to the device manager: */
	{
	Threads::Mutex::Lock deviceValuesLock(deviceValuesMutex);
	reportEvents=false;
	}
	}

/*************************************
Object creation/destruction functions:
*************************************/

extern "C" VRDevice* createObjectPCTracker(VRFactory<VRDevice>* factory,VRFactoryManager<VRDevice>* factoryManager,Misc::ConfigurationFile& configFile)
	{
	VRDeviceManager* deviceManager=static_cast<VRDeviceManager::DeviceFactoryManager*>(factoryManager)->getDeviceManager();
	return new PCTracker(factory,deviceManager,configFile);
	}

extern "C" void destroyObjectPCTracker(VRDevice* device,VRFactory<VRDevice>* factory,VRFactoryManager<VRDevice>* factoryManager)
	{
	delete device;
	}
