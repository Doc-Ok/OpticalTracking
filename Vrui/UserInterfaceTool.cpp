/***********************************************************************
UserInterfaceTool - Base class for tools related to user interfaces
(interaction with dialog boxes, context menus, virtual input devices).
Copyright (c) 2008-2014 Oliver Kreylos

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

#include <Vrui/UserInterfaceTool.h>

#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Geometry/OrthonormalTransformation.h>
#include <GL/GLValueCoders.h>
#include <Vrui/Vrui.h>
#include <Vrui/Viewer.h>
#include <Vrui/VRScreen.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/********************************************************
Methods of class UserInterfaceToolFactory::Configuration:
********************************************************/

UserInterfaceToolFactory::Configuration::Configuration(void)
	:useEyeRay(false),
	 rayOffset(getUiSize()*Scalar(2)),
	 drawRay(true),rayColor(1.0f,0.0f,0.0f),rayWidth(3.0f)
	{
	}

void UserInterfaceToolFactory::Configuration::read(const Misc::ConfigurationFileSection& cfs)
	{
	useEyeRay=cfs.retrieveValue<bool>("./useEyeRay",useEyeRay);
	if(useEyeRay)
		{
		/* The default is not to draw the interaction ray when eye rays are used: */
		drawRay=false;
		}
	rayOffset=cfs.retrieveValue<Scalar>("./rayOffset",rayOffset);
	drawRay=cfs.retrieveValue<bool>("./drawRay",drawRay);
	rayColor=cfs.retrieveValue<GLColor<GLfloat,4> >("./rayColor",rayColor);
	rayWidth=cfs.retrieveValue<GLfloat>("./rayWidth",rayWidth);
	}

void UserInterfaceToolFactory::Configuration::write(Misc::ConfigurationFileSection& cfs) const
	{
	cfs.storeValue<bool>("./useEyeRay",useEyeRay);
	cfs.storeValue<Scalar>("./rayOffset",rayOffset);
	cfs.storeValue<bool>("./drawRay",drawRay);
	cfs.storeValue<GLColor<GLfloat,4> >("./rayColor",rayColor);
	cfs.storeValue<GLfloat>("./rayWidth",rayWidth);
	}

/*****************************************
Methods of class UserInterfaceToolFactory:
*****************************************/

UserInterfaceToolFactory::UserInterfaceToolFactory(ToolManager& toolManager)
	:ToolFactory("UserInterfaceTool",toolManager)
	{
	#if 0
	/* Insert class into class hierarchy: */
	ToolFactory* toolFactory=toolManager.loadClass("Tool");
	toolFactory->addChildClass(this);
	addParentClass(toolFactory);
	#endif
	
	/* Load class settings: */
	Misc::ConfigurationFileSection cfs=toolManager.getToolClassSection(getClassName());
	configuration.read(cfs);
	
	/* Set tool class' factory pointer: */
	UserInterfaceTool::factory=this;
	}

UserInterfaceToolFactory::~UserInterfaceToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	UserInterfaceTool::factory=0;
	}

const char* UserInterfaceToolFactory::getName(void) const
	{
	return "User Interface";
	}

/******************************************
Static elements of class UserInterfaceTool:
******************************************/

UserInterfaceToolFactory* UserInterfaceTool::factory=0;

/**********************************
Methods of class UserInterfaceTool:
**********************************/

Ray UserInterfaceTool::calcInteractionRay(void) const
	{
	Ray result;
	
	if(configuration.useEyeRay)
		{
		/* Shoot a ray from the main viewer: */
		Point start=getMainViewer()->getHeadPosition();
		Vector direction=interactionDevice->getPosition()-start;
		direction.normalize();
		result=Ray(start,direction);
		}
	else
		{
		/* Use the device's ray direction: */
		result=interactionDevice->getRay();
		
		/* Offset the ray start point backwards: */
		result.setOrigin(result.getOrigin()-result.getDirection()*(configuration.rayOffset/result.getDirection().mag()));
		}
	
	return result;
	}

Point UserInterfaceTool::calcRayPoint(const Ray& ray) const
	{
	/* Intersect the given ray with the UI plane: */
	Point center=getUiPlane().getOrigin();
	Vector normal=getUiPlane().getDirection(2);
	Scalar denom=normal*ray.getDirection();
	if(denom!=Scalar(0))
		{
		/* Return the intersection point: */
		return ray(((center-ray.getOrigin())*normal)/denom);
		}
	else
		{
		/* Return the projection of the ray's origin into the plane: */
		return ray.getOrigin()+normal*(center-ray.getOrigin())*normal;
		}
	}

ONTransform UserInterfaceTool::calcRayTransform(const Ray& ray) const
	{
	/* Intersect the given ray with the UI plane: */
	ONTransform result=getUiPlane();
	Point center=result.getOrigin();
	Vector normal=result.getDirection(2);
	Scalar denom=normal*ray.getDirection();
	Point newCenter;
	if(denom!=Scalar(0))
		{
		/* Center the result transformation at the intersection point: */
		newCenter=ray(((center-ray.getOrigin())*normal)/denom);
		}
	else
		{
		/* Center the result transformation at the projection of the ray's origin into the plane: */
		newCenter=ray.getOrigin()+normal*(center-ray.getOrigin())*normal;
		}
	
	result.getTranslation()=newCenter-Point::origin;
	return result;
	}

UserInterfaceTool::UserInterfaceTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment)
	:Tool(factory,inputAssignment),
	 configuration(UserInterfaceTool::factory->configuration),
	 interactionDevice(0)
	{
	}

void UserInterfaceTool::configure(const Misc::ConfigurationFileSection& configFileSection)
	{
	/* Override private configuration data from given configuration file section: */
	configuration.read(configFileSection);
	}

void UserInterfaceTool::storeState(Misc::ConfigurationFileSection& configFileSection) const
	{
	/* Write private configuration data to given configuration file section: */
	configuration.write(configFileSection);
	}

}
