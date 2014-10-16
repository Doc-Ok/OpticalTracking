/***********************************************************************
WaldoLocatorTool - Class for 6-DOF localization with scaled-down
transformations while the tool button is pressed.
Copyright (c) 2006-2010 Oliver Kreylos

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

#include <Vrui/Tools/WaldoLocatorTool.h>

#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Vrui/Vrui.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/****************************************
Methods of class WaldoLocatorToolFactory:
****************************************/

WaldoLocatorToolFactory::WaldoLocatorToolFactory(ToolManager& toolManager)
	:ToolFactory("WaldoLocatorTool",toolManager),
	 linearScale(0.25),angularScale(0.25)
	{
	/* Initialize tool layout: */
	layout.setNumButtons(1);
	
	/* Insert class into class hierarchy: */
	ToolFactory* locatorToolFactory=toolManager.loadClass("LocatorTool");
	locatorToolFactory->addChildClass(this);
	addParentClass(locatorToolFactory);
	
	/* Load class settings: */
	Misc::ConfigurationFileSection cfs=toolManager.getToolClassSection(getClassName());
	linearScale=cfs.retrieveValue<Scalar>("./linearScale",linearScale);
	angularScale=cfs.retrieveValue<Scalar>("./angularScale",angularScale);
	
	/* Set tool class' factory pointer: */
	WaldoLocatorTool::factory=this;
	}

WaldoLocatorToolFactory::~WaldoLocatorToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	WaldoLocatorTool::factory=0;
	}

const char* WaldoLocatorToolFactory::getName(void) const
	{
	return "Waldo (Scaled) Locator";
	}

Tool* WaldoLocatorToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new WaldoLocatorTool(this,inputAssignment);
	}

void WaldoLocatorToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveWaldoLocatorToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("LocatorTool");
	}

extern "C" ToolFactory* createWaldoLocatorToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	WaldoLocatorToolFactory* sixDofLocatorToolFactory=new WaldoLocatorToolFactory(*toolManager);
	
	/* Return factory object: */
	return sixDofLocatorToolFactory;
	}

extern "C" void destroyWaldoLocatorToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/*****************************************
Static elements of class WaldoLocatorTool:
*****************************************/

WaldoLocatorToolFactory* WaldoLocatorTool::factory=0;

/*********************************
Methods of class WaldoLocatorTool:
*********************************/

WaldoLocatorTool::WaldoLocatorTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment)
	:LocatorTool(factory,inputAssignment),
	 active(false)
	{
	}

const ToolFactory* WaldoLocatorTool::getFactory(void) const
	{
	return factory;
	}

void WaldoLocatorTool::buttonCallback(int,InputDevice::ButtonCallbackData* cbData)
	{
	if(cbData->newButtonState) // Button has just been pressed
		{
		/* Activate the tool: */
		active=true;
		
		/* Initialize the scaled transformation: */
		initial=Vrui::getDeviceTransformation(getButtonDevice(0));
		increment=NavTrackerState::identity;
		last=initial;
		
		/* Call button press callbacks: */
		ButtonPressCallbackData cbData(this,initial);
		buttonPressCallbacks.call(&cbData);
		}
	else
		{
		/* Deactivate the tool: */
		active=false;
		
		/* Call button release callbacks: */
		ButtonReleaseCallbackData cbData(this,initial*increment);
		buttonReleaseCallbacks.call(&cbData);
		}
	}

void WaldoLocatorTool::frame(void)
	{
	/* Act depending on this tool's current state: */
	if(active)
		{
		/* Update the incremental transformation: */
		NavTrackerState dev=Vrui::getDeviceTransformation(getButtonDevice(0));
		NavTrackerState update=dev;
		update*=Geometry::invert(last);
		last=dev;
		
		/* Scale linear and angular motion: */
		Vector translation=update.getTranslation()*factory->linearScale;
		Vector rotation=update.getRotation().getScaledAxis()*factory->angularScale;
		update=NavTrackerState(translation,NavTrackerState::Rotation(rotation),update.getScaling());
		
		/* Calculate result transformations: */
		increment.leftMultiply(update);
		NavTrackerState current=initial;
		current.leftMultiply(increment);
		
		/* Call motion callbacks with the scaled device transformation: */
		MotionCallbackData cbData(this,current);
		motionCallbacks.call(&cbData);
		}
	else
		{
		/* Call motion callbacks with the true device transformation: */
		MotionCallbackData cbData(this,Vrui::getDeviceTransformation(getButtonDevice(0)));
		motionCallbacks.call(&cbData);
		}
	}

}
