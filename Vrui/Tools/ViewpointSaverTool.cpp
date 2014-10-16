/***********************************************************************
ViewpointSaverTool - Class for tools to save environment-independent
viewing parameters.
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

#include <Vrui/Tools/ViewpointSaverTool.h>

#include <stdio.h>
#include <Misc/File.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/OrthogonalTransformation.h>
#include <GL/gl.h>
#include <GL/GLGeometryWrappers.h>
#include <Vrui/Vrui.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/******************************************
Methods of class ViewpointSaverToolFactory:
******************************************/

ViewpointSaverToolFactory::ViewpointSaverToolFactory(ToolManager& toolManager)
	:ToolFactory("ViewpointSaverTool",toolManager),
	 viewpointFileName("ViewpointSaverTool.views"),
	 axisLength(getDisplaySize()*Scalar(0.5)),
	 axisLineWidth(3.0f),
	 viewpointFile(0)
	{
	/* Initialize tool layout: */
	layout.setNumButtons(1);
	
	/* Insert class into class hierarchy: */
	ToolFactory* toolFactory=toolManager.loadClass("UtilityTool");
	toolFactory->addChildClass(this);
	addParentClass(toolFactory);
	
	/* Load class settings: */
	Misc::ConfigurationFileSection cfs=toolManager.getToolClassSection(getClassName());
	viewpointFileName=cfs.retrieveString("./viewpointFileName",viewpointFileName);
	axisLength=cfs.retrieveValue<Scalar>("./axisLength",axisLength);
	axisLineWidth=cfs.retrieveValue<float>("./axisLineWidth",axisLineWidth);
	
	/* Set tool class' factory pointer: */
	ViewpointSaverTool::factory=this;
	}

ViewpointSaverToolFactory::~ViewpointSaverToolFactory(void)
	{
	/* Close the viewpoint file: */
	delete viewpointFile;
	
	/* Reset tool class' factory pointer: */
	ViewpointSaverTool::factory=0;
	}

const char* ViewpointSaverToolFactory::getName(void) const
	{
	return "Viewpoint Recorder";
	}

const char* ViewpointSaverToolFactory::getButtonFunction(int) const
	{
	return "Save Viewpoint";
	}

Tool* ViewpointSaverToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new ViewpointSaverTool(this,inputAssignment);
	}

void ViewpointSaverToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveViewpointSaverToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("UtilityTool");
	}

extern "C" ToolFactory* createViewpointSaverToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	ViewpointSaverToolFactory* viewpointSaverToolFactory=new ViewpointSaverToolFactory(*toolManager);
	
	/* Return factory object: */
	return viewpointSaverToolFactory;
	}

extern "C" void destroyViewpointSaverToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/*******************************************
Static elements of class ViewpointSaverTool:
*******************************************/

ViewpointSaverToolFactory* ViewpointSaverTool::factory=0;

/***********************************
Methods of class ViewpointSaverTool:
***********************************/

ViewpointSaverTool::ViewpointSaverTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment)
	:UtilityTool(factory,inputAssignment)
	{
	}

const ToolFactory* ViewpointSaverTool::getFactory(void) const
	{
	return factory;
	}

void ViewpointSaverTool::buttonCallback(int,InputDevice::ButtonCallbackData* cbData)
	{
	if(cbData->newButtonState) // Activation button has just been pressed
		{
		/* Ensure that the viewpoint file is open: */
		if(factory->viewpointFile==0)
			{
			try
				{
				factory->viewpointFile=new Misc::File(factory->viewpointFileName.c_str(),"wt");
				}
			catch(Misc::File::OpenError)
				{
				/* Just don't open the file, then! */
				}
			}
		
		if(factory->viewpointFile!=0)
			{
			/* Write a time interval: */
			fprintf(factory->viewpointFile->getFilePtr(),"%f",1.0);
			
			/* Write the environment's center point in navigational coordinates: */
			Point center=getInverseNavigationTransformation().transform(getDisplayCenter());
			fprintf(factory->viewpointFile->getFilePtr()," (%f, %f, %f)",center[0],center[1],center[2]);
			
			/* Write the environment's size in navigational coordinates: */
			Scalar size=getDisplaySize()*getInverseNavigationTransformation().getScaling();
			fprintf(factory->viewpointFile->getFilePtr()," %f",size);
			
			/* Write the environment's forward direction in navigational coordinates: */
			Vector forward=getInverseNavigationTransformation().transform(getForwardDirection());
			fprintf(factory->viewpointFile->getFilePtr()," (%f, %f, %f)",forward[0],forward[1],forward[2]);
			
			/* Write the environment's up direction in navigational coordinates: */
			Vector up=getInverseNavigationTransformation().transform(getUpDirection());
			fprintf(factory->viewpointFile->getFilePtr()," (%f, %f, %f)\n",up[0],up[1],up[2]);
			}
		}
	}

void ViewpointSaverTool::display(GLContextData&) const
	{
	/* Show the environment's display center and forward and up directions: */
	glPushAttrib(GL_ENABLE_BIT|GL_LINE_BIT);
	glDisable(GL_LIGHTING);
	glLineWidth(factory->axisLineWidth);
	glBegin(GL_LINES);
	glColor3f(0.0f,1.0f,0.0f);
	glVertex(getDisplayCenter());
	glVertex(getDisplayCenter()+getForwardDirection()*factory->axisLength);
	glVertex(getDisplayCenter());
	glVertex(getDisplayCenter()+getUpDirection()*factory->axisLength);
	glEnd();
	glPopAttrib();
	}

}
