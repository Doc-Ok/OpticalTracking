/***********************************************************************
Tool - Abstract base class for user interaction tools (navigation, menu
selection, selection, etc.).
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

#include <Vrui/Tool.h>

#include <Misc/ThrowStdErr.h>
#include <Vrui/Vrui.h>

namespace Vrui {

/****************************
Methods of class ToolFactory:
****************************/

ToolFactory::ToolFactory(const char* sClassName,ToolManager&)
	:Factory(sClassName)
	{
	}

const char* ToolFactory::getButtonFunction(int buttonSlotIndex) const
	{
	/* Check if any of the class's parent classes are derived from ToolFactory: */
	for(ClassList::const_iterator pIt=parentsBegin();pIt!=parentsEnd();++pIt)
		{
		const ToolFactory* tf=dynamic_cast<const ToolFactory*>(*pIt);
		if(tf!=0)
			return tf->getButtonFunction(buttonSlotIndex);
		}
	
	/* Return a default name otherwise: */
	return "(unknown function)";
	}

const char* ToolFactory::getValuatorFunction(int valuatorSlotIndex) const
	{
	/* Check if any of the class's parent classes are derived from ToolFactory: */
	for(ClassList::const_iterator pIt=parentsBegin();pIt!=parentsEnd();++pIt)
		{
		const ToolFactory* tf=dynamic_cast<const ToolFactory*>(*pIt);
		if(tf!=0)
			return tf->getValuatorFunction(valuatorSlotIndex);
		}
	
	/* Return a default name otherwise: */
	return "(unknown function)";
	}

Tool* ToolFactory::createTool(const ToolInputAssignment&) const
	{
	Misc::throwStdErr("Cannot create tool of abstract class %s",getClassName());
	
	/* Dummy statement to make the compiler happy: */
	return 0;
	}

void ToolFactory::destroyTool(Tool*) const
	{
	Misc::throwStdErr("Cannot destroy tool of abstract class %s",getClassName());
	}

/*********************
Methods of class Tool:
*********************/

void Tool::buttonCallbackWrapper(Misc::CallbackData* cbData,void* userData)
	{
	/* Determine callback target tool: */
	Tool* tool=static_cast<Tool*>(userData);
	
	/* Retrieve the callback data: */
	InputDevice::ButtonCallbackData* bcbData=static_cast<InputDevice::ButtonCallbackData*>(cbData);
	
	/* Find the index of the button slot to which the input device button is assigned: */
	int buttonSlotIndex;
	for(buttonSlotIndex=0;buttonSlotIndex<tool->input.getNumButtonSlots();++buttonSlotIndex)
		{
		const ToolInputAssignment::Slot& slot=tool->input.getButtonSlot(buttonSlotIndex);
		if(slot.index==bcbData->buttonIndex&&slot.device==bcbData->inputDevice)
			break;
		}
	
	/* Check if the callback is really for this tool: */
	if(buttonSlotIndex<tool->input.getNumButtonSlots())
		{
		/* Call the tool's callback method: */
		tool->buttonCallback(buttonSlotIndex,bcbData);
		}
	}

void Tool::valuatorCallbackWrapper(Misc::CallbackData* cbData,void* userData)
	{
	/* Determine callback target tool: */
	Tool* tool=static_cast<Tool*>(userData);
	
	/* Retrieve the callback data: */
	InputDevice::ValuatorCallbackData* vcbData=static_cast<InputDevice::ValuatorCallbackData*>(cbData);
	
	/* Find the index of the valuator slot to which the input device valuator is assigned: */
	int valuatorSlotIndex;
	for(valuatorSlotIndex=0;valuatorSlotIndex<tool->input.getNumValuatorSlots();++valuatorSlotIndex)
		{
		const ToolInputAssignment::Slot& slot=tool->input.getValuatorSlot(valuatorSlotIndex);
		if(slot.index==vcbData->valuatorIndex&&slot.device==vcbData->inputDevice)
			break;
		}
	
	/* Check if the callback is really for this tool: */
	if(valuatorSlotIndex<tool->input.getNumValuatorSlots())
		{
		/* Call the tool's callback method: */
		tool->valuatorCallback(valuatorSlotIndex,vcbData);
		}
	}

Tool::Tool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment)
	:layout(factory->getLayout()),
	 input(inputAssignment)
	{
	/* Register input device callbacks for all button slot assignments: */
	for(int buttonSlotIndex=0;buttonSlotIndex<input.getNumButtonSlots();++buttonSlotIndex)
		{
		const ToolInputAssignment::Slot& slot=input.getButtonSlot(buttonSlotIndex);
		if(slot.device!=0&&slot.index>=0)
			slot.device->getButtonCallbacks(slot.index).add(buttonCallbackWrapper,this);
		}
	
	/* Register input device callbacks for all valuator slot assignments: */
	for(int valuatorSlotIndex=0;valuatorSlotIndex<input.getNumValuatorSlots();++valuatorSlotIndex)
		{
		const ToolInputAssignment::Slot& slot=input.getValuatorSlot(valuatorSlotIndex);
		if(slot.device!=0&&slot.index>=0)
			slot.device->getValuatorCallbacks(slot.index).add(valuatorCallbackWrapper,this);
		}
	}

Tool::~Tool(void)
	{
	/* Remove input device callbacks for all button slot assignments: */
	for(int buttonSlotIndex=0;buttonSlotIndex<input.getNumButtonSlots();++buttonSlotIndex)
		{
		const ToolInputAssignment::Slot& slot=input.getButtonSlot(buttonSlotIndex);
		if(slot.device!=0&&slot.index>=0)
			slot.device->getButtonCallbacks(slot.index).remove(buttonCallbackWrapper,this);
		}
	
	/* Remove input device callbacks for all valuator slot assignments: */
	for(int valuatorSlotIndex=0;valuatorSlotIndex<input.getNumValuatorSlots();++valuatorSlotIndex)
		{
		const ToolInputAssignment::Slot& slot=input.getValuatorSlot(valuatorSlotIndex);
		if(slot.device!=0&&slot.index>=0)
			slot.device->getValuatorCallbacks(slot.index).remove(valuatorCallbackWrapper,this);
		}
	}

void Tool::configure(const Misc::ConfigurationFileSection&)
	{
	}

void Tool::configure(Misc::ConfigurationFileSection&)
	{
	}

void Tool::storeState(Misc::ConfigurationFileSection&) const
	{
	}

void Tool::initialize(void)
	{
	}

void Tool::deinitialize(void)
	{
	}

const ToolFactory* Tool::getFactory(void) const
	{
	Misc::throwStdErr("Tool::getFactory: Tool of abstract class does not have factory object");
	
	/* Just to make compiler happy: */
	return 0;
	}

std::string Tool::getName(void) const
	{
	/* Return the descriptive class name: */
	return getFactory()->getName();
	}

void Tool::assignButtonSlot(int buttonSlotIndex,InputDevice* newSlotDevice,int newSlotButtonIndex)
	{
	const ToolInputAssignment::Slot& slot=input.getButtonSlot(buttonSlotIndex);
	
	/* Remove input device callback from previously assigned input device button: */
	if(slot.device!=0&&slot.index>=0)
		slot.device->getButtonCallbacks(slot.index).remove(buttonCallbackWrapper,this);
	
	/* Assign the new input device button: */
	input.setButtonSlot(buttonSlotIndex,newSlotDevice,newSlotButtonIndex);
	
	/* Register input device callback with newly assigned input device button: */
	if(slot.device!=0&&slot.index>=0)
		slot.device->getButtonCallbacks(slot.index).add(buttonCallbackWrapper,this);
	}

void Tool::assignValuatorSlot(int valuatorSlotIndex,InputDevice* newSlotDevice,int newSlotValuatorIndex)
	{
	const ToolInputAssignment::Slot& slot=input.getValuatorSlot(valuatorSlotIndex);
	
	/* Remove input device callback from previously assigned input device valuator: */
	if(slot.device!=0&&slot.index>=0)
		slot.device->getValuatorCallbacks(slot.index).remove(valuatorCallbackWrapper,this);
	
	/* Assign the new input device valuator: */
	input.setValuatorSlot(valuatorSlotIndex,newSlotDevice,newSlotValuatorIndex);
	
	/* Register input device callback with newly assigned input device valuator: */
	if(slot.device!=0&&slot.index>=0)
		slot.device->getValuatorCallbacks(slot.index).add(valuatorCallbackWrapper,this);
	}

void Tool::buttonCallback(int,InputDevice::ButtonCallbackData*)
	{
	/* No default behavior */
	}

void Tool::valuatorCallback(int,InputDevice::ValuatorCallbackData*)
	{
	/* No default behavior */
	}

void Tool::frame(void)
	{
	/* No default behavior */
	}

void Tool::display(GLContextData&) const
	{
	/* No default behavior */
	}

}
