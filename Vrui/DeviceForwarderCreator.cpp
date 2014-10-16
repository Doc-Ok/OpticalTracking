/***********************************************************************
DeviceForwarderCreator - Helper class to create virtual input devices
and maintain associations from input slots to forwarded devices for
device-forwarding tools.
Copyright (c) 2014 Oliver Kreylos

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

#include <Vrui/DeviceForwarderCreator.h>

#include <Vrui/Vrui.h>
#include <Vrui/InputDevice.h>
#include <Vrui/InputDeviceManager.h>

namespace Vrui {

/***************************************
Methods of class DeviceForwarderCreator:
***************************************/

DeviceForwarderCreator::DeviceForwarderCreator(int sNumButtonSlots,int sNumValuatorSlots)
	:numButtonSlots(sNumButtonSlots),buttonSlots(numButtonSlots>0?new ForwardingSlot[numButtonSlots]:0),
	 numValuatorSlots(sNumValuatorSlots),valuatorSlots(numValuatorSlots>0?new ForwardingSlot[numValuatorSlots]:0),
	 deviceLayoutMap(((numButtonSlots+numValuatorSlots)*5)/4)
	{
	}

DeviceForwarderCreator::~DeviceForwarderCreator(void)
	{
	delete[] buttonSlots;
	delete[] valuatorSlots;
	
	/* Destroy all uncollected virtual input devices: */
	for(DeviceLayoutMap::Iterator dlmIt=deviceLayoutMap.begin();!dlmIt.isFinished();++dlmIt)
		if(dlmIt->getDest().device!=0)
			getInputDeviceManager()->destroyInputDevice(dlmIt->getDest().device);
	}

void DeviceForwarderCreator::forwardButton(int buttonSlotIndex,InputDevice* sourceDevice,int sourceDeviceButtonIndex)
	{
	/* Find the source device in the device layout map: */
	DeviceLayoutMap::Iterator dlmIt=deviceLayoutMap.findEntry(sourceDevice);
	int virtualDeviceFeatureIndex;
	if(dlmIt.isFinished())
		{
		/* Assign the first button on the new device: */
		virtualDeviceFeatureIndex=0;
		
		/* Add a new virtual device to the map: */
		VirtualDeviceLayout newLayout;
		newLayout.device=0;
		newLayout.numButtons=1;
		newLayout.numValuators=0;
		deviceLayoutMap[sourceDevice]=newLayout;
		}
	else
		{
		/* Assign the next button on the new device: */
		virtualDeviceFeatureIndex=dlmIt->getDest().numButtons;
		
		/* Add a button to the existing virtual device: */
		++dlmIt->getDest().numButtons;
		}
	
	/* Associate the button slot with the found feature on the virtual device: */
	buttonSlots[buttonSlotIndex].sourceDevice=sourceDevice;
	buttonSlots[buttonSlotIndex].sourceDeviceFeatureIndex=sourceDeviceButtonIndex;
	buttonSlots[buttonSlotIndex].virtualDevice=0;
	buttonSlots[buttonSlotIndex].virtualDeviceFeatureIndex=virtualDeviceFeatureIndex;
	}

void DeviceForwarderCreator::forwardValuator(int valuatorSlotIndex,InputDevice* sourceDevice,int sourceDeviceValuatorIndex)
	{
	/* Find the source device in the device layout map: */
	DeviceLayoutMap::Iterator dlmIt=deviceLayoutMap.findEntry(sourceDevice);
	int virtualDeviceFeatureIndex;
	if(dlmIt.isFinished())
		{
		/* Assign the first valuator on the new device: */
		virtualDeviceFeatureIndex=0;
		
		/* Add a new virtual device to the map: */
		VirtualDeviceLayout newLayout;
		newLayout.device=0;
		newLayout.numButtons=0;
		newLayout.numValuators=1;
		deviceLayoutMap[sourceDevice]=newLayout;
		}
	else
		{
		/* Assign the next valuator on the new device: */
		virtualDeviceFeatureIndex=dlmIt->getDest().numValuators;
		
		/* Add a valuator to the existing virtual device: */
		++dlmIt->getDest().numValuators;
		}
	
	/* Associate the valuator slot with the found feature on the virtual device: */
	valuatorSlots[valuatorSlotIndex].sourceDevice=sourceDevice;
	valuatorSlots[valuatorSlotIndex].sourceDeviceFeatureIndex=sourceDeviceValuatorIndex;
	valuatorSlots[valuatorSlotIndex].virtualDevice=0;
	valuatorSlots[valuatorSlotIndex].virtualDeviceFeatureIndex=virtualDeviceFeatureIndex;
	}

void DeviceForwarderCreator::createDevices(void)
	{
	/* Create all required virtual input devices: */
	for(DeviceLayoutMap::Iterator dlmIt=deviceLayoutMap.begin();!dlmIt.isFinished();++dlmIt)
		dlmIt->getDest().device=addVirtualInputDevice("ForwardedDevice",dlmIt->getDest().numButtons,dlmIt->getDest().numValuators);
	
	/* Assign all button and valuator slots to their respective virtual device features: */
	for(int i=0;i<numButtonSlots;++i)
		buttonSlots[i].virtualDevice=deviceLayoutMap[buttonSlots[i].sourceDevice].getDest().device;
	for(int i=0;i<numValuatorSlots;++i)
		valuatorSlots[i].virtualDevice=deviceLayoutMap[valuatorSlots[i].sourceDevice].getDest().device;
	}

InputDevice* DeviceForwarderCreator::getSourceDevice(size_t index) const
	{
	/* Get the source device by iterating through the device layout map: */
	DeviceLayoutMap::ConstIterator dlmIt=deviceLayoutMap.begin();
	for(size_t i=0;i<index;++i,++dlmIt)
		;
	return dlmIt->getSource();
	}

InputDevice* DeviceForwarderCreator::collectDevice(InputDevice* sourceDevice)
	{
	/* Find the virtual device associated with the given source device: */
	DeviceLayoutMap::Iterator dlmIt=deviceLayoutMap.findEntry(sourceDevice);
	if(dlmIt.isFinished())
		throw std::runtime_error("Vrui::DeviceForwarderCreator::collectDevice: Requested device is not forwarded");
	
	/* Return the forwarded device: */
	InputDevice* result=dlmIt->getDest().device;
	
	/* Release the device: */
	dlmIt->getDest().device=0;
	
	return result;
	}

}
