/***********************************************************************
TwoRayTransformTool - Class to select 3D positions using ray-based input
devices by intersecting two rays from two different starting points.
Copyright (c) 2010-2014 Oliver Kreylos

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

#include <Vrui/Tools/TwoRayTransformTool.h>

#include <Geometry/ComponentArray.h>
#include <Geometry/Matrix.h>
#include <Geometry/OrthonormalTransformation.h>
#include <GL/gl.h>
#include <GL/GLGeometryWrappers.h>
#include <Vrui/Vrui.h>
#include <Vrui/InputGraphManager.h>
#include <Vrui/InputDeviceManager.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/*******************************************
Methods of class TwoRayTransformToolFactory:
*******************************************/

TwoRayTransformToolFactory::TwoRayTransformToolFactory(ToolManager& toolManager)
	:ToolFactory("TwoRayTransformTool",toolManager)
	{
	/* Insert class into class hierarchy: */
	TransformToolFactory* transformToolFactory=dynamic_cast<TransformToolFactory*>(toolManager.loadClass("TransformTool"));
	transformToolFactory->addChildClass(this);
	addParentClass(transformToolFactory);
	
	/* Initialize tool layout: */
	layout.setNumButtons(0,true);
	layout.setNumValuators(0,true);
	
	/* Set tool class' factory pointer: */
	TwoRayTransformTool::factory=this;
	}


TwoRayTransformToolFactory::~TwoRayTransformToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	TwoRayTransformTool::factory=0;
	}

const char* TwoRayTransformToolFactory::getName(void) const
	{
	return "Dual Ray Intersector";
	}

Tool* TwoRayTransformToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new TwoRayTransformTool(this,inputAssignment);
	}

void TwoRayTransformToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveTwoRayTransformToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("TransformTool");
	}

extern "C" ToolFactory* createTwoRayTransformToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	TwoRayTransformToolFactory* twoRayTransformToolFactory=new TwoRayTransformToolFactory(*toolManager);
	
	/* Return factory object: */
	return twoRayTransformToolFactory;
	}

extern "C" void destroyTwoRayTransformToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/********************************************
Static elements of class TwoRayTransformTool:
********************************************/

TwoRayTransformToolFactory* TwoRayTransformTool::factory=0;

/************************************
Methods of class TwoRayTransformTool:
************************************/

TwoRayTransformTool::TwoRayTransformTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment)
	:TransformTool(factory,inputAssignment),
	 numRays(0),active(false)
	{
	/* Set the transformation source device: */
	if(input.getNumButtonSlots()>0)
		sourceDevice=getButtonDevice(0);
	else
		sourceDevice=getValuatorDevice(0);
	}

TwoRayTransformTool::~TwoRayTransformTool(void)
	{
	}

void TwoRayTransformTool::initialize(void)
	{
	/* Initialize the base tool: */
	TransformTool::initialize();
	
	/* Disable the transformed device's glyph: */
	getInputGraphManager()->getInputDeviceGlyph(transformedDevice).disable();
	}

const ToolFactory* TwoRayTransformTool::getFactory(void) const
	{
	return factory;
	}

void TwoRayTransformTool::buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData)
	{
	if(buttonSlotIndex==0)
		{
		if(cbData->newButtonState) // Button was just pressed
			{
			/* Update the tool's state: */
			if(numRays==2)
				{
				/* Reset the tool: */
				numRays=0;
				}
			else if(numRays==1)
				{
				/* Press the transformed tool's first button: */
				transformedDevice->setButtonState(0,true);
				}
			
			/* Start dragging: */
			active=true;
			}
		else // Button was just released
			{
			/* Update the tool's state: */
			++numRays;
			if(numRays==2)
				{
				/* Release the transformed tool's first button: */
				transformedDevice->setButtonState(0,false);
				}
			
			/* Stop dragging: */
			active=false;
			}
		}
	else
		{
		/* Let the transform tool handle it: */
		TransformTool::buttonCallback(buttonSlotIndex,cbData);
		}
	}

void TwoRayTransformTool::frame(void)
	{
	/* Check if the device is currently dragging a ray: */
	if(active)
		{
		/* Calculate the device's ray equation in navigational coordinates: */
		rays[numRays]=sourceDevice->getRay();
		rays[numRays].transform(getInverseNavigationTransformation());
		
		/* Check if there are two rays (one final and one intermediate): */
		if(numRays==1)
			{
			/* Calculate the "intersection" point between the two rays: */
			Geometry::Matrix<Scalar,3,3> a;
			Geometry::ComponentArray<Scalar,3> b;
			Vector bin=rays[0].getDirection()^rays[1].getDirection();
			for(int i=0;i<3;++i)
				{
				a(i,0)=rays[0].getDirection()[i];
				a(i,1)=-rays[1].getDirection()[i];
				a(i,2)=bin[i];
				b[i]=rays[1].getOrigin()[i]-rays[0].getOrigin()[i];
				}
			Geometry::ComponentArray<Scalar,3> x=b/a;
			intersection=Geometry::mid(rays[0](x[0]),rays[1](x[1]));
			}
		}
	if(numRays>=(active?1:2))
		{
		/* Set the transformed device's position and orientation: */
		transformedDevice->setDeviceRay(sourceDevice->getDeviceRayDirection(),Scalar(0));
		transformedDevice->setTransformation(ONTransform(getNavigationTransformation().transform(intersection)-Point::origin,sourceDevice->getTransformation().getRotation()));
		}
	}

void TwoRayTransformTool::display(GLContextData& contextData) const
	{
	if(numRays>0)
		{
		/* Draw the selection rays: */
		glPushAttrib(GL_ENABLE_BIT|GL_LINE_BIT);
		glDisable(GL_LIGHTING);
		glColor3f(1.0f,0.0f,0.0f);
		glLineWidth(3.0f);
		glBegin(GL_LINES);
		for(int i=0;i<(active?numRays+1:numRays);++i)
			{
			Ray tr=rays[i];
			tr.transform(getNavigationTransformation());
			glVertex(tr.getOrigin());
			glVertex(tr(getDisplaySize()*Scalar(5)));
			}
		glEnd();
		glPopAttrib();
		
		}
	}

}
