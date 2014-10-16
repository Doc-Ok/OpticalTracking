/***********************************************************************
MultiShiftButtonTool - Class to switch between mulitple planes of
buttons and/or valuators by pressing one from an array of "radio
buttons."
Copyright (c) 2012-2014 Oliver Kreylos

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

#include <Vrui/Tools/MultiShiftButtonTool.h>

#include <Misc/ThrowStdErr.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Vrui/Vrui.h>
#include <Vrui/InputDeviceManager.h>
#include <Vrui/InputGraphManager.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/***********************************************************
Methods of class MultiShiftButtonToolFactory::Configuration:
***********************************************************/

MultiShiftButtonToolFactory::Configuration::Configuration(void)
	:numPlanes(2),
	 forwardRadioButtons(false),
	 resetFeatures(false),
	 initialPlane(0)
	{
	}

void MultiShiftButtonToolFactory::Configuration::read(const Misc::ConfigurationFileSection& cfs)
	{
	numPlanes=cfs.retrieveValue<int>("./numPlanes",numPlanes);
	forwardRadioButtons=cfs.retrieveValue<bool>("./forwardRadioButtons",forwardRadioButtons);
	resetFeatures=cfs.retrieveValue<bool>("./resetFeatures",resetFeatures);
	initialPlane=cfs.retrieveValue<int>("./initialPlane",initialPlane);
	}

void MultiShiftButtonToolFactory::Configuration::write(Misc::ConfigurationFileSection& cfs) const
	{
	cfs.storeValue<int>("./numPlanes",numPlanes);
	cfs.storeValue<bool>("./forwardRadioButtons",forwardRadioButtons);
	cfs.storeValue<bool>("./resetFeatures",resetFeatures);
	cfs.storeValue<int>("./initialPlane",initialPlane);
	}

/********************************************
Methods of class MultiShiftButtonToolFactory:
********************************************/

MultiShiftButtonToolFactory::MultiShiftButtonToolFactory(ToolManager& toolManager)
	:ToolFactory("MultiShiftButtonTool",toolManager)
	{
	/* Initialize tool layout: */
	layout.setNumButtons(1,true);
	layout.setNumValuators(0,true);
	
	/* Insert class into class hierarchy: */
	TransformToolFactory* transformToolFactory=dynamic_cast<TransformToolFactory*>(toolManager.loadClass("TransformTool"));
	transformToolFactory->addChildClass(this);
	addParentClass(transformToolFactory);
	
	/* Load class settings: */
	config.read(toolManager.getToolClassSection(getClassName()));
	
	/* Set tool class' factory pointer: */
	MultiShiftButtonTool::factory=this;
	}

MultiShiftButtonToolFactory::~MultiShiftButtonToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	MultiShiftButtonTool::factory=0;
	}

const char* MultiShiftButtonToolFactory::getName(void) const
	{
	return "Radio Buttons";
	}

const char* MultiShiftButtonToolFactory::getButtonFunction(int buttonSlotIndex) const
	{
	if(buttonSlotIndex==0)
		return "First Radio Button";
	else
		return "Additional Radio or Forwarded Button";
	}

Tool* MultiShiftButtonToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new MultiShiftButtonTool(this,inputAssignment);
	}

void MultiShiftButtonToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveMultiShiftButtonToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("TransformTool");
	}

extern "C" ToolFactory* createMultiShiftButtonToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	MultiShiftButtonToolFactory* multiShiftButtonToolFactory=new MultiShiftButtonToolFactory(*toolManager);
	
	/* Return factory object: */
	return multiShiftButtonToolFactory;
	}

extern "C" void destroyMultiShiftButtonToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/*********************************************
Static elements of class MultiShiftButtonTool:
*********************************************/

MultiShiftButtonToolFactory* MultiShiftButtonTool::factory=0;

/*************************************
Methods of class MultiShiftButtonTool:
*************************************/

MultiShiftButtonTool::MultiShiftButtonTool(const ToolFactory* sFactory,const ToolInputAssignment& inputAssignment)
	:TransformTool(sFactory,inputAssignment),
	 config(factory->config)
	{
	}

MultiShiftButtonTool::~MultiShiftButtonTool(void)
	{
	}

void MultiShiftButtonTool::configure(const Misc::ConfigurationFileSection& configFileSection)
	{
	/* Read settings: */
	config.read(configFileSection);
	}

void MultiShiftButtonTool::storeState(Misc::ConfigurationFileSection& configFileSection) const
	{
	/* Write settings: */
	config.write(configFileSection);
	}

void MultiShiftButtonTool::initialize(void)
	{
	/* Set the transformation source device: */
	if(input.getNumButtonSlots()>config.numPlanes)
		sourceDevice=getButtonDevice(config.numPlanes);
	else if(input.getNumValuatorSlots()>0)
		sourceDevice=getValuatorDevice(0);
	else
		sourceDevice=getButtonDevice(0); // User didn't select anything to forward; let's just pretend it makes sense
	
	/* Create a virtual input device to shadow the source input device: */
	numForwardedButtons=input.getNumButtonSlots()-config.numPlanes;
	if(config.forwardRadioButtons)
		++numForwardedButtons;
	firstForwardedButton=config.forwardRadioButtons?1:0;
	transformedDevice=addVirtualInputDevice("MultiShiftButtonToolTransformedDevice",config.numPlanes*numForwardedButtons,config.numPlanes*input.getNumValuatorSlots());
	
	/* Copy the source device's tracking type: */
	transformedDevice->setTrackType(sourceDevice->getTrackType());
	
	/* Disable the virtual input device's glyph: */
	getInputGraphManager()->getInputDeviceGlyph(transformedDevice).disable();
	
	/* Permanently grab the virtual input device: */
	getInputGraphManager()->grabInputDevice(transformedDevice,this);
	
	/* Initialize the virtual input device's position: */
	resetDevice();
	
	/* Activate the requested button/valuator plane on the next frame: */
	requestedPlane=nextPlane=config.initialPlane;
	currentPlane=-1;
	}

void MultiShiftButtonTool::deinitialize(void)
	{
	/* Release the virtual input device: */
	getInputGraphManager()->releaseInputDevice(transformedDevice,this);
	
	/* Destroy the virtual input device: */
	getInputDeviceManager()->destroyInputDevice(transformedDevice);
	transformedDevice=0;
	}

const ToolFactory* MultiShiftButtonTool::getFactory(void) const
	{
	return factory;
	}

void MultiShiftButtonTool::buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData)
	{
	if(buttonSlotIndex<config.numPlanes)
		{
		/* Start the plane changing process: */
		requestedPlane=buttonSlotIndex;
		}
	else
		{
		/* Pass the button event through to the virtual input device: */
		int buttonBase=currentPlane*numForwardedButtons;
		transformedDevice->setButtonState(buttonBase-config.numPlanes+firstForwardedButton+buttonSlotIndex,cbData->newButtonState);
		}
	}

void MultiShiftButtonTool::valuatorCallback(int valuatorSlotIndex,InputDevice::ValuatorCallbackData* cbData)
	{
	/* Pass the valuator event through to the virtual input device: */
	int valuatorBase=currentPlane*input.getNumValuatorSlots();
	transformedDevice->setValuator(valuatorSlotIndex+valuatorBase,cbData->newValuatorValue);
	}

void MultiShiftButtonTool::frame(void)
	{
	/* Set the forwarded device's position and orientation: */
	resetDevice();
	
	/* Check for the second step in changing button/valuator planes: */
	if(currentPlane!=nextPlane)
		{
		int buttonBase=nextPlane*numForwardedButtons;
		
		if(config.forwardRadioButtons)
			{
			/* Enable the forwarded radio button on the newly mapped plane: */
			transformedDevice->setButtonState(buttonBase,true);
			}
		
		/* Set the newly mapped plane's state to the input device's button and valuator states: */
		for(int i=config.numPlanes;i<input.getNumButtonSlots();++i)
			transformedDevice->setButtonState(buttonBase-config.numPlanes+firstForwardedButton+i,getButtonState(i));
		int valuatorBase=nextPlane*input.getNumValuatorSlots();
		for(int i=0;i<input.getNumValuatorSlots();++i)
			transformedDevice->setValuator(valuatorBase+i,getValuatorState(i));
		
		/* Finish changing planes: */
		currentPlane=nextPlane;
		}
	
	/* Check for the first step in changing button/valuator planes: */
	if(nextPlane!=requestedPlane)
		{
		/* Disable all features on the current plane: */
		int buttonBase=currentPlane*numForwardedButtons;
		
		if(config.forwardRadioButtons)
			{
			/* Disable the forwarded radio button on the currently mapped plane: */
			transformedDevice->setButtonState(buttonBase,false);
			}
		
		if(config.resetFeatures)
			{
			/* Reset all buttons and valuators in the currently mapped plane: */
			for(int i=config.numPlanes;i<input.getNumButtonSlots();++i)
				transformedDevice->setButtonState(buttonBase-config.numPlanes+firstForwardedButton+i,false);
			int valuatorBase=currentPlane*input.getNumValuatorSlots();
			for(int i=0;i<input.getNumValuatorSlots();++i)
				transformedDevice->setValuator(valuatorBase+i,0.0);
			}
		
		/* Prepare for the next step: */
		nextPlane=requestedPlane;
		requestUpdate();
		}
	}

InputDeviceFeatureSet MultiShiftButtonTool::getSourceFeatures(const InputDeviceFeature& forwardedFeature)
	{
	/* Paranoia: Check if the forwarded feature is on the transformed device: */
	if(forwardedFeature.getDevice()!=transformedDevice)
		Misc::throwStdErr("MultiShiftButtonTool::getSourceFeatures: Forwarded feature is not on transformed device");
	
	/* Create an empty feature set: */
	InputDeviceFeatureSet result;
	
	if(forwardedFeature.isButton())
		{
		/* Find the source button slot index: */
		int buttonSlotIndex=forwardedFeature.getIndex();
		buttonSlotIndex%=numForwardedButtons;
		
		/* Add the button slot's feature to the result set: */
		result.push_back(input.getButtonSlotFeature(buttonSlotIndex+firstForwardedButton));
		}
	
	if(forwardedFeature.isValuator())
		{
		/* Find the source valuator slot index: */
		int valuatorSlotIndex=forwardedFeature.getIndex();
		valuatorSlotIndex%=input.getNumValuatorSlots();
		
		/* Add the valuator slot's feature to the result set: */
		result.push_back(input.getValuatorSlotFeature(valuatorSlotIndex));
		}
	
	return result;
	}

InputDeviceFeatureSet MultiShiftButtonTool::getForwardedFeatures(const InputDeviceFeature& sourceFeature)
	{
	/* Find the input assignment slot for the given feature: */
	int slotIndex=input.findFeature(sourceFeature);
	
	/* Check if the source feature belongs to this tool: */
	if(slotIndex<0)
		Misc::throwStdErr("MultiShiftButtonTool::getForwardedFeatures: Source feature is not part of tool's input assignment");
	
	/* Create an empty feature set: */
	InputDeviceFeatureSet result;
	
	/* Check if the feature is a button or valuator: */
	if(sourceFeature.isButton())
		{
		/* Get the slot's button slot index: */
		int buttonSlotIndex=input.getButtonSlotIndex(slotIndex);
		
		/* Check if the button is part of the forwarded subset: */
		if(buttonSlotIndex>=firstForwardedButton)
			{
			/* Add the forwarded feature for the current button plane to the result set: */
			int buttonBase=currentPlane*numForwardedButtons;
			result.push_back(InputDeviceFeature(transformedDevice,InputDevice::BUTTON,buttonBase-firstForwardedButton+buttonSlotIndex));
			}
		}
	
	if(sourceFeature.isValuator())
		{
		/* Get the slot's valuator slot index: */
		int valuatorSlotIndex=input.getValuatorSlotIndex(slotIndex);
		
		/* Add the forwarded feature for the current chamber to the result set: */
		int valuatorBase=currentPlane*input.getNumValuatorSlots();
		result.push_back(InputDeviceFeature(transformedDevice,InputDevice::VALUATOR,valuatorSlotIndex+valuatorBase));
		}
	
	return result;
	}

}
