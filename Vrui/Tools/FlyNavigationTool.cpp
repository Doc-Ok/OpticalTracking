/***********************************************************************
FlyNavigationTool - Class encapsulating the behaviour of the old
infamous Vrui single-handed flying navigation tool.
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

#include <Vrui/Tools/FlyNavigationTool.h>

#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Math/Math.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/OrthogonalTransformation.h>
#include <Geometry/GeometryValueCoders.h>
#include <Vrui/Vrui.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/*****************************************
Methods of class FlyNavigationToolFactory:
*****************************************/

FlyNavigationToolFactory::FlyNavigationToolFactory(ToolManager& toolManager)
	:ToolFactory("FlyNavigationTool",toolManager),
	 flyDirection(Vector(0,1,0)),
	 flyFactor(getDisplaySize()*Scalar(0.5))
	{
	/* Initialize tool layout: */
	layout.setNumButtons(1);
	
	/* Insert class into class hierarchy: */
	ToolFactory* navigationToolFactory=toolManager.loadClass("NavigationTool");
	navigationToolFactory->addChildClass(this);
	addParentClass(navigationToolFactory);
	
	/* Load class settings: */
	Misc::ConfigurationFileSection cfs=toolManager.getToolClassSection(getClassName());
	flyDirection=cfs.retrieveValue<Vector>("./flyDirection",flyDirection);
	flyDirection.normalize();
	flyFactor=cfs.retrieveValue<Scalar>("./flyFactor",flyFactor);
	
	/* Set tool class' factory pointer: */
	FlyNavigationTool::factory=this;
	}


FlyNavigationToolFactory::~FlyNavigationToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	FlyNavigationTool::factory=0;
	}

const char* FlyNavigationToolFactory::getName(void) const
	{
	return "Fly (Direction Only)";
	}

const char* FlyNavigationToolFactory::getButtonFunction(int) const
	{
	return "Fly";
	}

Tool* FlyNavigationToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new FlyNavigationTool(this,inputAssignment);
	}

void FlyNavigationToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveFlyNavigationToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("NavigationTool");
	}

extern "C" ToolFactory* createFlyNavigationToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	FlyNavigationToolFactory* flyNavigationToolFactory=new FlyNavigationToolFactory(*toolManager);
	
	/* Return factory object: */
	return flyNavigationToolFactory;
	}

extern "C" void destroyFlyNavigationToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/******************************************
Static elements of class FlyNavigationTool:
******************************************/

FlyNavigationToolFactory* FlyNavigationTool::factory=0;

/**********************************
Methods of class FlyNavigationTool:
**********************************/

FlyNavigationTool::FlyNavigationTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment)
	:NavigationTool(factory,inputAssignment)
	{
	}

const ToolFactory* FlyNavigationTool::getFactory(void) const
	{
	return factory;
	}

void FlyNavigationTool::buttonCallback(int,InputDevice::ButtonCallbackData* cbData)
	{
	if(cbData->newButtonState) // Button has just been pressed
		{
		/* Try activating this tool: */
		activate();
		}
	else // Button has just been released
		{
		/* Deactivate this tool: */
		deactivate();
		}
	}

void FlyNavigationTool::frame(void)
	{
	/* Act depending on this tool's current state: */
	if(isActive())
		{
		/* Get the current state of the input device: */
		const TrackerState& ts=getButtonDeviceTransformation(0);
		
		/* Calculate the current flying velocity: */
		Vector v=ts.transform(factory->flyDirection);
		v*=-factory->flyFactor*getFrameTime();
		
		/* Compose the new navigation transformation: */
		NavTransform t=NavTransform::translate(v);
		t*=getNavigationTransformation();
		
		/* Update Vrui's navigation transformation: */
		setNavigationTransformation(t);
		
		/* Request another frame: */
		scheduleUpdate(getApplicationTime()+1.0/125.0);
		}
	}

}
