/***********************************************************************
WaldoDraggingTool - Class for 6-DOF dragging with scaled-down dragging
transformations.
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

#include <Vrui/Tools/WaldoDraggingTool.h>

#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Vrui/Vrui.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/******************************************
Methods of class WaldoDraggingToolFactory:
******************************************/

WaldoDraggingToolFactory::WaldoDraggingToolFactory(ToolManager& toolManager)
	:ToolFactory("WaldoDraggingTool",toolManager),
	 linearScale(0.25),angularScale(0.25)
	{
	/* Initialize tool layout: */
	layout.setNumButtons(1);
	
	/* Insert class into class hierarchy: */
	ToolFactory* draggingToolFactory=toolManager.loadClass("DraggingTool");
	draggingToolFactory->addChildClass(this);
	addParentClass(draggingToolFactory);
	
	/* Load class settings: */
	Misc::ConfigurationFileSection cfs=toolManager.getToolClassSection(getClassName());
	linearScale=cfs.retrieveValue<Scalar>("./linearScale",linearScale);
	angularScale=cfs.retrieveValue<Scalar>("./angularScale",angularScale);
	
	/* Set tool class' factory pointer: */
	WaldoDraggingTool::factory=this;
	}

WaldoDraggingToolFactory::~WaldoDraggingToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	WaldoDraggingTool::factory=0;
	}

const char* WaldoDraggingToolFactory::getName(void) const
	{
	return "Waldo (Scaled) Dragger";
	}

Tool* WaldoDraggingToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new WaldoDraggingTool(this,inputAssignment);
	}

void WaldoDraggingToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveWaldoDraggingToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("DraggingTool");
	}

extern "C" ToolFactory* createWaldoDraggingToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	WaldoDraggingToolFactory* sixDofDraggingToolFactory=new WaldoDraggingToolFactory(*toolManager);
	
	/* Return factory object: */
	return sixDofDraggingToolFactory;
	}

extern "C" void destroyWaldoDraggingToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/*******************************************
Static elements of class WaldoDraggingTool:
*******************************************/

WaldoDraggingToolFactory* WaldoDraggingTool::factory=0;

/***********************************
Methods of class WaldoDraggingTool:
***********************************/

WaldoDraggingTool::WaldoDraggingTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment)
	:DraggingTool(factory,inputAssignment),
	 draggingMode(IDLE)
	{
	}

const ToolFactory* WaldoDraggingTool::getFactory(void) const
	{
	return factory;
	}

void WaldoDraggingTool::buttonCallback(int,InputDevice::ButtonCallbackData* cbData)
	{
	/* Get pointer to input device that caused the event: */
	InputDevice* device=getButtonDevice(0);
	
	if(cbData->newButtonState) // Button has just been pressed
		{
		/* Act depending on this tool's current state: */
		switch(draggingMode)
			{
			case IDLE:
				{
				/* Initialize the dragging transformations: */
				initial=Vrui::getDeviceTransformation(device);
				increment=NavTrackerState::identity;
				last=initial;
				
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
				/* Calculate dragging transformation increment: */
				NavTrackerState dev=Vrui::getDeviceTransformation(device);
				NavTrackerState update=dev;
				update*=Geometry::invert(last);
				last=dev;
				
				/* Scale linear and angular motion: */
				Vector translation=update.getTranslation()*factory->linearScale;
				Vector rotation=update.getRotation().getScaledAxis()*factory->angularScale;
				update=NavTrackerState(translation,NavTrackerState::Rotation(rotation),update.getScaling());
				
				/* Calculate result transformations: */
				increment*=update;
				NavTrackerState final=initial;
				final*=increment;
				
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

void WaldoDraggingTool::frame(void)
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
			/* Calculate dragging transformation increment: */
			NavTrackerState dev=Vrui::getDeviceTransformation(getButtonDevice(0));
			NavTrackerState update=dev;
			update*=Geometry::invert(last);
			last=dev;
			
			/* Scale linear and angular motion: */
			Vector translation=update.getTranslation()*factory->linearScale;
			Vector rotation=update.getRotation().getScaledAxis()*factory->angularScale;
			update=NavTrackerState(translation,NavTrackerState::Rotation(rotation),update.getScaling());
			
			/* Calculate result transformations: */
			increment*=update;
			NavTrackerState current=initial;
			current*=increment;
			
			/* Call drag callbacks: */
			DragCallbackData cbData(this,current,increment);
			dragCallbacks.call(&cbData);
			break;
			}
		}
	}

}
