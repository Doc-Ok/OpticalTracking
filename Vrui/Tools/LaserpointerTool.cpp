/***********************************************************************
LaserpointerTool - Class for tools using rays to point out features in a
3D display.
Copyright (c) 2006-2011 Oliver Kreylos

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

#include <Vrui/Tools/LaserpointerTool.h>

#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLValueCoders.h>
#include <GL/GLGeometryWrappers.h>
#include <Vrui/Vrui.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/****************************************
Methods of class LaserpointerToolFactory:
****************************************/

LaserpointerToolFactory::LaserpointerToolFactory(ToolManager& toolManager)
	:ToolFactory("LaserpointerTool",toolManager),
	 rayLength(getDisplaySize()*Scalar(5)),
	 rayLineWidth(3.0f),
	 rayColor(1.0f,0.0f,0.0f)
	{
	/* Initialize tool layout: */
	layout.setNumButtons(1);
	
	/* Insert class into class hierarchy: */
	ToolFactory* toolFactory=toolManager.loadClass("PointingTool");
	toolFactory->addChildClass(this);
	addParentClass(toolFactory);
	
	/* Load class settings: */
	Misc::ConfigurationFileSection cfs=toolManager.getToolClassSection(getClassName());
	rayLength=cfs.retrieveValue<Scalar>("./rayLength",rayLength);
	rayLineWidth=cfs.retrieveValue<float>("./rayLineWidth",rayLineWidth);
	rayColor=cfs.retrieveValue<Color>("./rayColor",rayColor);
	
	/* Set tool class' factory pointer: */
	LaserpointerTool::factory=this;
	}

LaserpointerToolFactory::~LaserpointerToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	LaserpointerTool::factory=0;
	}

const char* LaserpointerToolFactory::getName(void) const
	{
	return "Laser Pointer";
	}

Tool* LaserpointerToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new LaserpointerTool(this,inputAssignment);
	}

void LaserpointerToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveLaserpointerToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("UtilityTool");
	}

extern "C" ToolFactory* createLaserpointerToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	LaserpointerToolFactory* laserpointerToolFactory=new LaserpointerToolFactory(*toolManager);
	
	/* Return factory object: */
	return laserpointerToolFactory;
	}

extern "C" void destroyLaserpointerToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/*****************************************
Static elements of class LaserpointerTool:
*****************************************/

LaserpointerToolFactory* LaserpointerTool::factory=0;

/*********************************
Methods of class LaserpointerTool:
*********************************/

LaserpointerTool::LaserpointerTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment)
	:PointingTool(factory,inputAssignment),
	 active(false)
	{
	}

const ToolFactory* LaserpointerTool::getFactory(void) const
	{
	return factory;
	}

void LaserpointerTool::buttonCallback(int,InputDevice::ButtonCallbackData* cbData)
	{
	if(cbData->newButtonState) // Activation button has just been pressed
		{
		/* Activate the laser pointer: */
		active=true;
		}
	else
		{
		/* Deactivate the laser pointer: */
		active=false;
		}
	}

void LaserpointerTool::frame(void)
	{
	if(active)
		{
		/* Update the laser ray: */
		ray=getButtonDeviceRay(0);
		}
	}

void LaserpointerTool::display(GLContextData&) const
	{
	if(active)
		{
		/* Draw the laser ray: */
		glPushAttrib(GL_ENABLE_BIT|GL_LINE_BIT);
		glDisable(GL_LIGHTING);
		glColor(factory->rayColor);
		glLineWidth(factory->rayLineWidth);
		glBegin(GL_LINES);
		glVertex(ray.getOrigin());
		glVertex(ray(factory->rayLength*scaleFactor));
		glEnd();
		glPopAttrib();
		}
	}

}
