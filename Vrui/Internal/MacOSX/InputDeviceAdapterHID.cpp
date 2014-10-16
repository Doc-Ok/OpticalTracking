/***********************************************************************
InputDeviceAdapterHID - Mac OSX-specific version of HID input device
adapter.
Copyright (c) 2009-2010 Oliver Kreylos

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

#include <Vrui/Internal/MacOSX/InputDeviceAdapterHID.h>

#include <stdlib.h>
#include <IOKit/hid/IOHIDKeys.h>
#include <Misc/ThrowStdErr.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/CompoundValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Math/Math.h>
#include <Math/Constants.h>
#include <Math/MathValueCoders.h>
#include <Vrui/InputDevice.h>
#include <Vrui/InputDeviceFeature.h>
#include <Vrui/InputDeviceManager.h>
#include <Vrui/Vrui.h>

namespace Vrui {

/**************************************
Methods of class InputDeviceAdapterHID:
**************************************/

void InputDeviceAdapterHID::createInputDevice(int deviceIndex,const Misc::ConfigurationFileSection& configFileSection)
	{
	/* Create a new device structure: */
	Device newDevice;
	
	/* Read input device name: */
	newDevice.name=configFileSection.retrieveString("./name");
	
	/* Read HID's vendor / product IDs: */
	std::string deviceVendorProductId=configFileSection.retrieveString("./deviceVendorProductId");
	
	/* Split ID string into vendor ID / product ID: */
	char* colonPtr;
	newDevice.vendorId=strtol(deviceVendorProductId.c_str(),&colonPtr,16);
	char* endPtr;
	newDevice.productId=strtol(colonPtr+1,&endPtr,16);
	if(*colonPtr!=':'||*endPtr!='\0'||newDevice.vendorId<0||newDevice.productId<0)
		Misc::throwStdErr("InputDeviceAdapterHID::InputDeviceAdapterHID: Malformed vendorId:productId string \"%s\" for device %s",deviceVendorProductId.c_str(),newDevice.name.c_str());
	
	/* Get the device index: */
	newDevice.deviceIndex=configFileSection.retrieveValue<int>("./deviceIndex",0);
	
	/* Set state indices to bogus values: */
	newDevice.firstButtonIndex=-1;
	newDevice.numButtons=0;
	newDevice.firstValuatorIndex=-1;
	newDevice.numValuators=0;
	
	/* Don't create Vrui input device: */
	newDevice.device=0;
	
	/* Read the names of all button and valuator features: */
	typedef std::vector<std::string> StringList;
	newDevice.buttonNames=configFileSection.retrieveValue<StringList>("./buttonNames",StringList());
	newDevice.valuatorNames=configFileSection.retrieveValue<StringList>("./valuatorNames",StringList());
	
	/* Store the new device structure: */
	devices.push_back(newDevice);
	}

void InputDeviceAdapterHID::hidDeviceValueChangedCallback(IOReturn result,void* device,IOHIDValueRef newValue)
	{
	if(result!=kIOReturnSuccess)
		return;
	
	/* Get the element's descriptor: */
	ElementMap::Iterator elementIt=elementMap.findEntry(ElementKey(device,IOHIDElementGetCookie(IOHIDValueGetElement(newValue))));
	if(elementIt.isFinished())
		return;
	ElementDescriptor& ed=elementIt->getDest();
	
	/* Lock the device state: */
	{
	Threads::Mutex::Lock deviceStateLock(deviceStateMutex);
	
	/* Update the state arrays: */
	switch(ed.elementType)
		{
		case ElementDescriptor::BUTTON:
			/* Update a button state: */
			buttonStates[ed.index]=IOHIDValueGetIntegerValue(newValue)!=0;
			break;
		
		case ElementDescriptor::VALUATOR:
			/* Update a valuator state: */
			valuatorStates[ed.index]=ed.axisMapper.map(double(IOHIDValueGetIntegerValue(newValue)));
			break;
		
		case ElementDescriptor::HATSWITCH:
			{
			/* Update two valuators based on the hat switch's angle: */
			int value=IOHIDValueGetIntegerValue(newValue);
			if(value>=ed.hsMin&&value<=ed.hsMax)
				{
				/* Convert value into angle: */
				double angle=2.0*Math::Constants<double>::pi*double(value-ed.hsMin)/double(ed.hsMax+1-ed.hsMin);
				
				/* Update axis valuator states: */
				valuatorStates[ed.index+0]=Math::sin(angle);
				valuatorStates[ed.index+1]=Math::cos(angle);
				}
			else
				{
				/* Handle Null value: */
				valuatorStates[ed.index+0]=0.0;
				valuatorStates[ed.index+1]=0.0;
				}
			break;
			}
		}
	}
	
	/* Request a Vrui update: */
	requestUpdate();
	}

void* InputDeviceAdapterHID::devicePollingThreadMethod(void)
	{
	/* Enable immediate cancellation: */
	Threads::Thread::setCancelState(Threads::Thread::CANCEL_ENABLE);
	// Threads::Thread::setCancelType(Threads::Thread::CANCEL_ASYNCHRONOUS);
	
	/* Associate the HID manager with the current run loop: */
	IOHIDManagerScheduleWithRunLoop(hidManager,CFRunLoopGetCurrent(),kCFRunLoopDefaultMode);
	
	/* Execute the run loop: */
	CFRunLoopRun();
	
	return 0;
	}

namespace {

/****************
Helper functions:
****************/

bool setDictionaryValue(CFMutableDictionaryRef dictionary,const char* key,long value)
	{
	/* Create a Core Foundation string for the key: */
	MacOSX::AutoRef<CFStringRef> keyString=CFStringCreateWithCString(kCFAllocatorDefault,key,kCFStringEncodingUTF8);
	if(keyString==0)
		return false;
	
	/* Create a Core Foundation number for the value: */
	MacOSX::AutoRef<CFNumberRef> valueNumber=CFNumberCreate(kCFAllocatorDefault,kCFNumberLongType,&value);
	if(valueNumber==0)
		return false;
	CFDictionaryAddValue(dictionary,keyString,valueNumber);
	
	return true;
	}

bool hidDeviceMatches(IOHIDDeviceRef device,long vendorId,long productId)
	{
	CFTypeRef vendorIdRef=IOHIDDeviceGetProperty(device,CFSTR(kIOHIDVendorIDKey));
	if(vendorIdRef==0||CFGetTypeID(vendorIdRef)!=CFNumberGetTypeID())
		return false;
	long dVendorId;
	if(!CFNumberGetValue(static_cast<CFNumberRef>(vendorIdRef),kCFNumberLongType,&dVendorId)||dVendorId!=vendorId)
		return false;
	CFTypeRef productIdRef=IOHIDDeviceGetProperty(device,CFSTR(kIOHIDProductIDKey));
	if(productIdRef==0||CFGetTypeID(productIdRef)!=CFNumberGetTypeID())
		return false;
	long dProductId;
	if(!CFNumberGetValue(static_cast<CFNumberRef>(productIdRef),kCFNumberLongType,&dProductId)||dProductId!=productId)
		return false;
	
	return true;
	}

}

InputDeviceAdapterHID::InputDeviceAdapterHID(InputDeviceManager* sInputDeviceManager,const Misc::ConfigurationFileSection& configFileSection)
	:InputDeviceAdapter(sInputDeviceManager),
	 buttonStates(0),valuatorStates(0),
	 hidManager(0),elementMap(31)
	{
	/* Get a reference to the HID manager: */
	hidManager=IOHIDManagerCreate(kCFAllocatorDefault,kIOHIDOptionsTypeNone);
	if(hidManager==0)
		Misc::throwStdErr("InputDeviceAdapterHID::InputDeviceAdapterHID: Could not access HID manager");
	
	/* Initialize input device adapter: */
	InputDeviceAdapter::initializeAdapter(configFileSection);
	
	{
	/* Create a set of dictionaries to match the vendor / product IDs of all configured devices: */
	MacOSX::AutoRef<CFMutableArrayRef> dictionarySet=CFArrayCreateMutable(kCFAllocatorDefault,0,&kCFTypeArrayCallBacks);
	if(dictionarySet==0)
		Misc::throwStdErr("InputDeviceAdapterHID::InputDeviceAdapterHID: Could not create dictionary set");
	for(std::vector<Device>::iterator dIt=devices.begin();dIt!=devices.end();++dIt)
		{
		/* Create a dictionary to match the device's vendor / product IDs: */
		MacOSX::AutoRef<CFMutableDictionaryRef> dictionary=CFDictionaryCreateMutable(kCFAllocatorDefault,2,&kCFTypeDictionaryKeyCallBacks,&kCFTypeDictionaryValueCallBacks);
		if(dictionary==0)
			Misc::throwStdErr("InputDeviceAdapterHID::InputDeviceAdapterHID: Could not create dictionary");
		
		/* Add keys for vendor and product IDs: */
		if(!setDictionaryValue(dictionary,kIOHIDVendorIDKey,dIt->vendorId)||!setDictionaryValue(dictionary,kIOHIDProductIDKey,dIt->productId))
			Misc::throwStdErr("InputDeviceAdapterHID::InputDeviceAdapterHID: Could not set product / vendor IDs in dictionary");
		
		/* Add the dictionary to the set: */
		CFArrayAppendValue(dictionarySet,dictionary);
		}
	
	/* Install the dictionary set with the HID manager: */
	IOHIDManagerSetDeviceMatchingMultiple(hidManager,dictionarySet);
	}
	
	/* Open the HID manager: */
	if(IOHIDManagerOpen(hidManager,kIOHIDOptionsTypeNone)!=kIOReturnSuccess)
		Misc::throwStdErr("InputDeviceAdapterHID::InputDeviceAdapterHID: Could not open HID manager");
	
	int totalNumButtons=0;
	int totalNumValuators=0;
	
	{
	/* Get the set of matching devices: */
	MacOSX::AutoRef<CFSetRef> deviceSet=IOHIDManagerCopyDevices(hidManager);
	if(deviceSet==0||CFSetGetCount(deviceSet)==0)
		Misc::throwStdErr("InputDeviceAdapterHID::InputDeviceAdapterHID: No devices found");
	
	/* Access the found device handles: */
	CFIndex numHidDevices=CFSetGetCount(deviceSet);
	IOHIDDeviceRef* hidDevices=new IOHIDDeviceRef[numHidDevices];
	CFSetGetValues(deviceSet,const_cast<const void**>(reinterpret_cast<void**>(hidDevices)));
	
	/* Process all configured devices: */
	int deviceIndex=0;
	for(std::vector<Device>::iterator dIt=devices.begin();dIt!=devices.end();++dIt)
		{
		dIt->firstButtonIndex=totalNumButtons;
		dIt->firstValuatorIndex=totalNumValuators;
		
		/* Find the device in the list of HID devices: */
		IOHIDDeviceRef device=0;
		int matched=0;
		for(CFIndex i=0;i<numHidDevices;++i)
			{
			/* Check if the HID device matches the configured device's vendor / product ID: */
			if(hidDeviceMatches(hidDevices[i],dIt->vendorId,dIt->productId))
				{
				/* Check if it's the configured index in a set of identical devices: */
				if(matched==dIt->deviceIndex)
					{
					/* Found it! */
					device=hidDevices[i];
					break;
					}
				
				/* Keep looking: */
				++matched;
				}
			}
		if(device!=0)
			{
			/* Get all elements on the device: */
			MacOSX::AutoRef<CFArrayRef> elements=IOHIDDeviceCopyMatchingElements(device,0,kIOHIDOptionsTypeNone);
			if(elements!=0)
				{
				/* Iterate through the element list: */
				CFIndex numElements=CFArrayGetCount(elements);
				for(CFIndex i=0;i<numElements;++i)
					{
					CFTypeRef elementObject=CFArrayGetValueAtIndex(elements,i);
					if(elementObject!=0&&CFGetTypeID(elementObject)==IOHIDElementGetTypeID())
						{
						IOHIDElementRef element=static_cast<IOHIDElementRef>(const_cast<void*>(elementObject));
						switch(IOHIDElementGetType(element))
							{
							case kIOHIDElementTypeInput_Button:
								{
								/* Add a button element to the device: */
								ElementKey ek(device,IOHIDElementGetCookie(element));
								ElementDescriptor ed;
								ed.elementType=ElementDescriptor::BUTTON;
								ed.index=dIt->firstButtonIndex+dIt->numButtons;
								elementMap.setEntry(ElementMap::Entry(ek,ed));
								++dIt->numButtons;
								break;
								}
							
							case kIOHIDElementTypeInput_Misc:
							case kIOHIDElementTypeInput_Axis:
								{
								/* Check the element's usage for a hat switch control: */
								if(IOHIDElementGetUsagePage(element)==0x01&&IOHIDElementGetUsage(element)==0x39)
									{
									/* Add a hat switch to the device: */
									ElementKey ek(device,IOHIDElementGetCookie(element));
									ElementDescriptor ed;
									ed.elementType=ElementDescriptor::HATSWITCH;
									ed.index=dIt->firstValuatorIndex+dIt->numValuators;
									ed.hsMin=int(IOHIDElementGetLogicalMin(element));
									ed.hsMax=int(IOHIDElementGetLogicalMax(element));
									elementMap.setEntry(ElementMap::Entry(ek,ed));
									dIt->numValuators+=2;
									}
								else
									{
									/* Add a valuator to the device: */
									ElementKey ek(device,IOHIDElementGetCookie(element));
									ElementDescriptor ed;
									ed.elementType=ElementDescriptor::VALUATOR;
									ed.index=dIt->firstValuatorIndex+dIt->numValuators;
									ed.axisMapper=ElementDescriptor::AxisMapper(double(IOHIDElementGetLogicalMin(element)),double(IOHIDElementGetLogicalMax(element)));
									elementMap.setEntry(ElementMap::Entry(ek,ed));
									++dIt->numValuators;
									}
								break;
								}
							
							default:
								/* Just to make compiler happy... */
								;
							}
						}
					}
				
				/* Create new input device as a physical device: */
				dIt->device=inputDeviceManager->createInputDevice(dIt->name.c_str(),InputDevice::TRACK_NONE,dIt->numButtons,dIt->numValuators,true);
				
				/* Complete the button and valuator name arrays: */
				int buttonIndex=int(dIt->buttonNames.size());
				for(;buttonIndex<dIt->numButtons;++buttonIndex)
					{
					char buttonName[40];
					snprintf(buttonName,sizeof(buttonName),"Button%d",buttonIndex);
					dIt->buttonNames.push_back(buttonName);
					}
				int valuatorIndex=int(dIt->valuatorNames.size());
				for(;valuatorIndex<dIt->numValuators;++valuatorIndex)
					{
					char valuatorName[40];
					snprintf(valuatorName,sizeof(valuatorName),"Valuator%d",valuatorIndex);
					dIt->valuatorNames.push_back(valuatorName);
					}
				
				/* Register a value change callback with the HID device: */
				IOHIDDeviceRegisterInputValueCallback(device,hidDeviceValueChangedCallbackWrapper,this);
				}
			}
		
		/* Save the new input device: */
		inputDevices[deviceIndex]=dIt->device;
		
		totalNumButtons+=dIt->numButtons;
		totalNumValuators+=dIt->numValuators;
		}
	
	delete[] hidDevices;
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
	
	/* Delete the state arrays: */
	delete[] buttonStates;
	delete[] valuatorStates;
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
		/* Set the device's button and valuator states: */
		for(int i=0;i<dIt->numButtons;++i)
			dIt->device->setButtonState(i,buttonStates[dIt->firstButtonIndex+i]);
		for(int i=0;i<dIt->numValuators;++i)
			dIt->device->setValuator(i,valuatorStates[dIt->firstValuatorIndex+i]);
		}
	}

}
