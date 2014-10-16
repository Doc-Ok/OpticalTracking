/***********************************************************************
WandNavigationTool - Class encapsulating the navigation behaviour of a
classical CAVE wand.
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

#include <Vrui/Tools/WandNavigationTool.h>

#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Math/Math.h>
#include <Vrui/Vrui.h>
#include <Vrui/InputDeviceManager.h>
#include <Vrui/InputGraphManager.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/******************************************
Methods of class WandNavigationToolFactory:
******************************************/

WandNavigationToolFactory::WandNavigationToolFactory(ToolManager& toolManager)
	:ToolFactory("WandNavigationTool",toolManager),
	 scaleFactor(getInchFactor()*Scalar(-8))
	{
	/* Initialize tool layout: */
	layout.setNumButtons(2);
	
	/* Insert class into class hierarchy: */
	ToolFactory* navigationToolFactory=toolManager.loadClass("NavigationTool");
	navigationToolFactory->addChildClass(this);
	addParentClass(navigationToolFactory);
	
	/* Load class settings: */
	Misc::ConfigurationFileSection cfs=toolManager.getToolClassSection(getClassName());
	scaleFactor=cfs.retrieveValue<Scalar>("./scaleFactor",scaleFactor);
	
	/* Set tool class' factory pointer: */
	WandNavigationTool::factory=this;
	}

WandNavigationToolFactory::~WandNavigationToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	WandNavigationTool::factory=0;
	}

const char* WandNavigationToolFactory::getName(void) const
	{
	return "Wand (6-DOF + Scaling)";
	}

const char* WandNavigationToolFactory::getButtonFunction(int buttonSlotIndex) const
	{
	switch(buttonSlotIndex)
		{
		case 0:
			return "Grab Space";
		
		case 1:
			return "Zoom / Forwarded Button";
		}
	
	/* Never reached; just to make compiler happy: */
	return 0;
	}

Tool* WandNavigationToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new WandNavigationTool(this,inputAssignment);
	}

void WandNavigationToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveWandNavigationToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("NavigationTool");
	}

extern "C" ToolFactory* createWandNavigationToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	WandNavigationToolFactory* wandNavigationToolFactory=new WandNavigationToolFactory(*toolManager);
	
	/* Return factory object: */
	return wandNavigationToolFactory;
	}

extern "C" void destroyWandNavigationToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/*******************************************
Static elements of class WandNavigationTool:
*******************************************/

WandNavigationToolFactory* WandNavigationTool::factory=0;

/***********************************
Methods of class WandNavigationTool:
***********************************/

WandNavigationTool::WandNavigationTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment)
	:NavigationTool(factory,inputAssignment),
	 buttonDevice(0),
	 navigationMode(IDLE)
	{
	}

void WandNavigationTool::initialize(void)
	{
	/* Get the source input device: */
	InputDevice* device=getButtonDevice(1);
	
	/* Create a virtual input device to shadow the zoom button: */
	buttonDevice=addVirtualInputDevice("WandNavigationToolButtonDevice",1,0);
	
	/* Copy the source device's tracking type: */
	buttonDevice->setTrackType(device->getTrackType());
	
	/* Disable the virtual device's glyph: */
	getInputGraphManager()->getInputDeviceGlyph(buttonDevice).disable();
	
	/* Permanently grab the virtual input device: */
	getInputGraphManager()->grabInputDevice(buttonDevice,this);
	
	/* Initialize the virtual input device's position: */
	buttonDevice->setDeviceRay(device->getDeviceRayDirection(),device->getDeviceRayStart());
	buttonDevice->setTransformation(device->getTransformation());
	}

void WandNavigationTool::deinitialize(void)
	{
	/* Release the virtual input device: */
	getInputGraphManager()->releaseInputDevice(buttonDevice,this);
	
	/* Destroy the virtual input device: */
	getInputDeviceManager()->destroyInputDevice(buttonDevice);
	buttonDevice=0;
	}

const ToolFactory* WandNavigationTool::getFactory(void) const
	{
	return factory;
	}

void WandNavigationTool::buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData)
	{
	/* Process based on which button was pressed: */
	switch(buttonSlotIndex)
		{
		case 0:
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
							preScale=Geometry::invert(getButtonDeviceTransformation(0));
							preScale*=getNavigationTransformation();
							
							/* Go from IDLE to MOVING mode: */
							navigationMode=MOVING;
							}
						break;
					
					case PASSTHROUGH:
						/* Remember that the main button is pressed: */
						navigationMode=PASSTHROUGH_MOVING;
						break;
					
					case SCALING_PAUSED:
						/* Determine the scaling center and direction: */
						scalingCenter=getButtonDevicePosition(0);
						scalingDirection=getButtonDeviceRayDirection(0);
						initialScale=scalingCenter*scalingDirection;
						
						/* Initialize the transformation parts: */
						preScale=NavTrackerState::translateFromOriginTo(scalingCenter);
						postScale=NavTrackerState::translateToOriginFrom(scalingCenter);
						postScale*=getNavigationTransformation();
						
						/* Go from SCALING_PAUSED to SCALING mode: */
						navigationMode=SCALING;
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
					case PASSTHROUGH_MOVING:
						/* Remember that the main button is released: */
						navigationMode=PASSTHROUGH;
						break;
					
					case SCALING:
						/* Pause scaling until button is pressed again: */
						
						/* Go from SCALING to SCALING_PAUSED mode: */
						navigationMode=SCALING_PAUSED;
						break;
					
					case MOVING:
						/* Deactivate this tool: */
						deactivate();
						
						/* Go from MOVING to IDLE mode: */
						navigationMode=IDLE;
						break;
					
					default:
						/* This shouldn't happen; just ignore the event */
						break;
					}
				}
			break;
		
		case 1:
			if(cbData->newButtonState) // Button has just been pressed
				{
				/* Act depending on this tool's current state: */
				switch(navigationMode)
					{
					case IDLE:
						/* Pass the button event to the virtual input device: */
						buttonDevice->setButtonState(0,true);
						
						/* Go to pass-through mode: */
						navigationMode=PASSTHROUGH;
						break;
					
					case MOVING:
						/* Determine the scaling center and direction: */
						scalingCenter=getButtonDevicePosition(0);
						scalingDirection=getButtonDeviceRayDirection(0);
						initialScale=scalingCenter*scalingDirection;
						
						/* Initialize the transformation parts: */
						preScale=NavTrackerState::translateFromOriginTo(scalingCenter);
						postScale=NavTrackerState::translateToOriginFrom(scalingCenter);
						postScale*=getNavigationTransformation();
						
						/* Go from MOVING to SCALING mode: */
						navigationMode=SCALING;
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
					case PASSTHROUGH:
						/* Pass the button event to the virtual input device: */
						buttonDevice->setButtonState(0,false);
						
						/* Go to idle mode: */
						navigationMode=IDLE;
						break;
					
					case PASSTHROUGH_MOVING:
						/* Pass the button event to the virtual input device: */
						buttonDevice->setButtonState(0,false);
						
						/* Try activating this tool: */
						if(activate())
							{
							/* Initialize the navigation transformations: */
							preScale=Geometry::invert(getButtonDeviceTransformation(0));
							preScale*=getNavigationTransformation();
							
							/* Go to MOVING mode: */
							navigationMode=MOVING;
							}
						else
							{
							/* Go to idle mode: */
							navigationMode=IDLE;
							}
						break;
					
					case SCALING:
						/* Initialize the transformation parts: */
						preScale=Geometry::invert(getButtonDeviceTransformation(0));
						preScale*=getNavigationTransformation();
						
						/* Go from SCALING to MOVING mode: */
						navigationMode=MOVING;
						break;
					
					case SCALING_PAUSED:
						/* Deactivate this tool: */
						deactivate();
						
						/* Go from SCALING_PAUSED to IDLE mode: */
						navigationMode=IDLE;
						break;
					
					default:
						/* This shouldn't happen; just ignore the event */
						break;
					}
				}
			break;
		}
	}

void WandNavigationTool::frame(void)
	{
	/* Act depending on this tool's current state: */
	switch(navigationMode)
		{
		case MOVING:
			{
			/* Compose the new navigation transformation: */
			NavTrackerState navigation=getButtonDeviceTransformation(0);
			navigation*=preScale;
			
			/* Update Vrui's navigation transformation: */
			setNavigationTransformation(navigation);
			break;
			}
		
		case SCALING:
			{
			/* Compose the new navigation transformation: */
			NavTrackerState navigation=preScale;
			Scalar currentScale=getButtonDevicePosition(0)*scalingDirection-initialScale;
			navigation*=NavTrackerState::scale(Math::exp(currentScale/factory->scaleFactor));
			navigation*=postScale;
			
			/* Update Vrui's navigation transformation: */
			setNavigationTransformation(navigation);
			break;
			}
		
		default:
			/* Do nothing */
			break;
		}
	
	/* Update the virtual input device: */
	InputDevice* device=getButtonDevice(1);
	buttonDevice->setDeviceRay(device->getDeviceRayDirection(),device->getDeviceRayStart());
	buttonDevice->setTransformation(device->getTransformation());
	}

std::vector<InputDevice*> WandNavigationTool::getForwardedDevices(void)
	{
	std::vector<InputDevice*> result;
	result.push_back(buttonDevice);
	return result;
	}

InputDeviceFeatureSet WandNavigationTool::getSourceFeatures(const InputDeviceFeature& forwardedFeature)
	{
	/* Paranoia: Check if the forwarded feature is on the transformed device: */
	if(forwardedFeature.getDevice()!=buttonDevice)
		Misc::throwStdErr("WandNavigationTool::getSourceFeatures: Forwarded feature is not on transformed device");
	
	/* Return the source feature: */
	InputDeviceFeatureSet result;
	result.push_back(input.getButtonSlotFeature(1));
	return result;
	}

InputDevice* WandNavigationTool::getSourceDevice(const InputDevice* forwardedDevice)
	{
	/* Paranoia: Check if the forwarded device is the same as the transformed device: */
	if(forwardedDevice!=buttonDevice)
		Misc::throwStdErr("WandNavigationTool::getSourceDevice: Given forwarded device is not transformed device");
	
	/* Return the source device: */
	return getButtonDevice(1);
	}

InputDeviceFeatureSet WandNavigationTool::getForwardedFeatures(const InputDeviceFeature& sourceFeature)
	{
	/* Get the source feature's assignment slot index: */
	int slotIndex=input.findFeature(sourceFeature);
	
	/* Paranoia: Check if the source feature belongs to this tool: */
	if(slotIndex<0)
		Misc::throwStdErr("WandNavigationTool::getForwardedFeatures: Source feature is not part of tool's input assignment");
	
	/* Return the forwarded feature: */
	InputDeviceFeatureSet result;
	if(slotIndex==1)
		result.push_back(InputDeviceFeature(buttonDevice,InputDevice::BUTTON,0));
	return result;
	}

}
