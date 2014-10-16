/***********************************************************************
PanelMenuTool - Class for menu tools that attach the program's main menu
to an input device and allow any widget interaction tool to select items
from it.
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

#include <Vrui/Tools/PanelMenuTool.h>

#include <GLMotif/WidgetManager.h>
#include <GLMotif/PopupMenu.h>
#include <Vrui/Vrui.h>
#include <Vrui/MutexMenu.h>
#include <Vrui/InputDeviceManager.h>
#include <Vrui/InputGraphManager.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/*************************************
Methods of class PanelMenuToolFactory:
*************************************/

PanelMenuToolFactory::PanelMenuToolFactory(ToolManager& toolManager)
	:ToolFactory("PanelMenuTool",toolManager)
	{
	/* Initialize tool layout: */
	layout.setNumButtons(1);
	
	/* Insert class into class hierarchy: */
	ToolFactory* menuToolFactory=toolManager.loadClass("MenuTool");
	menuToolFactory->addChildClass(this);
	addParentClass(menuToolFactory);
	
	/* Set tool class' factory pointer: */
	PanelMenuTool::factory=this;
	}

PanelMenuToolFactory::~PanelMenuToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	PanelMenuTool::factory=0;
	}

const char* PanelMenuToolFactory::getName(void) const
	{
	return "Device-Attached Menu";
	}

const char* PanelMenuToolFactory::getButtonFunction(int) const
	{
	return "Forwarded Button";
	}

Tool* PanelMenuToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new PanelMenuTool(this,inputAssignment);
	}

void PanelMenuToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolvePanelMenuToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("MenuTool");
	}

extern "C" ToolFactory* createPanelMenuToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	PanelMenuToolFactory* panelMenuToolFactory=new PanelMenuToolFactory(*toolManager);
	
	/* Return factory object: */
	return panelMenuToolFactory;
	}

extern "C" void destroyPanelMenuToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/**************************************
Static elements of class PanelMenuTool:
**************************************/

PanelMenuToolFactory* PanelMenuTool::factory=0;

/******************************
Methods of class PanelMenuTool:
******************************/

PanelMenuTool::PanelMenuTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment)
	:MenuTool(factory,inputAssignment),
	 buttonDevice(0)
	{
	/* Set the interaction device: */
	interactionDevice=getButtonDevice(0);
	}

PanelMenuTool::~PanelMenuTool(void)
	{
	if(isActive())
		{
		/* Pop down the menu: */
		getWidgetManager()->popdownWidget(menu->getPopup());
		
		/* Deactivate the tool again: */
		deactivate();
		}
	}

void PanelMenuTool::initialize(void)
	{
	/* Create a virtual input device to shadow the zoom button: */
	buttonDevice=addVirtualInputDevice("PanelMenuToolButtonDevice",1,0);
	
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

void PanelMenuTool::deinitialize(void)
	{
	/* Release the virtual input device: */
	getInputGraphManager()->releaseInputDevice(buttonDevice,this);
	
	/* Destroy the virtual input device: */
	getInputDeviceManager()->destroyInputDevice(buttonDevice);
	buttonDevice=0;
	}

const ToolFactory* PanelMenuTool::getFactory(void) const
	{
	return factory;
	}

void PanelMenuTool::buttonCallback(int,InputDevice::ButtonCallbackData* cbData)
	{
	/* Pass the button event to the virtual input device: */
	buttonDevice->setButtonState(0,cbData->newButtonState);
	}

void PanelMenuTool::frame(void)
	{
	if(isActive())
		{
		/* Calculate the menu transformation: */
		GLMotif::WidgetManager::Transformation menuTransformation=getButtonDeviceTransformation(0);
		GLMotif::Vector topLeft=menu->getPopup()->getExterior().getCorner(2);
		menuTransformation*=GLMotif::WidgetManager::Transformation::translate(-Vector(topLeft.getXyzw()));
		
		/* Set the menu's position: */
		getWidgetManager()->setPrimaryWidgetTransformation(menu->getPopup(),menuTransformation);
		}
	
	/* Update the virtual input device's position: */
	buttonDevice->setDeviceRay(interactionDevice->getDeviceRayDirection(),interactionDevice->getDeviceRayStart());
	buttonDevice->setTransformation(interactionDevice->getTransformation());
	}

void PanelMenuTool::setMenu(MutexMenu* newMenu)
	{
	/* Call the base class method first: */
	MenuTool::setMenu(newMenu);
	
	/* Try activating this tool (it will grab the main menu until it is destroyed): */
	if(activate())
		{
		/* Calculate the menu transformation: */
		GLMotif::WidgetManager::Transformation menuTransformation=getDeviceTransformation(0);
		GLMotif::Vector topLeft=menu->getPopup()->getExterior().getCorner(2);
		menuTransformation*=GLMotif::WidgetManager::Transformation::translate(-Vector(topLeft.getXyzw()));
		
		/* Pop up the menu: */
		getWidgetManager()->popupPrimaryWidget(menu->getPopup(),menuTransformation);
		}
	}

std::vector<InputDevice*> PanelMenuTool::getForwardedDevices(void)
	{
	std::vector<InputDevice*> result;
	result.push_back(buttonDevice);
	return result;
	}

InputDeviceFeatureSet PanelMenuTool::getSourceFeatures(const InputDeviceFeature& forwardedFeature)
	{
	/* Paranoia: Check if the forwarded feature is on the transformed device: */
	if(forwardedFeature.getDevice()!=buttonDevice)
		Misc::throwStdErr("PanelMenuTool::getSourceFeatures: Forwarded feature is not on transformed device");
	
	/* Return the source feature: */
	InputDeviceFeatureSet result;
	result.push_back(input.getButtonSlotFeature(0));
	return result;
	}

InputDevice* PanelMenuTool::getSourceDevice(const InputDevice* forwardedDevice)
	{
	/* Paranoia: Check if the forwarded device is the same as the transformed device: */
	if(forwardedDevice!=buttonDevice)
		Misc::throwStdErr("PanelMenuTool::getSourceDevice: Given forwarded device is not transformed device");
	
	/* Return the designated source device: */
	return interactionDevice;
	}

InputDeviceFeatureSet PanelMenuTool::getForwardedFeatures(const InputDeviceFeature& sourceFeature)
	{
	/* Paranoia: Check if the source feature belongs to this tool: */
	if(input.findFeature(sourceFeature)!=0)
		Misc::throwStdErr("PanelMenuTool::getForwardedFeatures: Source feature is not part of tool's input assignment");
	
	/* Return the forwarded feature: */
	InputDeviceFeatureSet result;
	result.push_back(InputDeviceFeature(buttonDevice,InputDevice::BUTTON,0));
	return result;
	}

}
