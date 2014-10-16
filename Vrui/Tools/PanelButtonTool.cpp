/***********************************************************************
PanelButtonTool - Class to map a single input device button to several
virtual input device buttons by presenting an extensible panel with GUI
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

#include <Vrui/Tools/PanelButtonTool.h>

#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <GLMotif/PopupWindow.h>
#include <GLMotif/WidgetStateHelper.h>
#include <Vrui/Vrui.h>
#include <Vrui/InputDeviceManager.h>
#include <Vrui/GlyphRenderer.h>
#include <Vrui/InputGraphManager.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/******************************************************
Methods of class PanelButtonToolFactory::Configuration:
******************************************************/

PanelButtonToolFactory::Configuration::Configuration(void)
	:panelVertical(true),
	 dynamic(true),
	 numButtons(2)
	{
	}

void PanelButtonToolFactory::Configuration::load(const Misc::ConfigurationFileSection& cfs)
	{
	panelVertical=cfs.retrieveValue<bool>("./panelVertical",panelVertical);
	dynamic=cfs.retrieveValue<bool>("./dynamic",dynamic);
	numButtons=cfs.retrieveValue<int>("./numButtons",numButtons);
	}

void PanelButtonToolFactory::Configuration::save(Misc::ConfigurationFileSection& cfs) const
	{
	cfs.storeValue<bool>("./panelVertical",panelVertical);
	cfs.storeValue<bool>("./dynamic",dynamic);
	cfs.storeValue<int>("./numButtons",numButtons);
	}

/***************************************
Methods of class PanelButtonToolFactory:
***************************************/

PanelButtonToolFactory::PanelButtonToolFactory(ToolManager& toolManager)
	:ToolFactory("PanelButtonTool",toolManager)
	{
	/* Insert class into class hierarchy: */
	TransformToolFactory* transformToolFactory=dynamic_cast<TransformToolFactory*>(toolManager.loadClass("TransformTool"));
	transformToolFactory->addChildClass(this);
	addParentClass(transformToolFactory);
	
	/* Load class settings: */
	config.load(toolManager.getToolClassSection(getClassName()));
	
	/* Initialize tool layout: */
	layout.setNumButtons(1);
	
	/* Set tool class' factory pointer: */
	PanelButtonTool::factory=this;
	}

PanelButtonToolFactory::~PanelButtonToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	PanelButtonTool::factory=0;
	}

const char* PanelButtonToolFactory::getName(void) const
	{
	return "Panel Multi-Button";
	}

const char* PanelButtonToolFactory::getButtonFunction(int buttonSlotIndex) const
	{
	return "Forwarded Button";
	}

Tool* PanelButtonToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new PanelButtonTool(this,inputAssignment);
	}

void PanelButtonToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolvePanelButtonToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("TransformTool");
	}

extern "C" ToolFactory* createPanelButtonToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	PanelButtonToolFactory* revolverToolFactory=new PanelButtonToolFactory(*toolManager);
	
	/* Return factory object: */
	return revolverToolFactory;
	}

extern "C" void destroyPanelButtonToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/****************************************
Static elements of class PanelButtonTool:
****************************************/

PanelButtonToolFactory* PanelButtonTool::factory=0;

/********************************
Methods of class PanelButtonTool:
********************************/

PanelButtonTool::PanelButtonTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment)
	:TransformTool(factory,inputAssignment),
	 config(PanelButtonTool::factory->config),
	 panelPopup(0),currentButton(0)
	{
	/* Set the transformation source device and forwarding parameters: */
	sourceDevice=getButtonDevice(0);
	numPrivateButtons=0;
	}

PanelButtonTool::~PanelButtonTool(void)
	{
	/* Pop down and delete the button panel: */
	delete panelPopup;
	}

void PanelButtonTool::configure(const Misc::ConfigurationFileSection& configFileSection)
	{
	/* Override current configuration from the configuration file section: */
	config.load(configFileSection);
	
	/* Read the measurement dialog's position, orientation, and size: */
	GLMotif::readTopLevelPosition(panelPopup,configFileSection);
	
	/* Read the currently selected virtual button: */
	currentButton=configFileSection.retrieveValue<int>("./currentButton",currentButton);
	if(currentButton>config.numButtons-1)
		{
		/* Do something: */
		}
	}

void PanelButtonTool::storeState(Misc::ConfigurationFileSection& configFileSection) const
	{
	/* Store current configuration to the configuration file section: */
	config.save(configFileSection);
	}

void PanelButtonTool::initialize(void)
	{
	/* Create a virtual input device to shadow the source input device: */
	transformedDevice=addVirtualInputDevice("PanelButtonToolTransformedDevice",config.numButtons,0);
	
	/* Copy the source device's tracking type: */
	transformedDevice->setTrackType(sourceDevice->getTrackType());
	
	/* Disable the virtual input device's glyph: */
	getInputGraphManager()->getInputDeviceGlyph(transformedDevice).disable();
	
	/* Permanently grab the virtual input device: */
	getInputGraphManager()->grabInputDevice(transformedDevice,this);
	
	/* Initialize the virtual input device's position: */
	resetDevice();
	}

const ToolFactory* PanelButtonTool::getFactory(void) const
	{
	return factory;
	}

void PanelButtonTool::buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData)
	{
	}

}
