/***********************************************************************
TransformTool - Base class for tools used to transform the position or
orientation of input devices.
Copyright (c) 2007-2013 Oliver Kreylos

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

#include <Vrui/TransformTool.h>

#include <Misc/ThrowStdErr.h>
#include <Vrui/Vrui.h>
#include <Vrui/InputGraphManager.h>
#include <Vrui/InputDeviceManager.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/*************************************
Methods of class TransformToolFactory:
*************************************/

TransformToolFactory::TransformToolFactory(ToolManager& toolManager)
	:ToolFactory("TransformTool",toolManager)
	{
	#if 0
	/* Insert class into class hierarchy: */
	ToolFactory* toolFactory=toolManager.loadClass("Tool");
	toolFactory->addChildClass(this);
	addParentClass(toolFactory);
	#endif
	
	/* Set tool class' factory pointer: */
	TransformTool::factory=this;
	}

TransformToolFactory::~TransformToolFactory(void)
	{
	}

const char* TransformToolFactory::getName(void) const
	{
	return "Transformer";
	}

const char* TransformToolFactory::getButtonFunction(int) const
	{
	return "Forwarded Button";
	}

const char* TransformToolFactory::getValuatorFunction(int) const
	{
	return "Forwarded Valuator";
	}

/**************************************
Static elements of class TransformTool:
**************************************/

TransformToolFactory* TransformTool::factory=0;

/******************************
Methods of class TransformTool:
******************************/

void TransformTool::resetDevice(void)
	{
	if(sourceDevice!=0)
		{
		/* Copy the source device's position and orientation to the transformed device: */
		transformedDevice->setDeviceRay(sourceDevice->getDeviceRayDirection(),sourceDevice->getDeviceRayStart());
		transformedDevice->setTransformation(sourceDevice->getTransformation());
		}
	}

TransformTool::TransformTool(const ToolFactory* sFactory,const ToolInputAssignment& inputAssignment)
	:Tool(sFactory,inputAssignment),
	 sourceDevice(0),transformedDevice(0)
	{
	/* Initialize the number of private buttons and valuators by assuming that required buttons/valuators are private: */
	numPrivateButtons=sFactory->getLayout().getNumButtons();
	numPrivateValuators=sFactory->getLayout().getNumValuators();
	}

TransformTool::~TransformTool(void)
	{
	}

void TransformTool::initialize(void)
	{
	/* Create a virtual input device to shadow the source input device: */
	transformedDevice=addVirtualInputDevice("TransformedDevice",input.getNumButtonSlots()-numPrivateButtons,input.getNumValuatorSlots()-numPrivateValuators);
	
	if(sourceDevice!=0)
		{
		/* Set the virtual input device's glyph to the source device's glyph: */
		getInputGraphManager()->getInputDeviceGlyph(transformedDevice)=getInputGraphManager()->getInputDeviceGlyph(sourceDevice);
		}
	
	/* Permanently grab the virtual input device: */
	getInputGraphManager()->grabInputDevice(transformedDevice,this);
	
	/* Initialize the virtual input device's position: */
	resetDevice();
	}

void TransformTool::deinitialize(void)
	{
	/* Release the virtual input device: */
	getInputGraphManager()->releaseInputDevice(transformedDevice,this);
	
	/* Destroy the virtual input device: */
	getInputDeviceManager()->destroyInputDevice(transformedDevice);
	transformedDevice=0;
	}

const ToolFactory* TransformTool::getFactory(void) const
	{
	return factory;
	}

void TransformTool::buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData)
	{
	/* Check if the button is a forwarded button: */
	if(buttonSlotIndex>=numPrivateButtons)
		{
		int forwardButtonIndex=buttonSlotIndex-numPrivateButtons;
		
		/* Forward the button's state to the transformed device: */
		transformedDevice->setButtonState(forwardButtonIndex,cbData->newButtonState);
		}
	}

void TransformTool::valuatorCallback(int valuatorSlotIndex,InputDevice::ValuatorCallbackData* cbData)
	{
	/* Check if the valuator is a forwarded valuator: */
	if(valuatorSlotIndex>=numPrivateValuators)
		{
		int forwardValuatorIndex=valuatorSlotIndex-numPrivateValuators;
		
		/* Forward the new valuator state to the transformed device: */
		transformedDevice->setValuator(forwardValuatorIndex,cbData->newValuatorValue);
		}
	}

void TransformTool::frame(void)
	{
	/* Let the transformed device shadow the source device: */
	resetDevice();
	}

std::vector<InputDevice*> TransformTool::getForwardedDevices(void)
	{
	std::vector<InputDevice*> result;
	result.push_back(transformedDevice);
	return result;
	}

InputDeviceFeatureSet TransformTool::getSourceFeatures(const InputDeviceFeature& forwardedFeature)
	{
	/* Paranoia: Check if the forwarded feature is on the transformed device: */
	if(forwardedFeature.getDevice()!=transformedDevice)
		Misc::throwStdErr("TransformTool::getSourceFeatures: Forwarded feature is not on transformed device");
	
	/* Create an empty feature set: */
	InputDeviceFeatureSet result;
	
	if(forwardedFeature.isButton())
		{
		/* Add the slot's feature to the result set: */
		result.push_back(input.getButtonSlotFeature(forwardedFeature.getIndex()+numPrivateButtons));
		}
	
	if(forwardedFeature.isValuator())
		{
		/* Add the slot's feature to the result set: */
		result.push_back(input.getValuatorSlotFeature(forwardedFeature.getIndex()+numPrivateValuators));
		}
	
	return result;
	}

InputDevice* TransformTool::getSourceDevice(const InputDevice* forwardedDevice)
	{
	/* Paranoia: Check if the forwarded device is the same as the transformed device: */
	if(forwardedDevice!=transformedDevice)
		Misc::throwStdErr("TransformTool::getSourceDevice: Forwarded device is not transformed device");
	
	/* Return the designated source device: */
	return sourceDevice;
	}

InputDeviceFeatureSet TransformTool::getForwardedFeatures(const InputDeviceFeature& sourceFeature)
	{
	/* Find the input assignment slot for the given feature: */
	int slotIndex=input.findFeature(sourceFeature);
	
	/* Check if the source feature belongs to this tool: */
	if(slotIndex<0)
		Misc::throwStdErr("TransformTool::getForwardedFeatures: Source feature is not part of tool's input assignment");
	
	/* Create an empty feature set: */
	InputDeviceFeatureSet result;
	
	/* Check if the feature is a button or valuator: */
	if(sourceFeature.isButton())
		{
		/* Get the slot's button slot index: */
		int buttonSlotIndex=input.getButtonSlotIndex(slotIndex);
		
		/* Check if the button is part of the forwarded subset: */
		if(buttonSlotIndex>=numPrivateButtons)
			{
			/* Add the forwarded feature to the result set: */
			result.push_back(InputDeviceFeature(transformedDevice,InputDevice::BUTTON,buttonSlotIndex-numPrivateButtons));
			}
		}
	
	if(sourceFeature.isValuator())
		{
		/* Get the slot's valuator slot index: */
		int valuatorSlotIndex=input.getValuatorSlotIndex(slotIndex);
		
		/* Check if the valuator is part of the forwarded subset: */
		if(valuatorSlotIndex>=numPrivateValuators)
			{
			/* Add the forwarded feature to the result set: */
			result.push_back(InputDeviceFeature(transformedDevice,InputDevice::VALUATOR,valuatorSlotIndex-numPrivateValuators));
			}
		}
	
	return result;
	}

}
