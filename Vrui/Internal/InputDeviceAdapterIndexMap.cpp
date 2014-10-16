/***********************************************************************
InputDeviceAdapterIndexMap - Base class for input device adapters that
use index maps to translate from "flat" device states to Vrui input
devices.
Copyright (c) 2004-2010 Oliver Kreylos

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

#include <Vrui/Internal/InputDeviceAdapterIndexMap.h>

#include <stdio.h>
#include <Misc/ThrowStdErr.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Vrui/InputDevice.h>

namespace Vrui {

/*******************************************
Methods of class InputDeviceAdapterIndexMap:
*******************************************/

void InputDeviceAdapterIndexMap::createIndexMappings(void)
	{
	/* If the index mapping arrays have not been created yet, do so now: */
	if(trackerIndexMapping==0)
		trackerIndexMapping=new int[numInputDevices];
	if(buttonIndexMapping==0)
		{
		buttonIndexMapping=new int*[numInputDevices];
		for(int i=0;i<numInputDevices;++i)
			buttonIndexMapping[i]=0;
		}
	if(valuatorIndexMapping==0)
		{
		valuatorIndexMapping=new int*[numInputDevices];
		for(int i=0;i<numInputDevices;++i)
			valuatorIndexMapping[i]=0;
		}
	}

void InputDeviceAdapterIndexMap::createInputDevice(int deviceIndex,const Misc::ConfigurationFileSection& configFileSection)
	{
	/* Ensure that the index mapping tables exist: */
	createIndexMappings();
	
	/* Call base class method to initialize basic input device settings: */
	InputDeviceAdapter::createInputDevice(deviceIndex,configFileSection);
	
	if(inputDevices[deviceIndex]->getTrackType()!=InputDevice::TRACK_NONE)
		{
		/* Create tracker index mapping: */
		int trackerIndex=configFileSection.retrieveValue<int>("./trackerIndex");
		if(trackerIndex>=numRawTrackers)
			Misc::throwStdErr("InputDeviceAdapterIndexMap: Tracker index out of valid range");
		trackerIndexMapping[deviceIndex]=trackerIndex;
		}
	else
		trackerIndexMapping[deviceIndex]=-1;
	
	/* Create button index mapping: */
	int numButtons=inputDevices[deviceIndex]->getNumButtons();
	if(numButtons>0)
		{
		buttonIndexMapping[deviceIndex]=new int[numButtons];
		int buttonIndexBase=configFileSection.retrieveValue<int>("./buttonIndexBase",0);
		for(int j=0;j<numButtons;++j)
			{
			char buttonIndexTag[40];
			snprintf(buttonIndexTag,sizeof(buttonIndexTag),"./buttonIndex%d",j);
			int buttonIndex=configFileSection.retrieveValue<int>(buttonIndexTag,buttonIndexBase+j);
			if(buttonIndex>=numRawButtons)
				Misc::throwStdErr("InputDeviceAdapterIndexMap: Button index out of valid range");
			buttonIndexMapping[deviceIndex][j]=buttonIndex;
			}
		}
	else
		buttonIndexMapping[deviceIndex]=0;
	
	/* Create valuator index mapping: */
	int numValuators=inputDevices[deviceIndex]->getNumValuators();
	if(numValuators>0)
		{
		valuatorIndexMapping[deviceIndex]=new int[numValuators];
		int valuatorIndexBase=configFileSection.retrieveValue<int>("./valuatorIndexBase",0);
		for(int j=0;j<numValuators;++j)
			{
			char valuatorIndexTag[40];
			snprintf(valuatorIndexTag,sizeof(valuatorIndexTag),"./valuatorIndex%d",j);
			int valuatorIndex=configFileSection.retrieveValue<int>(valuatorIndexTag,valuatorIndexBase+j);
			if(valuatorIndex>=numRawValuators)
				Misc::throwStdErr("InputDeviceAdapterIndexMap: Valuator index out of valid range");
			valuatorIndexMapping[deviceIndex][j]=valuatorIndex;
			}
		}
	else
		valuatorIndexMapping[deviceIndex]=0;
	}

void InputDeviceAdapterIndexMap::initializeAdapter(int newNumRawTrackers,int newNumRawButtons,int newNumRawValuators,const Misc::ConfigurationFileSection& configFileSection)
	{
	/* Set numbers of raw device states: */
	numRawTrackers=newNumRawTrackers;
	numRawButtons=newNumRawButtons;
	numRawValuators=newNumRawValuators;
	
	/* Call base class method to initialize input devices: */
	InputDeviceAdapter::initializeAdapter(configFileSection);
	}

InputDeviceAdapterIndexMap::~InputDeviceAdapterIndexMap(void)
	{
	/* Delete adapter state arrays: */
	delete[] trackerIndexMapping;
	if(buttonIndexMapping!=0)
		{
		for(int i=0;i<numInputDevices;++i)
			delete[] buttonIndexMapping[i];
		delete[] buttonIndexMapping;
		}
	if(valuatorIndexMapping!=0)
		{
		for(int i=0;i<numInputDevices;++i)
			delete[] valuatorIndexMapping[i];
		delete[] valuatorIndexMapping;
		}
	}

}
