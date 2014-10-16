/***********************************************************************
FakespacePinchGlove - Class for pinch glove device of same name.
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

#include <VRDeviceDaemon/VRDevices/FakespacePinchGlove.h>

#include <Misc/ThrowStdErr.h>
#include <Misc/Time.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>

#include <VRDeviceDaemon/VRDeviceManager.h>

/************************************
Methods of class FakespacePinchGlove:
************************************/

void FakespacePinchGlove::writeCommand(const char* command)
	{
	devicePort.putChar(command[0]);
	devicePort.flush();
	Misc::sleep(0.25);
	devicePort.putChar(command[1]);
	devicePort.flush();
	Misc::sleep(0.25);
	}

char* FakespacePinchGlove::readReply(int stringBufferLen,char* stringBuffer)
	{
	while(devicePort.getChar()!=char(0x82))
		;
	char* sPtr=stringBuffer;
	char* stringBufferEnd=stringBuffer+(stringBufferLen-1);
	while(sPtr<stringBufferEnd&&(*sPtr=devicePort.getChar())!=char(0x8F))
		++sPtr;
	*sPtr='\0';
	return stringBuffer;
	}

void FakespacePinchGlove::ignoreReply(void)
	{
	while(devicePort.getChar()!=char(0x82))
		;
	while(devicePort.getChar()!=char(0x8F))
		;
	}

void FakespacePinchGlove::deviceThreadMethod(void)
	{
	while(true)
		{
		/* Wait for start byte: */
		while(devicePort.getChar()!=char(0x80))
			;
		
		/* Process all contacts in the message: */
		int numContacts=0;
		int contacts[10][2];
		while(true)
			{
			contacts[numContacts][0]=devicePort.getChar();
			if(contacts[numContacts][0]==char(0x8F))
				break;
			contacts[numContacts][1]=devicePort.getChar();
			++numContacts;
			}
		
		/************************************************
		For the time being, count only contacts involving
		the thumb and other fingers from the same hand.
		************************************************/
		
		/* Reset new pinch masks: */
		int newPinchMasks[2]={0x0,0x0};
		
		/* Find all fingers touching the thumb of the same hand: */
		for(int i=0;i<numContacts;++i)
			{
			if(contacts[i][0]&&!contacts[i][1])
				{
				/* Left-hand-only contact: */
				if(contacts[i][0]&0x10)
					{
					/* Involving thumb: */
					newPinchMasks[0]|=contacts[i][0]&~0x10;
					}
				}
			else if(!contacts[i][0]&&contacts[i][1])
				{
				/* Right-hand-only contact: */
				if(contacts[i][1]&0x10)
					{
					/* Involving thumb: */
					newPinchMasks[1]|=contacts[i][1]&~0x10;
					}
				}
			}

		/* Calculate changes in pinch mask for each hand: */
		{
		Threads::Mutex::Lock pinchMaskLock(pinchMaskMutex);
		for(int i=0;i<2;++i)
			{
			/* Update button state for each changed bit: */
			int deltaPinchMask=newPinchMasks[i]^pinchMasks[i];
			for(int j=0;j<4;++j)
				{
				/* Invert device's finger count to assign index 0 to index finger, 3 to pinky: */
				int bit=1<<(3-j);
				if((deltaPinchMask&bit)&&reportEvents)
					setButtonState(i*4+j,newPinchMasks[i]&bit);
				}
			}
		
		/* Update pinch masks: */
		for(int i=0;i<2;++i)
			pinchMasks[i]=newPinchMasks[i];
		}
		}
	}

FakespacePinchGlove::FakespacePinchGlove(VRDevice::Factory* sFactory,VRDeviceManager* sDeviceManager,Misc::ConfigurationFile& configFile)
	:VRDevice(sFactory,sDeviceManager,configFile),
	 devicePort(configFile.retrieveString("./devicePort").c_str()),
	 reportEvents(false)
	{
	/* Set device configuration: */
	setNumButtons(2*4,configFile); // Four buttons for each hand
	
	/* Set device port parameters: */
	devicePort.ref();
	int deviceBaudRate=configFile.retrieveValue<int>("./deviceBaudRate");
	devicePort.setSerialSettings(deviceBaudRate,8,Comm::SerialPort::NoParity,1,false);
	devicePort.setRawMode(1,0);
	
	/* Synchronize the device's command stream: */
	#ifdef VERBOSE
	printf("FakespacePinchGlove: Synchronizing command stream\n");
	fflush(stdout);
	#endif
	devicePort.putChar('*'); // Write a byte to complete eventually incomplete command sequences
	devicePort.flush();
	if(!devicePort.waitForData(Misc::Time(0.5)))
		{
		devicePort.putChar('*'); // We screwed things up; write another byte to complete the first completion byte
		devicePort.flush();
		}
	if(devicePort.waitForData(Misc::Time(0.5)))
		ignoreReply(); // Ignore the device's reply
	else
		Misc::throwStdErr("FakespacePinchGlove: Pinch glove device not responding");
	
	/* Set device parameters: */
	char replyBuffer[256];
	
	/* Turn off timestamps: */
	#ifdef VERBOSE
	printf("FakespacePinchGlove: Turning off timestamps\n");
	fflush(stdout);
	#endif
	writeCommand("T0");
	readReply(256,replyBuffer);
	if(replyBuffer[0]!='0')
		Misc::throwStdErr("FakespacePinchGlove: Unable to disable time stamps on pinch glove device");
	
	/* Set inferface to version 1: */
	#ifdef VERBOSE
	printf("FakespacePinchGlove: Setting interface version\n");
	fflush(stdout);
	#endif
	writeCommand("V1");
	readReply(256,replyBuffer);
	if(replyBuffer[0]!='1')
		Misc::throwStdErr("FakespacePinchGlove: Unable to set interface version on pinch glove device");
	
	/* Initialize pinch masks: */
	for(int i=0;i<2;++i)
		pinchMasks[i]=0x0;
	
	/* Start device thread (pinch glove device cannot be disabled): */
	startDeviceThread();
	}

FakespacePinchGlove::~FakespacePinchGlove(void)
	{
	/* Stop device thread (pinch glove device cannot be disabled): */
	{
	Threads::Mutex::Lock pinchMaskLock(pinchMaskMutex);
	stopDeviceThread();
	}
	}

void FakespacePinchGlove::start(void)
	{
	/* Set device manager's button states to current pinch state: */
	{
	Threads::Mutex::Lock pinchMaskLock(pinchMaskMutex);
	for(int i=0;i<2;++i)
		for(int j=0;j<4;++j)
			setButtonState(i*4+j,pinchMasks[i]&(1<<j));
	
	/* Start reporting events to the device manager: */
	reportEvents=true;
	}
	}

void FakespacePinchGlove::stop(void)
	{
	/* Stop reporting events to the device manager: */
	{
	Threads::Mutex::Lock pinchMaskLock(pinchMaskMutex);
	reportEvents=false;
	}
	}

/*************************************
Object creation/destruction functions:
*************************************/

extern "C" VRDevice* createObjectFakespacePinchGlove(VRFactory<VRDevice>* factory,VRFactoryManager<VRDevice>* factoryManager,Misc::ConfigurationFile& configFile)
	{
	VRDeviceManager* deviceManager=static_cast<VRDeviceManager::DeviceFactoryManager*>(factoryManager)->getDeviceManager();
	return new FakespacePinchGlove(factory,deviceManager,configFile);
	}

extern "C" void destroyObjectFakespacePinchGlove(VRDevice* device,VRFactory<VRDevice>* factory,VRFactoryManager<VRDevice>* factoryManager)
	{
	delete device;
	}
