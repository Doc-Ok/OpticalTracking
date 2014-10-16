/***********************************************************************
ClutchTool - Class to offset the position and orientation of an input
device using a "clutch" button to disengage a virtual device from a
source device.
Copyright (c) 2007-2010 Oliver Kreylos

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

#include <Vrui/Tools/ClutchTool.h>

#include <Misc/ThrowStdErr.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Geometry/OrthonormalTransformation.h>
#include <Vrui/Vrui.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/**********************************
Methods of class ClutchToolFactory:
**********************************/

ClutchToolFactory::ClutchToolFactory(ToolManager& toolManager)
	:ToolFactory("ClutchTool",toolManager),
	 clutchButtonToggleFlag(false)
	{
	/* Initialize tool layout: */
	layout.setNumButtons(1,true);
	layout.setNumValuators(0,true);
	
	/* Insert class into class hierarchy: */
	TransformToolFactory* transformToolFactory=dynamic_cast<TransformToolFactory*>(toolManager.loadClass("TransformTool"));
	transformToolFactory->addChildClass(this);
	addParentClass(transformToolFactory);
	
	/* Load class settings: */
	Misc::ConfigurationFileSection cfs=toolManager.getToolClassSection(getClassName());
	clutchButtonToggleFlag=cfs.retrieveValue<bool>("./clutchButtonToggleFlag",clutchButtonToggleFlag);
	
	/* Set tool class' factory pointer: */
	ClutchTool::factory=this;
	}

ClutchToolFactory::~ClutchToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	ClutchTool::factory=0;
	}

const char* ClutchToolFactory::getName(void) const
	{
	return "Clutch Transformation";
	}

const char* ClutchToolFactory::getButtonFunction(int buttonSlotIndex) const
	{
	if(buttonSlotIndex==0)
		{
		if(clutchButtonToggleFlag)
			return "Toggle Clutch";
		else
			return "Disengage Clutch";
		}
	else
		return ToolFactory::getButtonFunction(buttonSlotIndex-1);
	}

Tool* ClutchToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new ClutchTool(this,inputAssignment);
	}

void ClutchToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveClutchToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("TransformTool");
	}

extern "C" ToolFactory* createClutchToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	ClutchToolFactory* mouseToolFactory=new ClutchToolFactory(*toolManager);
	
	/* Return factory object: */
	return mouseToolFactory;
	}

extern "C" void destroyClutchToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/**********************************
Static elements of class ClutchTool:
**********************************/

ClutchToolFactory* ClutchTool::factory=0;

/**************************
Methods of class ClutchTool:
**************************/

ClutchTool::ClutchTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment)
	:TransformTool(factory,inputAssignment),
	 offset(TrackerState::identity),
	 clutchButtonState(false)
	{
	/* Set the transformation source device: */
	if(input.getNumButtonSlots()>1)
		sourceDevice=getButtonDevice(1);
	else if(input.getNumValuatorSlots()>0)
		sourceDevice=getValuatorDevice(0);
	else
		sourceDevice=getButtonDevice(0); // User didn't select anything to forward; let's just pretend it makes sense
	}

ClutchTool::~ClutchTool(void)
	{
	}

const ToolFactory* ClutchTool::getFactory(void) const
	{
	return factory;
	}

void ClutchTool::buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData)
	{
	if(buttonSlotIndex==0) // Clutch button
		{
		bool mustInit=false;
		if(factory->clutchButtonToggleFlag)
			{
			if(!cbData->newButtonState)
				{
				clutchButtonState=!clutchButtonState;
				mustInit=!clutchButtonState;
				}
			}
		else
			{
			clutchButtonState=cbData->newButtonState;
			mustInit=!clutchButtonState;
			}
		
		if(mustInit)
			{
			/* Calculate the new offset transformation: */
			Vector offsetT=transformedDevice->getPosition()-getButtonDevicePosition(0);
			Rotation offsetR=transformedDevice->getTransformation().getRotation()*Geometry::invert(sourceDevice->getTransformation().getRotation());
			offset=TrackerState(offsetT,offsetR);
			}
		}
	else // Pass-through button
		{
		/* Let transform tool handle it: */
		TransformTool::buttonCallback(buttonSlotIndex,cbData);
		}
	}

void ClutchTool::frame(void)
	{
	if(!clutchButtonState)
		{
		/* Update the transformation of the transformed device: */
		TrackerState clutch=sourceDevice->getTransformation();
		clutch.leftMultiply(TrackerState::rotateAround(sourceDevice->getPosition(),offset.getRotation()));
		clutch.leftMultiply(TrackerState::translate(offset.getTranslation()));
		clutch.renormalize();
		transformedDevice->setTransformation(clutch);
		}
	}

}
