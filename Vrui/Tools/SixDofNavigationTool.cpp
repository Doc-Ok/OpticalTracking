/***********************************************************************
SixDofNavigationTool - Class for 6-DOF navigation by grabbing space
using a single input device.
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

#include <Vrui/Tools/SixDofNavigationTool.h>

#include <Vrui/Vrui.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/********************************************
Methods of class SixDofNavigationToolFactory:
********************************************/

SixDofNavigationToolFactory::SixDofNavigationToolFactory(ToolManager& toolManager)
	:ToolFactory("SixDofNavigationTool",toolManager)
	{
	/* Initialize tool layout: */
	layout.setNumButtons(1);
	
	/* Insert class into class hierarchy: */
	ToolFactory* navigationToolFactory=toolManager.loadClass("NavigationTool");
	navigationToolFactory->addChildClass(this);
	addParentClass(navigationToolFactory);
	
	/* Set tool class' factory pointer: */
	SixDofNavigationTool::factory=this;
	}

SixDofNavigationToolFactory::~SixDofNavigationToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	SixDofNavigationTool::factory=0;
	}

const char* SixDofNavigationToolFactory::getName(void) const
	{
	return "6-DOF";
	}

const char* SixDofNavigationToolFactory::getButtonFunction(int) const
	{
	return "Grab Space";
	}

Tool* SixDofNavigationToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new SixDofNavigationTool(this,inputAssignment);
	}

void SixDofNavigationToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveSixDofNavigationToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("NavigationTool");
	}

extern "C" ToolFactory* createSixDofNavigationToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	SixDofNavigationToolFactory* sixDofNavigationToolFactory=new SixDofNavigationToolFactory(*toolManager);
	
	/* Return factory object: */
	return sixDofNavigationToolFactory;
	}

extern "C" void destroySixDofNavigationToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/*********************************************
Static elements of class SixDofNavigationTool:
*********************************************/

SixDofNavigationToolFactory* SixDofNavigationTool::factory=0;

/*************************************
Methods of class SixDofNavigationTool:
*************************************/

SixDofNavigationTool::SixDofNavigationTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment)
	:NavigationTool(factory,inputAssignment)
	{
	}

const ToolFactory* SixDofNavigationTool::getFactory(void) const
	{
	return factory;
	}

void SixDofNavigationTool::buttonCallback(int,InputDevice::ButtonCallbackData* cbData)
	{
	if(cbData->newButtonState) // Button has just been pressed
		{
		/* Try activating this tool: */
		if(activate())
			{
			/* Initialize the navigation transformations: */
			preScale=Geometry::invert(getButtonDeviceTransformation(0));
			preScale*=getNavigationTransformation();
			}
		}
	else // Button has just been released
		{
		/* Deactivate this tool: */
		deactivate();
		}
	}

void SixDofNavigationTool::frame(void)
	{
	/* Act depending on this tool's current state: */
	if(isActive())
		{
		/* Compose the new navigation transformation: */
		NavTrackerState navigation=getButtonDeviceTransformation(0);
		navigation*=preScale;
		
		/* Update Vrui's navigation transformation: */
		setNavigationTransformation(navigation);
		}
	}

}
