/***********************************************************************
OffsetTool - Class to offset the position of an input device by a fixed
amount to extend the user's arm.
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

#include <Vrui/Tools/OffsetTool.h>

#include <Misc/ThrowStdErr.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Geometry/GeometryValueCoders.h>
#include <Vrui/Vrui.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/**********************************
Methods of class OffsetToolFactory:
**********************************/

OffsetToolFactory::OffsetToolFactory(ToolManager& toolManager)
	:ToolFactory("OffsetTool",toolManager),
	 offset(ONTransform::translate(Vector(0,getDisplaySize()*Scalar(0.5),0)))
	{
	/* Insert class into class hierarchy: */
	TransformToolFactory* transformToolFactory=dynamic_cast<TransformToolFactory*>(toolManager.loadClass("TransformTool"));
	transformToolFactory->addChildClass(this);
	addParentClass(transformToolFactory);
	
	/* Load class settings: */
	Misc::ConfigurationFileSection cfs=toolManager.getToolClassSection(getClassName());
	offset=cfs.retrieveValue<ONTransform>("./offset",offset);
	
	/* Initialize tool layout: */
	layout.setNumButtons(0,true);
	layout.setNumValuators(0,true);
	
	/* Set tool class' factory pointer: */
	OffsetTool::factory=this;
	}

OffsetToolFactory::~OffsetToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	OffsetTool::factory=0;
	}

const char* OffsetToolFactory::getName(void) const
	{
	return "Offset Transformation";
	}

Tool* OffsetToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new OffsetTool(this,inputAssignment);
	}

void OffsetToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveOffsetToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("TransformTool");
	}

extern "C" ToolFactory* createOffsetToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	OffsetToolFactory* offsetToolFactory=new OffsetToolFactory(*toolManager);
	
	/* Return factory object: */
	return offsetToolFactory;
	}

extern "C" void destroyOffsetToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/***********************************
Static elements of class OffsetTool:
***********************************/

OffsetToolFactory* OffsetTool::factory=0;

/***************************
Methods of class OffsetTool:
***************************/

OffsetTool::OffsetTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment)
	:TransformTool(factory,inputAssignment)
	{
	/* Set the transformation source device: */
	if(input.getNumButtonSlots()>0)
		sourceDevice=getButtonDevice(0);
	else
		sourceDevice=getValuatorDevice(0);
	}

OffsetTool::~OffsetTool(void)
	{
	}

const ToolFactory* OffsetTool::getFactory(void) const
	{
	return factory;
	}

void OffsetTool::frame(void)
	{
	/* Calculate the transformed device's transformation: */
	TrackerState offsetT=sourceDevice->getTransformation();
	offsetT*=factory->offset;
	transformedDevice->setTransformation(offsetT);
	}

}
