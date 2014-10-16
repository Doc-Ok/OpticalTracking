/***********************************************************************
ValuatorFlyTurnNavigationTool - Class providing a fly navigation tool
with turning using two valuators.
Copyright (c) 2005-2010 Oliver Kreylos

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

#include <Vrui/Tools/ValuatorFlyTurnNavigationTool.h>

#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Math/Math.h>
#include <Math/Constants.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/OrthogonalTransformation.h>
#include <Geometry/GeometryValueCoders.h>
#include <Vrui/Vrui.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/*****************************************************
Methods of class ValuatorFlyTurnNavigationToolFactory:
*****************************************************/

ValuatorFlyTurnNavigationToolFactory::ValuatorFlyTurnNavigationToolFactory(ToolManager& toolManager)
	:ToolFactory("ValuatorFlyTurnNavigationTool",toolManager),
	 valuatorThreshold(Scalar(0.25)),
	 valuatorExponent(Scalar(1)),
	 superAccelerationFactor(Scalar(1.1)),
	 flyDirectionDeviceCoordinates(true),flyDirection(Vector(0,1,0)),
	 flyFactor(getDisplaySize()*Scalar(2)),
	 rotationAxisDeviceCoordinates(true),rotationAxis(Vector(0,0,1)),
	 rotationCenterDeviceCoordinates(true),rotationCenter(Point::origin),
	 rotationFactor(Scalar(90))
	{
	/* Initialize tool layout: */
	layout.setNumValuators(2);
	
	/* Insert class into class hierarchy: */
	ToolFactory* navigationToolFactory=toolManager.loadClass("NavigationTool");
	navigationToolFactory->addChildClass(this);
	addParentClass(navigationToolFactory);
	
	/* Load class settings: */
	Misc::ConfigurationFileSection cfs=toolManager.getToolClassSection(getClassName());
	valuatorThreshold=cfs.retrieveValue<Scalar>("./valuatorThreshold",valuatorThreshold);
	valuatorExponent=cfs.retrieveValue<Scalar>("./valuatorExponent",valuatorExponent);
	superAccelerationFactor=cfs.retrieveValue<Scalar>("./superAccelerationFactor",superAccelerationFactor);
	flyDirectionDeviceCoordinates=cfs.retrieveValue<bool>("./flyDirectionDeviceCoordinates",flyDirectionDeviceCoordinates);
	flyDirection=cfs.retrieveValue<Vector>("./flyDirection",flyDirection);
	flyDirection.normalize();
	flyFactor=cfs.retrieveValue<Scalar>("./flyFactor",flyFactor);
	rotationAxisDeviceCoordinates=cfs.retrieveValue<bool>("./rotationAxisDeviceCoordinates",rotationAxisDeviceCoordinates);
	rotationAxis=cfs.retrieveValue<Vector>("./rotationAxis",rotationAxis);
	rotationAxis.normalize();
	rotationCenterDeviceCoordinates=cfs.retrieveValue<bool>("./rotationCenterDeviceCoordinates",rotationCenterDeviceCoordinates);
	rotationCenter=cfs.retrieveValue<Point>("./rotationCenter",rotationCenter);
	rotationFactor=Math::rad(cfs.retrieveValue<Scalar>("./rotationFactor",rotationFactor));
	
	/* Set tool class' factory pointer: */
	ValuatorFlyTurnNavigationTool::factory=this;
	}


ValuatorFlyTurnNavigationToolFactory::~ValuatorFlyTurnNavigationToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	ValuatorFlyTurnNavigationTool::factory=0;
	}

const char* ValuatorFlyTurnNavigationToolFactory::getName(void) const
	{
	return "Valuator Fly + Turn";
	}

const char* ValuatorFlyTurnNavigationToolFactory::getValuatorFunction(int valuatorSlotIndex) const
	{
	switch(valuatorSlotIndex)
		{
		case 0:
			return "Fly";
		
		case 1:
			return "Rotate";
		}
	
	/* Never reached; just to make compiler happy: */
	return 0;
	}

Tool* ValuatorFlyTurnNavigationToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new ValuatorFlyTurnNavigationTool(this,inputAssignment);
	}

void ValuatorFlyTurnNavigationToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveValuatorFlyTurnNavigationToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("NavigationTool");
	}

extern "C" ToolFactory* createValuatorFlyTurnNavigationToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	ValuatorFlyTurnNavigationToolFactory* valuatorFlyTurnNavigationToolFactory=new ValuatorFlyTurnNavigationToolFactory(*toolManager);
	
	/* Return factory object: */
	return valuatorFlyTurnNavigationToolFactory;
	}

extern "C" void destroyValuatorFlyTurnNavigationToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/******************************************************
Static elements of class ValuatorFlyTurnNavigationTool:
******************************************************/

ValuatorFlyTurnNavigationToolFactory* ValuatorFlyTurnNavigationTool::factory=0;

/**********************************************
Methods of class ValuatorFlyTurnNavigationTool:
**********************************************/

ValuatorFlyTurnNavigationTool::ValuatorFlyTurnNavigationTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment)
	:NavigationTool(factory,inputAssignment),
	 superAcceleration(1)
	{
	for(int i=0;i<2;++i)
		currentValues[i]=Scalar(0);
	}

const ToolFactory* ValuatorFlyTurnNavigationTool::getFactory(void) const
	{
	return factory;
	}

void ValuatorFlyTurnNavigationTool::valuatorCallback(int valuatorSlotIndex,InputDevice::ValuatorCallbackData* cbData)
	{
	/* Map the raw valuator value according to a "broken line plus exponent" scheme: */
	Scalar v=Scalar(cbData->newValuatorValue);
	Scalar th=factory->valuatorThreshold;
	Scalar s=Scalar(1)-th;
	if(v<-th)
		{
		v=(v+th)/s;
		currentValues[valuatorSlotIndex]=-Math::pow(-v,factory->valuatorExponent);
		}
	else if(v>th)
		{
		v=(v-th)/s;
		currentValues[valuatorSlotIndex]=Math::pow(v,factory->valuatorExponent);
		}
	else
		currentValues[valuatorSlotIndex]=Scalar(0);
	
	if(currentValues[0]!=Scalar(0)||currentValues[1]!=Scalar(0))
		{
		/* Try activating this tool: */
		if(!isActive()&&activate())
			{
			/* Reset the super acceleration: */
			superAcceleration=Scalar(1);
			}
		}
	else
		{
		/* Deactivate this tool: */
		deactivate();
		}
	}

void ValuatorFlyTurnNavigationTool::frame(void)
	{
	/* Act depending on this tool's current state: */
	if(isActive())
		{
		/* Get the current state of the input device: */
		const TrackerState& ts=getValuatorDeviceTransformation(0);
		
		/* Check whether to change the super acceleration factor: */
		if(Math::abs(currentValues[0])==Scalar(1))
			superAcceleration*=Math::pow(factory->superAccelerationFactor,getFrameTime());
		
		/* Calculate the current flying velocity: */
		Vector v=factory->flyDirectionDeviceCoordinates?ts.transform(factory->flyDirection):factory->flyDirection;
		v*=-currentValues[0]*factory->flyFactor*superAcceleration*getFrameTime();
		
		/* Calculate the current angular velocity: */
		Vector w=factory->rotationAxisDeviceCoordinates?ts.transform(factory->rotationAxis):factory->rotationAxis;
		w*=currentValues[1]*factory->rotationFactor*getFrameTime();
		
		/* Compose the new navigation transformation: */
		Point p=factory->rotationCenterDeviceCoordinates?ts.transform(factory->rotationCenter):factory->rotationCenter;
		NavTransform t=NavTransform::translate(v);
		t*=NavTransform::translateFromOriginTo(p);
		t*=NavTransform::rotate(NavTransform::Rotation::rotateScaledAxis(w));
		t*=NavTransform::translateToOriginFrom(p);
		t*=getNavigationTransformation();
		
		/* Update Vrui's navigation transformation: */
		setNavigationTransformation(t);
		
		/* Request another frame: */
		scheduleUpdate(getApplicationTime()+1.0/125.0);
		}
	}

}
