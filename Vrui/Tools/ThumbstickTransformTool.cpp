/***********************************************************************
ThumbstickTransformTool - Class to convert a two-axis analog thumb stick
into a 6-DOF input device moving inside a rectangle.
Copyright (c) 2014 Oliver Kreylos

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

#include <Vrui/Tools/ThumbstickTransformTool.h>

#include <Misc/StandardValueCoders.h>
#include <Misc/ArrayValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Geometry/Vector.h>
#include <Geometry/GeometryValueCoders.h>
#include <GL/GLValueCoders.h>
#include <Vrui/Vrui.h>
#include <Vrui/GlyphRenderer.h>
#include <Vrui/InputGraphManager.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/**************************************************************
Methods of class ThumbstickTransformToolFactory::Configuration:
**************************************************************/

ThumbstickTransformToolFactory::Configuration::Configuration(void)
	:translateFactors(getDisplaySize()*Scalar(0.5)),
	 deviceGlyphType("Cone"),
	 deviceGlyphMaterial(GLMaterial::Color(0.5f,0.5f,0.5f),GLMaterial::Color(1.0f,1.0f,1.0f),25.0f)
	{
	/* Initialize the base position and orientation: */
	baseTransform=TrackerState::translateFromOriginTo(getDisplayCenter());
	Vector x=getForwardDirection()^getUpDirection();
	baseTransform*=TrackerState::rotate(Rotation::fromBaseVectors(x,getUpDirection()));
	}

void ThumbstickTransformToolFactory::Configuration::load(const Misc::ConfigurationFileSection& cfs)
	{
	/* Get parameters: */
	baseTransform=cfs.retrieveValue<TrackerState>("./baseTransform",baseTransform);
	translateFactors=cfs.retrieveValue<Misc::FixedArray<Scalar,2> >("./translateFactors",translateFactors);
	deviceGlyphType=cfs.retrieveValue<std::string>("./deviceGlyphType",deviceGlyphType);
	deviceGlyphMaterial=cfs.retrieveValue<GLMaterial>("./deviceGlyphMaterial",deviceGlyphMaterial);
	}

void ThumbstickTransformToolFactory::Configuration::save(Misc::ConfigurationFileSection& cfs) const
	{
	/* Save parameters: */
	cfs.storeValue<TrackerState>("./baseTransform",baseTransform);
	cfs.storeValue<Misc::FixedArray<Scalar,2> >("./translateFactors",translateFactors);
	cfs.storeValue<std::string>("./deviceGlyphType",deviceGlyphType);
	cfs.storeValue<GLMaterial>("./deviceGlyphMaterial",deviceGlyphMaterial);
	}

/***********************************************
Methods of class ThumbstickTransformToolFactory:
***********************************************/

ThumbstickTransformToolFactory::ThumbstickTransformToolFactory(ToolManager& toolManager)
	:ToolFactory("ThumbstickTransformTool",toolManager)
	{
	/* Initialize tool layout: */
	layout.setNumButtons(0,true);
	layout.setNumValuators(2,true);
	
	/* Insert class into class hierarchy: */
	ToolFactory* transformToolFactory=toolManager.loadClass("TransformTool");
	transformToolFactory->addChildClass(this);
	addParentClass(transformToolFactory);
	
	/* Load class settings: */
	config.load(toolManager.getToolClassSection(getClassName()));
	
	/* Set tool class' factory pointer: */
	ThumbstickTransformTool::factory=this;
	}

ThumbstickTransformToolFactory::~ThumbstickTransformToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	ThumbstickTransformTool::factory=0;
	}

const char* ThumbstickTransformToolFactory::getName(void) const
	{
	return "Thumbstick Device";
	}

const char* ThumbstickTransformToolFactory::getButtonFunction(int) const
	{
	return "Forwarded Button";
	}

const char* ThumbstickTransformToolFactory::getValuatorFunction(int valuatorSlotIndex) const
	{
	switch(valuatorSlotIndex)
		{
		case 0:
			return "Translate X";
		
		case 1:
			return "Translate Y";
		
		default:
			return "Forwarded Valuator";
		}
	}

Tool* ThumbstickTransformToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new ThumbstickTransformTool(this,inputAssignment);
	}

void ThumbstickTransformToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveThumbstickTransformToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("TransformTool");
	}

extern "C" ToolFactory* createThumbstickTransformToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	ThumbstickTransformToolFactory* thumbstickTransformToolFactory=new ThumbstickTransformToolFactory(*toolManager);
	
	/* Return factory object: */
	return thumbstickTransformToolFactory;
	}

extern "C" void destroyThumbstickTransformToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/************************************************
Static elements of class ThumbstickTransformTool:
************************************************/

ThumbstickTransformToolFactory* ThumbstickTransformTool::factory=0;

/****************************************
Methods of class ThumbstickTransformTool:
****************************************/

ThumbstickTransformTool::ThumbstickTransformTool(const ToolFactory* sFactory,const ToolInputAssignment& inputAssignment)
	:TransformTool(sFactory,inputAssignment),
	 config(factory->config)
	{
	}

void ThumbstickTransformTool::configure(const Misc::ConfigurationFileSection& configFileSection)
	{
	/* Update the configuration: */
	config.load(configFileSection);
	}

void ThumbstickTransformTool::storeState(Misc::ConfigurationFileSection& configFileSection) const
	{
	/* Save the current configuration: */
	config.save(configFileSection);
	}

void ThumbstickTransformTool::initialize(void)
	{
	/* Let the base class do its thing: */
	TransformTool::initialize();
	
	/* Set the virtual input device's glyph: */
	getInputGraphManager()->getInputDeviceGlyph(transformedDevice).setGlyphType(config.deviceGlyphType.c_str());
	getInputGraphManager()->getInputDeviceGlyph(transformedDevice).setGlyphMaterial(config.deviceGlyphMaterial);
	
	/* Initialize the virtual input device's position: */
	transformedDevice->setDeviceRay(Vector(0,1,0),-getInchFactor());
	transformedDevice->setTransformation(config.baseTransform);
	}

const ToolFactory* ThumbstickTransformTool::getFactory(void) const
	{
	return factory;
	}

void ThumbstickTransformTool::frame(void)
	{
	/* Update the transformed device's position: */
	TrackerState currentTransform=config.baseTransform;
	currentTransform*=TrackerState::translate(Vector(Scalar(getValuatorState(0))*config.translateFactors[0],Scalar(getValuatorState(1))*config.translateFactors[1],Scalar(0)));
	transformedDevice->setTransformation(currentTransform);
	}

}
