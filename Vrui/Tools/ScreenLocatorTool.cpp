/***********************************************************************
ScreenLocator - Simple locator class to use in simulated VR
environments.
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

#include <Vrui/Tools/ScreenLocatorTool.h>

#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/Ray.h>
#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLGeometryWrappers.h>
#include <GL/GLTransformationWrappers.h>
#include <Vrui/Vrui.h>
#include <Vrui/VRScreen.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/*****************************************
Methods of class ScreenLocatorToolFactory:
*****************************************/

ScreenLocatorToolFactory::ScreenLocatorToolFactory(ToolManager& toolManager)
	:ToolFactory("ScreenLocatorTool",toolManager),
	 crosshairSize(getUiSize()*Scalar(2))
	{
	/* Initialize tool layout: */
	layout.setNumButtons(1);
	
	/* Insert class into class hierarchy: */
	ToolFactory* locatorToolFactory=toolManager.loadClass("LocatorTool");
	locatorToolFactory->addChildClass(this);
	addParentClass(locatorToolFactory);
	
	/* Load class settings: */
	Misc::ConfigurationFileSection cfs=toolManager.getToolClassSection(getClassName());
	crosshairSize=cfs.retrieveValue<Scalar>("./crosshairSize",crosshairSize);
	
	/* Set tool class' factory pointer: */
	ScreenLocatorTool::factory=this;
	}

ScreenLocatorToolFactory::~ScreenLocatorToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	ScreenLocatorTool::factory=0;
	}

const char* ScreenLocatorToolFactory::getName(void) const
	{
	return "Screen Locator";
	}

Tool* ScreenLocatorToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new ScreenLocatorTool(this,inputAssignment);
	}

void ScreenLocatorToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveScreenLocatorToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("LocatorTool");
	}

extern "C" ToolFactory* createScreenLocatorToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	ScreenLocatorToolFactory* screenLocatorToolFactory=new ScreenLocatorToolFactory(*toolManager);
	
	/* Return factory object: */
	return screenLocatorToolFactory;
	}

extern "C" void destroyScreenLocatorToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/******************************************
Static elements of class ScreenLocatorTool:
******************************************/

ScreenLocatorToolFactory* ScreenLocatorTool::factory=0;

/**********************************
Methods of class ScreenLocatorTool:
**********************************/

void ScreenLocatorTool::calcTransformation(void)
	{
	/* Calculate the ray equation: */
	Ray ray=getButtonDeviceRay(0);
	
	/* Find the closest intersection with any screen: */
	std::pair<VRScreen*,Scalar> si=findScreen(ray);
	
	/* Set the current transformation to the input device: */
	if(si.first!=0)
		{
		/* Compute the locator transformation: */
		currentTransformation=NavTrackerState::translateFromOriginTo(ray(si.second));
		}
	}

ScreenLocatorTool::ScreenLocatorTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment)
	:LocatorTool(factory,inputAssignment)
	{
	}

const ToolFactory* ScreenLocatorTool::getFactory(void) const
	{
	return factory;
	}

void ScreenLocatorTool::buttonCallback(int,InputDevice::ButtonCallbackData* cbData)
	{
	if(cbData->newButtonState) // Button has just been pressed
		{
		/* Call button press callbacks: */
		ButtonPressCallbackData cbData(this,getInverseNavigationTransformation()*currentTransformation);
		buttonPressCallbacks.call(&cbData);
		}
	else // Button has just been released
		{
		/* Call button release callbacks: */
		ButtonReleaseCallbackData cbData(this,getInverseNavigationTransformation()*currentTransformation);
		buttonReleaseCallbacks.call(&cbData);
		}
	}

void ScreenLocatorTool::frame(void)
	{
	/* Calculate current locator transformation: */
	calcTransformation();
	
	/* Call motion callbacks: */
	MotionCallbackData cbData(this,getInverseNavigationTransformation()*currentTransformation);
	motionCallbacks.call(&cbData);
	}

void ScreenLocatorTool::display(GLContextData&) const
	{
	if(factory->crosshairSize>Scalar(0))
		{
		/* Draw crosshairs at the virtual device's current position: */
		glPushAttrib(GL_ENABLE_BIT|GL_LINE_BIT);
		glDisable(GL_LIGHTING);
		glPushMatrix();
		glMultMatrix(currentTransformation);
		glLineWidth(3.0f);
		Color lineCol=getBackgroundColor();
		glColor(lineCol);
		glBegin(GL_LINES);
		glVertex(-factory->crosshairSize,Scalar(0),Scalar(0));
		glVertex( factory->crosshairSize,Scalar(0),Scalar(0));
		glVertex(Scalar(0),Scalar(0),-factory->crosshairSize);
		glVertex(Scalar(0),Scalar(0), factory->crosshairSize);
		glEnd();
		glLineWidth(1.0f);
		for(int i=0;i<3;++i)
			lineCol[i]=1.0f-lineCol[i];
		glColor(lineCol);
		glBegin(GL_LINES);
		glVertex(-factory->crosshairSize,Scalar(0),Scalar(0));
		glVertex( factory->crosshairSize,Scalar(0),Scalar(0));
		glVertex(Scalar(0),Scalar(0),-factory->crosshairSize);
		glVertex(Scalar(0),Scalar(0), factory->crosshairSize);
		glEnd();
		glPopMatrix();
		glPopAttrib();
		}
	}

}
