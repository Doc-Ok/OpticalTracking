/***********************************************************************
ValuatorScalingNavigationTool - Class for tools that allow scaling the
navigation transformation using a valuator.
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

#include <Vrui/Tools/ValuatorScalingNavigationTool.h>

#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Math/Math.h>
#include <Vrui/Vrui.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/*****************************************************
Methods of class ValuatorScalingNavigationToolFactory:
*****************************************************/

ValuatorScalingNavigationToolFactory::ValuatorScalingNavigationToolFactory(ToolManager& toolManager)
	:ToolFactory("ValuatorScalingNavigationTool",toolManager),
	 valuatorThreshold(Scalar(0.25)),
	 scalingFactor(Scalar(0.25))
	{
	/* Initialize tool layout: */
	layout.setNumValuators(1);
	
	/* Insert class into class hierarchy: */
	ToolFactory* navigationToolFactory=toolManager.loadClass("NavigationTool");
	navigationToolFactory->addChildClass(this);
	addParentClass(navigationToolFactory);
	
	/* Load class settings: */
	Misc::ConfigurationFileSection cfs=toolManager.getToolClassSection(getClassName());
	valuatorThreshold=cfs.retrieveValue<Scalar>("./valuatorThreshold",valuatorThreshold);
	scalingFactor=cfs.retrieveValue<Scalar>("./scalingFactor",scalingFactor);
	
	/* Set tool class' factory pointer: */
	ValuatorScalingNavigationTool::factory=this;
	}

ValuatorScalingNavigationToolFactory::~ValuatorScalingNavigationToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	ValuatorScalingNavigationTool::factory=0;
	}

const char* ValuatorScalingNavigationToolFactory::getName(void) const
	{
	return "Valuator Scaling";
	}

const char* ValuatorScalingNavigationToolFactory::getValuatorFunction(int) const
	{
	return "Zoom";
	}

Tool* ValuatorScalingNavigationToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new ValuatorScalingNavigationTool(this,inputAssignment);
	}

void ValuatorScalingNavigationToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveValuatorScalingNavigationToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("NavigationTool");
	}

extern "C" ToolFactory* createValuatorScalingNavigationToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	ValuatorScalingNavigationToolFactory* valuatorScalingNavigationToolFactory=new ValuatorScalingNavigationToolFactory(*toolManager);
	
	/* Return factory object: */
	return valuatorScalingNavigationToolFactory;
	}

extern "C" void destroyValuatorScalingNavigationToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/******************************************************
Static elements of class ValuatorScalingNavigationTool:
******************************************************/

ValuatorScalingNavigationToolFactory* ValuatorScalingNavigationTool::factory=0;

/****************************************
Methods of class ValuatorScalingNavigationTool:
****************************************/

ValuatorScalingNavigationTool::ValuatorScalingNavigationTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment)
	:NavigationTool(factory,inputAssignment),
	 currentValue(0)
	{
	}

const ToolFactory* ValuatorScalingNavigationTool::getFactory(void) const
	{
	return factory;
	}

void ValuatorScalingNavigationTool::valuatorCallback(int,InputDevice::ValuatorCallbackData* cbData)
	{
	currentValue=Scalar(cbData->newValuatorValue);
	if(Math::abs(currentValue)>factory->valuatorThreshold)
		{
		/* Try activating this tool: */
		if(!isActive()&&activate())
			{
			/* Initialize the navigation transformations: */
			scalingCenter=getValuatorDevicePosition(0);
			preScale=NavTrackerState::translateFromOriginTo(scalingCenter);
			postScale=NavTrackerState::translateToOriginFrom(scalingCenter);
			postScale*=getNavigationTransformation();
			currentScale=Scalar(1);
			}
		}
	else
		{
		/* Deactivate this tool: */
		deactivate();
		}
	}

void ValuatorScalingNavigationTool::frame(void)
	{
	/* Act depending on this tool's current state: */
	if(isActive())
		{
		/* Update the scaling factor: */
		Scalar v=currentValue;
		Scalar t=factory->valuatorThreshold;
		Scalar s=Scalar(1)-t;
		if(v<-t)
			v=(v+t)/s;
		else if(v>t)
			v=(v-t)/s;
		else
			v=Scalar(0);
		currentScale*=Math::pow(factory->scalingFactor,v*getFrameTime());
		
		/* Compose the new navigation transformation: */
		NavTrackerState navigation=preScale;
		navigation*=NavTrackerState::scale(currentScale);
		navigation*=postScale;
		
		/* Update Vrui's navigation transformation: */
		setNavigationTransformation(navigation);
		
		/* Request another frame: */
		scheduleUpdate(getApplicationTime()+1.0/125.0);
		}
	}

}
