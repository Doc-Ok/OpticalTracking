/***********************************************************************
PCWand - Class for communicating with button/valuator devices on a
dedicated DOS PC.
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

#include <VRDeviceDaemon/VRDevices/PCWand.h>

#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Math/Math.h>
#include <Math/Constants.h>

#include <VRDeviceDaemon/VRDeviceManager.h>

/***********************
Methods of class PCWand:
***********************/

void PCWand::deviceThreadMethod(void)
	{
	/* Process messages until killed: */
	while(true)
		{
		/* Wait for next message: */
		unsigned char byte=(unsigned char)devicePort.getChar();
		
		/* Parse message: */
		if(byte>='0'&&byte<='1')
			{
			/* It's a valuator value packet; skip the next byte and read the value: */
			int valuatorIndex=byte-'0';
			devicePort.getChar();
			unsigned char valueByte=(unsigned char)devicePort.getChar();
			float value=(float(valueByte)*2.0f)/255.f-1.0f;
			deviceValuatorStates[valuatorIndex]=value;
			
			/* Set the valuator value in the device manager: */
			{
			Threads::Mutex::Lock deviceValuesLock(deviceValuesMutex);
			if(reportEvents)
				setValuatorState(valuatorIndex,value);
			}
			}
		else if(byte>=216&&byte<=218)
			{
			/* It's a button press packet: */
			int buttonIndex=byte-216;
			deviceButtonStates[buttonIndex]=true;
			
			/* Set the button state in the device manager: */
			{
			Threads::Mutex::Lock deviceValuesLock(deviceValuesMutex);
			if(reportEvents)
				setButtonState(buttonIndex,true);
			}
			}
		else if(byte>=248&&byte<=250)
			{
			/* It's a button release packet: */
			int buttonIndex=byte-248;
			deviceButtonStates[buttonIndex]=false;
			
			/* Set the button state in the device manager: */
			{
			Threads::Mutex::Lock deviceValuesLock(deviceValuesMutex);
			if(reportEvents)
				setButtonState(buttonIndex,false);
			}
			}
		}
	}

PCWand::PCWand(VRDevice::Factory* sFactory,VRDeviceManager* sDeviceManager,Misc::ConfigurationFile& configFile)
	:VRDevice(sFactory,sDeviceManager,configFile),
	 devicePort(configFile.retrieveString("./devicePort").c_str()),
	 reportEvents(false)
	{
	/* Set device configuration: */
	setNumButtons(3,configFile);
	setNumValuators(2,configFile);
	
	/* Set device port parameters: */
	devicePort.ref();
	int deviceBaudRate=configFile.retrieveValue<int>("./deviceBaudRate");
	devicePort.setSerialSettings(deviceBaudRate,8,Comm::SerialPort::NoParity,1,false);
	devicePort.setRawMode(1,0);
	
	/* Initialize device values: */
	for(int i=0;i<numButtons;++i)
		deviceButtonStates[i]=false;
	for(int i=0;i<numValuators;++i)
		deviceValuatorStates[i]=0.0f;
	
	/* Start device thread (dedicated PC cannot be disabled): */
	startDeviceThread();
	}

PCWand::~PCWand(void)
	{
	/* Stop device thread (dedicated PC cannot be disabled): */
	{
	Threads::Mutex::Lock deviceValuesLock(deviceValuesMutex);
	stopDeviceThread();
	}
	}

void PCWand::start(void)
	{
	/* Set device manager's button and valuator values to current device values: */
	{
	Threads::Mutex::Lock deviceValuesLock(deviceValuesMutex);
	for(int i=0;i<numButtons;++i)
		setButtonState(i,deviceButtonStates[i]);
	for(int i=0;i<numValuators;++i)
		setValuatorState(i,deviceValuatorStates[i]);
	
	/* Start reporting events to the device manager: */
	reportEvents=true;
	}
	}

void PCWand::stop(void)
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

extern "C" VRDevice* createObjectPCWand(VRFactory<VRDevice>* factory,VRFactoryManager<VRDevice>* factoryManager,Misc::ConfigurationFile& configFile)
	{
	VRDeviceManager* deviceManager=static_cast<VRDeviceManager::DeviceFactoryManager*>(factoryManager)->getDeviceManager();
	return new PCWand(factory,deviceManager,configFile);
	}

extern "C" void destroyObjectPCWand(VRDevice* device,VRFactory<VRDevice>* factory,VRFactoryManager<VRDevice>* factoryManager)
	{
	delete device;
	}
