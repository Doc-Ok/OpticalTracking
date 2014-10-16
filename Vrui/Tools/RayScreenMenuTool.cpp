/***********************************************************************
RayScreenMenuTool - Class for menu selection tools using ray selection
that align menus to screen planes.
Copyright (c) 2004-2014 Oliver Kreylos

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

#include <Vrui/Tools/RayScreenMenuTool.h>

#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Geometry/Vector.h>
#include <GLMotif/WidgetManager.h>
#include <GLMotif/PopupMenu.h>
#include <Vrui/Vrui.h>
#include <Vrui/MutexMenu.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/*****************************************
Methods of class RayScreenMenuToolFactory:
*****************************************/

RayScreenMenuToolFactory::RayScreenMenuToolFactory(ToolManager& toolManager)
	:ToolFactory("RayScreenMenuTool",toolManager),
	 interactWithWidgets(false)
	{
	/* Initialize tool layout: */
	layout.setNumButtons(1);
	
	/* Insert class into class hierarchy: */
	ToolFactory* menuToolFactory=toolManager.loadClass("MenuTool");
	menuToolFactory->addChildClass(this);
	addParentClass(menuToolFactory);
	
	/* Load class settings: */
	Misc::ConfigurationFileSection cfs=toolManager.getToolClassSection(getClassName());
	interactWithWidgets=cfs.retrieveValue<bool>("./interactWithWidgets",interactWithWidgets);
	
	/* Set tool class' factory pointer: */
	RayScreenMenuTool::factory=this;
	}

RayScreenMenuToolFactory::~RayScreenMenuToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	RayScreenMenuTool::factory=0;
	}

const char* RayScreenMenuToolFactory::getName(void) const
	{
	return "Screen-Aligned Menu";
	}

Tool* RayScreenMenuToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new RayScreenMenuTool(this,inputAssignment);
	}

void RayScreenMenuToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveRayScreenMenuToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("MenuTool");
	}

extern "C" ToolFactory* createRayScreenMenuToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	RayScreenMenuToolFactory* rayMenuToolFactory=new RayScreenMenuToolFactory(*toolManager);
	
	/* Return factory object: */
	return rayMenuToolFactory;
	}

extern "C" void destroyRayScreenMenuToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/******************************************
Static elements of class RayScreenMenuTool:
******************************************/

RayScreenMenuToolFactory* RayScreenMenuTool::factory=0;

/**********************************
Methods of class RayScreenMenuTool:
**********************************/

RayScreenMenuTool::RayScreenMenuTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment)
	:MenuTool(factory,inputAssignment),
	 GUIInteractor(isUseEyeRay(),getRayOffset(),getButtonDevice(0))
	{
	/* Set the interaction device: */
	interactionDevice=getButtonDevice(0);
	}

const ToolFactory* RayScreenMenuTool::getFactory(void) const
	{
	return factory;
	}

void RayScreenMenuTool::buttonCallback(int,InputDevice::ButtonCallbackData* cbData)
	{
	if(cbData->newButtonState) // Button has just been pressed
		{
		/* Check if the GUI interactor refuses the event: */
		GUIInteractor::updateRay();
		if(!(factory->interactWithWidgets&&GUIInteractor::buttonDown(false)))
			{
			/* Try activating this tool: */
			if(GUIInteractor::canActivate()&&activate())
				{
				/* Pop up the tool's menu at the intersection of the interaction ray and the UI plane: */
				popupPrimaryWidget(menu->getPopup(),calcRayPoint(GUIInteractor::getRay()),false);
				
				/* Grab the pointer: */
				getWidgetManager()->grabPointer(menu->getPopup());
				
				/* Force the event on the GUI interactor: */
				GUIInteractor::buttonDown(true);
				}
			}
		}
	else // Button has just been released
		{
		/* Check if the GUI interactor is active: */
		if(GUIInteractor::isActive())
			{
			/* Deliver the event: */
			GUIInteractor::buttonUp();
			
			/* Check if the tool's menu is popped up: */
			if(MenuTool::isActive())
				{
				/* Release the pointer: */
				getWidgetManager()->releasePointer(menu->getPopup());
				
				/* Pop down the menu: */
				getWidgetManager()->popdownWidget(menu->getPopup());
				
				/* Deactivate the tool: */
				deactivate();
				}
			}
		}
	}

void RayScreenMenuTool::frame(void)
	{
	if(factory->interactWithWidgets||GUIInteractor::isActive())
		{
		/* Update the GUI interactor: */
		GUIInteractor::updateRay();
		GUIInteractor::move();
		}
	}

void RayScreenMenuTool::display(GLContextData& contextData) const
	{
	if(isDrawRay()&&(factory->interactWithWidgets||GUIInteractor::isActive()))
		{
		/* Draw the GUI interactor's state: */
		GUIInteractor::glRenderAction(getRayWidth(),getRayColor(),contextData);
		}
	}

}
