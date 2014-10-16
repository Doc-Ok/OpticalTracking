/***********************************************************************
PolhemusFastrak - Class for tracking device of same name.
Copyright (c) 1998-2011 Oliver Kreylos

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

#include <VRDeviceDaemon/VRDevices/PolhemusFastrak.h>

#include <stdarg.h>
#include <stdio.h>
#include <Misc/Endianness.h>
#include <Misc/ThrowStdErr.h>
#include <Misc/Time.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Math/Math.h>
#include <Geometry/GeometryValueCoders.h>

#include <VRDeviceDaemon/VRDeviceManager.h>

namespace {

/****************
Helper functions:
****************/

inline bool waitForData(Comm::Pipe& pipe,const Misc::Time& deadline)
	{
	/* Check if there is still data in the read buffer: */
	if(pipe.canReadImmediately())
		return true;
	
	/* Calculate a timeout: */
	Misc::Time timeout=deadline;
	timeout-=Misc::Time::now();
	
	/* Check if the deadline has already passed: */
	if(timeout.tv_sec<0)
		return false;
	
	/* Wait for data: */
	return pipe.waitForData(timeout);
	}

inline void writeCommand(Comm::Pipe& pipe,char command)
	{
	/* Write the command character: */
	pipe.putChar(command);
	
	/* Flush the pipe: */
	pipe.flush();
	}

inline void writeCommand(Comm::Pipe& pipe,const char* format,...)
	{
	/* Assemble the command string: */
	static char commandBuffer[256];
	va_list ap;
	va_start(ap,format);
	vsnprintf(commandBuffer,sizeof(commandBuffer),format,ap);
	va_end(ap);
	
	/* Write the command string: */
	char* cPtr=commandBuffer;
	while(*cPtr!='\0')
		pipe.putChar(*(cPtr++));
	
	/* Flush the pipe: */
	pipe.flush();
	}

int readStationId(Comm::Pipe& pipe)
	{
	/* Check for the synchronization sequence: */
	if(pipe.getChar()!='\r'||pipe.getChar()!='\n'||pipe.getChar()!='0')
		return -1;
	
	/* Extract the station ID: */
	int stationId=-1;
	int idTag=pipe.getChar();
	if(idTag>='1'&&idTag<='4')
		stationId=idTag-'0';
	else
		return -1;
	
	/* Check for filler character: */
	int filler=pipe.getChar();
	if(filler!=' '&&(filler<'a'||filler>'z')&&(filler<'A'||filler>'Z'))
		return -1;
	
	return stationId;
	}

}

/********************************
Methods of class PolhemusFastrak:
********************************/

char* PolhemusFastrak::readLine(int lineBufferSize,char* lineBuffer,const Misc::Time& deadline)
	{
	/* Read bytes from the serial port until CR/LF has been read or deadline is reached: */
	int state=0;
	char* bufPtr=lineBuffer;
	int bytesLeft=lineBufferSize-1;
	while(state<2)
		{
		/* Wait for more data: */
		if(!waitForData(devicePort,deadline))
			break;
		
		/* Read next byte: */
		int input=devicePort.getChar();
		
		/* Process byte: */
		switch(state)
			{
			case 0: // Still waiting for CR
				if(input=='\r') // CR read?
					{
					/* Go to next state: */
					state=1;
					}
				else if(bytesLeft>0) // Still room in buffer?
					{
					/* Store byte in buffer: */
					*bufPtr=char(input);
					++bufPtr;
					--bytesLeft;
					}
				break;
			
			case 1: // Waiting for LF
				if(input=='\n') // LF read?
					{
					/* Go to final state: */
					state=2;
					}
				else
					{
					/* Continue reading characters: */
					state=0;
					}
				break;
			}
		}
	
	/* Terminate and return read string: */
	*bufPtr='\0';
	return lineBuffer;
	}

bool PolhemusFastrak::readStatusReply(void)
	{
	/* Create a deadline by which the complete status reply has to have arrived: */
	Misc::Time deadline=Misc::Time::now();
	deadline.tv_sec+=10;
	
	/* Read bytes from the serial port until the status report's prefix has been matched or the deadline is reached: */
	int state=0;
	while(state<4)
		{
		/* Wait for more data: */
		if(!waitForData(devicePort,deadline))
			break;
		
		/* Read next byte and try matching status reply's prefix: */
		int input=devicePort.getChar();
		switch(state)
			{
			case 0: // Haven't matched anything
				if(input=='2')
					state=1;
				break;
			
			case 1: // Have matched "2"
				if(input=='2')
					state=2;
				else if(input>='1'&&input<='4')
					state=3;
				else
					state=0;
				break;
			
			case 2: // Have matched "22"
				if(input=='S')
					state=4;
				else if(input=='2')
					state=2;
				else if(input>='1'&&input<='4')
					state=3;
				else
					state=0;
				break;
			
			case 3: // Have matched "2[1,3,4]"
				if(input=='S')
					state=4;
				else if(input=='2')
					state=1;
				else
					state=0;
				break;
			}
		}
	
	/* Fail if we timed out while trying to match the prefix: */
	if(state!=4)
		return false;
	
	/* Read rest of status reply until final CR/LF pair: */
	char buffer[256];
	readLine(sizeof(buffer),buffer,deadline);
	#ifdef VERBOSE
	printf("PolhemusFastrak: Received status reply\n  %s",buffer);
	fflush(stdout);
	#endif
	
	return true;
	}

bool PolhemusFastrak::processRecord(void)
	{
	/* Check for the synchronization sequence: */
	bool lostSync=false;
	int stationId=readStationId(devicePort);
	if(stationId<0)
		{
		lostSync=true;
		
		/* Re-synchronize with the data stream: */
		int state=0;
		while(state<5)
			{
			int input=devicePort.getChar();
			switch(state)
				{
				case 0: // Haven't matched anything
					if(input=='\r')
						state=1;
					else
						state=0;
					break;
				
				case 1: // Have matched CR
					if(input=='\n')
						state=2;
					else if(input=='\r')
						state=1;
					else
						state=0;
					break;
				
				case 2: // Have matched CR/LF
					if(input=='0')
						state=3;
					else if(input=='\r')
						state=1;
					else
						state=0;
					break;
				
				case 3: // Have matched CR/LF + 0
					if(input>='1'&&input<='4')
						{
						stationId=input-'0';
						state=4;
						}
					else if(input=='\r')
						state=1;
					else
						state=0;
					break;
				
				case 4: // Have matched CR/LF + 0 + <receiver number>
					if(input==' '||(input>='A'&&input<='Z')||(input>='a'&&input<='z'))
						state=5;
					else if(input=='\r')
						{
						stationId=-1;
						state=1;
						}
					else
						{
						stationId=-1;
						state=0;
						}
					break;
				}
			}
		}
	
	/* Process the binary part of the record: */
	int stationIndex=stationId-1;
	Vrui::VRDeviceState::TrackerState ts;
	
	/* Calculate raw position and orientation: */
	float trans[3];
	devicePort.read<float>(trans,3);
	typedef PositionOrientation::Vector Vector;
	Vector v(trans);
	float rotAngles[3];
	devicePort.read<float>(rotAngles,3);
	typedef PositionOrientation::Rotation Rotation;
	typedef Rotation::Scalar RScalar;
	Rotation o=Rotation::rotateZ(Math::rad(RScalar(rotAngles[0])));
	o*=Rotation::rotateY(Math::rad(RScalar(rotAngles[1])));
	o*=Rotation::rotateX(Math::rad(RScalar(rotAngles[2])));
	
	/* Set new position and orientation: */
	ts.positionOrientation=Vrui::VRDeviceState::TrackerState::PositionOrientation(v,o);
	
	/* Calculate linear and angular velocities: */
	timers[stationIndex].elapse();
	if(notFirstMeasurements[stationIndex])
		{
		/* Estimate velocities by dividing position/orientation differences by elapsed time since last measurement: */
		double time=timers[stationIndex].getTime();
		ts.linearVelocity=(v-oldPositionOrientations[stationIndex].getTranslation())/Vrui::VRDeviceState::TrackerState::LinearVelocity::Scalar(time);
		Rotation dO=o*Geometry::invert(oldPositionOrientations[stationIndex].getRotation());
		ts.angularVelocity=dO.getScaledAxis()/Vrui::VRDeviceState::TrackerState::AngularVelocity::Scalar(time);
		}
	else
		{
		/* Force initial velocities to zero: */
		ts.linearVelocity=Vrui::VRDeviceState::TrackerState::LinearVelocity::zero;
		ts.angularVelocity=Vrui::VRDeviceState::TrackerState::AngularVelocity::zero;
		notFirstMeasurements[stationIndex]=true;
		}
	oldPositionOrientations[stationIndex]=ts.positionOrientation;
	
	if(stationIndex==0&&stylusEnabled)
		{
		/* Read the stylus bit: */
		devicePort.getChar();
		setButtonState(0,devicePort.getChar()=='1');
		}
	
	/* Update tracker state: */
	setTrackerState(stationIndex,ts);
	
	return lostSync;
	}

void PolhemusFastrak::deviceThreadMethod(void)
	{
	/* Reset first measurement flags: */
	for(int i=0;i<numTrackers;++i)
		notFirstMeasurements[i]=false;
	
	/* Process first record: */
	processRecord();
	
	while(true)
		{
		/* Process the next record and check for loss of synchronization: */
		if(processRecord())
			{
			/* Fall back to synchronizing mode: */
			#ifdef VERBOSE
			printf("PolhemusFastrak: Lost synchronization with tracker stream\n");
			fflush(stdout);
			#endif
			}
		}
	}

PolhemusFastrak::PolhemusFastrak(VRDevice::Factory* sFactory,VRDeviceManager* sDeviceManager,Misc::ConfigurationFile& configFile)
	:VRDevice(sFactory,sDeviceManager,configFile),
	 devicePort(configFile.retrieveString("./devicePort").c_str()),
	 stylusEnabled(configFile.retrieveValue<bool>("./stylusEnabled",true)),
	 timers(0),notFirstMeasurements(0),oldPositionOrientations(0)
	{
	/* Set device configuration: */
	setNumTrackers(configFile.retrieveValue<int>("./numReceivers",4),configFile);
	if(stylusEnabled)
		setNumButtons(1,configFile); // Assume that first receiver is a stylus
	
	/* Create free-running timers: */
	timers=new Misc::Timer[numTrackers];
	notFirstMeasurements=new bool[numTrackers];
	oldPositionOrientations=new PositionOrientation[numTrackers];
	
	/* Set device port parameters: */
	devicePort.ref();
	int deviceBaudRate=configFile.retrieveValue<int>("./deviceBaudRate");
	devicePort.setSerialSettings(deviceBaudRate,8,Comm::SerialPort::NoParity,1,false);
	devicePort.setRawMode(1,0);
	devicePort.setEndianness(Misc::LittleEndian);
	
	if(configFile.retrieveValue<bool>("./resetDevice",false))
		{
		/* Reset device: */
		#ifdef VERBOSE
		printf("PolhemusFastrak: Resetting device\n");
		fflush(stdout);
		#endif
		writeCommand(devicePort,'\31');
		Misc::sleep(15.0);
		}
	else
		{
		/* Stop continuous mode (in case it's still active): */
		#ifdef VERBOSE
		printf("PolhemusFastrak: Disabling continuous mode\n");
		fflush(stdout);
		#endif
		writeCommand(devicePort,'c');
		}
	
	/* Request status record to check if device is okey-dokey: */
	#ifdef VERBOSE
	printf("PolhemusFastrak: Requesting status record\n");
	fflush(stdout);
	#endif
	writeCommand(devicePort,'S');
	if(!readStatusReply())
		{
		/* Try resetting the device, seeing if that helps: */
		#ifdef VERBOSE
		printf("PolhemusFastrak: Resetting device\n");
		fflush(stdout);
		#endif
		writeCommand(devicePort,'\31');
		Misc::sleep(15.0);
		
		/* Request another status record: */
		#ifdef VERBOSE
		printf("PolhemusFastrak: Re-requesting status record\n");
		fflush(stdout);
		#endif
		writeCommand(devicePort,'S');
		if(!readStatusReply())
			Misc::throwStdErr("PolhemusFastrak: Device not responding");
		}
	
	/* Retrieve tracker hemisphere: */
	int hemisphereVectors[6][3]={{1,0,0},{-1,0,0},{0,0,1},{0,0,-1},{0,1,0},{0,-1,0}};
	std::string hemisphere=configFile.retrieveString("./trackerHemisphere","+X");
	int hemisphereIndex=-1;
	if(hemisphere=="+X")
		hemisphereIndex=0;
	else if(hemisphere=="-X")
		hemisphereIndex=1;
	else if(hemisphere=="+Z")
		hemisphereIndex=2;
	else if(hemisphere=="-Z")
		hemisphereIndex=3;
	else if(hemisphere=="+Y")
		hemisphereIndex=4;
	else if(hemisphere=="-Y")
		hemisphereIndex=5;
	else
		Misc::throwStdErr("PolhemusFastrak: Unrecognized hemisphere value %s",hemisphere.c_str());
	
	/* Initialize all receivers: */
	#ifdef VERBOSE
	printf("PolhemusFastrak: Initializing receivers\n");
	fflush(stdout);
	#endif
	for(int i=0;i<numTrackers;++i)
		{
		/* Enable receiver: */
		writeCommand(devicePort,"l%d,1\r\n",i+1);
		Misc::sleep(0.1);
		
		/* Reset receiver's alignment frame: */
		writeCommand(devicePort,"R%d\r\n",i+1);
		Misc::sleep(0.1);
		
		/* Disable boresight mode: */
		writeCommand(devicePort,"b%d\r\n",i+1);
		Misc::sleep(0.1);
		
		/* Set receiver's hemisphere of operation: */
		writeCommand(devicePort,"H%d,%d,%d,%d\r\n",i+1,hemisphereVectors[hemisphereIndex][0],hemisphereVectors[hemisphereIndex][1],hemisphereVectors[hemisphereIndex][2]);
		Misc::sleep(0.1);
		
		/* Set receiver's output format: */
		writeCommand(devicePort,"O%d,2,4,16,1\r\n",i+1);
		Misc::sleep(0.1);
		}
	
	/* Set stylus tip offset: */
	if(configFile.hasTag("./stylusTipOffset"))
		{
		/* Get the tip offset from the configuration file: */
		Geometry::Vector<float,3> tipOffset=configFile.retrieveValue<Geometry::Vector<float,3> >("./stylusTipOffset");
		
		/* Set the stylus tip offset: */
		#ifdef VERBOSE
		printf("PolhemusFastrak: Setting stylus tip offset\n");
		fflush(stdout);
		#endif
		writeCommand(devicePort,"N1,%8.4f,%8.4f,%8.4f\r\n",tipOffset[0],tipOffset[1],tipOffset[2]);
		Misc::sleep(0.1);
		}
	
	/* Set stylus button to "mouse mode": */
	#ifdef VERBOSE
	printf("PolhemusFastrak: Setting stylus button mode\n");
	fflush(stdout);
	#endif
	writeCommand(devicePort,"e1,0\r\n");
	Misc::sleep(0.1);
	
	#if 1
	/* Query stylus tip offset: */
	writeCommand(devicePort,'F');
	Misc::sleep(0.1);
	writeCommand(devicePort,"N1,\r\n");
	Misc::sleep(0.1);
	char lineBuffer[80];
	printf("%s\n",readLine(80,lineBuffer,Misc::Time(5,0)));
	fflush(stdout);
	#endif
	
	/* Set fixed metal compensation: */
	#ifdef VERBOSE
	printf("PolhemusFastrak: Setting fixed metal compensation mode\n");
	fflush(stdout);
	#endif
	if(configFile.retrieveValue<bool>("./enableMetalCompensation",false))
		writeCommand(devicePort,'D');
	else
		writeCommand(devicePort,'d');
	Misc::sleep(0.1);
	
	/* Set unit mode to inches: */
	#ifdef VERBOSE
	printf("PolhemusFastrak: Setting unit mode\n");
	fflush(stdout);
	#endif
	writeCommand(devicePort,'U');
	Misc::sleep(0.1);
	
	/* Enable binary mode: */
	#ifdef VERBOSE
	printf("PolhemusFastrak: Enabling binary mode\n");
	fflush(stdout);
	#endif
	writeCommand(devicePort,'f');
	}

PolhemusFastrak::~PolhemusFastrak(void)
	{
	if(isActive())
		stop();
	delete[] timers;
	delete[] notFirstMeasurements;
	delete[] oldPositionOrientations;
	}

void PolhemusFastrak::start(void)
	{
	/* Start device communication thread: */
	startDeviceThread();
	
	/* Enable continuous mode: */
	#ifdef VERBOSE
	printf("PolhemusFastrak: Enabling continuous mode\n");
	fflush(stdout);
	#endif
	writeCommand(devicePort,'C');
	}

void PolhemusFastrak::stop(void)
	{
	/* Disable continuous mode: */
	#ifdef VERBOSE
	printf("PolhemusFastrak: Disabling continuous mode\n");
	fflush(stdout);
	#endif
	writeCommand(devicePort,'c');
	
	/* Stop device communication thread: */
	stopDeviceThread();
	}

/*************************************
Object creation/destruction functions:
*************************************/

extern "C" VRDevice* createObjectPolhemusFastrak(VRFactory<VRDevice>* factory,VRFactoryManager<VRDevice>* factoryManager,Misc::ConfigurationFile& configFile)
	{
	VRDeviceManager* deviceManager=static_cast<VRDeviceManager::DeviceFactoryManager*>(factoryManager)->getDeviceManager();
	return new PolhemusFastrak(factory,deviceManager,configFile);
	}

extern "C" void destroyObjectPolhemusFastrak(VRDevice* device,VRFactory<VRDevice>* factory,VRFactoryManager<VRDevice>* factoryManager)
	{
	delete device;
	}
