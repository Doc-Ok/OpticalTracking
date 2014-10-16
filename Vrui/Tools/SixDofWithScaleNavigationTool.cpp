/***********************************************************************
SixDofWithScaleNavigationTool - Class for simple 6-DOF dragging using a
single input device, with an additional input device used as a slider
for zooming.
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

#include <Vrui/Tools/SixDofWithScaleNavigationTool.h>

#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Math/Math.h>
#include <Geometry/GeometryValueCoders.h>
#include <GL/gl.h>
#include <GL/GLContextData.h>
#include <GL/GLGeometryWrappers.h>
#include <GL/GLTransformationWrappers.h>
#include <GL/GLModels.h>
#include <Vrui/Vrui.h>
#include <Vrui/InputDeviceManager.h>
#include <Vrui/InputGraphManager.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/***************************************************************
Methods of class SixDofWithScaleNavigationToolFactory::DataItem:
***************************************************************/

SixDofWithScaleNavigationToolFactory::DataItem::DataItem(void)
	{
	/* Create tools' model display list: */
	modelListId=glGenLists(1);
	}

SixDofWithScaleNavigationToolFactory::DataItem::~DataItem(void)
	{
	/* Destroy tools' model display list: */
	glDeleteLists(modelListId,1);
	}

/*****************************************************
Methods of class SixDofWithScaleNavigationToolFactory:
*****************************************************/

SixDofWithScaleNavigationToolFactory::SixDofWithScaleNavigationToolFactory(ToolManager& toolManager)
	:ToolFactory("SixDofWithScaleNavigationTool",toolManager),
	 scaleDeviceDistance(getInchFactor()*Scalar(4)),
	 deviceScaleDirection(0,1,0),
	 scaleFactor(getInchFactor()*Scalar(8))
	{
	/* Initialize tool layout: */
	layout.setNumButtons(2);
	
	/* Insert class into class hierarchy: */
	ToolFactory* navigationToolFactory=toolManager.loadClass("NavigationTool");
	navigationToolFactory->addChildClass(this);
	addParentClass(navigationToolFactory);
	
	/* Load class settings: */
	Misc::ConfigurationFileSection cfs=toolManager.getToolClassSection(getClassName());
	scaleDeviceDistance=cfs.retrieveValue<Scalar>("./scaleDeviceDistance",scaleDeviceDistance);
	scaleDeviceDistance2=scaleDeviceDistance*scaleDeviceDistance;
	deviceScaleDirection=cfs.retrieveValue<Vector>("./deviceScaleDirection",deviceScaleDirection);
	scaleFactor=cfs.retrieveValue<Scalar>("./scaleFactor",scaleFactor);
	
	/* Set tool class' factory pointer: */
	SixDofWithScaleNavigationTool::factory=this;
	}

SixDofWithScaleNavigationToolFactory::~SixDofWithScaleNavigationToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	SixDofWithScaleNavigationTool::factory=0;
	}

const char* SixDofWithScaleNavigationToolFactory::getName(void) const
	{
	return "6-DOF + Scaling Device";
	}

const char* SixDofWithScaleNavigationToolFactory::getButtonFunction(int buttonSlotIndex) const
	{
	switch(buttonSlotIndex)
		{
		case 0:
			return "Grab Space / Zoom";
		
		case 1:
			return "Forwarded Button";
		}
	
	/* Never reached; just to make compiler happy: */
	return 0;
	}

Tool* SixDofWithScaleNavigationToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new SixDofWithScaleNavigationTool(this,inputAssignment);
	}

void SixDofWithScaleNavigationToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

void SixDofWithScaleNavigationToolFactory::initContext(GLContextData& contextData) const
	{
	/* Create a new data item: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	
	/* Create the tool model display list: */
	glNewList(dataItem->modelListId,GL_COMPILE);
	
	/* Set up OpenGL state: */
	glPushAttrib(GL_ENABLE_BIT|GL_POLYGON_BIT|GL_LINE_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	glLineWidth(1.0f);
	glColor3f(1.0f,1.0f,1.0f);
	
	/* Render a sphere of radius scaleDeviceDistance around the scaling device's position: */
	glDrawSphereIcosahedron(scaleDeviceDistance,3);
	
	/* Render the scaling direction: */
	glLineWidth(3.0f);
	glColor3f(1.0f,0.0f,0.0f);
	glBegin(GL_LINES);
	Point pos=Point::origin;
	glVertex(pos);
	pos+=deviceScaleDirection*scaleDeviceDistance*Scalar(1.25);
	glVertex(pos);
	glEnd();
	
	/* Reset OpenGL state: */
	glPopAttrib();
	
	glEndList();
	}

extern "C" void resolveSixDofWithScaleNavigationToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("NavigationTool");
	}

extern "C" ToolFactory* createSixDofWithScaleNavigationToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	SixDofWithScaleNavigationToolFactory* sixDofWithScaleNavigationToolFactory=new SixDofWithScaleNavigationToolFactory(*toolManager);
	
	/* Return factory object: */
	return sixDofWithScaleNavigationToolFactory;
	}

extern "C" void destroySixDofWithScaleNavigationToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/******************************************************
Static elements of class SixDofWithScaleNavigationTool:
******************************************************/

SixDofWithScaleNavigationToolFactory* SixDofWithScaleNavigationTool::factory=0;

/**********************************************
Methods of class SixDofWithScaleNavigationTool:
**********************************************/

SixDofWithScaleNavigationTool::SixDofWithScaleNavigationTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment)
	:NavigationTool(factory,inputAssignment),
	 buttonDevice(0),
	 navigationMode(IDLE)
	{
	}

void SixDofWithScaleNavigationTool::initialize(void)
	{
	/* Get the source input device: */
	InputDevice* device=getButtonDevice(1);
	
	/* Create a virtual input device to shadow the zoom button: */
	buttonDevice=addVirtualInputDevice("SixDofWithScaleNavigationToolButtonDevice",1,0);
	
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

void SixDofWithScaleNavigationTool::deinitialize(void)
	{
	/* Release the virtual input device: */
	getInputGraphManager()->releaseInputDevice(buttonDevice,this);
	
	/* Destroy the virtual input device: */
	getInputDeviceManager()->destroyInputDevice(buttonDevice);
	buttonDevice=0;
	}

const ToolFactory* SixDofWithScaleNavigationTool::getFactory(void) const
	{
	return factory;
	}

void SixDofWithScaleNavigationTool::buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData)
	{
	switch(buttonSlotIndex)
		{
		case 0:
			if(cbData->newButtonState) // Button has just been pressed
				{
				if(navigationMode==IDLE&&activate())
					{
					/* Decide whether to go to moving or scaling mode: */
					if(Geometry::sqrDist(getButtonDevicePosition(0),getButtonDevicePosition(1))<=factory->scaleDeviceDistance2) // Want to scale
						{
						/* Determine the scaling center and initial scale: */
						scalingCenter=getButtonDevicePosition(1);
						Vector scaleDirection=getButtonDeviceTransformation(1).transform(factory->deviceScaleDirection);
						initialScale=getButtonDevicePosition(0)*scaleDirection;
						
						/* Initialize the navigation transformations: */
						preScale=NavTrackerState::translateFromOriginTo(scalingCenter);
						postScale=NavTrackerState::translateToOriginFrom(scalingCenter);
						postScale*=getNavigationTransformation();
						
						/* Go from MOVING to SCALING mode: */
						navigationMode=SCALING;
						}
					else // Want to move
						{
						/* Initialize the navigation transformations: */
						preScale=Geometry::invert(getDeviceTransformation(0));
						preScale*=getNavigationTransformation();
						
						/* Go from IDLE to MOVING mode: */
						navigationMode=MOVING;
						}
					}
				}
			else // Button has just been released
				{
				/* Deactivate this tool: */
				deactivate();
				
				/* Go from MOVING or SCALING to IDLE mode: */
				navigationMode=IDLE;
				}
			
			break;
		
		case 1:
			/* Pass the button event to the virtual input device: */
			buttonDevice->setButtonState(0,cbData->newButtonState);
			
			break;
		}
	}

void SixDofWithScaleNavigationTool::frame(void)
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
			Vector scaleDirection=getButtonDeviceTransformation(1).transform(factory->deviceScaleDirection);
			Scalar currentScale=Math::exp((getButtonDevicePosition(0)*scaleDirection-initialScale)/factory->scaleFactor);
			navigation*=NavTrackerState::scale(currentScale);
			navigation*=postScale;
			
			/* Update Vrui's navigation transformation: */
			setNavigationTransformation(navigation);
			break;
			}
		}
	
	/* Update the virtual input device: */
	InputDevice* device=getButtonDevice(1);
	buttonDevice->setDeviceRay(device->getDeviceRayDirection(),device->getDeviceRayStart());
	buttonDevice->setTransformation(device->getTransformation());
	}

void SixDofWithScaleNavigationTool::display(GLContextData& contextData) const
	{
	/* Get a pointer to the context entry: */
	SixDofWithScaleNavigationToolFactory::DataItem* dataItem=contextData.retrieveDataItem<SixDofWithScaleNavigationToolFactory::DataItem>(factory);
	
	/* Translate coordinate system to scaling device's position and orientation: */
	glPushMatrix();
	glMultMatrix(getButtonDeviceTransformation(1));
	
	/* Execute the tool model display list: */
	glCallList(dataItem->modelListId);
	
	/* Go back to physical coordinate system: */
	glPopMatrix();
	}

std::vector<InputDevice*> SixDofWithScaleNavigationTool::getForwardedDevices(void)
	{
	std::vector<InputDevice*> result;
	result.push_back(buttonDevice);
	return result;
	}

InputDeviceFeatureSet SixDofWithScaleNavigationTool::getSourceFeatures(const InputDeviceFeature& forwardedFeature)
	{
	/* Paranoia: Check if the forwarded feature is on the transformed device: */
	if(forwardedFeature.getDevice()!=buttonDevice)
		Misc::throwStdErr("SixDofWithScaleNavigationTool::getSourceFeatures: Forwarded feature is not on transformed device");
	
	/* Return the source feature: */
	InputDeviceFeatureSet result;
	result.push_back(input.getButtonSlotFeature(1));
	return result;
	}

InputDevice* SixDofWithScaleNavigationTool::getSourceDevice(const InputDevice* forwardedDevice)
	{
	/* Paranoia: Check if the forwarded device is the same as the transformed device: */
	if(forwardedDevice!=buttonDevice)
		Misc::throwStdErr("SixDofWithScaleNavigationTool::getSourceDevice: Given forwarded device is not transformed device");
	
	/* Return the source device: */
	return getButtonDevice(1);
	}

InputDeviceFeatureSet SixDofWithScaleNavigationTool::getForwardedFeatures(const InputDeviceFeature& sourceFeature)
	{
	/* Get the source feature's assignment slot index: */
	int slotIndex=input.findFeature(sourceFeature);
	
	/* Paranoia: Check if the source feature belongs to this tool: */
	if(slotIndex<0)
		Misc::throwStdErr("SixDofWithScaleNavigationTool::getForwardedFeatures: Source feature is not part of tool's input assignment");
	
	/* Return the forwarded feature: */
	InputDeviceFeatureSet result;
	if(slotIndex==1)
		result.push_back(InputDeviceFeature(buttonDevice,InputDevice::BUTTON,0));
	return result;
	}

}
