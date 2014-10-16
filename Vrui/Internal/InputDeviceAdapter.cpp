/***********************************************************************
InputDeviceAdapter - Base class to convert from diverse "raw" input
device representations to Vrui's internal input device representation.
Copyright (c) 2004-2013 Oliver Kreylos

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

#include <Vrui/Internal/InputDeviceAdapter.h>

#include <string.h>
#include <stdio.h>
#include <Misc/ThrowStdErr.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/CompoundValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Geometry/Vector.h>
#include <Geometry/GeometryValueCoders.h>
#include <Vrui/Vrui.h>
#include <Vrui/GlyphRenderer.h>
#include <Vrui/InputDevice.h>
#include <Vrui/InputDeviceFeature.h>
#include <Vrui/InputGraphManager.h>
#include <Vrui/InputDeviceManager.h>

namespace Vrui {

/***********************************
Methods of class InputDeviceAdapter:
***********************************/

void InputDeviceAdapter::createInputDevice(int deviceIndex,const Misc::ConfigurationFileSection& configFileSection)
	{
	/* Read input device name: */
	std::string name=configFileSection.retrieveString("./name");
	
	/* Determine input device type: */
	int trackType=InputDevice::TRACK_NONE;
	std::string trackTypeString=configFileSection.retrieveString("./trackType","None");
	if(trackTypeString=="None")
		trackType=InputDevice::TRACK_NONE;
	else if(trackTypeString=="3D")
		trackType=InputDevice::TRACK_POS;
	else if(trackTypeString=="Ray")
		trackType=InputDevice::TRACK_POS|InputDevice::TRACK_DIR;
	else if(trackTypeString=="6D")
		trackType=InputDevice::TRACK_POS|InputDevice::TRACK_DIR|InputDevice::TRACK_ORIENT;
	else
		Misc::throwStdErr("InputDeviceAdapter: Unknown tracking type \"%s\"",trackTypeString.c_str());
	
	/* Determine numbers of buttons and valuators: */
	int numButtons=configFileSection.retrieveValue<int>("./numButtons",0);
	int numValuators=configFileSection.retrieveValue<int>("./numValuators",0);
	
	/* Create new input device as a physical device: */
	InputDevice* newDevice=inputDeviceManager->createInputDevice(name.c_str(),trackType,numButtons,numValuators,true);
	Vector deviceRayDirection=configFileSection.retrieveValue<Vector>("./deviceRayDirection",Vector(0,1,0));
	Scalar deviceRayStart=configFileSection.retrieveValue<Scalar>("./deviceRayStart",-getInchFactor());
	newDevice->setDeviceRay(deviceRayDirection,deviceRayStart);
	
	/* Initialize the new device's glyph from the current configuration file section: */
	Glyph& deviceGlyph=inputDeviceManager->getInputGraphManager()->getInputDeviceGlyph(newDevice);
	deviceGlyph.configure(configFileSection,"./deviceGlyphType","./deviceGlyphMaterial");
	
	/* Save the new input device: */
	inputDevices[deviceIndex]=newDevice;
	}

void InputDeviceAdapter::initializeAdapter(const Misc::ConfigurationFileSection& configFileSection)
	{
	/* Allocate adapter state arrays: */
	typedef std::vector<std::string> StringList;
	StringList inputDeviceNames=configFileSection.retrieveValue<StringList>("./inputDeviceNames");
	numInputDevices=inputDeviceNames.size();
	inputDevices=new InputDevice*[numInputDevices];
	for(int i=0;i<numInputDevices;++i)
		inputDevices[i]=0;
	
	/* Initialize input devices: */
	for(int i=0;i<numInputDevices;++i)
		{
		/* Go to device's section: */
		Misc::ConfigurationFileSection deviceSection=configFileSection.getSection(inputDeviceNames[i].c_str());
		
		/* Initialize input device: */
		createInputDevice(i,deviceSection);
		}
	}

InputDeviceAdapter::~InputDeviceAdapter(void)
	{
	/* Destroy all input devices: */
	for(int i=0;i<numInputDevices;++i)
		if(inputDevices[i]!=0)
			inputDeviceManager->destroyInputDevice(inputDevices[i]);
	delete[] inputDevices;
	}

std::string InputDeviceAdapter::getDefaultFeatureName(const InputDeviceFeature& feature)
	{
	char featureName[40];
	featureName[0]='\0';
	
	/* Check if the feature is a button or a valuator: */
	if(feature.isButton())
		{
		/* Return a default button name: */
		snprintf(featureName,sizeof(featureName),"Button%d",feature.getIndex());
		}
	if(feature.isValuator())
		{
		/* Return a default valuator name: */
		snprintf(featureName,sizeof(featureName),"Valuator%d",feature.getIndex());
		}
	
	return std::string(featureName);
	}

int InputDeviceAdapter::getDefaultFeatureIndex(InputDevice* device,const char* featureName)
	{
	int result=-1;
	
	/* Check if the feature names a button or a valuator: */
	if(strncmp(featureName,"Button",6)==0)
		{
		/* Get the button index: */
		int buttonIndex=atoi(featureName+6);
		if(buttonIndex<device->getNumButtons())
			result=device->getButtonFeatureIndex(buttonIndex);
		else
			result=-1;
		}
	if(strncmp(featureName,"Valuator",8)==0)
		{
		/* Get the valuator index: */
		int valuatorIndex=atoi(featureName+8);
		if(valuatorIndex<device->getNumValuators())
			result=device->getValuatorFeatureIndex(valuatorIndex);
		else
			result=-1;
		}
	
	return result;
	}

std::string InputDeviceAdapter::getFeatureName(const InputDeviceFeature& feature) const
	{
	/* Return a default feature name: */
	return getDefaultFeatureName(feature);
	}

int InputDeviceAdapter::getFeatureIndex(InputDevice* device,const char* featureName) const
	{
	/* Parse a default feature name: */
	return getDefaultFeatureIndex(device,featureName);
	}

void InputDeviceAdapter::glRenderAction(GLContextData& contextData) const
	{
	}

}
