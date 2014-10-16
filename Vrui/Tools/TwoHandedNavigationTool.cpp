/***********************************************************************
TwoHandedNavigationTool - Class encapsulating the behaviour of the old
famous Vrui two-handed navigation tool.
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

#include <Vrui/Tools/TwoHandedNavigationTool.h>

#include <Vrui/Vrui.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/***********************************************
Methods of class TwoHandedNavigationToolFactory:
***********************************************/

TwoHandedNavigationToolFactory::TwoHandedNavigationToolFactory(ToolManager& toolManager)
	:ToolFactory("TwoHandedNavigationTool",toolManager)
	{
	/* Initialize tool layout: */
	layout.setNumButtons(2);
	
	/* Insert class into class hierarchy: */
	ToolFactory* navigationToolFactory=toolManager.loadClass("NavigationTool");
	navigationToolFactory->addChildClass(this);
	addParentClass(navigationToolFactory);
	
	/* Set tool class' factory pointer: */
	TwoHandedNavigationTool::factory=this;
	}

TwoHandedNavigationToolFactory::~TwoHandedNavigationToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	TwoHandedNavigationTool::factory=0;
	}

const char* TwoHandedNavigationToolFactory::getName(void) const
	{
	return "Ambidextrous 6-DOF + Scaling";
	}

const char* TwoHandedNavigationToolFactory::getButtonFunction(int) const
	{
	return "Grab Space / Zoom";
	}

Tool* TwoHandedNavigationToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new TwoHandedNavigationTool(this,inputAssignment);
	}

void TwoHandedNavigationToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveTwoHandedNavigationToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("NavigationTool");
	}

extern "C" ToolFactory* createTwoHandedNavigationToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	TwoHandedNavigationToolFactory* twoHandedNavigationToolFactory=new TwoHandedNavigationToolFactory(*toolManager);
	
	/* Return factory object: */
	return twoHandedNavigationToolFactory;
	}

extern "C" void destroyTwoHandedNavigationToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/************************************************
Static elements of class TwoHandedNavigationTool:
************************************************/

TwoHandedNavigationToolFactory* TwoHandedNavigationTool::factory=0;

/****************************************
Methods of class TwoHandedNavigationTool:
****************************************/

TwoHandedNavigationTool::TwoHandedNavigationTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment)
	:NavigationTool(factory,inputAssignment),
	 navigationMode(IDLE)
	{
	}

const ToolFactory* TwoHandedNavigationTool::getFactory(void) const
	{
	return factory;
	}

void TwoHandedNavigationTool::buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData)
	{
	if(cbData->newButtonState) // Button has just been pressed
		{
		/* Act depending on this tool's current state: */
		switch(navigationMode)
			{
			case IDLE:
				/* Try activating this tool: */
				if(activate())
					{
					/* Initialize the navigation transformations: */
					preScale=Geometry::invert(getButtonDeviceTransformation(buttonSlotIndex));
					preScale*=getNavigationTransformation();
					
					/* Remember which button slot is moving: */
					movingButtonSlotIndex=buttonSlotIndex;
					
					/* Go from IDLE to MOVING mode: */
					navigationMode=MOVING;
					}
				break;
			
			case MOVING:
				/* Check if the correct button has been pressed: */
				if(buttonSlotIndex!=movingButtonSlotIndex)
					{
					/* Determine the scaling center and initial scale: */
					scalingCenter=getButtonDevicePosition(movingButtonSlotIndex);
					initialScale=Geometry::dist(getButtonDevicePosition(buttonSlotIndex),scalingCenter);
					
					/* Initialize the navigation transformations: */
					preScale=Geometry::invert(getButtonDeviceTransformation(movingButtonSlotIndex));
					preScale*=NavTrackerState::translateFromOriginTo(scalingCenter);
					postScale=NavTrackerState::translateToOriginFrom(scalingCenter);
					postScale*=getNavigationTransformation();
					
					/* Go from MOVING to SCALING mode: */
					navigationMode=SCALING;
					}
				break;
			
			default:
				/* This shouldn't happen; just ignore the event */
				break;
			}
		}
	else // Button has just been released
		{
		/* Act depending on this tool's current state: */
		switch(navigationMode)
			{
			case SCALING:
				/* If the released button is on the moving device, switch over to the other device: */
				if(buttonSlotIndex==movingButtonSlotIndex)
					movingButtonSlotIndex=1-buttonSlotIndex;
				
				/* Initialize the navigation transformations: */
				preScale=Geometry::invert(getButtonDeviceTransformation(movingButtonSlotIndex));
				preScale*=getNavigationTransformation();
				
				/* Go from SCALING to MOVING mode: */
				navigationMode=MOVING;
				break;
			
			case MOVING:
				/* Check if the correct button has been released: */
				if(buttonSlotIndex==movingButtonSlotIndex)
					{
					/* Deactivate this tool: */
					deactivate();
					
					/* Go from MOVING to IDLE mode: */
					navigationMode=IDLE;
					}
				break;
			
			default:
				/* This shouldn't happen; just ignore the event */
				break;
			}
		}
	}

void TwoHandedNavigationTool::frame(void)
	{
	/* Act depending on this tool's current state: */
	switch(navigationMode)
		{
		case IDLE:
			/* Do nothing */
			break;
		
		case MOVING:
			{
			/* Compose the new navigation transformation: */
			NavTrackerState navigation=getButtonDeviceTransformation(movingButtonSlotIndex);
			navigation*=preScale;
			
			/* Update Vrui's navigation transformation: */
			setNavigationTransformation(navigation);
			break;
			}
		
		case SCALING:
			{
			/* Compose the new navigation transformation: */
			NavTrackerState navigation=getButtonDeviceTransformation(movingButtonSlotIndex);
			navigation*=preScale;
			Scalar currentScale=Geometry::dist(getButtonDevicePosition(0),getButtonDevicePosition(1))/initialScale;
			navigation*=NavTrackerState::scale(currentScale);
			navigation*=postScale;
			
			/* Update Vrui's navigation transformation: */
			setNavigationTransformation(navigation);
			break;
			}
		}
	}

}
