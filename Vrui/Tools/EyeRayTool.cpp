/***********************************************************************
EyeRayTool - Class to transform the ray direction of an input device to
point along the sight line from the main viewer to the input device.
Copyright (c) 2008-2010 Oliver Kreylos

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

#include <Vrui/Tools/EyeRayTool.h>

#include <Misc/ThrowStdErr.h>
#include <Vrui/Vrui.h>
#include <Vrui/GlyphRenderer.h>
#include <Vrui/InputGraphManager.h>
#include <Vrui/Viewer.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/**********************************
Methods of class EyeRayToolFactory:
**********************************/

EyeRayToolFactory::EyeRayToolFactory(ToolManager& toolManager)
	:ToolFactory("EyeRayTool",toolManager)
	{
	/* Initialize tool layout: */
	layout.setNumButtons(0,true);
	layout.setNumValuators(0,true);
	
	/* Insert class into class hierarchy: */
	TransformToolFactory* transformToolFactory=dynamic_cast<TransformToolFactory*>(toolManager.loadClass("TransformTool"));
	transformToolFactory->addChildClass(this);
	addParentClass(transformToolFactory);
	
	/* Set tool class' factory pointer: */
	EyeRayTool::factory=this;
	}

EyeRayToolFactory::~EyeRayToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	EyeRayTool::factory=0;
	}

const char* EyeRayToolFactory::getName(void) const
	{
	return "View-Aligned Ray";
	}

Tool* EyeRayToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new EyeRayTool(this,inputAssignment);
	}

void EyeRayToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveEyeRayToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("TransformTool");
	}

extern "C" ToolFactory* createEyeRayToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	EyeRayToolFactory* eyeRayToolFactory=new EyeRayToolFactory(*toolManager);
	
	/* Return factory object: */
	return eyeRayToolFactory;
	}

extern "C" void destroyEyeRayToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/***********************************
Static elements of class EyeRayTool:
***********************************/

EyeRayToolFactory* EyeRayTool::factory=0;

/***************************
Methods of class EyeRayTool:
***************************/

EyeRayTool::EyeRayTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment)
	:TransformTool(factory,inputAssignment)
	{
	/* Set the transformation source device: */
	if(input.getNumButtonSlots()>0)
		sourceDevice=getButtonDevice(0);
	else
		sourceDevice=getValuatorDevice(0);
	}

EyeRayTool::~EyeRayTool(void)
	{
	}

void EyeRayTool::initialize(void)
	{
	/* Initialize the base tool: */
	TransformTool::initialize();
	
	/* Copy the source device's tracking type: */
	transformedDevice->setTrackType(sourceDevice->getTrackType());
	
	/* Disable the transformed device's glyph: */
	getInputGraphManager()->getInputDeviceGlyph(transformedDevice).disable();
	}

const ToolFactory* EyeRayTool::getFactory(void) const
	{
	return factory;
	}

void EyeRayTool::frame(void)
	{
	/* Calculate a sight line from the main viewer to the device's position in physical space: */
	Vector physRayDir=sourceDevice->getPosition()-getMainViewer()->getHeadPosition();
	
	/* Transform the ray direction to device space: */
	Vector deviceRayDir=sourceDevice->getTransformation().inverseTransform(physRayDir);
	deviceRayDir.normalize();
	transformedDevice->setDeviceRay(deviceRayDir,-getInchFactor());
	
	/* Copy the source device's position and orientation: */
	transformedDevice->setTransformation(sourceDevice->getTransformation());
	}

}
