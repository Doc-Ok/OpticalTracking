/***********************************************************************
PlaneSnapInputDeviceTool - Class for tools that snap a virtual input
device to a plane defined by three points.
Copyright (c) 2009-2013 Oliver Kreylos

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

#include <Vrui/Tools/PlaneSnapInputDeviceTool.h>

#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Geometry/Vector.h>
#include <Geometry/OrthonormalTransformation.h>
#include <Geometry/OrthogonalTransformation.h>
#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLGeometryWrappers.h>
#include <GL/GLTransformationWrappers.h>
#include <Vrui/Vrui.h>
#include <Vrui/ToolManager.h>
#include <Vrui/DisplayState.h>

namespace Vrui {

/************************************************
Methods of class PlaneSnapInputDeviceToolFactory:
************************************************/

PlaneSnapInputDeviceToolFactory::PlaneSnapInputDeviceToolFactory(ToolManager& toolManager)
	:ToolFactory("PlaneSnapInputDeviceTool",toolManager),
	 markerSize(getUiSize())
	{
	/* Initialize tool layout: */
	layout.setNumButtons(1);
	
	/* Insert class into class hierarchy: */
	ToolFactory* inputDeviceToolFactory=toolManager.loadClass("InputDeviceTool");
	inputDeviceToolFactory->addChildClass(this);
	addParentClass(inputDeviceToolFactory);
	
	/* Load class settings: */
	Misc::ConfigurationFileSection cfs=toolManager.getToolClassSection(getClassName());
	markerSize=cfs.retrieveValue<Scalar>("./markerSize",markerSize);
	
	/* Set tool class' factory pointer: */
	PlaneSnapInputDeviceTool::factory=this;
	}

PlaneSnapInputDeviceToolFactory::~PlaneSnapInputDeviceToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	PlaneSnapInputDeviceTool::factory=0;
	}

const char* PlaneSnapInputDeviceToolFactory::getName(void) const
	{
	return "Plane Snapper";
	}

const char* PlaneSnapInputDeviceToolFactory::getButtonFunction(int) const
	{
	return "Grab Device / Select Points";
	}

Tool* PlaneSnapInputDeviceToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new PlaneSnapInputDeviceTool(this,inputAssignment);
	}

void PlaneSnapInputDeviceToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolvePlaneSnapInputDeviceToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("InputDeviceTool");
	}

extern "C" ToolFactory* createPlaneSnapInputDeviceToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	PlaneSnapInputDeviceToolFactory* rayInputDeviceToolFactory=new PlaneSnapInputDeviceToolFactory(*toolManager);
	
	/* Return factory object: */
	return rayInputDeviceToolFactory;
	}

extern "C" void destroyPlaneSnapInputDeviceToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/*************************************************
Static elements of class PlaneSnapInputDeviceTool:
*************************************************/

PlaneSnapInputDeviceToolFactory* PlaneSnapInputDeviceTool::factory=0;

/*****************************************
Methods of class PlaneSnapInputDeviceTool:
*****************************************/

PlaneSnapInputDeviceTool::PlaneSnapInputDeviceTool(const ToolFactory* sFactory,const ToolInputAssignment& inputAssignment)
	:InputDeviceTool(sFactory,inputAssignment),
	 numSelectedPoints(0),draggingPoint(false)
	{
	/* Set the interaction device: */
	interactionDevice=getButtonDevice(0);
	}

const ToolFactory* PlaneSnapInputDeviceTool::getFactory(void) const
	{
	return factory;
	}

void PlaneSnapInputDeviceTool::buttonCallback(int,InputDevice::ButtonCallbackData* cbData)
	{
	if(cbData->newButtonState) // Button has just been pressed
		{
		/* Try activating the tool: */
		if(interactionDevice->isRayDevice())
			{
			/* Pick a virtual input device using ray selection: */
			activate(calcInteractionRay());
			}
		else
			{
			/* Pick a virtual input device using point selection: */
			activate(getInteractionPosition());
			}
		
		/* Check if the tool was activated: */
		if(isActive())
			{
			/* Check if there are currently three selected points: */
			if(numSelectedPoints==3)
				{
				/* Snap the selected virtual input device to the plane defined by the three selected points: */
				Vector y=(selectedPoints[1]-selectedPoints[0])^(selectedPoints[2]-selectedPoints[0]);
				Scalar offset=(selectedPoints[0]*y+selectedPoints[1]*y+selectedPoints[2]*y)/Scalar(3);
				Vector x=Geometry::normal(y);
				Point devicePos=getInverseNavigationTransformation().transform(getGrabbedDevice()->getPosition());
				Scalar lambda=(offset-devicePos*y)/Geometry::sqr(y);
				devicePos+=y*lambda;
				NavTransform dt=NavTransform(devicePos-Point::origin,NavTransform::Rotation::fromBaseVectors(x,y),Scalar(1));
				dt.leftMultiply(getNavigationTransformation());
				getGrabbedDevice()->setTransformation(TrackerState(dt.getTranslation(),dt.getRotation()));
				}
			
			/* Deactivate the tool again: */
			deactivate();
			}
		else
			{
			/* Start dragging another selection point: */
			if(numSelectedPoints==3)
				numSelectedPoints=0;
			draggingPoint=true;
			++numSelectedPoints;
			}
		}
	else // Button has just been released
		{
		/* Stop dragging a selection point: */
		draggingPoint=false;
		}
	}

void PlaneSnapInputDeviceTool::frame(void)
	{
	if(draggingPoint)
		{
		/* Set the position of the currently dragged point: */
		selectedPoints[numSelectedPoints-1]=getInverseNavigationTransformation().transform(getInteractionPosition());
		}
	}

void PlaneSnapInputDeviceTool::display(GLContextData& contextData) const
	{
	if(numSelectedPoints>0)
		{
		/* Set up and save OpenGL state: */
		GLboolean lightingEnabled=glIsEnabled(GL_LIGHTING);
		if(lightingEnabled)
			glDisable(GL_LIGHTING);
		GLfloat lineWidth;
		glGetFloatv(GL_LINE_WIDTH,&lineWidth);
		
		/* Calculate the marker size: */
		Scalar markerSize=factory->markerSize;
		markerSize/=getNavigationTransformation().getScaling();
		
		/* Go to navigational coordinates: */
		glPushMatrix();
		glLoadIdentity();
		glMultMatrix(getDisplayState(contextData).modelviewNavigational);
		
		/* Determine the marker color: */
		Color bgColor=getBackgroundColor();
		Color fgColor;
		for(int i=0;i<3;++i)
			fgColor[i]=1.0f-bgColor[i];
		fgColor[3]=bgColor[3];
		
		
		/* Mark all measurement points: */
		glLineWidth(3.0f);
		glBegin(GL_LINES);
		glColor(bgColor);
		for(int i=0;i<numSelectedPoints;++i)
			{
			glVertex(selectedPoints[i][0]-markerSize,selectedPoints[i][1],selectedPoints[i][2]);
			glVertex(selectedPoints[i][0]+markerSize,selectedPoints[i][1],selectedPoints[i][2]);
			glVertex(selectedPoints[i][0],selectedPoints[i][1]-markerSize,selectedPoints[i][2]);
			glVertex(selectedPoints[i][0],selectedPoints[i][1]+markerSize,selectedPoints[i][2]);
			glVertex(selectedPoints[i][0],selectedPoints[i][1],selectedPoints[i][2]-markerSize);
			glVertex(selectedPoints[i][0],selectedPoints[i][1],selectedPoints[i][2]+markerSize);
			}
		glEnd();
		glLineWidth(1.0f);
		glBegin(GL_LINES);
		glColor(fgColor);
		for(int i=0;i<numSelectedPoints;++i)
			{
			glVertex(selectedPoints[i][0]-markerSize,selectedPoints[i][1],selectedPoints[i][2]);
			glVertex(selectedPoints[i][0]+markerSize,selectedPoints[i][1],selectedPoints[i][2]);
			glVertex(selectedPoints[i][0],selectedPoints[i][1]-markerSize,selectedPoints[i][2]);
			glVertex(selectedPoints[i][0],selectedPoints[i][1]+markerSize,selectedPoints[i][2]);
			glVertex(selectedPoints[i][0],selectedPoints[i][1],selectedPoints[i][2]-markerSize);
			glVertex(selectedPoints[i][0],selectedPoints[i][1],selectedPoints[i][2]+markerSize);
			}
		glEnd();
		
		/* Restore OpenGL state: */
		glPopMatrix();
		glLineWidth(lineWidth);
		if(lightingEnabled)
			glEnable(GL_LIGHTING);
		}
	}

}
