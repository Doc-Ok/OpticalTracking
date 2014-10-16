/***********************************************************************
Joystick - VR device driver class for joysticks having arbitrary numbers
of axes and buttons using the Linux joystick driver API (thanks
Vojtech).
Copyright (c) 2004-2010 Oliver Kreylos

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

#include <VRDeviceDaemon/VRDevices/Joystick.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/joystick.h>
#include <Misc/ThrowStdErr.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>

#include <VRDeviceDaemon/VRDeviceManager.h>

/*************************
Methods of class Joystick:
*************************/

void Joystick::deviceThreadMethod(void)
	{
	while(true)
		{
		/* Try reading a bunch of joystick events: */
		struct js_event joyEvents[32];
		int numEvents=read(joystickDeviceFd,joyEvents,32*sizeof(struct js_event));
		
		/* Process all reported joystick events: */
		if(numEvents>0)
			{
			{
			Threads::Mutex::Lock stateLock(stateMutex);
			numEvents/=sizeof(struct js_event);
			for(int i=0;i<numEvents;++i)
				{
				switch(joyEvents[i].type&~JS_EVENT_INIT)
					{
					case JS_EVENT_BUTTON:
						{
						int buttonIndex=joyEvents[i].number;
						bool newButtonState=joyEvents[i].value;
						if(newButtonState!=buttonStates[buttonIndex]&&reportEvents)
							setButtonState(buttonIndex,newButtonState);
						buttonStates[buttonIndex]=newButtonState;
						break;
						}
					
					case JS_EVENT_AXIS:
						{
						int valuatorIndex=joyEvents[i].number;
						float newValuatorState=float(joyEvents[i].value)/32767.0f;
						newValuatorState=Math::pow(newValuatorState,axisGains[valuatorIndex]);
						if(newValuatorState!=valuatorStates[valuatorIndex]&&reportEvents)
							setValuatorState(valuatorIndex,newValuatorState);
						valuatorStates[valuatorIndex]=newValuatorState;
						break;
						}
					}
				}
			}
			}
		}
	}

Joystick::Joystick(VRDevice::Factory* sFactory,VRDeviceManager* sDeviceManager,Misc::ConfigurationFile& configFile)
	:VRDevice(sFactory,sDeviceManager,configFile),
	 joystickDeviceFd(open(configFile.retrieveString("./joystickDeviceFile").c_str(),O_RDONLY)),
	 axisGains(0),
	 reportEvents(false),
	 buttonStates(0),valuatorStates(0)
	{
	/* Check if the joystick device port was properly opened: */
	if(joystickDeviceFd<0)
		Misc::throwStdErr("Joystick: Unable to open joystick device port");
	
	/* Query the joystick's geometry: */
	char cNumButtons,cNumAxes;
	ioctl(joystickDeviceFd,JSIOCGBUTTONS,&cNumButtons);
	ioctl(joystickDeviceFd,JSIOCGAXES,&cNumAxes);
	#ifdef VERBOSE
	/* Query the joystick's name: */
	char joystickName[256];
	if(ioctl(joystickDeviceFd,JSIOCGNAME(sizeof(joystickName)),joystickName)>=0)
		{
		joystickName[sizeof(joystickName)-1]='\0';
		printf("Joystick: %s with %d buttons and %d axes found\n",joystickName,int(cNumButtons),int(cNumAxes));
		}
	else
		printf("Joystick: Unknown joystick with %d buttons and %d axes found\n",int(cNumButtons),int(cNumAxes));
	#endif
	
	/* Set device configuration: */
	setNumTrackers(0,configFile);
	setNumButtons(cNumButtons,configFile);
	setNumValuators(cNumAxes,configFile);
	
	/* Initialize gain arrays: */
	axisGains=new float[getNumValuators()];
	for(int i=0;i<getNumValuators();++i)
		{
		char axisGainTag[40];
		snprintf(axisGainTag,sizeof(axisGainTag),"./axisGain%d",i);
		axisGains[i]=configFile.retrieveValue<float>(axisGainTag,1.0f);
		}
	
	/* Initialize state arrays: */
	buttonStates=new bool[getNumButtons()];
	for(int i=0;i<getNumButtons();++i)
		buttonStates[i]=false;
	valuatorStates=new float[getNumValuators()];
	for(int i=0;i<getNumValuators();++i)
		valuatorStates[i]=0.0f;
	
	/* Start device thread (joystick device cannot be disabled): */
	startDeviceThread();
	}

Joystick::~Joystick(void)
	{
	/* Stop device thread (joystick device cannot be disabled): */
	{
	Threads::Mutex::Lock stateLock(stateMutex);
	stopDeviceThread();
	}
	delete[] buttonStates;
	delete[] valuatorStates;
	delete[] axisGains;
	close(joystickDeviceFd);
	}

void Joystick::start(void)
	{
	/* Set device manager's button and valuator states to current states: */
	{
	Threads::Mutex::Lock stateLock(stateMutex);
	for(int i=0;i<getNumButtons();++i)
		setButtonState(i,buttonStates[i]);
	for(int i=0;i<getNumValuators();++i)
		setValuatorState(i,valuatorStates[i]);
	
	/* Start reporting events to the device manager: */
	reportEvents=true;
	}
	}

void Joystick::stop(void)
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

extern "C" VRDevice* createObjectJoystick(VRFactory<VRDevice>* factory,VRFactoryManager<VRDevice>* factoryManager,Misc::ConfigurationFile& configFile)
	{
	VRDeviceManager* deviceManager=static_cast<VRDeviceManager::DeviceFactoryManager*>(factoryManager)->getDeviceManager();
	return new Joystick(factory,deviceManager,configFile);
	}

extern "C" void destroyObjectJoystick(VRDevice* device,VRFactory<VRDevice>* factory,VRFactoryManager<VRDevice>* factoryManager)
	{
	delete device;
	}
