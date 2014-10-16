/***********************************************************************
InputDeviceTool - Base class for tools used to interact with virtual
input devices.
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

#include <Vrui/InputDeviceTool.h>

#include <Vrui/Vrui.h>
#include <Vrui/InputDevice.h>
#include <Vrui/VirtualInputDevice.h>
#include <Vrui/InputGraphManager.h>
#include <Vrui/InputDeviceManager.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/***************************************
Methods of class InputDeviceToolFactory:
***************************************/

InputDeviceToolFactory::InputDeviceToolFactory(ToolManager& toolManager)
	:ToolFactory("InputDeviceTool",toolManager),
	 virtualInputDevice(getVirtualInputDevice())
	{
	/* Insert class into class hierarchy: */
	ToolFactory* toolFactory=toolManager.loadClass("UserInterfaceTool");
	toolFactory->addChildClass(this);
	addParentClass(toolFactory);
	
	/* Set tool class' factory pointer: */
	InputDeviceTool::factory=this;
	}

InputDeviceToolFactory::~InputDeviceToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	InputDeviceTool::factory=0;
	}

const char* InputDeviceToolFactory::getName(void) const
	{
	return "Input Device Driver";
	}

const char* InputDeviceToolFactory::getButtonFunction(int buttonSlotIndex) const
	{
	if(buttonSlotIndex==0)
		return "Grab Device";
	else
		return "Forwarded Button";
	}

const char* InputDeviceToolFactory::getValuatorFunction(int valuatorSlotIndex) const
	{
	return "Forwarded Valuator";
	}

/****************************************
Static elements of class InputDeviceTool:
****************************************/

InputDeviceToolFactory* InputDeviceTool::factory=0;

/********************************
Methods of class InputDeviceTool:
********************************/

bool InputDeviceTool::activate(const Point& position)
	{
	/* Find an ungrabbed input device at the given position: */
	InputDevice* device=getInputGraphManager()->findInputDevice(position);
	if(device!=0)
		{
		/* Check if the event was meant for one of the input device's buttons: */
		int buttonIndex=getVirtualInputDevice()->pickButton(device,position);
		if(buttonIndex>=0)
			{
			if(buttonIndex==device->getNumButtons())
				{
				/* Toggle the input device's navigation mode: */
				getInputGraphManager()->setNavigational(device,!getInputGraphManager()->isNavigational(device));
				}
			else
				{
				/* Toggle the input device's button state: */
				device->setButtonState(buttonIndex,!device->getButtonState(buttonIndex));
				}
			}
		else if(getInputGraphManager()->grabInputDevice(device,this))
			{
			/* Activate the tool: */
			active=true;
			grabbedDevice=device;
			}
		}
	
	return active;
	}

bool InputDeviceTool::activate(const Ray& ray)
	{
	/* Find an ungrabbed input device with the given ray: */
	InputDevice* device=getInputGraphManager()->findInputDevice(ray);
	if(device!=0)
		{
		/* Check if the event was meant for one of the input device's buttons: */
		int buttonIndex=getVirtualInputDevice()->pickButton(device,ray);
		if(buttonIndex>=0)
			{
			if(buttonIndex==device->getNumButtons())
				{
				/* Toggle the input device's navigation mode: */
				getInputGraphManager()->setNavigational(device,!getInputGraphManager()->isNavigational(device));
				}
			else
				{
				/* Toggle the input device's button state: */
				device->setButtonState(buttonIndex,!device->getButtonState(buttonIndex));
				}
			}
		else if(getInputGraphManager()->grabInputDevice(device,this))
			{
			/* Activate the tool: */
			active=true;
			grabbedDevice=device;
			}
		}
	
	return active;
	}

void InputDeviceTool::deactivate(void)
	{
	if(active)
		{
		/* Release the grabbed input device: */
		getInputGraphManager()->releaseInputDevice(grabbedDevice,this);
		
		/* Deactivate the tool: */
		active=false;
		grabbedDevice=0;
		}
	}

bool InputDeviceTool::grabNextDevice(void)
	{
	if(active)
		{
		/* Release the grabbed input device: */
		getInputGraphManager()->releaseInputDevice(grabbedDevice,this);
		
		/* Deactivate the tool: */
		active=false;
		}
	
	/* Try grabbing the next device: */
	if(grabbedDevice==0)
		grabbedDevice=getInputGraphManager()->getFirstInputDevice();
	else
		grabbedDevice=getInputGraphManager()->getNextInputDevice(grabbedDevice);
	if(grabbedDevice!=0&&getInputGraphManager()->grabInputDevice(grabbedDevice,this))
		{
		/* Activate the tool: */
		active=true;
		}
	else
		grabbedDevice=0;
	
	return active;
	}

InputDeviceTool::InputDeviceTool(const ToolFactory* sFactory,const ToolInputAssignment& inputAssignment)
	:UserInterfaceTool(sFactory,inputAssignment),
	 active(false),grabbedDevice(0)
	{
	/* Initialize the number of private buttons and valuators by assuming that required buttons/valuators are private: */
	numPrivateButtons=sFactory->getLayout().getNumButtons();
	numPrivateValuators=sFactory->getLayout().getNumValuators();
	}

InputDeviceTool::~InputDeviceTool(void)
	{
	}

void InputDeviceTool::initialize(void)
	{
	}

void InputDeviceTool::deinitialize(void)
	{
	/* Deactivate the tool if it is still active: */
	if(active)
		deactivate();
	}

const ToolFactory* InputDeviceTool::getFactory(void) const
	{
	return factory;
	}

void InputDeviceTool::buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData)
	{
	/* Check if the button is a forwarded button: */
	if(active&&buttonSlotIndex>=numPrivateButtons)
		{
		int forwardButtonIndex=buttonSlotIndex-numPrivateButtons;
		
		if(forwardButtonIndex<grabbedDevice->getNumButtons())
			{
			/* Forward the new button state to the grabbed device: */
			grabbedDevice->setButtonState(forwardButtonIndex,cbData->newButtonState);
			}
		}
	}

void InputDeviceTool::valuatorCallback(int valuatorSlotIndex,InputDevice::ValuatorCallbackData* cbData)
	{
	/* Check if the valuator is a forwarded valuator: */
	if(active&&valuatorSlotIndex>=numPrivateValuators)
		{
		int forwardValuatorIndex=valuatorSlotIndex-numPrivateValuators;
		
		if(forwardValuatorIndex<grabbedDevice->getNumValuators())
			{
			/* Forward the new valuator state to the grabbed device: */
			grabbedDevice->setValuator(forwardValuatorIndex,cbData->newValuatorValue);
			}
		}
	}

}
