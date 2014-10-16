/***********************************************************************
ForceJumpNavigationTool - Class to navigate by jumping from the viewer's
current position to the position of an input device.
Copyright (c) 2010-2013 Oliver Kreylos

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

#include <Vrui/Tools/ForceJumpNavigationTool.h>

#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Math/Math.h>
#include <Geometry/Point.h>
#include <Geometry/Plane.h>
#include <Vrui/Vrui.h>
#include <Vrui/ToolManager.h>
#include <Vrui/Viewer.h>

namespace Vrui {

/***********************************************
Methods of class ForceJumpNavigationToolFactory:
***********************************************/

ForceJumpNavigationToolFactory::ForceJumpNavigationToolFactory(ToolManager& toolManager)
	:ToolFactory("ForceJumpNavigationTool",toolManager),
	 gravity(getMeterFactor()*Scalar(9.81)),
	 jumpTime(1)
	{
	/* Initialize tool layout: */
	layout.setNumButtons(1);
	
	/* Insert class into class hierarchy: */
	ToolFactory* navigationToolFactory=toolManager.loadClass("NavigationTool");
	navigationToolFactory->addChildClass(this);
	addParentClass(navigationToolFactory);
	
	/* Load class settings: */
	Misc::ConfigurationFileSection cfs=toolManager.getToolClassSection(getClassName());
	gravity=cfs.retrieveValue<Scalar>("./gravity",gravity);
	jumpTime=cfs.retrieveValue<Scalar>("./jumpTime",jumpTime);
	
	/* Set tool class' factory pointer: */
	ForceJumpNavigationTool::factory=this;
	}

ForceJumpNavigationToolFactory::~ForceJumpNavigationToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	ForceJumpNavigationTool::factory=0;
	}

const char* ForceJumpNavigationToolFactory::getName(void) const
	{
	return "Force Jump";
	}

const char* ForceJumpNavigationToolFactory::getButtonFunction(int) const
	{
	return "Jump";
	}

Tool* ForceJumpNavigationToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new ForceJumpNavigationTool(this,inputAssignment);
	}

void ForceJumpNavigationToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveForceJumpNavigationToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("NavigationTool");
	}

extern "C" ToolFactory* createForceJumpNavigationToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	ForceJumpNavigationToolFactory* forceJumpNavigationToolFactory=new ForceJumpNavigationToolFactory(*toolManager);
	
	/* Return factory object: */
	return forceJumpNavigationToolFactory;
	}

extern "C" void destroyForceJumpNavigationToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/************************************************
Static elements of class ForceJumpNavigationTool:
************************************************/

ForceJumpNavigationToolFactory* ForceJumpNavigationTool::factory=0;

/****************************************
Methods of class ForceJumpNavigationTool:
****************************************/

ForceJumpNavigationTool::ForceJumpNavigationTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment)
	:NavigationTool(factory,inputAssignment)
	{
	}

const ToolFactory* ForceJumpNavigationTool::getFactory(void) const
	{
	return factory;
	}

void ForceJumpNavigationTool::buttonCallback(int,InputDevice::ButtonCallbackData* cbData)
	{
	if(cbData->newButtonState) // Button has just been pressed
		{
		if(!isActive()&&activate())
			{
			/* Get the start navigation transformation: */
			startNav=getNavigationTransformation();
			startTime=getApplicationTime();
			
			/* Find the initial foot position: */
			Point startPos;
			{
			const Vector& normal=getFloorPlane().getNormal();
			Scalar lambda=(getFloorPlane().getOffset()-getMainViewer()->getHeadPosition()*normal)/(getUpDirection()*normal);
			startPos=getMainViewer()->getHeadPosition()+getUpDirection()*lambda;
			}
			
			/* Get the final foot position: */
			Point endPos=getButtonDevicePosition(0);
			endTime=startTime+factory->jumpTime;
			
			/* Calculate the complete translation vector: */
			translation=endPos-startPos;
			
			/* Calculate the elevation difference: */
			Scalar deltaZ=(translation*getUpDirection())/Geometry::mag(getUpDirection());
			
			/* Calculate the initial velocity vector: */
			initialVelocity=translation;
			
			/* Project the translation vector to the horizontal: */
			initialVelocity-=getUpDirection()*(deltaZ/Geometry::mag(getUpDirection()));
			/* Add the vertical jumping component: */
			initialVelocity/=factory->jumpTime;
			initialVelocity+=getUpDirection()*(deltaZ/factory->jumpTime+Scalar(0.5)*factory->gravity*factory->jumpTime);
			
			/* Calculate the flip rotation axis: */
			flipAxis=translation^getUpDirection();
			flipAxis.normalize();
			}
		}
	}

void ForceJumpNavigationTool::frame(void)
	{
	/* Act depending on this tool's current state: */
	if(isActive())
		{
		/* Get the current application time: */
		double time=getApplicationTime();
		
		if(time>=endTime)
			{
			/* Set the final navigation transformation: */
			NavTransform nav=NavTransform::translate(-translation);
			nav*=startNav;
			setNavigationTransformation(nav);
			
			/* Deactivate this tool: */
			deactivate();
			}
		else
			{
			/* Compute and set the intermediate navigation transformation: */
			double deltaTime=time-startTime;
			Vector t=initialVelocity*deltaTime-getUpDirection()*(Scalar(0.5)*factory->gravity*Math::sqr(deltaTime)/Geometry::mag(getUpDirection()));
			
			/* Find the current body center: */
			Point bodyCenter;
			{
			const Vector& normal=getFloorPlane().getNormal();
			Scalar lambda=(getFloorPlane().getOffset()-getMainViewer()->getHeadPosition()*normal)/(getUpDirection()*normal);
			Point foot=getMainViewer()->getHeadPosition()+getUpDirection()*lambda;
			bodyCenter=Geometry::mid(getMainViewer()->getHeadPosition(),foot);
			}
			
			NavTransform nav=NavTransform::rotateAround(bodyCenter,Rotation::rotateScaledAxis(flipAxis*(Scalar(2)*Math::Constants<Scalar>::pi*Scalar(deltaTime)/Scalar(factory->jumpTime))));
			nav*=NavTransform::translate(-t);
			nav*=startNav;
			setNavigationTransformation(nav);
			
			/* Request another frame: */
			scheduleUpdate(getApplicationTime()+1.0/125.0);
			}
		}
	}

}
