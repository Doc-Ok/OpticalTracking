/***********************************************************************
ButtonInputDeviceTool - Class for tools using buttons (such as keyboard
keys) to interact with virtual input devices.
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

#include <Vrui/Tools/ButtonInputDeviceTool.h>

#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Math/Math.h>
#include <Geometry/Point.h>
#include <Vrui/Vrui.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/*********************************************
Methods of class ButtonInputDeviceToolFactory:
*********************************************/

ButtonInputDeviceToolFactory::ButtonInputDeviceToolFactory(ToolManager& toolManager)
	:ToolFactory("ButtonInputDeviceTool",toolManager)
	{
	/* Initialize tool layout: */
	layout.setNumButtons(8,true);
	layout.setNumValuators(0,true);
	
	/* Load class settings: */
	Misc::ConfigurationFileSection cfs=toolManager.getToolClassSection(getClassName());
	
	/* Insert class into class hierarchy: */
	ToolFactory* inputDeviceToolFactory=toolManager.loadClass("InputDeviceTool");
	inputDeviceToolFactory->addChildClass(this);
	addParentClass(inputDeviceToolFactory);
	
	/* Initialize translation and rotation vectors: */
	Scalar translateFactor=cfs.retrieveValue<Scalar>("./translateFactor",Scalar(4)*getInchFactor());
	Scalar rotateFactor=Math::rad(cfs.retrieveValue<Scalar>("./rotateFactor",Scalar(60)));
	for(int i=0;i<6;++i)
		translations[i]=Vector::zero;
	for(int i=0;i<3;++i)
		{
		translations[2*i+0][i]=-translateFactor;
		translations[2*i+1][i]=translateFactor;
		}
	rotations[0]=Vector(0,0,-rotateFactor);
	rotations[1]=Vector(0,0,rotateFactor);
	rotations[2]=Vector(0,-rotateFactor,0);
	rotations[3]=Vector(0,rotateFactor,0);
	rotations[4]=Vector(rotateFactor,0,0);
	rotations[5]=Vector(-rotateFactor,0,0);
	
	/* Set tool class' factory pointer: */
	ButtonInputDeviceTool::factory=this;
	}

ButtonInputDeviceToolFactory::~ButtonInputDeviceToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	ButtonInputDeviceTool::factory=0;
	}

const char* ButtonInputDeviceToolFactory::getName(void) const
	{
	return "Button-Based Driver";
	}

const char* ButtonInputDeviceToolFactory::getButtonFunction(int buttonSlotIndex) const
	{
	switch(buttonSlotIndex)
		{
		case 0:
			return "Switch Device";
		
		case 1:
			return "Switch Translation / Rotation";
		
		case 2:
			return "Translate -X / Rotate -Z";
		
		case 3:
			return "Translate +X / Rotate +Z";
		
		case 4:
			return "Translate -Y / Rotate -Y";
		
		case 5:
			return "Translate +Y / Rotate +Y";
		
		case 6:
			return "Translate -Z / Rotate -X";
		
		case 7:
			return "Translate +Z / Rotate +X";
		
		default:
			return "Forwarded Button";
		}
	}

Tool* ButtonInputDeviceToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new ButtonInputDeviceTool(this,inputAssignment);
	}

void ButtonInputDeviceToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveButtonInputDeviceToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("InputDeviceTool");
	}

extern "C" ToolFactory* createButtonInputDeviceToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	ButtonInputDeviceToolFactory* buttonInputDeviceToolFactory=new ButtonInputDeviceToolFactory(*toolManager);
	
	/* Return factory object: */
	return buttonInputDeviceToolFactory;
	}

extern "C" void destroyButtonInputDeviceToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/**********************************************
Static elements of class ButtonInputDeviceTool:
**********************************************/

ButtonInputDeviceToolFactory* ButtonInputDeviceTool::factory=0;

/**************************************
Methods of class ButtonInputDeviceTool:
**************************************/

ButtonInputDeviceTool::ButtonInputDeviceTool(const ToolFactory* sFactory,const ToolInputAssignment& inputAssignment)
	:InputDeviceTool(sFactory,inputAssignment),
	 transformationMode(TRANSLATING),
	 numPressedNavButtons(0),
	 lastFrameTime(getApplicationTime())
	{
	for(int i=0;i<6;++i)
		navButtonStates[i]=false;
	}

const ToolFactory* ButtonInputDeviceTool::getFactory(void) const
	{
	return factory;
	}

void ButtonInputDeviceTool::buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData)
	{
	if(cbData->newButtonState) // Button has just been pressed
		{
		switch(buttonSlotIndex)
			{
			case 0: // Device switch button
				grabNextDevice();
				break;
			
			case 1: // Transformation mode toggle
				if(transformationMode==TRANSLATING)
					transformationMode=ROTATING;
				else
					transformationMode=TRANSLATING;
				break;
			
			case 2: // Navigation buttons
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
				if(!navButtonStates[buttonSlotIndex-2])
					{
					navButtonStates[buttonSlotIndex-2]=true;
					if(numPressedNavButtons==0)
						lastFrameTime=getApplicationTime();
					++numPressedNavButtons;
					}
				break;
			
			default: // Forwarded buttons
				/* Let input device tool handle it: */
				InputDeviceTool::buttonCallback(buttonSlotIndex,cbData);
			}
		}
	else // Button has just been released
		{
		switch(buttonSlotIndex)
			{
			case 0: // Device switch button
			case 1: // Transformation mode toggle
				break;
			
			case 2: // Navigation buttons
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
				if(navButtonStates[buttonSlotIndex-2])
					{
					navButtonStates[buttonSlotIndex-2]=false;
					--numPressedNavButtons;
					}
				break;
			
			default: // Forwarded buttons
				/* Let input device tool handle it: */
				InputDeviceTool::buttonCallback(buttonSlotIndex,cbData);
			}
		}
	}

void ButtonInputDeviceTool::frame(void)
	{
	if(isActive()&&numPressedNavButtons>0)
		{
		/* Calculate the current frame time step: */
		double frameTime=getApplicationTime();
		double timeStep=frameTime-lastFrameTime;
		lastFrameTime=frameTime;
		
		/* Update the input device transformation: */
		TrackerState ts=getGrabbedDevice()->getTransformation();
		if(transformationMode==TRANSLATING)
			{
			for(int i=0;i<6;++i)
				if(navButtonStates[i])
					ts.leftMultiply(TrackerState::translate(factory->translations[i]*timeStep));
			}
		else
			{
			Point p=ts.getOrigin();
			ts.leftMultiply(TrackerState::translateToOriginFrom(p));
			for(int i=0;i<6;++i)
				if(navButtonStates[i])
					ts.leftMultiply(TrackerState::rotate(Rotation::rotateScaledAxis(factory->rotations[i]*timeStep)));
			ts.leftMultiply(TrackerState::translateFromOriginTo(p));
			}
		getGrabbedDevice()->setTransformation(ts);
		
		/* Request another frame: */
		scheduleUpdate(getApplicationTime()+1.0/125.0);
		}
	}

}
