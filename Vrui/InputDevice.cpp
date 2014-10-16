/***********************************************************************
InputDevice - Class to represent input devices (6-DOF tracker with
associated buttons and valuators) in virtual reality environments.
Copyright (c) 2000-2014 Oliver Kreylos

This file is part of the Virtual Reality User Interface Library (Vrui).

The Virtual Reality User Interface Library is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The Virtual Reality User Interface Library is distributed in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Virtual Reality User Interface Library; if not, write to the
Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#include <string.h>
#include <Misc/ThrowStdErr.h>

#include <Vrui/InputDevice.h>

namespace Vrui {

/****************************
Methods of class InputDevice:
****************************/

InputDevice::InputDevice(void)
	:deviceName(new char[1]),trackType(TRACK_NONE),
	 numButtons(0),numValuators(0),
	 buttonCallbacks(0),valuatorCallbacks(0),
	 deviceRayDirection(0,1,0),deviceRayStart(0),
	 transformation(TrackerState::identity),linearVelocity(Vector::zero),angularVelocity(Vector::zero),
	 buttonStates(0),valuatorValues(0),
	 callbacksEnabled(true),
	 savedButtonStates(0),savedValuatorValues(0)
	{
	deviceName[0]='\0';
	}

InputDevice::InputDevice(const char* sDeviceName,int sTrackType,int sNumButtons,int sNumValuators)
	:deviceName(new char[strlen(sDeviceName)+1]),trackType(sTrackType),
	 numButtons(sNumButtons),numValuators(sNumValuators),
	 buttonCallbacks(numButtons>0?new Misc::CallbackList[numButtons]:0),
	 valuatorCallbacks(numValuators>0?new Misc::CallbackList[numValuators]:0),
	 deviceRayDirection(0,1,0),deviceRayStart(0),
	 transformation(TrackerState::identity),linearVelocity(Vector::zero),angularVelocity(Vector::zero),
	 buttonStates(numButtons>0?new bool[numButtons]:0),
	 valuatorValues(numValuators>0?new double[numValuators]:0),
	 callbacksEnabled(true),
	 savedButtonStates(numButtons>0?new bool[numButtons]:0),
	 savedValuatorValues(numValuators>0?new double[numValuators]:0)
	{
	/* Copy device name: */
	strcpy(deviceName,sDeviceName);
	
	/* Initialize button and valuator states: */
	for(int i=0;i<numButtons;++i)
		{
		buttonStates[i]=false;
		savedButtonStates[i]=false;
		}
	for(int i=0;i<numValuators;++i)
		{
		valuatorValues[i]=0.0;
		savedValuatorValues[i]=0.0;
		}
	}

InputDevice::InputDevice(const InputDevice& source)
	:deviceName(new char[1]),trackType(TRACK_NONE),
	 numButtons(0),numValuators(0),
	 buttonCallbacks(0),valuatorCallbacks(0),
	 deviceRayDirection(0,1,0),deviceRayStart(0),
	 transformation(TrackerState::identity),linearVelocity(Vector::zero),angularVelocity(Vector::zero),
	 buttonStates(0),valuatorValues(0),
	 callbacksEnabled(true),
	 savedButtonStates(0),savedValuatorValues(0)
	{
	deviceName[0]='\0';
	
	/*********************************************************************
	Since we don't actually copy the source data here, throw an exception
	if somebody attempts to copy an already initialized input device.
	That'll teach them.
	*********************************************************************/
	
	if(source.deviceName[0]!='\0'||source.numButtons!=0||source.numValuators!=0)
		Misc::throwStdErr("InputDevice: Attempt to copy initialized input device");
	}

InputDevice::~InputDevice(void)
	{
	delete[] deviceName;
	
	/* Delete state arrays: */
	delete[] buttonCallbacks;
	delete[] valuatorCallbacks;
	delete[] buttonStates;
	delete[] valuatorValues;
	delete[] savedButtonStates;
	delete[] savedValuatorValues;
	}

InputDevice& InputDevice::set(const char* sDeviceName,int sTrackType,int sNumButtons,int sNumValuators)
	{
	delete[] deviceName;
	
	/* Delete old state arrays: */
	delete[] buttonCallbacks;
	delete[] valuatorCallbacks;
	delete[] buttonStates;
	delete[] valuatorValues;
	delete[] savedButtonStates;
	delete[] savedValuatorValues;
	
	/* Set new device layout: */
	deviceName=new char[strlen(sDeviceName)+1];
	strcpy(deviceName,sDeviceName);
	trackType=sTrackType;
	numButtons=sNumButtons;
	numValuators=sNumValuators;
	
	/* Allocate new state arrays: */
	buttonCallbacks=numButtons!=0?new Misc::CallbackList[numButtons]:0;
	valuatorCallbacks=numValuators>0?new Misc::CallbackList[numValuators]:0;
	buttonStates=numButtons!=0?new bool[numButtons]:0;
	valuatorValues=numValuators>0?new double[numValuators]:0;
	savedButtonStates=numButtons!=0?new bool[numButtons]:0;
	savedValuatorValues=numValuators>0?new double[numValuators]:0;
	
	/* Clear all button and valuator states: */
	for(int i=0;i<numButtons;++i)
		{
		buttonStates[i]=false;
		savedButtonStates[i]=false;
		}
	for(int i=0;i<numValuators;++i)
		{
		valuatorValues[i]=0.0;
		savedValuatorValues[i]=0.0;
		}
	
	return *this;
	}

void InputDevice::setTrackType(int newTrackType)
	{
	/* Set the tracking type: */
	trackType=newTrackType;
	}

void InputDevice::setDeviceRay(const Vector& newDeviceRayDirection,Scalar newDeviceRayStart)
	{
	/* Set ray direction and starting parameter: */
	deviceRayDirection=newDeviceRayDirection;
	deviceRayStart=newDeviceRayStart;
	}

void InputDevice::setTransformation(const TrackerState& newTransformation)
	{
	/* Set transformation: */
	transformation=newTransformation;
	
	/* Call callbacks: */
	if(callbacksEnabled)
		{
		/* Call all tracking callbacks: */
		CallbackData cbData(this);
		trackingCallbacks.call(&cbData);
		}
	}

void InputDevice::copyTrackingState(const InputDevice* source)
	{
	deviceRayDirection=source->deviceRayDirection;
	deviceRayStart=source->deviceRayStart;
	transformation=source->transformation;
	linearVelocity=source->linearVelocity;
	angularVelocity=source->angularVelocity;
	}

void InputDevice::clearButtonStates(void)
	{
	for(int i=0;i<numButtons;++i)
		{
		if(buttonStates[i])
			{
			buttonStates[i]=false;
			if(callbacksEnabled)
				{
				ButtonCallbackData cbData(this,i,false);
				buttonCallbacks[i].call(&cbData);
				}
			}
		}
	}

void InputDevice::setButtonState(int index,bool newButtonState)
	{
	ButtonCallbackData cbData(this,index,newButtonState);
	if(buttonStates[index]!=newButtonState)
		{
		buttonStates[index]=newButtonState;
		if(callbacksEnabled)
			buttonCallbacks[index].call(&cbData);
		}
	}

void InputDevice::setSingleButtonPressed(int index)
	{
	for(int i=0;i<numButtons;++i)
		{
		if(i!=index)
			{
			if(buttonStates[i])
				{
				buttonStates[i]=false;
				if(callbacksEnabled)
					{
					ButtonCallbackData cbData(this,i,false);
					buttonCallbacks[i].call(&cbData);
					}
				}
			}
		}
	ButtonCallbackData cbData(this,index,true);
	if(!buttonStates[index])
		{
		buttonStates[index]=true;
		if(callbacksEnabled)
			buttonCallbacks[index].call(&cbData);
		}
	}

void InputDevice::setValuator(int index,double value)
	{
	ValuatorCallbackData cbData(this,index,valuatorValues[index],value);
	if(valuatorValues[index]!=value)
		{
		valuatorValues[index]=value;
		if(callbacksEnabled)
			valuatorCallbacks[index].call(&cbData);
		}
	}

void InputDevice::disableCallbacks(void)
	{
	callbacksEnabled=false;
	
	/* Save all button states and valuator values to call the appropriate callbacks once callbacks are enabled again: */
	for(int i=0;i<numButtons;++i)
		savedButtonStates[i]=buttonStates[i];
	for(int i=0;i<numValuators;++i)
		savedValuatorValues[i]=valuatorValues[i];
	}

void InputDevice::enableCallbacks(void)
	{
	callbacksEnabled=true;
	
	/* Call callbacks for everything that has changed, to update the user program's state: */
	CallbackData trackingCbData(this);
	trackingCallbacks.call(&trackingCbData);
	for(int i=0;i<numButtons;++i)
		if(savedButtonStates[i]!=buttonStates[i])
			{
			ButtonCallbackData cbData(this,i,buttonStates[i]);
			buttonCallbacks[i].call(&cbData);
			}
	for(int i=0;i<numValuators;++i)
		if(savedValuatorValues[i]!=valuatorValues[i])
			{
			ValuatorCallbackData cbData(this,i,savedValuatorValues[i],valuatorValues[i]);
			valuatorCallbacks[i].call(&cbData);
			}
	}

}
