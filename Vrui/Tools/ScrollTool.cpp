/***********************************************************************
ScrollTool - Class for tools that can scroll inside certain GLMotif GUI
widgets. ScrollTool objects are cascadable and prevent valuator events
if they would fall into the area of interest of scrollable widgets.
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

#include <Vrui/Tools/ScrollTool.h>

#include <Misc/ThrowStdErr.h>
#include <Math/Math.h>
#include <GLMotif/TextControlEvent.h>
#include <Vrui/Vrui.h>
#include <Vrui/InputGraphManager.h>
#include <Vrui/InputDeviceManager.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/**********************************
Methods of class ScrollToolFactory:
**********************************/

ScrollToolFactory::ScrollToolFactory(ToolManager& toolManager)
	:ToolFactory("ScrollTool",toolManager)
	{
	/* Initialize tool layout: */
	layout.setNumValuators(1);
	
	/* Insert class into class hierarchy: */
	ToolFactory* toolFactory=toolManager.loadClass("UserInterfaceTool");
	toolFactory->addChildClass(this);
	addParentClass(toolFactory);
	
	/* Set tool class' factory pointer: */
	ScrollTool::factory=this;
	}

ScrollToolFactory::~ScrollToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	ScrollTool::factory=0;
	}

const char* ScrollToolFactory::getName(void) const
	{
	return "GUI Scrolling";
	}

const char* ScrollToolFactory::getValuatorFunction(int) const
	{
	return "Scroll";
	}

Tool* ScrollToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new ScrollTool(this,inputAssignment);
	}

void ScrollToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveScrollToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("UserInterfaceTool");
	}

extern "C" ToolFactory* createScrollToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	ScrollToolFactory* widgetToolFactory=new ScrollToolFactory(*toolManager);
	
	/* Return factory object: */
	return widgetToolFactory;
	}

extern "C" void destroyScrollToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/***********************************
Static elements of class ScrollTool:
***********************************/

ScrollToolFactory* ScrollTool::factory=0;

/***************************
Methods of class ScrollTool:
***************************/

ScrollTool::ScrollTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment)
	:UserInterfaceTool(factory,inputAssignment),
	 GUIInteractor(isUseEyeRay(),getRayOffset(),getValuatorDevice(0)),
	 valuatorDevice(0),
	 sendingEvents(false)
	{
	/* Set the interaction device: */
	interactionDevice=getValuatorDevice(0);
	}

void ScrollTool::initialize(void)
	{
	/* Create a virtual input device to shadow the valuator: */
	valuatorDevice=addVirtualInputDevice("ScrollToolValuatorDevice",0,1);
	
	/* Copy the source device's tracking type: */
	valuatorDevice->setTrackType(interactionDevice->getTrackType());
	
	/* Disable the virtual device's glyph: */
	getInputGraphManager()->getInputDeviceGlyph(valuatorDevice).disable();
	
	/* Permanently grab the virtual input device: */
	getInputGraphManager()->grabInputDevice(valuatorDevice,this);
	
	/* Initialize the virtual input device's position: */
	valuatorDevice->setDeviceRay(interactionDevice->getDeviceRayDirection(),interactionDevice->getDeviceRayStart());
	valuatorDevice->setTransformation(interactionDevice->getTransformation());
	}

void ScrollTool::deinitialize(void)
	{
	/* Release the virtual input device: */
	getInputGraphManager()->releaseInputDevice(valuatorDevice,this);
	
	/* Destroy the virtual input device: */
	getInputDeviceManager()->destroyInputDevice(valuatorDevice);
	valuatorDevice=0;
	}

const ToolFactory* ScrollTool::getFactory(void) const
	{
	return factory;
	}

void ScrollTool::valuatorCallback(int,InputDevice::ValuatorCallbackData* cbData)
	{
	if(cbData->newValuatorValue!=0.0) // Valuator is pushed
		{
		/* Check if the GUI interactor accepts the event: */
		GUIInteractor::updateRay();
		GLMotif::TextControlEvent tce(cbData->newValuatorValue>0.0?GLMotif::TextControlEvent::CURSOR_UP:GLMotif::TextControlEvent::CURSOR_DOWN);
		int numEvents=int(Math::ceil(Math::abs(cbData->newValuatorValue)*10.0));
		sendingEvents=false;
		for(int i=0;i<numEvents;++i)
			sendingEvents=GUIInteractor::textControl(tce)||sendingEvents;
		if(sendingEvents)
			{
			/* Request another frame: */
			scheduleUpdate(getApplicationTime()+1.0/125.0);
			}
		else
			{
			/* Pass the valuator event to the virtual input device: */
			valuatorDevice->setValuator(0,cbData->newValuatorValue);
			}
		}
	else // Valuator has just been released
		{
		/* Check if the tool has been sending text control events: */
		if(!sendingEvents)
			{
			/* Pass the button event to the virtual input device: */
			valuatorDevice->setValuator(0,cbData->newValuatorValue);
			}
		sendingEvents=false;
		}
	}

void ScrollTool::frame(void)
	{
	/* Update the GUI interactor: */
	GUIInteractor::updateRay();
	GUIInteractor::move();
	
	/* Update the virtual input device: */
	valuatorDevice->setDeviceRay(interactionDevice->getDeviceRayDirection(),interactionDevice->getDeviceRayStart());
	valuatorDevice->setTransformation(interactionDevice->getTransformation());
	}

void ScrollTool::display(GLContextData& contextData) const
	{
	if(isDrawRay())
		{
		/* Draw the GUI interactor's state: */
		GUIInteractor::glRenderAction(getRayWidth(),getRayColor(),contextData);
		}
	}

std::vector<InputDevice*> ScrollTool::getForwardedDevices(void)
	{
	std::vector<InputDevice*> result;
	result.push_back(valuatorDevice);
	return result;
	}

InputDeviceFeatureSet ScrollTool::getSourceFeatures(const InputDeviceFeature& forwardedFeature)
	{
	/* Paranoia: Check if the forwarded feature is on the transformed device: */
	if(forwardedFeature.getDevice()!=valuatorDevice)
		Misc::throwStdErr("ScrollTool::getSourceFeatures: Forwarded feature is not on transformed device");
	
	/* Return the source feature: */
	InputDeviceFeatureSet result;
	result.push_back(input.getValuatorSlotFeature(0));
	return result;
	}

InputDevice* ScrollTool::getSourceDevice(const InputDevice* forwardedDevice)
	{
	/* Paranoia: Check if the forwarded device is the same as the transformed device: */
	if(forwardedDevice!=valuatorDevice)
		Misc::throwStdErr("ScrollTool::getSourceDevice: Given forwarded device is not transformed device");
	
	/* Return the designated source device: */
	return interactionDevice;
	}

InputDeviceFeatureSet ScrollTool::getForwardedFeatures(const InputDeviceFeature& sourceFeature)
	{
	/* Paranoia: Check if the source feature belongs to this tool: */
	if(input.findFeature(sourceFeature)!=0)
		Misc::throwStdErr("ScrollTool::getForwardedFeatures: Source feature is not part of tool's input assignment");
	
	/* Return the forwarded feature: */
	InputDeviceFeatureSet result;
	result.push_back(InputDeviceFeature(valuatorDevice,InputDevice::VALUATOR,0));
	return result;
	}

}
