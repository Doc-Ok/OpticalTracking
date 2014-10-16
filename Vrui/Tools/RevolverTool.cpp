/***********************************************************************
RevolverTool - Class to control multiple buttons (and tools) from a
single button using a revolver metaphor. Generalized from the rotator
tool initially developed by Braden Pellett and Jordan van Aalsburg.
Copyright (c) 2008-2013 Oliver Kreylos

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

#include <Vrui/Tools/RevolverTool.h>

#include <Misc/ThrowStdErr.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Math/Math.h>
#include <Math/Constants.h>
#include <GL/gl.h>
#include <GL/GLMatrixTemplates.h>
#include <GL/GLTransformationWrappers.h>
#include <Vrui/Vrui.h>
#include <Vrui/InputDeviceManager.h>
#include <Vrui/GlyphRenderer.h>
#include <Vrui/InputGraphManager.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/************************************
Methods of class RevolverToolFactory:
************************************/

RevolverToolFactory::RevolverToolFactory(ToolManager& toolManager)
	:ToolFactory("RevolverTool",toolManager),
	 numChambers(6)
	{
	/* Insert class into class hierarchy: */
	TransformToolFactory* transformToolFactory=dynamic_cast<TransformToolFactory*>(toolManager.loadClass("TransformTool"));
	transformToolFactory->addChildClass(this);
	addParentClass(transformToolFactory);
	
	/* Load class settings: */
	Misc::ConfigurationFileSection cfs=toolManager.getToolClassSection(getClassName());
	numChambers=cfs.retrieveValue<int>("./numChambers",numChambers);
	
	/* Initialize tool layout: */
	layout.setNumButtons(1,true);
	layout.setNumValuators(0,true);
	
	/* Set tool class' factory pointer: */
	RevolverTool::factory=this;
	}

RevolverToolFactory::~RevolverToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	RevolverTool::factory=0;
	}

const char* RevolverToolFactory::getName(void) const
	{
	return "Revolver Multi-Button";
	}

const char* RevolverToolFactory::getButtonFunction(int buttonSlotIndex) const
	{
	if(buttonSlotIndex==0)
		return "Cycle Chamber";
	else
		return ToolFactory::getButtonFunction(buttonSlotIndex-1);
	}

Tool* RevolverToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new RevolverTool(this,inputAssignment);
	}

void RevolverToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveRevolverToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("TransformTool");
	}

extern "C" ToolFactory* createRevolverToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	RevolverToolFactory* revolverToolFactory=new RevolverToolFactory(*toolManager);
	
	/* Return factory object: */
	return revolverToolFactory;
	}

extern "C" void destroyRevolverToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/*************************************
Static elements of class RevolverTool:
*************************************/

RevolverToolFactory* RevolverTool::factory=0;

/*****************************
Methods of class RevolverTool:
*****************************/

RevolverTool::RevolverTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment)
	:TransformTool(factory,inputAssignment),
	 numberRenderer(float(getUiSize())*1.5f,true),
	 currentChamber(0),
	 showNumbersTime(0.0)
	{
	/* Set the transformation source device: */
	if(input.getNumButtonSlots()>1)
		sourceDevice=getButtonDevice(1);
	else if(input.getNumValuatorSlots()>0)
		sourceDevice=getValuatorDevice(0);
	else
		sourceDevice=getButtonDevice(0); // User didn't select anything to forward; let's just pretend it makes sense
	}

RevolverTool::~RevolverTool(void)
	{
	}

void RevolverTool::configure(const Misc::ConfigurationFileSection& configFileSection)
	{
	/* Read the current chamber index: */
	currentChamber=configFileSection.retrieveValue<int>("./currentChamber",currentChamber);
	}

void RevolverTool::storeState(Misc::ConfigurationFileSection& configFileSection) const
	{
	/* Write the current chamber index: */
	configFileSection.storeValue<int>("./currentChamber",currentChamber);
	}

void RevolverTool::initialize(void)
	{
	/* Create a virtual input device to shadow the source input device: */
	transformedDevice=addVirtualInputDevice("RevolverToolTransformedDevice",factory->numChambers*(input.getNumButtonSlots()-1),factory->numChambers*input.getNumValuatorSlots());
	
	/* Copy the source device's tracking type: */
	transformedDevice->setTrackType(sourceDevice->getTrackType());
	
	/* Disable the virtual input device's glyph: */
	getInputGraphManager()->getInputDeviceGlyph(transformedDevice).disable();
	
	/* Permanently grab the virtual input device: */
	getInputGraphManager()->grabInputDevice(transformedDevice,this);
	
	/* Initialize the virtual input device's position: */
	resetDevice();
	}

const ToolFactory* RevolverTool::getFactory(void) const
	{
	return factory;
	}

void RevolverTool::buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData)
	{
	if(buttonSlotIndex==0)
		{
		if(cbData->newButtonState)
			{
			/* Change the currently mapped button set: */
			currentChamber=(currentChamber+1)%factory->numChambers;
			
			/* Set the newly mapped chamber's state to the input device's buttons' and valuators' states: */
			for(int i=1;i<input.getNumButtonSlots();++i)
				transformedDevice->setButtonState((i-1)*factory->numChambers+currentChamber,getButtonState(i));
			for(int i=0;i<input.getNumValuatorSlots();++i)
				transformedDevice->setValuator(i*factory->numChambers+currentChamber,getValuatorState(i));
			
			/* Show the current button assignment for one second: */
			showNumbersTime=getApplicationTime()+1.0;
			}
		}
	else
		{
		/* Pass the button event through to the virtual input device: */
		transformedDevice->setButtonState((buttonSlotIndex-1)*factory->numChambers+currentChamber,cbData->newButtonState);
		}
	}

void RevolverTool::valuatorCallback(int valuatorSlotIndex,InputDevice::ValuatorCallbackData* cbData)
	{
	/* Pass the valuator event through to the virtual input device: */
	transformedDevice->setValuator(valuatorSlotIndex*factory->numChambers+currentChamber,cbData->newValuatorValue);
	}

void RevolverTool::frame(void)
	{
	/* Call the base class method: */
	TransformTool::frame();
	
	/* Request a rendering update while the animation is going: */
	if(getApplicationTime()<showNumbersTime)
		{
		/* Request another frame: */
		scheduleUpdate(getApplicationTime()+1.0/125.0);
		}
	}

void RevolverTool::display(GLContextData& contextData) const
	{
	if(getApplicationTime()<showNumbersTime)
		{
		/* Set up OpenGL state: */
		glPushAttrib(GL_ENABLE_BIT|GL_LINE_BIT);
		glDisable(GL_LIGHTING);
		glLineWidth(1.0f);
		glColor3f(0.0f,1.0f,0.0f);
		glPushMatrix();
		
		/* Draw the "revolver chambers:" */
		glMultMatrix(calcHUDTransform(sourceDevice->getPosition()));
		
		Scalar chamberAngle=Scalar(2)*Math::Constants<Scalar>::pi/Scalar(factory->numChambers);
		Scalar angleOffset=Scalar(0);
		double animTime=(getApplicationTime()-(showNumbersTime-1.0))*2.0;
		if(animTime<1.0)
			angleOffset=chamberAngle*Scalar(1.0-animTime);
		
		for(int i=0;i<factory->numChambers;++i)
			{
			Scalar angle=chamberAngle*Scalar(i)+angleOffset;
			GLNumberRenderer::Vector pos;
			pos[0]=Math::sin(angle)*Scalar(getUiSize()*4.0f);
			pos[1]=Math::cos(angle)*Scalar(getUiSize()*4.0f);
			pos[2]=0.0f;
			numberRenderer.drawNumber(pos,(currentChamber+i)%factory->numChambers+1,contextData,0,0);
			}
		
		glPopMatrix();
		glPopAttrib();
		}
	}

InputDeviceFeatureSet RevolverTool::getSourceFeatures(const InputDeviceFeature& forwardedFeature)
	{
	/* Paranoia: Check if the forwarded feature is on the transformed device: */
	if(forwardedFeature.getDevice()!=transformedDevice)
		Misc::throwStdErr("RevolverTool::getSourceFeatures: Forwarded feature is not on transformed device");
	
	/* Create an empty feature set: */
	InputDeviceFeatureSet result;
	
	if(forwardedFeature.isButton())
		{
		/* Find the source button slot index: */
		int buttonSlotIndex=forwardedFeature.getIndex()/factory->numChambers+1;
		
		/* Add the button slot's feature to the result set: */
		result.push_back(input.getButtonSlotFeature(buttonSlotIndex));
		}
	
	if(forwardedFeature.isValuator())
		{
		/* Find the source valuator slot index: */
		int valuatorSlotIndex=forwardedFeature.getIndex()/factory->numChambers;
		
		/* Add the valuator slot's feature to the result set: */
		result.push_back(input.getValuatorSlotFeature(valuatorSlotIndex));
		}
	
	return result;
	}

InputDeviceFeatureSet RevolverTool::getForwardedFeatures(const InputDeviceFeature& sourceFeature)
	{
	/* Find the input assignment slot for the given feature: */
	int slotIndex=input.findFeature(sourceFeature);
	
	/* Check if the source feature belongs to this tool: */
	if(slotIndex<0)
		Misc::throwStdErr("RevolverTool::getForwardedFeatures: Source feature is not part of tool's input assignment");
	
	/* Create an empty feature set: */
	InputDeviceFeatureSet result;
	
	/* Check if the feature is a button or valuator: */
	if(sourceFeature.isButton())
		{
		/* Get the slot's button slot index: */
		int buttonSlotIndex=input.getButtonSlotIndex(slotIndex);
		
		/* Check if the button is part of the forwarded subset: */
		if(buttonSlotIndex>=1)
			{
			/* Add the forwarded feature for the current chamber to the result set: */
			int baseButtonIndex=(buttonSlotIndex-1)*factory->numChambers;
			result.push_back(InputDeviceFeature(transformedDevice,InputDevice::BUTTON,baseButtonIndex+currentChamber));
			}
		}
	
	if(sourceFeature.isValuator())
		{
		/* Get the slot's valuator slot index: */
		int valuatorSlotIndex=input.getValuatorSlotIndex(slotIndex);
		
		/* Add the forwarded feature for the current chamber to the result set: */
		int baseValuatorIndex=valuatorSlotIndex*factory->numChambers;
		result.push_back(InputDeviceFeature(transformedDevice,InputDevice::VALUATOR,baseValuatorIndex+currentChamber));
		}
	
	return result;
	}

}
