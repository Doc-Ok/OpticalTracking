/***********************************************************************
ValuatorToButtonTool - Class to convert a single valuator into a pair of
buttons.
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

#include <Vrui/Tools/ValuatorToButtonTool.h>

#include <Misc/ThrowStdErr.h>
#include <Vrui/Vrui.h>
#include <Vrui/InputGraphManager.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/********************************************
Methods of class ValuatorToButtonToolFactory:
********************************************/

ValuatorToButtonToolFactory::ValuatorToButtonToolFactory(ToolManager& toolManager)
	:ToolFactory("ValuatorToButtonTool",toolManager)
	{
	/* Insert class into class hierarchy: */
	TransformToolFactory* transformToolFactory=dynamic_cast<TransformToolFactory*>(toolManager.loadClass("TransformTool"));
	transformToolFactory->addChildClass(this);
	addParentClass(transformToolFactory);
	
	/* Initialize tool layout: */
	layout.setNumValuators(1,true);
	
	/* Set tool class' factory pointer: */
	ValuatorToButtonTool::factory=this;
	}

ValuatorToButtonToolFactory::~ValuatorToButtonToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	ValuatorToButtonTool::factory=0;
	}

const char* ValuatorToButtonToolFactory::getName(void) const
	{
	return "Valuator -> Button";
	}

const char* ValuatorToButtonToolFactory::getValuatorFunction(int) const
	{
	return "Button Pair";
	}

Tool* ValuatorToButtonToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new ValuatorToButtonTool(this,inputAssignment);
	}

void ValuatorToButtonToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveValuatorToButtonToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("TransformTool");
	}

extern "C" ToolFactory* createValuatorToButtonToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	ValuatorToButtonToolFactory* valuatorToButtonToolFactory=new ValuatorToButtonToolFactory(*toolManager);
	
	/* Return factory object: */
	return valuatorToButtonToolFactory;
	}

extern "C" void destroyValuatorToButtonToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/*********************************************
Static elements of class ValuatorToButtonTool:
*********************************************/

ValuatorToButtonToolFactory* ValuatorToButtonTool::factory=0;

/*************************************
Methods of class ValuatorToButtonTool:
*************************************/

ValuatorToButtonTool::ValuatorToButtonTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment)
	:TransformTool(factory,inputAssignment)
	{
	/* Set the transformation source device: */
	sourceDevice=getValuatorDevice(0);
	}

ValuatorToButtonTool::~ValuatorToButtonTool(void)
	{
	}

void ValuatorToButtonTool::initialize(void)
	{
	/* Create a virtual input device to shadow the source input device: */
	transformedDevice=addVirtualInputDevice("ValuatorToButtonToolTransformedDevice",input.getNumValuatorSlots()*2,0);
	
	/* Copy the source device's tracking type: */
	transformedDevice->setTrackType(sourceDevice->getTrackType());
	
	/* Disable the virtual input device's glyph: */
	getInputGraphManager()->getInputDeviceGlyph(transformedDevice).disable();
	
	/* Permanently grab the virtual input device: */
	getInputGraphManager()->grabInputDevice(transformedDevice,this);
	
	/* Initialize the virtual input device's position: */
	transformedDevice->setTransformation(sourceDevice->getTransformation());
	}

const ToolFactory* ValuatorToButtonTool::getFactory(void) const
	{
	return factory;
	}

void ValuatorToButtonTool::valuatorCallback(int valuatorSlotIndex,InputDevice::ValuatorCallbackData* cbData)
	{
	/* Update the transformed device's buttons according to the valuator event: */
	bool posPressed=transformedDevice->getButtonState(valuatorSlotIndex*2+0);
	bool negPressed=transformedDevice->getButtonState(valuatorSlotIndex*2+1);
	if(posPressed&&cbData->newValuatorValue<0.3)
		transformedDevice->setButtonState(valuatorSlotIndex*2+0,false);
	if(!posPressed&&cbData->newValuatorValue>0.7)
		transformedDevice->setButtonState(valuatorSlotIndex*2+0,true);
	if(negPressed&&cbData->newValuatorValue>-0.3)
		transformedDevice->setButtonState(valuatorSlotIndex*2+1,false);
	if(!negPressed&&cbData->newValuatorValue<-0.7)
		transformedDevice->setButtonState(valuatorSlotIndex*2+1,true);
	}

InputDeviceFeatureSet ValuatorToButtonTool::getSourceFeatures(const InputDeviceFeature& forwardedFeature)
	{
	/* Paranoia: Check if the forwarded feature is on the transformed device: */
	if(forwardedFeature.getDevice()!=transformedDevice)
		Misc::throwStdErr("ValuatorToButtonTool::getSourceFeatures: Forwarded feature is not on transformed device");
	
	/* Return the valuator slot feeding the forwarded button slot: */
	InputDeviceFeatureSet result;
	result.push_back(input.getValuatorSlotFeature(forwardedFeature.getIndex()/2));
	
	return result;
	}

InputDeviceFeatureSet ValuatorToButtonTool::getForwardedFeatures(const InputDeviceFeature& sourceFeature)
	{
	/* Find the input assignment slot for the given feature: */
	int slotIndex=input.findFeature(sourceFeature);
	
	/* Check if the source feature belongs to this tool: */
	if(slotIndex<0)
		Misc::throwStdErr("ValuatorToButtonTool::getForwardedFeatures: Source feature is not part of tool's input assignment");
	
	/* Get the slot's valuator slot index: */
	int valuatorSlotIndex=input.getValuatorSlotIndex(slotIndex);
	
	/* Return the two button slots fed by the source valuator slot: */
	InputDeviceFeatureSet result;
	for(int i=0;i<2;++i)
		result.push_back(InputDeviceFeature(transformedDevice,InputDevice::BUTTON,valuatorSlotIndex*2+i));
	
	return result;
	}

}
