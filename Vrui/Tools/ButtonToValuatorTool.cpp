/***********************************************************************
ButtonToValuatorTool - Class to convert a single button or two buttons
into a two- or three-state valuator, respectively.
Copyright (c) 2011 Oliver Kreylos

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

#include <Vrui/Tools/ButtonToValuatorTool.h>

#include <Misc/ThrowStdErr.h>
#include <Vrui/Vrui.h>
#include <Vrui/InputGraphManager.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/********************************************
Methods of class ButtonToValuatorToolFactory:
********************************************/

ButtonToValuatorToolFactory::ButtonToValuatorToolFactory(ToolManager& toolManager)
	:ToolFactory("ButtonToValuatorTool",toolManager)
	{
	/* Insert class into class hierarchy: */
	TransformToolFactory* transformToolFactory=dynamic_cast<TransformToolFactory*>(toolManager.loadClass("TransformTool"));
	transformToolFactory->addChildClass(this);
	addParentClass(transformToolFactory);
	
	/* Initialize tool layout: */
	layout.setNumButtons(1,true);
	
	/* Set tool class' factory pointer: */
	ButtonToValuatorTool::factory=this;
	}

ButtonToValuatorToolFactory::~ButtonToValuatorToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	ButtonToValuatorTool::factory=0;
	}

const char* ButtonToValuatorToolFactory::getName(void) const
	{
	return "Button -> Valuator";
	}

const char* ButtonToValuatorToolFactory::getButtonFunction(int buttonSlotIndex) const
	{
	if(buttonSlotIndex==0)
		return "+1";
	else
		return "-1";
	}

Tool* ButtonToValuatorToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new ButtonToValuatorTool(this,inputAssignment);
	}

void ButtonToValuatorToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveButtonToValuatorToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("TransformTool");
	}

extern "C" ToolFactory* createButtonToValuatorToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	ButtonToValuatorToolFactory* buttonToValuatorToolFactory=new ButtonToValuatorToolFactory(*toolManager);
	
	/* Return factory object: */
	return buttonToValuatorToolFactory;
	}

extern "C" void destroyButtonToValuatorToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/*********************************************
Static elements of class ButtonToValuatorTool:
*********************************************/

ButtonToValuatorToolFactory* ButtonToValuatorTool::factory=0;

/*************************************
Methods of class ButtonToValuatorTool:
*************************************/

ButtonToValuatorTool::ButtonToValuatorTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment)
	:TransformTool(factory,inputAssignment)
	{
	/* Set the transformation source device: */
	sourceDevice=getButtonDevice(0);
	}

ButtonToValuatorTool::~ButtonToValuatorTool(void)
	{
	}

void ButtonToValuatorTool::initialize(void)
	{
	/* Create a virtual input device to shadow the source input device: */
	transformedDevice=addVirtualInputDevice("ButtonToValuatorToolTransformedDevice",0,1);
	
	/* Copy the source device's tracking type: */
	transformedDevice->setTrackType(sourceDevice->getTrackType());
	
	/* Disable the virtual input device's glyph: */
	getInputGraphManager()->getInputDeviceGlyph(transformedDevice).disable();
	
	/* Permanently grab the virtual input device: */
	getInputGraphManager()->grabInputDevice(transformedDevice,this);
	
	/* Initialize the virtual input device's position: */
	transformedDevice->setTransformation(sourceDevice->getTransformation());
	}

const ToolFactory* ButtonToValuatorTool::getFactory(void) const
	{
	return factory;
	}

void ButtonToValuatorTool::buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData)
	{
	/* Update the transformed device's valuator according to the button event: */
	double value=transformedDevice->getValuator(0);
	double delta=buttonSlotIndex==0?1.0:-1.0;
	if(cbData->newButtonState) // Button was just pressed
		value+=delta;
	else // Button was just released
		value-=delta;
	if(value<-1.0)
		value=-1.0;
	if(value>1.0)
		value=1.0;
	transformedDevice->setValuator(0,value);
	}

InputDeviceFeatureSet ButtonToValuatorTool::getSourceFeatures(const InputDeviceFeature& forwardedFeature)
	{
	/* Paranoia: Check if the forwarded feature is on the transformed device: */
	if(forwardedFeature.getDevice()!=transformedDevice)
		Misc::throwStdErr("ButtonToValuatorTool::getSourceFeatures: Forwarded feature is not on transformed device");
	
	/* The source features are all button slots: */
	InputDeviceFeatureSet result;
	for(int i=0;i<input.getNumButtonSlots();++i)
		result.push_back(input.getButtonSlotFeature(i));
	
	return result;
	}

InputDeviceFeatureSet ButtonToValuatorTool::getForwardedFeatures(const InputDeviceFeature& sourceFeature)
	{
	/* Find the input assignment slot for the given feature: */
	int slotIndex=input.findFeature(sourceFeature);
	
	/* Check if the source feature belongs to this tool: */
	if(slotIndex<0)
		Misc::throwStdErr("ButtonToValuatorTool::getForwardedFeatures: Source feature is not part of tool's input assignment");
	
	/* The only forwarded feature is the valuator slot: */
	InputDeviceFeatureSet result;
	result.push_back(InputDeviceFeature(transformedDevice,InputDevice::VALUATOR,0));
	
	return result;
	}

}
