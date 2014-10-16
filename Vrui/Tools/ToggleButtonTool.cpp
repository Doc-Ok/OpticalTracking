/***********************************************************************
ToggleButtonTool - Class to convert a set of regular buttons into toggle
buttons.
Copyright (c) 2013 Oliver Kreylos

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

#include <Vrui/Tools/ToggleButtonTool.h>

#include <Misc/ThrowStdErr.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Vrui/Vrui.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/****************************************
Methods of class ToggleButtonToolFactory:
****************************************/

ToggleButtonToolFactory::ToggleButtonToolFactory(ToolManager& toolManager)
	:ToolFactory("ToggleButtonTool",toolManager),
	 onRisingEdge(true)
	{
	/* Insert class into class hierarchy: */
	TransformToolFactory* transformToolFactory=dynamic_cast<TransformToolFactory*>(toolManager.loadClass("TransformTool"));
	transformToolFactory->addChildClass(this);
	addParentClass(transformToolFactory);
	
	/* Load class settings: */
	Misc::ConfigurationFileSection cfs=toolManager.getToolClassSection(getClassName());
	onRisingEdge=cfs.retrieveValue<bool>("./onRisingEdge",onRisingEdge);
	
	/* Initialize tool layout: */
	layout.setNumButtons(0,true);
	
	/* Set tool class' factory pointer: */
	ToggleButtonTool::factory=this;
	}

ToggleButtonToolFactory::~ToggleButtonToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	ToggleButtonTool::factory=0;
	}

const char* ToggleButtonToolFactory::getName(void) const
	{
	return "Toggle Button";
	}

Tool* ToggleButtonToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new ToggleButtonTool(this,inputAssignment);
	}

void ToggleButtonToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveToggleButtonToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("TransformTool");
	}

extern "C" ToolFactory* createToggleButtonToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	ToggleButtonToolFactory* offsetToolFactory=new ToggleButtonToolFactory(*toolManager);
	
	/* Return factory object: */
	return offsetToolFactory;
	}

extern "C" void destroyToggleButtonToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/*****************************************
Static elements of class ToggleButtonTool:
*****************************************/

ToggleButtonToolFactory* ToggleButtonTool::factory=0;

/*********************************
Methods of class ToggleButtonTool:
*********************************/

ToggleButtonTool::ToggleButtonTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment)
	:TransformTool(factory,inputAssignment)
	{
	/* Set the transformation source device: */
	sourceDevice=getButtonDevice(0);
	}

ToggleButtonTool::~ToggleButtonTool(void)
	{
	}

const ToolFactory* ToggleButtonTool::getFactory(void) const
	{
	return factory;
	}

void ToggleButtonTool::buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData)
	{
	/* Detect if the button event corresponds to the configured active edge: */
	bool edge=cbData->newButtonState;
	if(!factory->onRisingEdge)
		edge=!edge;
	
	if(edge)
		{
		/* Toggle the state of the transformed button: */
		transformedDevice->setButtonState(buttonSlotIndex,!transformedDevice->getButtonState(buttonSlotIndex));
		}
	}

}
