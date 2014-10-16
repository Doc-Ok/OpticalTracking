/***********************************************************************
ValuatorTurnNavigationTool - Class providing a rotation navigation tool
using two valuators.
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

#include <Vrui/Tools/ValuatorTurnNavigationTool.h>

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

/**************************************************
Methods of class ValuatorTurnNavigationToolFactory:
**************************************************/

ValuatorTurnNavigationToolFactory::ValuatorTurnNavigationToolFactory(ToolManager& toolManager)
	:ToolFactory("ValuatorTurnNavigationTool",toolManager),
	 valuatorThreshold(0.25),
	 flyDirection(Vector(0,1,0)),
	 flyFactor(getDisplaySize()*Scalar(0.5)),
	 rotationAxis0(Vector(0,0,1)),
	 rotationAxis1(Vector(1,0,0)),
	 rotationCenter(Point::origin),
	 rotationFactor(Scalar(90))
	{
	/* Initialize tool layout: */
	layout.setNumButtons(1);
	layout.setNumValuators(2);
	
	/* Insert class into class hierarchy: */
	ToolFactory* navigationToolFactory=toolManager.loadClass("NavigationTool");
	navigationToolFactory->addChildClass(this);
	addParentClass(navigationToolFactory);
	
	/* Load class settings: */
	Misc::ConfigurationFileSection cfs=toolManager.getToolClassSection(getClassName());
	valuatorThreshold=cfs.retrieveValue<Scalar>("./valuatorThreshold",valuatorThreshold);
	flyDirection=cfs.retrieveValue<Vector>("./flyDirection",flyDirection);
	flyDirection.normalize();
	flyFactor=cfs.retrieveValue<Scalar>("./flyFactor",flyFactor);
	rotationAxis0=cfs.retrieveValue<Vector>("./rotationAxis0",rotationAxis0);
	rotationAxis0.normalize();
	rotationAxis1=cfs.retrieveValue<Vector>("./rotationAxis1",rotationAxis1);
	rotationAxis1.normalize();
	rotationCenter=cfs.retrieveValue<Point>("./rotationCenter",rotationCenter);
	rotationFactor=Math::rad(cfs.retrieveValue<Scalar>("./rotationFactor",rotationFactor));
	
	/* Set tool class' factory pointer: */
	ValuatorTurnNavigationTool::factory=this;
	}

ValuatorTurnNavigationToolFactory::~ValuatorTurnNavigationToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	ValuatorTurnNavigationTool::factory=0;
	}

const char* ValuatorTurnNavigationToolFactory::getName(void) const
	{
	return "Valuator Rotation";
	}

const char* ValuatorTurnNavigationToolFactory::getButtonFunction(int) const
	{
	return "Fly";
	}

const char* ValuatorTurnNavigationToolFactory::getValuatorFunction(int valuatorSlotIndex) const
	{
	switch(valuatorSlotIndex)
		{
		case 0:
			return "Rotate Axis 0";
		
		case 1:
			return "Rotate Axis 1";
		}
	
	/* Never reached; just to make compiler happy: */
	return 0;
	}

Tool* ValuatorTurnNavigationToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new ValuatorTurnNavigationTool(this,inputAssignment);
	}

void ValuatorTurnNavigationToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveValuatorTurnNavigationToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("NavigationTool");
	}

extern "C" ToolFactory* createValuatorTurnNavigationToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	ValuatorTurnNavigationToolFactory* valuatorTurnNavigationToolFactory=new ValuatorTurnNavigationToolFactory(*toolManager);
	
	/* Return factory object: */
	return valuatorTurnNavigationToolFactory;
	}

extern "C" void destroyValuatorTurnNavigationToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/***************************************************
Static elements of class ValuatorTurnNavigationTool:
***************************************************/

ValuatorTurnNavigationToolFactory* ValuatorTurnNavigationTool::factory=0;

/*******************************************
Methods of class ValuatorTurnNavigationTool:
*******************************************/

ValuatorTurnNavigationTool::ValuatorTurnNavigationTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment)
	:NavigationTool(factory,inputAssignment),
	 buttonState(false)
	{
	for(int i=0;i<2;++i)
		currentValues[i]=Scalar(0);
	}

const ToolFactory* ValuatorTurnNavigationTool::getFactory(void) const
	{
	return factory;
	}

void ValuatorTurnNavigationTool::buttonCallback(int,InputDevice::ButtonCallbackData* cbData)
	{
	buttonState=cbData->newButtonState;
	
	if(buttonState||currentValues[0]!=Scalar(0)||currentValues[1]!=Scalar(0))
		{
		/* Try activating this tool: */
		activate();
		}
	else
		{
		/* Deactivate this tool: */
		deactivate();
		}
	}

void ValuatorTurnNavigationTool::valuatorCallback(int valuatorSlotIndex,InputDevice::ValuatorCallbackData* cbData)
	{
	/* Map the raw valuator value according to a "broken line" scheme: */
	Scalar v=Scalar(cbData->newValuatorValue);
	Scalar th=factory->valuatorThreshold;
	Scalar s=Scalar(1)-th;
	if(v<-th)
		v=(v+th)/s;
	else if(v>th)
		v=(v-th)/s;
	else
		v=Scalar(0);
	currentValues[valuatorSlotIndex]=v;
	
	if(buttonState||currentValues[0]!=Scalar(0)||currentValues[1]!=Scalar(0))
		{
		/* Try activating this tool: */
		activate();
		}
	else
		{
		/* Deactivate this tool: */
		deactivate();
		}
	}

void ValuatorTurnNavigationTool::frame(void)
	{
	/* Act depending on this tool's current state: */
	if(isActive())
		{
		/* Get the current state of the input device: */
		const TrackerState& ts=getButtonDeviceTransformation(0);
		
		/* Calculate the current flying velocity: */
		Vector v=Vector::zero;
		if(buttonState)
			{
			v=ts.transform(factory->flyDirection);
			v*=-factory->flyFactor*getFrameTime();
			}
		
		/* Calculate the current angular velocities: */
		Vector w0=factory->rotationAxis0;
		w0*=currentValues[0]*factory->rotationFactor*getFrameTime();
		Vector w1=factory->rotationAxis1;
		w1*=currentValues[1]*factory->rotationFactor*getFrameTime();
		
		/* Compose the new navigation transformation: */
		Point p=ts.transform(factory->rotationCenter);
		NavTransform t=NavTransform::translate(v);
		t*=NavTransform::translateFromOriginTo(p);
		t*=NavTransform::rotate(NavTransform::Rotation::rotateScaledAxis(w0));
		t*=NavTransform::rotate(NavTransform::Rotation::rotateScaledAxis(w1));
		t*=NavTransform::translateToOriginFrom(p);
		t*=getNavigationTransformation();
		
		/* Update Vrui's navigation transformation: */
		setNavigationTransformation(t);
		
		/* Request another frame: */
		scheduleUpdate(getApplicationTime()+1.0/125.0);
		}
	}

}
