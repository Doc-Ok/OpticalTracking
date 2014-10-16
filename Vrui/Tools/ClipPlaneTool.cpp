/***********************************************************************
ClipPlaneTool - Class for tools that add a clipping plane into an
environment when activated.
Copyright (c) 2009-2010 Oliver Kreylos

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

#include <Vrui/Tools/ClipPlaneTool.h>

#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Geometry/GeometryValueCoders.h>
#include <Vrui/Vrui.h>
#include <Vrui/ClipPlane.h>
#include <Vrui/ClipPlaneManager.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/*************************************
Methods of class ClipPlaneToolFactory:
*************************************/

ClipPlaneToolFactory::ClipPlaneToolFactory(ToolManager& toolManager)
	:ToolFactory("ClipPlaneTool",toolManager),
	 normal(0,1,0)
	{
	/* Initialize tool layout: */
	layout.setNumButtons(1);
	
	/* Insert class into class hierarchy: */
	ToolFactory* toolFactory=toolManager.loadClass("PointingTool");
	toolFactory->addChildClass(this);
	addParentClass(toolFactory);
	
	/* Load class settings: */
	Misc::ConfigurationFileSection cfs=toolManager.getToolClassSection(getClassName());
	normal=cfs.retrieveValue<Vector>("./normal",normal);
	normal.normalize();
	
	/* Set tool class' factory pointer: */
	ClipPlaneTool::factory=this;
	}

ClipPlaneToolFactory::~ClipPlaneToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	ClipPlaneTool::factory=0;
	}

const char* ClipPlaneToolFactory::getName(void) const
	{
	return "Clipping Plane";
	}

Tool* ClipPlaneToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new ClipPlaneTool(this,inputAssignment);
	}

void ClipPlaneToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveClipPlaneToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("PointingTool");
	}

extern "C" ToolFactory* createClipPlaneToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	ClipPlaneToolFactory* flashlightToolFactory=new ClipPlaneToolFactory(*toolManager);
	
	/* Return factory object: */
	return flashlightToolFactory;
	}

extern "C" void destroyClipPlaneToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/**************************************
Static elements of class ClipPlaneTool:
**************************************/

ClipPlaneToolFactory* ClipPlaneTool::factory=0;

/******************************
Methods of class ClipPlaneTool:
******************************/

ClipPlaneTool::ClipPlaneTool(const ToolFactory* sFactory,const ToolInputAssignment& inputAssignment)
	:PointingTool(sFactory,inputAssignment),
	 clipPlane(0),active(false)
	{
	/* Create a disabled clipping plane: */
	clipPlane=getClipPlaneManager()->createClipPlane(true);
	}

ClipPlaneTool::~ClipPlaneTool(void)
	{
	/* Destroy the clipping plane: */
	getClipPlaneManager()->destroyClipPlane(clipPlane);
	}

const ToolFactory* ClipPlaneTool::getFactory(void) const
	{
	return factory;
	}

void ClipPlaneTool::buttonCallback(int,InputDevice::ButtonCallbackData* cbData)
	{
	if(cbData->newButtonState) // Button has just been pressed
		{
		/* Enable the clipping plane: */
		active=true;
		clipPlane->enable();
		}
	else // Button has just been released
		{
		/* Disable the light source: */
		active=false;
		clipPlane->disable();
		}
	}

void ClipPlaneTool::frame(void)
	{
	if(active)
		{
		/* Get the new normal vector and plane center point: */
		Vector normal=getButtonDeviceTransformation(0).transform(factory->normal);
		Point center=getButtonDevicePosition(0);
		
		/* Set the clipping plane's plane equation: */
		clipPlane->getPlane()=Plane(normal,center);
		}
	}

}
