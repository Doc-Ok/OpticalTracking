/***********************************************************************
WaldoTool - Class to scale translations and rotations on 6-DOF input
devices to improve interaction accuracy in tracked environments.
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

#include <Vrui/Tools/WaldoTool.h>

#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <GL/GLMaterial.h>
#include <Vrui/Vrui.h>
#include <Vrui/GlyphRenderer.h>
#include <Vrui/InputGraphManager.h>
#include <Vrui/InputDeviceManager.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/*********************************
Methods of class WaldoToolFactory:
*********************************/

WaldoToolFactory::WaldoToolFactory(ToolManager& toolManager)
	:ToolFactory("WaldoTool",toolManager),
	 linearScale(0.25),angularScale(0.25)
	{
	/* Insert class into class hierarchy: */
	TransformToolFactory* transformToolFactory=dynamic_cast<TransformToolFactory*>(toolManager.loadClass("TransformTool"));
	transformToolFactory->addChildClass(this);
	addParentClass(transformToolFactory);
	
	/* Load class settings: */
	Misc::ConfigurationFileSection cfs=toolManager.getToolClassSection(getClassName());
	linearScale=cfs.retrieveValue<Scalar>("./linearScale",linearScale);
	angularScale=cfs.retrieveValue<Scalar>("./angularScale",angularScale);
	
	/* Initialize tool layout: */
	layout.setNumButtons(0,true);
	layout.setNumValuators(0,true);
	
	/* Set tool class' factory pointer: */
	WaldoTool::factory=this;
	}

WaldoToolFactory::~WaldoToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	WaldoTool::factory=0;
	}

const char* WaldoToolFactory::getName(void) const
	{
	return "Waldo (Scaling) Transformation";
	}

Tool* WaldoToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new WaldoTool(this,inputAssignment);
	}

void WaldoToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveWaldoToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("TransformTool");
	}

extern "C" ToolFactory* createWaldoToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	WaldoToolFactory* waldoToolFactory=new WaldoToolFactory(*toolManager);
	
	/* Return factory object: */
	return waldoToolFactory;
	}

extern "C" void destroyWaldoToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/**********************************
Static elements of class WaldoTool:
**********************************/

WaldoToolFactory* WaldoTool::factory=0;

/**************************
Methods of class WaldoTool:
**************************/

WaldoTool::WaldoTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment)
	:TransformTool(factory,inputAssignment),
	 waldoGlyph(0),
	 numPressedButtons(0),transformActive(false)
	{
	/* Set the transformation source device: */
	if(input.getNumButtonSlots()>0)
		sourceDevice=getButtonDevice(0);
	else
		sourceDevice=getValuatorDevice(0);
	}

void WaldoTool::initialize(void)
	{
	/* Initialize the base tool: */
	TransformTool::initialize();
	
	/* Set the virtual input device's glyph to the same as the source device's: */
	waldoGlyph=&getInputGraphManager()->getInputDeviceGlyph(transformedDevice);
	*waldoGlyph=getInputGraphManager()->getInputDeviceGlyph(sourceDevice);
	GLMaterial waldoMaterial=waldoGlyph->getGlyphMaterial();
	waldoMaterial.ambient=waldoMaterial.diffuse=GLMaterial::Color(1.0f,0.0f,0.0f);
	waldoGlyph->setGlyphMaterial(waldoMaterial);
	waldoGlyph->disable();
	}

const ToolFactory* WaldoTool::getFactory(void) const
	{
	return factory;
	}

void WaldoTool::buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData)
	{
	if(cbData->newButtonState) // Button has just been pressed
		{
		if(numPressedButtons==0)
			{
			/* Activate the waldo transformation: */
			transformActive=true;
			
			/* Remember the current input device transformation: */
			last=sourceDevice->getTransformation();
			
			/* Activate the virtual input device's glyph: */
			waldoGlyph->enable();
			}
		++numPressedButtons;
		}
	else // Button has just been released
		--numPressedButtons;
	
	/* Forward the button event: */
	transformedDevice->setButtonState(buttonSlotIndex,cbData->newButtonState);
	}

void WaldoTool::frame(void)
	{
	/* Act depending on the waldo activation state: */
	if(transformActive)
		{
		/* Calculate the input device transformation update: */
		const TrackerState& current=sourceDevice->getTransformation();
		Vector translation=current.getTranslation()-last.getTranslation();
		Vector rotation=(current.getRotation()*Geometry::invert(last.getRotation())).getScaledAxis();
		last=current;
		
		/* Scale linear and angular motion: */
		translation*=factory->linearScale;
		rotation*=factory->angularScale;
		
		/* Set the virtual input device to the result transformation: */
		const TrackerState& waldoTransform=transformedDevice->getTransformation();
		TrackerState::Rotation waldoRotation=waldoTransform.getRotation();
		waldoRotation.leftMultiply(TrackerState::Rotation(rotation));
		waldoRotation.renormalize();
		Vector waldoTranslation=waldoTransform.getTranslation();
		waldoTranslation+=translation;
		transformedDevice->setTransformation(TrackerState(waldoTranslation,waldoRotation));
		
		if(numPressedButtons==0)
			{
			/* Deactivate the waldo transformation: */
			transformActive=false;
			
			/* Deactivate the virtual input device's glyph: */
			waldoGlyph->disable();
			}
		}
	else
		{
		/* Snap the virtual input device to the source input device: */
		resetDevice();
		}
	}

}
