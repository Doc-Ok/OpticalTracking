/***********************************************************************
ScaleNavigationTool - Class for scaling by grabbing space at the scale
center point and sliding along a device-relative direction
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

#include <Vrui/Tools/ScaleNavigationTool.h>

#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Math/Math.h>
#include <Geometry/GeometryValueCoders.h>
#include <Vrui/Vrui.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/*******************************************
Methods of class ScaleNavigationToolFactory:
*******************************************/

ScaleNavigationToolFactory::ScaleNavigationToolFactory(ToolManager& toolManager)
	:ToolFactory("ScaleNavigationTool",toolManager),
	 scaleDirection(0,1,0),
	 scaleFactor(getInchFactor()*Scalar(-8))
	{
	/* Initialize tool layout: */
	layout.setNumButtons(1);
	
	/* Insert class into class hierarchy: */
	ToolFactory* navigationToolFactory=toolManager.loadClass("NavigationTool");
	navigationToolFactory->addChildClass(this);
	addParentClass(navigationToolFactory);
	
	/* Load class settings: */
	Misc::ConfigurationFileSection cfs=toolManager.getToolClassSection(getClassName());
	scaleDirection=cfs.retrieveValue<Vector>("./scaleDirection",scaleDirection);
	scaleFactor=cfs.retrieveValue<Scalar>("./scaleFactor",scaleFactor);
	
	/* Set tool class' factory pointer: */
	ScaleNavigationTool::factory=this;
	}

ScaleNavigationToolFactory::~ScaleNavigationToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	ScaleNavigationTool::factory=0;
	}

const char* ScaleNavigationToolFactory::getName(void) const
	{
	return "Scaling Only";
	}

const char* ScaleNavigationToolFactory::getButtonFunction(int) const
	{
	return "Zoom";
	}

Tool* ScaleNavigationToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new ScaleNavigationTool(this,inputAssignment);
	}

void ScaleNavigationToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveScaleNavigationToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("NavigationTool");
	}

extern "C" ToolFactory* createScaleNavigationToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	ScaleNavigationToolFactory* scaleNavigationToolFactory=new ScaleNavigationToolFactory(*toolManager);
	
	/* Return factory object: */
	return scaleNavigationToolFactory;
	}

extern "C" void destroyScaleNavigationToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/********************************************
Static elements of class ScaleNavigationTool:
********************************************/

ScaleNavigationToolFactory* ScaleNavigationTool::factory=0;

/************************************
Methods of class ScaleNavigationTool:
************************************/

ScaleNavigationTool::ScaleNavigationTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment)
	:NavigationTool(factory,inputAssignment)
	{
	}

const ToolFactory* ScaleNavigationTool::getFactory(void) const
	{
	return factory;
	}

void ScaleNavigationTool::buttonCallback(int,InputDevice::ButtonCallbackData* cbData)
	{
	if(cbData->newButtonState) // Button has just been pressed
		{
		/* Try activating the tool: */
		if(activate())
			{
			/* Determine the scaling center and direction: */
			scalingCenter=getButtonDevicePosition(0);
			scalingDirection=getButtonDeviceTransformation(0).transform(factory->scaleDirection);
			initialScale=scalingCenter*scalingDirection;
			
			/* Initialize the transformation parts: */
			preScale=NavTrackerState::translateFromOriginTo(scalingCenter);
			postScale=NavTrackerState::translateToOriginFrom(scalingCenter);
			postScale*=getNavigationTransformation();
			}
		}
	else if(isActive())
		{
		/* Deactivate the tool: */
		deactivate();
		}
	}

void ScaleNavigationTool::frame(void)
	{
	/* Scale if the tool is active: */
	if(isActive())
		{
		/* Compose the new navigation transformation: */
		NavTrackerState navigation=preScale;
		Scalar currentScale=getButtonDevicePosition(0)*scalingDirection-initialScale;
		navigation*=NavTrackerState::scale(Math::exp(currentScale/factory->scaleFactor));
		navigation*=postScale;
		
		/* Update Vrui's navigation transformation: */
		setNavigationTransformation(navigation);
		}
	}

}
