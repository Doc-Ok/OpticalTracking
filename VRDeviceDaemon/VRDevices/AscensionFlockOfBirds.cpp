/***********************************************************************
AscensionFlockOfBirds - Class for tracking device of same name.
Copyright (c) 2000-2011 Oliver Kreylos

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

#include <VRDeviceDaemon/VRDevices/AscensionFlockOfBirds.h>

#include <Misc/ThrowStdErr.h>
#include <Misc/Time.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Math/Math.h>
#include <Math/Constants.h>

#include <VRDeviceDaemon/VRDeviceManager.h>

/**************************************
Methods of class AscensionFlockOfBirds:
**************************************/

short int AscensionFlockOfBirds::readShort(void)
	{
	unsigned char reply[2];
	devicePort.readRaw(reply,2);
	short int result=0;
	result|=reply[1];
	result<<=8;
	result|=reply[0];
	return result;
	}

void AscensionFlockOfBirds::writeShort(short int value)
	{
	char bytes[2];
	bytes[0]=char(value&0x0f);
	value>>=8;
	bytes[1]=char(value&0x0f);
	devicePort.writeRaw(bytes,2);
	}

short int AscensionFlockOfBirds::extractShort(const char* lsb) const
	{
	short int result=0;
	result|=(unsigned char)lsb[1]&(unsigned char)0x7f;
	result<<=7;
	result|=(unsigned char)lsb[0]&(unsigned char)0x7f;
	result<<=2;
	return result;
	}

void AscensionFlockOfBirds::deviceThreadMethod(void)
	{
	/* Reset first measurement flags: */
	for(int i=0;i<numTrackers;++i)
		notFirstMeasurements[i]=false;
	
	/* Process packets until killed: */
	while(true)
		{
		/* Read the next message: */
		char buffer[13];
		devicePort.readRaw(buffer,13);

		/* Check for sync: */
		if(!(buffer[0]&0x80))
			{
			/* Lost sync - discard the read data and wait for the next record start: */
			#ifdef VERBOSE
			printf("AscensionFlockOfBirds: Re-synchronizing with data stream\n");
			fflush(stdout);
			#endif
			while(!(buffer[0]&0x80))
				buffer[0]=devicePort.getChar();

			/* Read the rest of the record: */
			devicePort.readRaw(buffer+1,12);
			}
		
		/* Extract data from the buffer: */
		int deviceTrackerIndex=int(buffer[12])-1;
		Vrui::VRDeviceState::TrackerState ts;
		
		/* Calculate raw position and orientation: */
		typedef PositionOrientation::Vector Vector;
		typedef Vector::Scalar VScalar;
		VScalar vFactor=VScalar(trackerRange)/VScalar(32767);
		Vector v;
		v[0]=-VScalar(extractShort(buffer+0))*vFactor; // Hack: FOB seems to deliver left-hand coordinate system!
		v[1]=VScalar(extractShort(buffer+2))*vFactor;
		v[2]=VScalar(extractShort(buffer+4))*vFactor;
		
		typedef PositionOrientation::Rotation Rotation;
		typedef Rotation::Scalar RScalar;
		RScalar rFactor=Math::Constants<RScalar>::pi/RScalar(32767);
		RScalar angles[3];
		angles[2]=-RScalar(extractShort(buffer+6))*rFactor;
		angles[1]=-RScalar(extractShort(buffer+8))*rFactor;
		angles[0]=RScalar(extractShort(buffer+10))*rFactor;
		Rotation o=Rotation::identity;
		o*=Rotation::rotateZ(angles[2]);
		o*=Rotation::rotateY(angles[1]);
		o*=Rotation::rotateX(angles[0]);
		
		/* Set new position and orientation: */
		ts.positionOrientation=PositionOrientation(v,o);
		
		/* Calculate linear and angular velocities: */
		timers[deviceTrackerIndex].elapse();
		if(notFirstMeasurements[deviceTrackerIndex])
			{
			/* Estimate velocities by dividing position/orientation differences by elapsed time since last measurement: */
			double time=timers[deviceTrackerIndex].getTime();
			ts.linearVelocity=(v-oldPositionOrientations[deviceTrackerIndex].getTranslation())/Vrui::VRDeviceState::TrackerState::LinearVelocity::Scalar(time);
			Rotation dO=o*Geometry::invert(oldPositionOrientations[deviceTrackerIndex].getRotation());
			ts.angularVelocity=dO.getScaledAxis()/Vrui::VRDeviceState::TrackerState::AngularVelocity::Scalar(time);
			}
		else
			{
			/* Force initial velocities to zero: */
			ts.linearVelocity=Vrui::VRDeviceState::TrackerState::LinearVelocity::zero;
			ts.angularVelocity=Vrui::VRDeviceState::TrackerState::AngularVelocity::zero;
			notFirstMeasurements[deviceTrackerIndex]=true;
			}
		oldPositionOrientations[deviceTrackerIndex]=ts.positionOrientation;
		
		/* Update tracker state: */
		setTrackerState(deviceTrackerIndex,ts);
		}
	}

AscensionFlockOfBirds::AscensionFlockOfBirds(VRDevice::Factory* sFactory,VRDeviceManager* sDeviceManager,Misc::ConfigurationFile& configFile)
	:VRDevice(sFactory,sDeviceManager,configFile),
	 devicePort(configFile.retrieveString("./devicePort").c_str()),
	 masterId(configFile.retrieveValue<int>("./masterId",1)),
	 firstBirdId(configFile.retrieveValue<int>("./firstBirdId",1)),
	 ercId(configFile.retrieveValue<int>("./ercId",-1)),
	 ercTransmitterIndex(configFile.retrieveValue<int>("./ercTransmitterIndex",-1)),
	 trackerRange(configFile.retrieveValue<double>("./trackerRange",36.0)),
	 timers(0),notFirstMeasurements(0),oldPositionOrientations(0)
	{
	/* Query number of connected birds (includes ERC if present): */
	int numBirds=configFile.retrieveValue<int>("./numBirds");
	
	/* Set device configuration: */
	setNumTrackers(ercId==-1?numBirds:numBirds-1,configFile);
	
	/* Create free-running timers: */
	timers=new Misc::Timer[numTrackers];
	notFirstMeasurements=new bool[numTrackers];
	oldPositionOrientations=new PositionOrientation[numTrackers];
	
	/* Set device port parameters: */
	devicePort.ref();
	int deviceBaudRate=configFile.retrieveValue<int>("./deviceBaudRate");
	devicePort.setSerialSettings(deviceBaudRate,8,Comm::SerialPort::NoParity,1,false);
	devicePort.setRawMode(1,0);
	
	/* Check status of all birds: */
	bool allBirdsOk=true;
	for(int i=0;i<numBirds;++i)
		// if(i+firstBirdId!=ercId)
			{
			#ifdef VERBOSE
			printf("AscensionFlockOfBirds: Querying status of bird %d... ",i+firstBirdId);
			fflush(stdout);
			#endif
			devicePort.putChar(0xf0|char(i+firstBirdId)); // Talk to bird i
			devicePort.putChar(0x4f); // Examine value
			devicePort.putChar(0x00); // Bird status
			devicePort.flush();
			if(devicePort.waitForData(Misc::Time(2,0)))
				{
				/* Read the bird's answer: */
				int reply=readShort();
				#ifdef VERBOSE
				printf("ok (%.4x)\n",reply&0xffff);
				fflush(stdout);
				#endif
				}
			else
				{
				#ifdef VERBOSE
				printf("failed\n");
				fflush(stdout);
				#endif
				allBirdsOk=false;
				}
			}
	
	/* Stop if not all birds responded: */
	if(!allBirdsOk)
		Misc::throwStdErr("AscensionFlockOfBirds: Unable to communicate to all birds");
	
	#if 0
	/* Reset flock: */
	#ifdef VERBOSE
	printf("AscensionFlockOfBirds: Resetting flock\n");
	fflush(stdout);
	#endif
	devicePort.putChar(0xf0|masterId); // Talk to FOB master
	devicePort.putChar(0x2f); // FBB Reset
	devicePort.flush();
	Misc::sleep(0.25);
	#endif
	
	/* Query flock configuration: */
	#ifdef VERBOSE
	printf("AscensionFlockOfBirds: Querying flock configuration\n");
	fflush(stdout);
	devicePort.putChar(0xf0|masterId); // Talk to FOB master
	devicePort.putChar(0x4f); // Change Value
	devicePort.putChar(36); // Flock System Status
	devicePort.flush();
	Misc::sleep(0.25);
	
	/* Receive flock state: */
	char flockStates[14];
	devicePort.readRaw(flockStates,14);
	
	/* Print status of all present birds: */
	for(int i=0;i<14;++i)
		if(flockStates[i]&0x80)
			{
			printf("AscensionFlockOfBirds: Status of bird %d: %.2x\n",i+1,int(flockStates[i])&0xff);
			fflush(stdout);
			}
	#endif
	
	/* Set FOB to report position/angles: */
	#ifdef VERBOSE
	printf("AscensionFlockOfBirds: Setting position/angles mode on all birds\n");
	fflush(stdout);
	#endif
	for(int i=0;i<numBirds;++i)
		if(i+firstBirdId!=ercId)
			{
			devicePort.putChar(0xf0|char(i+firstBirdId)); // Talk to bird i
			devicePort.putChar(0x59); // Position/Angles mode
			devicePort.flush();
			Misc::sleep(0.25);
			}
	
	/* Set FOB's hemisphere: */
	char hemisphereBytes[6][2]={{0x00,0x00},{0x00,0x01},{0x0c,0x00},{0x0c,0x01},{0x06,0x00},{0x06,0x01}};
	std::string hemisphere=configFile.retrieveString("./trackerHemisphere","Forward");
	int hemisphereIndex=-1;
	if(hemisphere=="Forward"||hemisphere=="+X")
		hemisphereIndex=0;
	else if(hemisphere=="Aft"||hemisphere=="Rear"||hemisphere=="-X")
		hemisphereIndex=1;
	else if(hemisphere=="Lower"||hemisphere=="+Z")
		hemisphereIndex=2;
	else if(hemisphere=="Upper"||hemisphere=="-Z")
		hemisphereIndex=3;
	else if(hemisphere=="Right"||hemisphere=="+Y")
		hemisphereIndex=4;
	else if(hemisphere=="Left"||hemisphere=="-Y")
		hemisphereIndex=5;
	else
		Misc::throwStdErr("AscensionFlockOfBirds: Unrecognized hemisphere value %s",hemisphere.c_str());
	
	#ifdef VERBOSE
	printf("AscensionFlockOfBirds: Setting hemisphere on all birds\n");
	fflush(stdout);
	#endif
	for(int i=0;i<numBirds;++i)
		if(i+firstBirdId!=ercId)
			{
			devicePort.putChar(0xf0|char(i+firstBirdId)); // Talk to bird i
			devicePort.putChar(0x4c); // Hemisphere
			devicePort.putChar(hemisphereBytes[hemisphereIndex][0]); // Code (pt 1)
			devicePort.putChar(hemisphereBytes[hemisphereIndex][1]); // Code (pt 2)
			devicePort.flush();
			Misc::sleep(0.25);
			}
	
	/* Set FOB's tracking range: */
	if(ercId!=-1)
		trackerRange=144.0;
	else if(trackerRange<Math::sqrt(36.0*72.0))
		trackerRange=36.0;
	else
		trackerRange=72.0;
	
	if(ercId==-1)
		{
		#ifdef VERBOSE
		printf("AscensionFlockOfBirds: Setting tracking range on all birds\n");
		fflush(stdout);
		#endif
		for(int i=0;i<numBirds;++i)
			{
			devicePort.putChar(0xf0|char(i+firstBirdId)); // Talk to bird i
			devicePort.putChar(0x50); // Change Value
			devicePort.putChar(3); // Position Scaling
			if(trackerRange==36.0)
				{
				devicePort.putChar(0x0); // 36 inch max range, pt. 1
				devicePort.putChar(0x0); // 36 inch max range, pt. 2
				}
			else
				{
				devicePort.putChar(0x1); // 72 inch max range, pt. 1
				devicePort.putChar(0x0); // 72 inch max range, pt. 2
				}
			devicePort.flush();
			Misc::sleep(0.25);
			}
		}
	
	/* Enable flock mode: */
	#ifdef VERBOSE
	printf("AscensionFlockOfBirds: Starting auto-configuration\n");
	fflush(stdout);
	#endif
	Misc::sleep(0.35);
	devicePort.putChar(0xf0|masterId); // Talk to FOB master
	devicePort.putChar(0x50); // Change Value
	devicePort.putChar(50); // FBB Auto-Configuration
	devicePort.putChar(char(numBirds)); // numTrackers birds
	devicePort.flush();
	Misc::sleep(0.6);
	
	#ifdef VERBOSE
	printf("AscensionFlockOfBirds: Enabling group mode\n");
	fflush(stdout);
	#endif
	devicePort.putChar(0xf0|masterId); // Talk to FOB master
	devicePort.putChar(0x50); // Change Value
	devicePort.putChar(35); // Group Mode
	devicePort.putChar(1); // Enable
	devicePort.flush();
	Misc::sleep(0.25);
	
	if(ercId!=-1)
		{
		/* Activate ERC's transmitter: */
		#ifdef VERBOSE
		printf("AscensionFlockOfBirds: Enabling ERC transmitter\n");
		fflush(stdout);
		#endif
		devicePort.putChar(0xf0|masterId); // Talk to FOB master
		devicePort.putChar(0x30); // Next Transmitter
		devicePort.putChar(((ercId&0xf)<<4)|ercTransmitterIndex); // ID of extended range controller and transmitter index
		devicePort.flush();
		Misc::sleep(0.25);
		}
	
	/* Put birds to sleep: */
	#ifdef VERBOSE
	printf("AscensionFlockOfBirds: Disabling stream mode\n");
	fflush(stdout);
	#endif
	devicePort.putChar(0xf0|masterId); // Talk to FOB master
	devicePort.putChar(0x42); // Point
	devicePort.flush();
	Misc::sleep(0.25);
	
	#ifdef VERBOSE
	printf("AscensionFlockOfBirds: Disabling tracker device\n");
	fflush(stdout);
	#endif
	devicePort.putChar(0xf0|masterId); // Talk to FOB master
	devicePort.putChar(0x47); // Sleep
	devicePort.flush();
	}

AscensionFlockOfBirds::~AscensionFlockOfBirds(void)
	{
	if(isActive())
		stop();
	delete[] timers;
	delete[] notFirstMeasurements;
	delete[] oldPositionOrientations;
	}

void AscensionFlockOfBirds::start(void)
	{
	/* Start device communication thread: */
	startDeviceThread();
	
	/* Wake up birds: */
	#ifdef VERBOSE
	printf("AscensionFlockOfBirds: Enabling tracker device\n");
	fflush(stdout);
	#endif
	devicePort.putChar(0xf0|masterId); // Talk to FOB master
	devicePort.putChar(0x46); // Run
	devicePort.flush();
	Misc::sleep(0.25);
	
	#ifdef VERBOSE
	printf("AscensionFlockOfBirds: Enabling stream mode\n");
	fflush(stdout);
	#endif
	devicePort.putChar(0xf0|masterId); // Talk to FOB master
	devicePort.putChar(0x40); // Stream
	devicePort.flush();
	}

void AscensionFlockOfBirds::stop(void)
	{
	/* Put birds to sleep: */
	#ifdef VERBOSE
	printf("AscensionFlockOfBirds: Disabling stream mode\n");
	fflush(stdout);
	#endif
	devicePort.putChar(0xf0|masterId); // Talk to FOB master
	devicePort.putChar(0x42); // Point
	devicePort.flush();
	Misc::sleep(0.25);
	
	#ifdef VERBOSE
	printf("AscensionFlockOfBirds: Disabling tracker device\n");
	fflush(stdout);
	#endif
	devicePort.putChar(0xf0|masterId); // Talk to FOB master
	devicePort.putChar(0x47); // Sleep
	devicePort.flush();
	
	/* Stop device communication thread: */
	stopDeviceThread();
	}

/*************************************
Object creation/destruction functions:
*************************************/

extern "C" VRDevice* createObjectAscensionFlockOfBirds(VRFactory<VRDevice>* factory,VRFactoryManager<VRDevice>* factoryManager,Misc::ConfigurationFile& configFile)
	{
	VRDeviceManager* deviceManager=static_cast<VRDeviceManager::DeviceFactoryManager*>(factoryManager)->getDeviceManager();
	return new AscensionFlockOfBirds(factory,deviceManager,configFile);
	}

extern "C" void destroyObjectAscensionFlockOfBirds(VRDevice* device,VRFactory<VRDevice>* factory,VRFactoryManager<VRDevice>* factoryManager)
	{
	delete device;
	}
