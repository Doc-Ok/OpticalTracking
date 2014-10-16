/***********************************************************************
ToolInputAssignment - Class defining the input assignments of a tool.
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

#include <Vrui/ToolInputAssignment.h>

#include <Vrui/InputDevice.h>
#include <Vrui/InputDeviceFeature.h>
#include <Vrui/ToolInputLayout.h>

namespace Vrui {

/************************************
Methods of class ToolInputAssignment:
************************************/

ToolInputAssignment::ToolInputAssignment(const ToolInputLayout& layout)
	:numButtonSlots(layout.getNumButtons()),buttonSlots(new Slot[numButtonSlots]),
	 numValuatorSlots(layout.getNumValuators()),valuatorSlots(new Slot[numValuatorSlots])
	{
	}
	
ToolInputAssignment::ToolInputAssignment(const ToolInputAssignment& source)
	:numButtonSlots(source.numButtonSlots),buttonSlots(new Slot[numButtonSlots]),
	 numValuatorSlots(source.numValuatorSlots),valuatorSlots(new Slot[numValuatorSlots])
	{
	/* Copy all button assignments: */
	for(int buttonSlotIndex=0;buttonSlotIndex<numButtonSlots;++buttonSlotIndex)
		buttonSlots[buttonSlotIndex]=source.buttonSlots[buttonSlotIndex];
	
	/* Copy all valuator assignments: */
	for(int valuatorSlotIndex=0;valuatorSlotIndex<numValuatorSlots;++valuatorSlotIndex)
		valuatorSlots[valuatorSlotIndex]=source.valuatorSlots[valuatorSlotIndex];
	}
	
ToolInputAssignment::~ToolInputAssignment(void)
	{
	delete[] buttonSlots;
	delete[] valuatorSlots;
	}

void ToolInputAssignment::setButtonSlot(int buttonSlotIndex,InputDevice* slotDevice,int slotButtonIndex)
	{
	/* Set the slot's device and button index: */
	buttonSlots[buttonSlotIndex].device=slotDevice;
	buttonSlots[buttonSlotIndex].index=slotButtonIndex;
	}

void ToolInputAssignment::addButtonSlot(InputDevice* slotDevice,int slotButtonIndex)
	{
	/* Allocate a new button slot array: */
	Slot* newButtonSlots=new Slot[numButtonSlots+1];
	for(int i=0;i<numButtonSlots;++i)
		newButtonSlots[i]=buttonSlots[i];
	delete[] buttonSlots;
	++numButtonSlots;
	buttonSlots=newButtonSlots;
	
	/* Set the new slot's device and button index: */
	buttonSlots[numButtonSlots-1].device=slotDevice;
	buttonSlots[numButtonSlots-1].index=slotButtonIndex;
	}

void ToolInputAssignment::setValuatorSlot(int valuatorSlotIndex,InputDevice* slotDevice,int slotValuatorIndex)
	{
	/* Set the slot's device and valuator index: */
	valuatorSlots[valuatorSlotIndex].device=slotDevice;
	valuatorSlots[valuatorSlotIndex].index=slotValuatorIndex;
	}

void ToolInputAssignment::addValuatorSlot(InputDevice* slotDevice,int slotValuatorIndex)
	{
	/* Allocate a new valuator slot array: */
	Slot* newValuatorSlots=new Slot[numValuatorSlots+1];
	for(int i=0;i<numValuatorSlots;++i)
		newValuatorSlots[i]=valuatorSlots[i];
	delete[] valuatorSlots;
	++numValuatorSlots;
	valuatorSlots=newValuatorSlots;
	
	/* Set the new slot's device and valuator index: */
	valuatorSlots[numValuatorSlots-1].device=slotDevice;
	valuatorSlots[numValuatorSlots-1].index=slotValuatorIndex;
	}

InputDeviceFeature ToolInputAssignment::getButtonSlotFeature(int buttonSlotIndex) const
	{
	return InputDeviceFeature(buttonSlots[buttonSlotIndex].device,InputDevice::BUTTON,buttonSlots[buttonSlotIndex].index);
	}

InputDeviceFeature ToolInputAssignment::getValuatorSlotFeature(int valuatorSlotIndex) const
	{
	return InputDeviceFeature(valuatorSlots[valuatorSlotIndex].device,InputDevice::VALUATOR,valuatorSlots[valuatorSlotIndex].index);
	}

int ToolInputAssignment::getSlotFeatureIndex(int slotIndex) const
	{
	if(slotIndex<numButtonSlots)
		return buttonSlots[slotIndex].device->getButtonFeatureIndex(buttonSlots[slotIndex].index);
	else
		return valuatorSlots[slotIndex-numButtonSlots].device->getValuatorFeatureIndex(valuatorSlots[slotIndex-numButtonSlots].index);
	}

InputDeviceFeature ToolInputAssignment::getSlotFeature(int slotIndex) const
	{
	if(slotIndex<numButtonSlots)
		return InputDeviceFeature(buttonSlots[slotIndex].device,InputDevice::BUTTON,buttonSlots[slotIndex].index);
	else
		return InputDeviceFeature(valuatorSlots[slotIndex-numButtonSlots].device,InputDevice::VALUATOR,valuatorSlots[slotIndex-numButtonSlots].index);
	}

bool ToolInputAssignment::isAssigned(const InputDeviceFeature& feature) const
	{
	/* Check whether the feature is a button or a valuator: */
	if(feature.isButton())
		{
		/* Check if the button is already assigned: */
		int buttonIndex=feature.getIndex();
		for(int buttonSlotIndex=0;buttonSlotIndex<numButtonSlots;++buttonSlotIndex)
			if(buttonSlots[buttonSlotIndex].index==buttonIndex&&buttonSlots[buttonSlotIndex].device==feature.getDevice())
				return true;
		}
	if(feature.isValuator())
		{
		/* Check if the valuator is already assigned: */
		int valuatorIndex=feature.getIndex();
		for(int valuatorSlotIndex=0;valuatorSlotIndex<numValuatorSlots;++valuatorSlotIndex)
			if(valuatorSlots[valuatorSlotIndex].index==valuatorIndex&&valuatorSlots[valuatorSlotIndex].device==feature.getDevice())
				return true;
		}
	
	return false;
	}

int ToolInputAssignment::findFeature(const InputDeviceFeature& feature) const
	{
	/* Check whether the feature is a button or a valuator: */
	if(feature.isButton())
		{
		/* Search for a button assignment: */
		int buttonIndex=feature.getIndex();
		for(int buttonSlotIndex=0;buttonSlotIndex<numButtonSlots;++buttonSlotIndex)
			if(buttonSlots[buttonSlotIndex].index==buttonIndex&&buttonSlots[buttonSlotIndex].device==feature.getDevice())
				return buttonSlotIndex;
		}
	if(feature.isValuator())
		{
		/* Search for a valuator assignment: */
		int valuatorIndex=feature.getIndex();
		for(int valuatorSlotIndex=0;valuatorSlotIndex<numValuatorSlots;++valuatorSlotIndex)
			if(valuatorSlots[valuatorSlotIndex].index==valuatorIndex&&valuatorSlots[valuatorSlotIndex].device==feature.getDevice())
				return numButtonSlots+valuatorSlotIndex;
		}
	
	return -1;
	}

}
