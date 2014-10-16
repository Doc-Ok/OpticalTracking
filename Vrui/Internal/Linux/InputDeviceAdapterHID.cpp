/***********************************************************************
InputDeviceAdapterHID - Linux-specific version of HID input device
adapter.
Copyright (c) 2009-2014 Oliver Kreylos

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

#include <Vrui/Internal/Linux/InputDeviceAdapterHID.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#ifndef __USE_GNU
#define __USE_GNU
#include <dirent.h>
#undef __USE_GNU
#else
#include <dirent.h>
#endif
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <Misc/ThrowStdErr.h>
#include <Misc/FdSet.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/CompoundValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Math/MathValueCoders.h>
#include <Vrui/InputDevice.h>
#include <Vrui/InputDeviceFeature.h>
#include <Vrui/InputDeviceManager.h>
#include <Vrui/Vrui.h>
#include <Vrui/Internal/Config.h>

#if !VRUI_INTERNAL_CONFIG_INPUT_H_HAS_STRUCTS

/*******************************************************************
Classes to deal with HID devices (should all be defined in input.h):
*******************************************************************/

struct input_id
	{
	/* Elements: */
	public:
	unsigned short bustype;
	unsigned short vendor;
	unsigned short product;
	unsigned short version;
	};

struct input_absinfo
	{
	/* Elements: */
	public:
	int value;
	int minimum;
	int maximum;
	int fuzz;
	int flat;
	};

#endif

namespace {

/****************
Helper functions:
****************/

int isEventFile(const struct dirent* directoryEntry)
	{
	return strncmp(directoryEntry->d_name,"event",5)==0;
	}

}

namespace Vrui {

/**************************************
Methods of class InputDeviceAdapterHID:
**************************************/

void InputDeviceAdapterHID::createInputDevice(int deviceIndex,const Misc::ConfigurationFileSection& configFileSection)
	{
	/* Read input device name: */
	std::string name=configFileSection.retrieveString("./name");
	
	/* Read HID's vendor / product IDs: */
	std::string deviceVendorProductId=configFileSection.retrieveString("./deviceVendorProductId");
	
	/* Split ID string into vendor ID / product ID: */
	char* colonPtr;
	unsigned int vendorId=strtoul(deviceVendorProductId.c_str(),&colonPtr,16);
	char* endPtr;
	unsigned int productId=strtoul(colonPtr+1,&endPtr,16);
	if(*colonPtr!=':'||*endPtr!='\0')
		Misc::throwStdErr("InputDeviceAdapterHID::InputDeviceAdapterHID: Malformed vendorId:productId string \"%s\" for device %s",deviceVendorProductId.c_str(),name.c_str());
	
	/* Get the device index: */
	int matchingDeviceIndex=configFileSection.retrieveValue<int>("./deviceIndex",0);
	
	/* Create list of all available /dev/input/eventX devices, in numerical order: */
	struct dirent** eventFiles=0;
	int numEventFiles=scandir("/dev/input",&eventFiles,isEventFile,versionsort);
	
	/* Check all event files for the wanted device: */
	int deviceFd=-1;
	for(int eventFileIndex=0;eventFileIndex<numEventFiles;++eventFileIndex)
		{
		/* Open the event file: */
		char eventFileName[256];
		snprintf(eventFileName,sizeof(eventFileName),"/dev/input/%s",eventFiles[eventFileIndex]->d_name);
		int eventFd=open(eventFileName,O_RDONLY);
		if(eventFd>=0)
			{
			/* Get device information: */
			input_id deviceInformation;
			if(ioctl(eventFd,EVIOCGID,&deviceInformation)>=0)
				{
				if(deviceInformation.vendor==vendorId&&deviceInformation.product==productId)
					{
					/* We have a match: */
					if(matchingDeviceIndex==0)
						{
						/* We have a winner! */
						deviceFd=eventFd;
						break;
						}
					
					/* Try again on the next matching device: */
					--matchingDeviceIndex;
					}
				}
			
			/* This is not the device you are looking for, go to the next: */
			close(eventFd);
			}
		}
	
	/* Destroy list of event files: */
	for(int i=0;i<numEventFiles;++i)
		free(eventFiles[i]);
	free(eventFiles);
	
	/* Check if a matching device was found: */
	if(deviceFd<0)
		Misc::throwStdErr("InputDeviceAdapterHID::InputDeviceAdapterHID: No match for vendorId:productId \"%s\" for device %s",deviceVendorProductId.c_str(),name.c_str());
	
	/* Create a new device structure: */
	Device newDevice;
	newDevice.deviceFd=deviceFd;
	
	/* Query all feature types of the device: */
	unsigned char featureTypeBits[EV_MAX/8+1];
	memset(featureTypeBits,0,EV_MAX/8+1);
	if(ioctl(deviceFd,EVIOCGBIT(0,sizeof(featureTypeBits)),featureTypeBits)<0)
		Misc::throwStdErr("InputDeviceAdapterHID::InputDeviceAdapterHID: Unable to query device feature types for device %s",name.c_str());
	
	/* Count the number of keys: */
	newDevice.numButtons=0;
	
	/* Query the number of keys/buttons on the device: */
	if(featureTypeBits[EV_KEY/8]&(1<<(EV_KEY%8)))
		{
		/* Query key features: */
		unsigned char keyBits[KEY_MAX/8+1];
		memset(keyBits,0,KEY_MAX/8+1);
		if(ioctl(deviceFd,EVIOCGBIT(EV_KEY,sizeof(keyBits)),keyBits)<0)
			Misc::throwStdErr("InputDeviceAdapterHID::InputDeviceAdapterHID: Unable to query keys for device %s",name.c_str());
		
		/* Initialize the key translation array: */
		newDevice.keyMap.reserve(KEY_MAX+1);
		for(int i=0;i<=KEY_MAX;++i)
			{
			if(keyBits[i/8]&(1<<(i%8)))
				{
				newDevice.keyMap.push_back(newDevice.numButtons);
				++newDevice.numButtons;
				}
			else
				newDevice.keyMap.push_back(-1);
			}
		}
	
	/* Count the number of absolute and relative axes: */
	newDevice.numValuators=0;
	
	/* Query the number of absolute axes on the device: */
	if(featureTypeBits[EV_ABS/8]&(1<<(EV_ABS%8)))
		{
		/* Query absolute axis features: */
		unsigned char absAxisBits[ABS_MAX/8+1];
		memset(absAxisBits,0,ABS_MAX/8+1);
		if(ioctl(deviceFd,EVIOCGBIT(EV_ABS,sizeof(absAxisBits)),absAxisBits)<0)
			Misc::throwStdErr("InputDeviceAdapterHID::InputDeviceAdapterHID: Unable to query absolute axes for device %s",name.c_str());
		
		/* Initialize the axis translation array: */
		newDevice.absAxisMap.reserve(ABS_MAX+1);
		for(int i=0;i<=ABS_MAX;++i)
			{
			if(absAxisBits[i/8]&(1<<(i%8)))
				{
				/* Enter the next valuator index into the axis map: */
				newDevice.absAxisMap.push_back(newDevice.numValuators);
				
				/* Query the configuration of this axis: */
				input_absinfo absAxisConf;
				if(ioctl(deviceFd,EVIOCGABS(i),&absAxisConf)<0)
					Misc::throwStdErr("InputDeviceAdapterHID::InputDeviceAdapterHID: Unable to query absolute axis configuration for device %s",name.c_str());
				
				/* Create an absolute axis value mapper: */
				Device::AxisValueMapper avm;
				avm.min=double(absAxisConf.minimum);
				double mid=Math::mid(double(absAxisConf.minimum),double(absAxisConf.maximum));
				avm.deadMin=mid-double(absAxisConf.flat);
				avm.deadMax=mid+double(absAxisConf.flat);
				avm.max=double(absAxisConf.maximum);
				
				/* Override axis value mapper from configuration file: */
				char axisSettingsTag[20];
				snprintf(axisSettingsTag,sizeof(axisSettingsTag),"axis%dSettings",newDevice.numValuators);
				avm=configFileSection.retrieveValue<Device::AxisValueMapper>(axisSettingsTag,avm);
				
				/* Store the axis value mapper: */
				newDevice.axisValueMappers.push_back(avm);
				
				++newDevice.numValuators;
				}
			else
				newDevice.absAxisMap.push_back(-1);
			}
		}
	
	/* Query the number of relative axes on the device: */
	if(featureTypeBits[EV_REL/8]&(1<<(EV_REL%8)))
		{
		/* Query relative axis features: */
		unsigned char relAxisBits[REL_MAX/8+1];
		memset(relAxisBits,0,REL_MAX/8+1);
		if(ioctl(deviceFd,EVIOCGBIT(EV_REL,sizeof(relAxisBits)),relAxisBits)<0)
			Misc::throwStdErr("InputDeviceAdapterHID::InputDeviceAdapterHID: Unable to query relative axes for device %s",name.c_str());
		
		/* Initialize the axis translation array: */
		newDevice.relAxisMap.reserve(REL_MAX+1);
		for(int i=0;i<=REL_MAX;++i)
			{
			if(relAxisBits[i/8]&(1<<(i%8)))
				{
				/* Enter the next valuator index into the axis map: */
				newDevice.relAxisMap.push_back(newDevice.numValuators);
				
				/* Create a relative axis value mapper: */
				Device::AxisValueMapper avm;
				avm.min=-1.0;
				avm.deadMin=0.0;
				avm.deadMax=0.0;
				avm.max=1.0;
				
				/* Override axis value mapper from configuration file: */
				char axisSettingsTag[20];
				snprintf(axisSettingsTag,sizeof(axisSettingsTag),"axis%dSettings",newDevice.numValuators);
				avm=configFileSection.retrieveValue<Device::AxisValueMapper>(axisSettingsTag,avm);
				
				/* Store the axis value mapper: */
				newDevice.axisValueMappers.push_back(avm);
				
				++newDevice.numValuators;
				}
			else
				newDevice.relAxisMap.push_back(-1);
			}
		}
	
	/* Check if the device is supposed to copy tracking data from another device: */
	newDevice.trackingDevice=0;
	if(configFileSection.hasTag("./trackingDeviceName"))
		{
		std::string trackingDeviceName=configFileSection.retrieveString("./trackingDeviceName");
		newDevice.trackingDevice=Vrui::findInputDevice(trackingDeviceName.c_str());
		if(newDevice.trackingDevice==0)
			Misc::throwStdErr("InputDeviceAdapterHID::InputDeviceAdapterHID: Tracking device %s not found",trackingDeviceName.c_str());
		}
	
	if(newDevice.trackingDevice!=0)
		{
		/* Create new input device as a physical device locked to the tracking device: */
		inputDevices[deviceIndex]=newDevice.device=inputDeviceManager->createInputDevice(name.c_str(),newDevice.trackingDevice->getTrackType(),newDevice.numButtons,newDevice.numValuators,true);
		inputDevices[deviceIndex]->copyTrackingState(newDevice.trackingDevice);
		}
	else
		{
		/* Create new input device as a non-tracked physical device: */
		inputDevices[deviceIndex]=newDevice.device=inputDeviceManager->createInputDevice(name.c_str(),InputDevice::TRACK_NONE,newDevice.numButtons,newDevice.numValuators,true);
		}
	
	/* Read the names of all button features: */
	typedef std::vector<std::string> StringList;
	StringList buttonNames=configFileSection.retrieveValue<StringList>("./buttonNames",StringList());
	int buttonIndex=0;
	for(StringList::iterator bnIt=buttonNames.begin();bnIt!=buttonNames.end()&&buttonIndex<newDevice.numButtons;++bnIt,++buttonIndex)
		{
		/* Store the button name: */
		newDevice.buttonNames.push_back(*bnIt);
		}
	for(;buttonIndex<newDevice.numButtons;++buttonIndex)
		{
		char buttonName[40];
		snprintf(buttonName,sizeof(buttonName),"Button%d",buttonIndex);
		newDevice.buttonNames.push_back(buttonName);
		}
	
	/* Read the names of all valuator features: */
	StringList valuatorNames=configFileSection.retrieveValue<StringList>("./valuatorNames",StringList());
	int valuatorIndex=0;
	for(StringList::iterator vnIt=valuatorNames.begin();vnIt!=valuatorNames.end()&&valuatorIndex<newDevice.numValuators;++vnIt,++valuatorIndex)
		{
		/* Store the valuator name: */
		newDevice.valuatorNames.push_back(*vnIt);
		}
	for(;valuatorIndex<newDevice.numValuators;++valuatorIndex)
		{
		char valuatorName[40];
		snprintf(valuatorName,sizeof(valuatorName),"Valuator%d",valuatorIndex);
		newDevice.valuatorNames.push_back(valuatorName);
		}
	
	/* Store the new device structure: */
	devices.push_back(newDevice);
	}

void* InputDeviceAdapterHID::devicePollingThreadMethod(void)
	{
	/* Enable immediate cancellation: */
	Threads::Thread::setCancelState(Threads::Thread::CANCEL_ENABLE);
	// Threads::Thread::setCancelType(Threads::Thread::CANCEL_ASYNCHRONOUS);
	
	while(true)
		{
		/* Poll the device files of all devices: */
		Misc::FdSet deviceFds;
		for(std::vector<Device>::iterator dIt=devices.begin();dIt!=devices.end();++dIt)
			deviceFds.add(dIt->deviceFd);
		if(Misc::select(&deviceFds,0,0)>0)
			{
			/* Read events from all device files: */
			{
			Threads::Mutex::Lock deviceStateLock(deviceStateMutex);
			for(std::vector<Device>::iterator dIt=devices.begin();dIt!=devices.end();++dIt)
				if(deviceFds.isSet(dIt->deviceFd))
					{
					/* Attempt to read multiple events at once: */
					input_event events[32];
					ssize_t numEvents=read(dIt->deviceFd,events,sizeof(events));
					if(numEvents>0)
						{
						/* Process all read events in order: */
						numEvents/=sizeof(input_event);
						for(ssize_t i=0;i<numEvents;++i)
							{
							switch(events[i].type)
								{
								case EV_KEY:
									{
									/* Check if the key has a valid button index: */
									int buttonIndex=dIt->keyMap[events[i].code];
									if(buttonIndex>=0)
										{
										/* Set the button's new state: */
										buttonStates[dIt->firstButtonIndex+buttonIndex]=events[i].value!=0;
										}
									break;
									}
								
								case EV_ABS:
									{
									/* Check if the absolute axis has a valid valuator index: */
									int valuatorIndex=dIt->absAxisMap[events[i].code];
									if(valuatorIndex>=0)
										{
										/* Set the valuators's new state: */
										valuatorStates[dIt->firstValuatorIndex+valuatorIndex]=dIt->axisValueMappers[valuatorIndex].map(double(events[i].value));
										}
									break;
									}
								
								case EV_REL:
									{
									/* Check if the relative axis has a valid valuator index: */
									int valuatorIndex=dIt->relAxisMap[events[i].code];
									if(valuatorIndex>=0)
										{
										/* Set the valuators's new state: */
										valuatorStates[dIt->firstValuatorIndex+valuatorIndex]=dIt->axisValueMappers[valuatorIndex].map(double(events[i].value));
										}
									break;
									}
								}
							}
						}
					}
			}
			
			/* Request a Vrui update: */
			requestUpdate();
			}
		}
	
	return 0;
	}

InputDeviceAdapterHID::InputDeviceAdapterHID(InputDeviceManager* sInputDeviceManager,const Misc::ConfigurationFileSection& configFileSection)
	:InputDeviceAdapter(sInputDeviceManager),
	 buttonStates(0),valuatorStates(0)
	{
	/* Initialize input device adapter: */
	InputDeviceAdapter::initializeAdapter(configFileSection);
	
	/* Count the total number of buttons and valuators: */
	int totalNumButtons=0;
	int totalNumValuators=0;
	for(std::vector<Device>::iterator dIt=devices.begin();dIt!=devices.end();++dIt)
		{
		dIt->firstButtonIndex=totalNumButtons;
		totalNumButtons+=dIt->numButtons;
		dIt->firstValuatorIndex=totalNumValuators;
		totalNumValuators+=dIt->numValuators;
		}
	
	/* Create the device state arrays: */
	buttonStates=new bool[totalNumButtons];
	for(int i=0;i<totalNumButtons;++i)
		buttonStates[i]=false;
	valuatorStates=new double[totalNumValuators];
	for(int i=0;i<totalNumValuators;++i)
		valuatorStates[i]=0.0;
	
	/* Start the device polling thread: */
	devicePollingThread.start(this,&InputDeviceAdapterHID::devicePollingThreadMethod);
	}

InputDeviceAdapterHID::~InputDeviceAdapterHID(void)
	{
	/* Shut down the device polling thread: */
	{
	Threads::Mutex::Lock deviceStateLock(deviceStateMutex);
	devicePollingThread.cancel();
	devicePollingThread.join();
	}
	
	/* Delete the device state arrays: */
	delete[] buttonStates;
	delete[] valuatorStates;
	
	/* Close all device files: */
	for(std::vector<Device>::iterator dIt=devices.begin();dIt!=devices.end();++dIt)
		close(dIt->deviceFd);
	}

std::string InputDeviceAdapterHID::getFeatureName(const InputDeviceFeature& feature) const
	{
	/* Find the HID structure for the given input device: */
	std::vector<Device>::const_iterator dIt;
	for(dIt=devices.begin();dIt!=devices.end()&&dIt->device!=feature.getDevice();++dIt)
		;
	if(dIt==devices.end())
		Misc::throwStdErr("InputDeviceAdapterHID::getFeatureName: Unknown device %s",feature.getDevice()->getDeviceName());
	
	/* Check whether the feature is a button or a valuator: */
	std::string result;
	if(feature.isButton())
		{
		/* Return the button feature's name: */
		result=dIt->buttonNames[feature.getIndex()];
		}
	if(feature.isValuator())
		{
		/* Return the valuator feature's name: */
		result=dIt->valuatorNames[feature.getIndex()];
		}
	
	return result;
	}

int InputDeviceAdapterHID::getFeatureIndex(InputDevice* device,const char* featureName) const
	{
	/* Find the HID structure for the given input device: */
	std::vector<Device>::const_iterator dIt;
	for(dIt=devices.begin();dIt!=devices.end()&&dIt->device!=device;++dIt)
		;
	if(dIt==devices.end())
		Misc::throwStdErr("InputDeviceAdapterHID::getFeatureIndex: Unknown device %s",device->getDeviceName());
	
	/* Check if the feature names a button or a valuator: */
	for(int buttonIndex=0;buttonIndex<dIt->numButtons;++buttonIndex)
		if(dIt->buttonNames[buttonIndex]==featureName)
			return device->getButtonFeatureIndex(buttonIndex);
	for(int valuatorIndex=0;valuatorIndex<dIt->numValuators;++valuatorIndex)
		if(dIt->valuatorNames[valuatorIndex]==featureName)
			return device->getValuatorFeatureIndex(valuatorIndex);
	
	return -1;
	}

void InputDeviceAdapterHID::updateInputDevices(void)
	{
	/* Copy the current device state array into the input devices: */
	Threads::Mutex::Lock deviceStateLock(deviceStateMutex);
	
	for(std::vector<Device>::iterator dIt=devices.begin();dIt!=devices.end();++dIt)
		{
		if(dIt->trackingDevice!=0)
			{
			/* Copy the source device's tracking state: */
			dIt->device->copyTrackingState(dIt->trackingDevice);
			}
		
		/* Set the device's button and valuator states: */
		for(int i=0;i<dIt->numButtons;++i)
			dIt->device->setButtonState(i,buttonStates[dIt->firstButtonIndex+i]);
		for(int i=0;i<dIt->numValuators;++i)
			dIt->device->setValuator(i,valuatorStates[dIt->firstValuatorIndex+i]);
		}
	}

}
