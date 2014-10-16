/***********************************************************************
ShiftButtonTool - Class to switch between planes of buttons and/or
valuators by pressing a "shift" button.
Copyright (c) 2010-2013 Oliver Kreylos

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

#include <Vrui/Tools/ShiftButtonTool.h>

#include <Misc/ThrowStdErr.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Vrui/Vrui.h>
#include <Vrui/InputDeviceManager.h>
#include <Vrui/InputGraphManager.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/***************************************
Methods of class ShiftButtonToolFactory:
***************************************/

ShiftButtonToolFactory::ShiftButtonToolFactory(ToolManager& toolManager)
	:ToolFactory("ShiftButtonTool",toolManager),
	 toggle(false),forwardShiftButton(false),resetFeatures(false)
	{
	/* Initialize tool layout: */
	layout.setNumButtons(1,true);
	layout.setNumValuators(0,true);
	
	/* Insert class into class hierarchy: */
	TransformToolFactory* transformToolFactory=dynamic_cast<TransformToolFactory*>(toolManager.loadClass("TransformTool"));
	transformToolFactory->addChildClass(this);
	addParentClass(transformToolFactory);
	
	/* Load class settings: */
	Misc::ConfigurationFileSection cfs=toolManager.getToolClassSection(getClassName());
	toggle=cfs.retrieveValue<bool>("./toggle",toggle);
	forwardShiftButton=cfs.retrieveValue<bool>("./forwardShiftButton",forwardShiftButton);
	resetFeatures=cfs.retrieveValue<bool>("./resetFeatures",resetFeatures);
	
	/* Set tool class' factory pointer: */
	ShiftButtonTool::factory=this;
	}

ShiftButtonToolFactory::~ShiftButtonToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	ShiftButtonTool::factory=0;
	}

const char* ShiftButtonToolFactory::getName(void) const
	{
	return "Shift Button";
	}

const char* ShiftButtonToolFactory::getButtonFunction(int buttonSlotIndex) const
	{
	if(buttonSlotIndex==0)
		return "Shift";
	else
		return ToolFactory::getButtonFunction(buttonSlotIndex-1);
	}

Tool* ShiftButtonToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new ShiftButtonTool(this,inputAssignment);
	}

void ShiftButtonToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveShiftButtonToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("TransformTool");
	}

extern "C" ToolFactory* createShiftButtonToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	ShiftButtonToolFactory* shiftButtonToolFactory=new ShiftButtonToolFactory(*toolManager);
	
	/* Return factory object: */
	return shiftButtonToolFactory;
	}

extern "C" void destroyShiftButtonToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/****************************************
Static elements of class ShiftButtonTool:
****************************************/

ShiftButtonToolFactory* ShiftButtonTool::factory=0;

/********************************
Methods of class ShiftButtonTool:
********************************/

ShiftButtonTool::ShiftButtonTool(const ToolFactory* sFactory,const ToolInputAssignment& inputAssignment)
	:TransformTool(sFactory,inputAssignment),
	 toggle(factory->toggle),forwardShiftButton(factory->forwardShiftButton),resetFeatures(factory->resetFeatures),
	 shifted(false)
	{
	/* Set the transformation source device: */
	if(input.getNumButtonSlots()>1)
		sourceDevice=getButtonDevice(1);
	else if(input.getNumValuatorSlots()>0)
		sourceDevice=getValuatorDevice(0);
	else
		sourceDevice=getButtonDevice(0); // User didn't select anything to forward; let's just pretend it makes sense
	}

ShiftButtonTool::~ShiftButtonTool(void)
	{
	}

void ShiftButtonTool::configure(const Misc::ConfigurationFileSection& configFileSection)
	{
	/* Read settings: */
	toggle=configFileSection.retrieveValue<bool>("./toggle",toggle);
	forwardShiftButton=configFileSection.retrieveValue<bool>("./forwardShiftButton",forwardShiftButton);
	resetFeatures=configFileSection.retrieveValue<bool>("./resetFeatures",resetFeatures);
	}

void ShiftButtonTool::storeState(Misc::ConfigurationFileSection& configFileSection) const
	{
	/* Write settings: */
	configFileSection.storeValue<bool>("./toggle",toggle);
	configFileSection.storeValue<bool>("./forwardShiftButton",forwardShiftButton);
	configFileSection.storeValue<bool>("./resetFeatures",resetFeatures);
	}

void ShiftButtonTool::initialize(void)
	{
	/* Create a virtual input device to shadow the source input device: */
	int numForwardedButtons=input.getNumButtonSlots()-1;
	if(forwardShiftButton)
		++numForwardedButtons;
	transformedDevice=addVirtualInputDevice("ShiftButtonToolTransformedDevice",2*numForwardedButtons,2*input.getNumValuatorSlots());
	
	/* Copy the source device's tracking type: */
	transformedDevice->setTrackType(sourceDevice->getTrackType());
	
	/* Disable the virtual input device's glyph: */
	getInputGraphManager()->getInputDeviceGlyph(transformedDevice).disable();
	
	/* Permanently grab the virtual input device: */
	getInputGraphManager()->grabInputDevice(transformedDevice,this);
	
	/* Initialize the virtual input device's position: */
	transformedDevice->setTransformation(sourceDevice->getTransformation());
	}

void ShiftButtonTool::deinitialize(void)
	{
	/* Release the virtual input device: */
	getInputGraphManager()->releaseInputDevice(transformedDevice,this);
	
	/* Destroy the virtual input device: */
	getInputDeviceManager()->destroyInputDevice(transformedDevice);
	transformedDevice=0;
	}

const ToolFactory* ShiftButtonTool::getFactory(void) const
	{
	return factory;
	}

void ShiftButtonTool::buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData)
	{
	int numForwardedButtons=input.getNumButtonSlots()-1;
	if(forwardShiftButton)
		++numForwardedButtons;
	int firstForwardedButton=forwardShiftButton?0:1;
	
	if(buttonSlotIndex==0)
		{
		/* Set the shift state: */
		bool oldShifted=shifted;
		if(toggle)
			{
			/* Toggle state upon button press: */
			if(cbData->newButtonState)
				shifted=!shifted;
			}
		else
			shifted=cbData->newButtonState;
		
		if(shifted!=oldShifted)
			{
			if(forwardShiftButton)
				{
				/* Set the states of the forwarded shift buttons: */
				transformedDevice->setButtonState(0,!shifted);
				transformedDevice->setButtonState(numForwardedButtons,shifted);
				}
			
			/* Set the newly mapped plane's state to the input device's button and valuator states: */
			int buttonBase=shifted?numForwardedButtons:0;
			for(int i=1;i<input.getNumButtonSlots();++i)
				transformedDevice->setButtonState(buttonBase-firstForwardedButton+i,getButtonState(i));
			int valuatorBase=shifted?input.getNumValuatorSlots():0;
			for(int i=0;i<input.getNumValuatorSlots();++i)
				transformedDevice->setValuator(valuatorBase+i,getValuatorState(i));
			
			if(resetFeatures)
				{
				/* Reset the previously mapped plane: */
				int buttonBase=oldShifted?numForwardedButtons:0;
				for(int i=1;i<input.getNumButtonSlots();++i)
					transformedDevice->setButtonState(buttonBase-firstForwardedButton+i,false);
				int valuatorBase=oldShifted?input.getNumValuatorSlots():0;
				for(int i=0;i<input.getNumValuatorSlots();++i)
					transformedDevice->setValuator(valuatorBase+i,0.0);
				}
			}
		}
	else
		{
		/* Pass the button event through to the virtual input device: */
		int buttonBase=shifted?numForwardedButtons:0;
		transformedDevice->setButtonState(buttonBase-firstForwardedButton+buttonSlotIndex,cbData->newButtonState);
		}
	}

void ShiftButtonTool::valuatorCallback(int valuatorSlotIndex,InputDevice::ValuatorCallbackData* cbData)
	{
	/* Pass the valuator event through to the virtual input device: */
	int valuatorBase=shifted?input.getNumValuatorSlots():0;
	transformedDevice->setValuator(valuatorSlotIndex+valuatorBase,cbData->newValuatorValue);
	}

InputDeviceFeatureSet ShiftButtonTool::getSourceFeatures(const InputDeviceFeature& forwardedFeature)
	{
	/* Paranoia: Check if the forwarded feature is on the transformed device: */
	if(forwardedFeature.getDevice()!=transformedDevice)
		Misc::throwStdErr("ShiftButtonTool::getSourceFeatures: Forwarded feature is not on transformed device");
	
	/* Create an empty feature set: */
	InputDeviceFeatureSet result;
	
	if(forwardedFeature.isButton())
		{
		/* Find the source button slot index: */
		int numForwardedButtons=input.getNumButtonSlots()-1;
		if(forwardShiftButton)
			++numForwardedButtons;
		int firstForwardedButton=forwardShiftButton?0:1;
		int buttonSlotIndex=forwardedFeature.getIndex();
		if(buttonSlotIndex>=numForwardedButtons)
			buttonSlotIndex-=numForwardedButtons;
		
		/* Add the button slot's feature to the result set: */
		result.push_back(input.getButtonSlotFeature(buttonSlotIndex+firstForwardedButton));
		}
	
	if(forwardedFeature.isValuator())
		{
		/* Find the source valuator slot index: */
		int valuatorSlotIndex=forwardedFeature.getIndex();
		if(valuatorSlotIndex>=input.getNumValuatorSlots())
			valuatorSlotIndex-=input.getNumValuatorSlots();
		
		/* Add the valuator slot's feature to the result set: */
		result.push_back(input.getValuatorSlotFeature(valuatorSlotIndex));
		}
	
	return result;
	}

InputDeviceFeatureSet ShiftButtonTool::getForwardedFeatures(const InputDeviceFeature& sourceFeature)
	{
	/* Find the input assignment slot for the given feature: */
	int slotIndex=input.findFeature(sourceFeature);
	
	/* Check if the source feature belongs to this tool: */
	if(slotIndex<0)
		Misc::throwStdErr("ShiftButtonTool::getForwardedFeatures: Source feature is not part of tool's input assignment");
	
	/* Create an empty feature set: */
	InputDeviceFeatureSet result;
	
	/* Check if the feature is a button or valuator: */
	if(sourceFeature.isButton())
		{
		/* Get the slot's button slot index: */
		int buttonSlotIndex=input.getButtonSlotIndex(slotIndex);
		
		/* Check if the button is part of the forwarded subset: */
		int numForwardedButtons=input.getNumButtonSlots()-1;
		if(forwardShiftButton)
			++numForwardedButtons;
		int firstForwardedButton=forwardShiftButton?0:1;
		if(buttonSlotIndex>=firstForwardedButton)
			{
			/* Add the forwarded feature for the current shift plane to the result set: */
			int buttonBase=shifted?numForwardedButtons:0;
			result.push_back(InputDeviceFeature(transformedDevice,InputDevice::BUTTON,buttonBase-firstForwardedButton+buttonSlotIndex));
			}
		}
	
	if(sourceFeature.isValuator())
		{
		/* Get the slot's valuator slot index: */
		int valuatorSlotIndex=input.getValuatorSlotIndex(slotIndex);
		
		/* Add the forwarded feature for the current chamber to the result set: */
		int valuatorBase=shifted?input.getNumValuatorSlots():0;
		result.push_back(InputDeviceFeature(transformedDevice,InputDevice::VALUATOR,valuatorSlotIndex+valuatorBase));
		}
	
	return result;
	}

}
