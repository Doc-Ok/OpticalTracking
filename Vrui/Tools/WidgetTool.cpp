/***********************************************************************
WidgetTool - Class for tools that can interact with GLMotif GUI widgets.
WidgetTool objects are cascadable and preempt button events if they
would fall into the area of interest of mapped widgets.
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

#include <Vrui/Tools/WidgetTool.h>

#include <Misc/ThrowStdErr.h>
#include <Vrui/Vrui.h>
#include <Vrui/InputGraphManager.h>
#include <Vrui/InputDeviceManager.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/**********************************
Methods of class WidgetToolFactory:
**********************************/

WidgetToolFactory::WidgetToolFactory(ToolManager& toolManager)
	:ToolFactory("WidgetTool",toolManager)
	{
	/* Initialize tool layout: */
	layout.setNumButtons(1);
	
	/* Insert class into class hierarchy: */
	ToolFactory* toolFactory=toolManager.loadClass("UserInterfaceTool");
	toolFactory->addChildClass(this);
	addParentClass(toolFactory);
	
	/* Set tool class' factory pointer: */
	WidgetTool::factory=this;
	}

WidgetToolFactory::~WidgetToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	WidgetTool::factory=0;
	}

const char* WidgetToolFactory::getName(void) const
	{
	return "GUI Interaction";
	}

const char* WidgetToolFactory::getButtonFunction(int) const
	{
	return "Interact";
	}

Tool* WidgetToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new WidgetTool(this,inputAssignment);
	}

void WidgetToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveWidgetToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("UserInterfaceTool");
	}

extern "C" ToolFactory* createWidgetToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	WidgetToolFactory* widgetToolFactory=new WidgetToolFactory(*toolManager);
	
	/* Return factory object: */
	return widgetToolFactory;
	}

extern "C" void destroyWidgetToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/***********************************
Static elements of class WidgetTool:
***********************************/

WidgetToolFactory* WidgetTool::factory=0;

/***************************
Methods of class WidgetTool:
***************************/

WidgetTool::WidgetTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment)
	:UserInterfaceTool(factory,inputAssignment),
	 GUIInteractor(isUseEyeRay(),getRayOffset(),getButtonDevice(0)),
	 buttonDevice(0)
	{
	/* Set the interaction device: */
	interactionDevice=getButtonDevice(0);
	}

void WidgetTool::initialize(void)
	{
	/* Create a virtual input device to shadow the button: */
	buttonDevice=addVirtualInputDevice("WidgetToolButtonDevice",1,0);
	
	/* Copy the source device's tracking type: */
	buttonDevice->setTrackType(interactionDevice->getTrackType());
	
	/* Disable the virtual device's glyph: */
	getInputGraphManager()->getInputDeviceGlyph(buttonDevice).disable();
	
	/* Permanently grab the virtual input device: */
	getInputGraphManager()->grabInputDevice(buttonDevice,this);
	
	/* Initialize the virtual input device's position: */
	buttonDevice->setDeviceRay(interactionDevice->getDeviceRayDirection(),interactionDevice->getDeviceRayStart());
	buttonDevice->setTransformation(interactionDevice->getTransformation());
	}

void WidgetTool::deinitialize(void)
	{
	/* Release the virtual input device: */
	getInputGraphManager()->releaseInputDevice(buttonDevice,this);
	
	/* Destroy the virtual input device: */
	getInputDeviceManager()->destroyInputDevice(buttonDevice);
	buttonDevice=0;
	}

const ToolFactory* WidgetTool::getFactory(void) const
	{
	return factory;
	}

void WidgetTool::buttonCallback(int,InputDevice::ButtonCallbackData* cbData)
	{
	if(cbData->newButtonState) // Button has just been pressed
		{
		/* Check if the GUI interactor accepts the event: */
		GUIInteractor::updateRay();
		if(!GUIInteractor::buttonDown(false))
			{
			/* Pass the button event to the virtual input device: */
			buttonDevice->setButtonState(0,true);
			}
		}
	else // Button has just been released
		{
		/* Check if the GUI interactor is active: */
		if(GUIInteractor::isActive())
			{
			/* Deliver the event: */
			GUIInteractor::buttonUp();
			}
		else
			{
			/* Pass the button event to the virtual input device: */
			buttonDevice->setButtonState(0,false);
			}
		}
	}

void WidgetTool::frame(void)
	{
	/* Update the GUI interactor: */
	GUIInteractor::updateRay();
	GUIInteractor::move();
	
	/* Update the virtual input device: */
	buttonDevice->setDeviceRay(interactionDevice->getDeviceRayDirection(),interactionDevice->getDeviceRayStart());
	buttonDevice->setTransformation(interactionDevice->getTransformation());
	}

void WidgetTool::display(GLContextData& contextData) const
	{
	if(isDrawRay())
		{
		/* Draw the GUI interactor's state: */
		GUIInteractor::glRenderAction(getRayWidth(),getRayColor(),contextData);
		}
	}

std::vector<InputDevice*> WidgetTool::getForwardedDevices(void)
	{
	std::vector<InputDevice*> result;
	result.push_back(buttonDevice);
	return result;
	}

InputDeviceFeatureSet WidgetTool::getSourceFeatures(const InputDeviceFeature& forwardedFeature)
	{
	/* Paranoia: Check if the forwarded feature is on the transformed device: */
	if(forwardedFeature.getDevice()!=buttonDevice)
		Misc::throwStdErr("WidgetTool::getSourceFeatures: Forwarded feature is not on transformed device");
	
	/* Return the source feature: */
	InputDeviceFeatureSet result;
	result.push_back(input.getButtonSlotFeature(0));
	return result;
	}

InputDevice* WidgetTool::getSourceDevice(const InputDevice* forwardedDevice)
	{
	/* Paranoia: Check if the forwarded device is the same as the transformed device: */
	if(forwardedDevice!=buttonDevice)
		Misc::throwStdErr("WidgetTool::getSourceDevice: Given forwarded device is not transformed device");
	
	/* Return the designated source device: */
	return interactionDevice;
	}

InputDeviceFeatureSet WidgetTool::getForwardedFeatures(const InputDeviceFeature& sourceFeature)
	{
	/* Paranoia: Check if the source feature belongs to this tool: */
	if(input.findFeature(sourceFeature)!=0)
		Misc::throwStdErr("WidgetTool::getForwardedFeatures: Source feature is not part of tool's input assignment");
	
	/* Return the forwarded feature: */
	InputDeviceFeatureSet result;
	result.push_back(InputDeviceFeature(buttonDevice,InputDevice::BUTTON,0));
	return result;
	}

}
