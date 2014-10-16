/***********************************************************************
MultiDeviceNavigationTool - Class to use multiple 3-DOF devices for full
navigation (translation, rotation, scaling).
Copyright (c) 2007-2013 Oliver Kreylos

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

#include <Vrui/Tools/MultiDeviceNavigationTool.h>

#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Math/Math.h>
#include <Geometry/Vector.h>
#include <Geometry/AffineCombiner.h>
#include <Vrui/Vrui.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/*************************************************
Methods of class MultiDeviceNavigationToolFactory:
*************************************************/

MultiDeviceNavigationToolFactory::MultiDeviceNavigationToolFactory(ToolManager& toolManager)
	:ToolFactory("MultiDeviceNavigationTool",toolManager),
	 translationFactor(1),
	 minRotationScalingDistance(getInchFactor()),
	 rotationFactor(1),
	 scalingFactor(1)
	{
	/* Load class settings: */
	Misc::ConfigurationFileSection cfs=toolManager.getToolClassSection(getClassName());
	translationFactor=cfs.retrieveValue<Scalar>("./translationFactor",translationFactor);
	minRotationScalingDistance=cfs.retrieveValue<Scalar>("./minRotationScalingDistance",minRotationScalingDistance);
	rotationFactor=cfs.retrieveValue<Scalar>("./rotationFactor",rotationFactor);
	scalingFactor=cfs.retrieveValue<Scalar>("./scalingFactor",scalingFactor);
	
	/* Initialize tool layout: */
	layout.setNumButtons(1,true);
	
	/* Insert class into class hierarchy: */
	ToolFactory* navigationToolFactory=toolManager.loadClass("NavigationTool");
	navigationToolFactory->addChildClass(this);
	addParentClass(navigationToolFactory);
	
	/* Set tool class' factory pointer: */
	MultiDeviceNavigationTool::factory=this;
	}

MultiDeviceNavigationToolFactory::~MultiDeviceNavigationToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	MultiDeviceNavigationTool::factory=0;
	}

const char* MultiDeviceNavigationToolFactory::getName(void) const
	{
	return "Multiple 3-DOF Devices";
	}

const char* MultiDeviceNavigationToolFactory::getButtonFunction(int) const
	{
	return "Move / Rotate / Zoom";
	}

Tool* MultiDeviceNavigationToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new MultiDeviceNavigationTool(this,inputAssignment);
	}

void MultiDeviceNavigationToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveMultiDeviceNavigationToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("NavigationTool");
	}

extern "C" ToolFactory* createMultiDeviceNavigationToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	MultiDeviceNavigationToolFactory* multiDeviceNavigationToolFactory=new MultiDeviceNavigationToolFactory(*toolManager);
	
	/* Return factory object: */
	return multiDeviceNavigationToolFactory;
	}

extern "C" void destroyMultiDeviceNavigationToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/**************************************************
Static elements of class MultiDeviceNavigationTool:
**************************************************/

MultiDeviceNavigationToolFactory* MultiDeviceNavigationTool::factory=0;

/******************************************
Methods of class MultiDeviceNavigationTool:
******************************************/

MultiDeviceNavigationTool::MultiDeviceNavigationTool(const ToolFactory* sFactory,const ToolInputAssignment& inputAssignment)
	:NavigationTool(factory,inputAssignment),
	 numPressedButtons(0),
	 lastDeviceButtonStates(new bool[input.getNumButtonSlots()]),
	 lastDevicePositions(new Point[input.getNumButtonSlots()])
	{
	/* Initialize old button states: */
	for(int i=0;i<input.getNumButtonSlots();++i)
		lastDeviceButtonStates[i]=false;
	}

const ToolFactory* MultiDeviceNavigationTool::getFactory(void) const
	{
	return factory;
	}

void MultiDeviceNavigationTool::buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData)
	{
	/* Update the number of pressed buttons: */
	if(cbData->newButtonState)
		{
		/* Activate navigation when the first button is pressed: */
		if(numPressedButtons==0)
			activate();
		++numPressedButtons;
		}
	else
		{
		if(numPressedButtons>0)
			--numPressedButtons;
		
		/* Deactivate navigation and reset all button states when the last button is released: */
		if(numPressedButtons==0)
			{
			deactivate();
			for(int i=0;i<input.getNumButtonSlots();++i)
				lastDeviceButtonStates[i]=false;
			}
		}
	}

void MultiDeviceNavigationTool::frame(void)
	{
	/* Do nothing if the tool is inactive: */
	if(isActive())
		{
		/* Calculate the centroid of all devices whose buttons were pressed in the last frame: */
		int numLastDevices=0;
		Point::AffineCombiner centroidC;
		for(int i=0;i<input.getNumButtonSlots();++i)
			if(lastDeviceButtonStates[i])
				{
				++numLastDevices;
				centroidC.addPoint(getButtonDevicePosition(i));
				}
		
		if(numLastDevices>0)
			{
			Point currentCentroid=centroidC.getPoint();
			
			/* Calculate the average rotation vector and scaling factor of all devices whose buttons were pressed in the last frame: */
			Vector rotation=Vector::zero;
			Scalar scaling(1);
			int numActiveDevices=0;
			for(int i=0;i<input.getNumButtonSlots();++i)
				if(lastDeviceButtonStates[i])
					{
					/* Calculate the previous vector to centroid: */
					Vector lastDist=lastDevicePositions[i]-lastCentroid;
					Scalar lastLen=Geometry::mag(lastDist);
					
					/* Calculate the new vector to centroid: */
					Vector currentDist=getButtonDevicePosition(i)-currentCentroid;
					Scalar currentLen=Geometry::mag(currentDist);
					
					if(lastLen>factory->minRotationScalingDistance&&currentLen>factory->minRotationScalingDistance)
						{
						/* Calculate the rotation axis and angle: */
						Vector rot=lastDist^currentDist;
						Scalar rotLen=Geometry::mag(rot);
						if(rotLen>Scalar(0))
							{
							Scalar angle=Math::asin(rotLen/(lastLen*currentLen));
							rot*=angle/rotLen;
							
							/* Accumulate the rotation vector: */
							rotation+=rot;
							}
						
						/* Calculate the scaling factor: */
						Scalar scal=currentLen/lastLen;
						
						/* Accumulate the scaling factor: */
						scaling*=scal;
						
						++numActiveDevices;
						}
					}
			
			/* Navigate: */
			NavTransform t=NavTransform::translate((currentCentroid-lastCentroid)*factory->translationFactor);
			if(numActiveDevices>0)
				{
				/* Lock rotation vector to the navigation coordinate system's up direction: */
				Vector up=getNavigationTransformation().transform(Vector(0,0,1));
				rotation=up*((rotation*up)/Geometry::sqr(up));
				
				/* Average and scale rotation and scaling: */
				rotation*=factory->rotationFactor/Scalar(numActiveDevices);
				scaling=Math::pow(scaling,factory->scalingFactor/Scalar(numActiveDevices));
				
				/* Apply rotation and scaling: */
				t*=NavTransform::translateFromOriginTo(currentCentroid);
				t*=NavTransform::rotate(Rotation::rotateScaledAxis(rotation));
				t*=NavTransform::scale(scaling);
				t*=NavTransform::translateToOriginFrom(currentCentroid);
				}
			concatenateNavigationTransformationLeft(t);
			}
		
		/* Update button states and device positions for next frame: */
		Point::AffineCombiner newLastCentroidC;
		for(int i=0;i<input.getNumButtonSlots();++i)
			{
			lastDeviceButtonStates[i]=getButtonState(i);
			lastDevicePositions[i]=getButtonDevicePosition(i);
			if(lastDeviceButtonStates[i])
				newLastCentroidC.addPoint(lastDevicePositions[i]);
			}
		lastCentroid=newLastCentroidC.getPoint();
		}
	}

}
