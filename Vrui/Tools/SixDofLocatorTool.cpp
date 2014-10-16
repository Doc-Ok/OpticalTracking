/***********************************************************************
SixDofLocatorTool - Class for simple 6-DOF localization using a single
input device.
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

#include <Vrui/Tools/SixDofLocatorTool.h>

#include <Geometry/OrthogonalTransformation.h>
#include <Vrui/Vrui.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/*****************************************
Methods of class SixDofLocatorToolFactory:
*****************************************/

SixDofLocatorToolFactory::SixDofLocatorToolFactory(ToolManager& toolManager)
	:ToolFactory("SixDofLocatorTool",toolManager)
	{
	/* Initialize tool layout: */
	layout.setNumButtons(1);
	
	/* Insert class into class hierarchy: */
	ToolFactory* locatorToolFactory=toolManager.loadClass("LocatorTool");
	locatorToolFactory->addChildClass(this);
	addParentClass(locatorToolFactory);
	
	/* Set tool class' factory pointer: */
	SixDofLocatorTool::factory=this;
	}

SixDofLocatorToolFactory::~SixDofLocatorToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	SixDofLocatorTool::factory=0;
	}

const char* SixDofLocatorToolFactory::getName(void) const
	{
	return "6-DOF Locator";
	}

Tool* SixDofLocatorToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new SixDofLocatorTool(this,inputAssignment);
	}

void SixDofLocatorToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveSixDofLocatorToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("LocatorTool");
	}

extern "C" ToolFactory* createSixDofLocatorToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	SixDofLocatorToolFactory* sixDofLocatorToolFactory=new SixDofLocatorToolFactory(*toolManager);
	
	/* Return factory object: */
	return sixDofLocatorToolFactory;
	}

extern "C" void destroySixDofLocatorToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/******************************************
Static elements of class SixDofLocatorTool:
******************************************/

SixDofLocatorToolFactory* SixDofLocatorTool::factory=0;

/**********************************
Methods of class SixDofLocatorTool:
**********************************/

SixDofLocatorTool::SixDofLocatorTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment)
	:LocatorTool(factory,inputAssignment)
	{
	}

const ToolFactory* SixDofLocatorTool::getFactory(void) const
	{
	return factory;
	}

void SixDofLocatorTool::buttonCallback(int,InputDevice::ButtonCallbackData* cbData)
	{
	if(cbData->newButtonState) // Button has just been pressed
		{
		/* Call button press callbacks: */
		ButtonPressCallbackData cbData(this,Vrui::getDeviceTransformation(getButtonDevice(0)));
		buttonPressCallbacks.call(&cbData);
		}
	else // Button has just been released
		{
		/* Call button release callbacks: */
		ButtonReleaseCallbackData cbData(this,Vrui::getDeviceTransformation(getButtonDevice(0)));
		buttonReleaseCallbacks.call(&cbData);
		}
	}

void SixDofLocatorTool::frame(void)
	{
	/* Call motion callbacks: */
	MotionCallbackData cbData(this,Vrui::getDeviceTransformation(getButtonDevice(0)));
	motionCallbacks.call(&cbData);
	}

}
