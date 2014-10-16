/***********************************************************************
SixDofDraggingTool - Class for simple 6-DOF dragging using a single
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

#include <Vrui/Tools/SixDofDraggingTool.h>

#include <Vrui/Vrui.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/******************************************
Methods of class SixDofDraggingToolFactory:
******************************************/

SixDofDraggingToolFactory::SixDofDraggingToolFactory(ToolManager& toolManager)
	:ToolFactory("SixDofDraggingTool",toolManager)
	{
	/* Initialize tool layout: */
	layout.setNumButtons(1);
	
	/* Insert class into class hierarchy: */
	ToolFactory* draggingToolFactory=toolManager.loadClass("DraggingTool");
	draggingToolFactory->addChildClass(this);
	addParentClass(draggingToolFactory);
	
	/* Set tool class' factory pointer: */
	SixDofDraggingTool::factory=this;
	}

SixDofDraggingToolFactory::~SixDofDraggingToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	SixDofDraggingTool::factory=0;
	}

const char* SixDofDraggingToolFactory::getName(void) const
	{
	return "6-DOF Dragger";
	}

Tool* SixDofDraggingToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new SixDofDraggingTool(this,inputAssignment);
	}

void SixDofDraggingToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveSixDofDraggingToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("DraggingTool");
	}

extern "C" ToolFactory* createSixDofDraggingToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	SixDofDraggingToolFactory* sixDofDraggingToolFactory=new SixDofDraggingToolFactory(*toolManager);
	
	/* Return factory object: */
	return sixDofDraggingToolFactory;
	}

extern "C" void destroySixDofDraggingToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/*******************************************
Static elements of class SixDofDraggingTool:
*******************************************/

SixDofDraggingToolFactory* SixDofDraggingTool::factory=0;

/***********************************
Methods of class SixDofDraggingTool:
***********************************/

SixDofDraggingTool::SixDofDraggingTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment)
	:DraggingTool(factory,inputAssignment),
	 draggingMode(IDLE)
	{
	}

const ToolFactory* SixDofDraggingTool::getFactory(void) const
	{
	return factory;
	}

void SixDofDraggingTool::buttonCallback(int,InputDevice::ButtonCallbackData* cbData)
	{
	/* Get the main device: */
	InputDevice* device=getButtonDevice(0);
	
	if(cbData->newButtonState) // Button has just been pressed
		{
		/* Act depending on this tool's current state: */
		switch(draggingMode)
			{
			case IDLE:
				{
				/* Initialize the dragging transformations: */
				NavTrackerState initial=Vrui::getDeviceTransformation(device);
				preScale=Geometry::invert(initial);
				
				/* Call drag start callbacks: */
				DragStartCallbackData cbData(this,initial);
				if(device->isRayDevice())
					cbData.setRay(Ray(initial.getOrigin(),initial.transform(device->getDeviceRayDirection())));
				dragStartCallbacks.call(&cbData);
				
				/* Go from IDLE to MOVING mode: */
				draggingMode=MOVING;
				break;
				}
			
			default:
				/* This shouldn't happen; just ignore the event */
				break;
			}
		}
	else // Button has just been released
		{
		/* Act depending on this tool's current state: */
		switch(draggingMode)
			{
			case MOVING:
				{
				/* Calculate dragging transformations: */
				NavTrackerState final=Vrui::getDeviceTransformation(device);
				NavTrackerState increment=preScale;
				increment*=final;
				
				/* Call drag callbacks one last time (there won't be a drag callback with the most recent transformation): */
				DragCallbackData cbData1(this,final,increment);
				dragCallbacks.call(&cbData1);
				
				/* Call drag end callbacks: */
				DragEndCallbackData cbData2(this,final,increment);
				dragEndCallbacks.call(&cbData2);
				
				/* Go from MOVING to IDLE mode: */
				draggingMode=IDLE;
				break;
				}
			
			default:
				/* This shouldn't happen; just ignore the event */
				break;
			}
		}
	}

void SixDofDraggingTool::frame(void)
	{
	/* Act depending on this tool's current state: */
	switch(draggingMode)
		{
		case IDLE:
			{
			/* Calculate dragging transformations: */
			NavTrackerState current=Vrui::getDeviceTransformation(getButtonDevice(0));
			
			/* Call idle motion callbacks: */
			IdleMotionCallbackData cbData(this,current);
			idleMotionCallbacks.call(&cbData);
			break;
			}
		
		case MOVING:
			{
			/* Calculate dragging transformations: */
			NavTrackerState current=Vrui::getDeviceTransformation(getButtonDevice(0));
			NavTrackerState increment=preScale;
			increment*=current;

			/* Call drag callbacks: */
			DragCallbackData cbData(this,current,increment);
			dragCallbacks.call(&cbData);
			break;
			}
		}
	}

}
