/***********************************************************************
TrackballNavigationTool - Class for trackball navigation in 3D space.
Copyright (c) 2004-2013 Oliver Kreylos

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

#include <Vrui/Tools/TrackballNavigationTool.h>

#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Math/Math.h>
#include <Geometry/Vector.h>
#include <Geometry/OrthogonalTransformation.h>
#include <Vrui/Vrui.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/***********************************************
Methods of class TrackballNavigationToolFactory:
***********************************************/

TrackballNavigationToolFactory::TrackballNavigationToolFactory(ToolManager& toolManager)
	:ToolFactory("TrackballNavigationTool",toolManager),
	 rotateFactor(1)
	{
	/* Initialize tool layout: */
	layout.setNumButtons(1);
	
	/* Insert class into class hierarchy: */
	ToolFactory* navigationToolFactory=toolManager.loadClass("NavigationTool");
	navigationToolFactory->addChildClass(this);
	addParentClass(navigationToolFactory);
	
	/* Load class settings: */
	Misc::ConfigurationFileSection cfs=toolManager.getToolClassSection(getClassName());
	rotateFactor=cfs.retrieveValue<Scalar>("./rotateFactor",rotateFactor);
	
	/* Set tool class' factory pointer: */
	TrackballNavigationTool::factory=this;
	}

TrackballNavigationToolFactory::~TrackballNavigationToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	TrackballNavigationTool::factory=0;
	}

const char* TrackballNavigationToolFactory::getName(void) const
	{
	return "Ray-Based Trackball";
	}

const char* TrackballNavigationToolFactory::getButtonFunction(int) const
	{
	return "Grab Sphere";
	}

Tool* TrackballNavigationToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new TrackballNavigationTool(this,inputAssignment);
	}

void TrackballNavigationToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveTrackballNavigationToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("NavigationTool");
	}

extern "C" ToolFactory* createTrackballNavigationToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	TrackballNavigationToolFactory* trackballNavigationToolFactory=new TrackballNavigationToolFactory(*toolManager);
	
	/* Return factory object: */
	return trackballNavigationToolFactory;
	}

extern "C" void destroyTrackballNavigationToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/************************************************
Static elements of class TrackballNavigationTool:
************************************************/

TrackballNavigationToolFactory* TrackballNavigationTool::factory=0;

/****************************************
Methods of class TrackballNavigationTool:
****************************************/

Point TrackballNavigationTool::calcTrackballPosition(void) const
	{
	/* Get device ray equation: */
	Ray ray=getButtonDeviceRay(0);
	
	/* Intersect ray with trackball sphere: */
	Vector d=getDisplayCenter()-ray.getOrigin();
	Scalar dLen2=Geometry::sqr(d);
	Scalar ph=ray.getDirection()*d;
	Scalar radius=getDisplaySize();
	Scalar det=Math::sqr(ph)+Math::sqr(radius)-dLen2;
	if(det>=Scalar(0))
		{
		/* Find first intersection of ray with sphere (even if behind start point): */
		det=Math::sqrt(det);
		Scalar lambda=ph-det;
		return ray(lambda);
		}
	else
		{
		/* Find closest point on sphere to ray: */
		Vector ctop=ray.getDirection()*((d*ray.getDirection())/Geometry::sqr(ray.getDirection()))-d;
		ctop*=radius/Geometry::mag(ctop);
		return getDisplayCenter()+ctop;
		}
	}

TrackballNavigationTool::TrackballNavigationTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment)
	:NavigationTool(factory,inputAssignment)
	{
	}

const ToolFactory* TrackballNavigationTool::getFactory(void) const
	{
	return factory;
	}

void TrackballNavigationTool::buttonCallback(int,InputDevice::ButtonCallbackData* cbData)
	{
	if(cbData->newButtonState) // Button has just been pressed
		{
		/* Try activating the tool: */
		if(!isActive()&&activate())
			{
			/* Calculate interaction point: */
			lastDragPosition=calcTrackballPosition();
			}
		}
	else // Button has just been released
		{
		/* Deactivate this tool: */
		deactivate();
		}
	}

void TrackballNavigationTool::frame(void)
	{
	/* Act depending on this tool's current state: */
	if(isActive())
		{
		/* Calculate new interaction position: */
		Point dragPosition=calcTrackballPosition();
		
		/* Calculate incremental transformation from old to new point: */
		Vector v1=lastDragPosition-getDisplayCenter();
		Vector v2=dragPosition-getDisplayCenter();
		Vector axis=v1^v2;
		Scalar axisLen=Geometry::mag(axis);
		if(axisLen!=Scalar(0))
			{
			/* Calculate rotation angle: */
			axis/=axisLen;
			Scalar angle=Math::acos((v1*v2)/(Geometry::mag(v1)*Geometry::mag(v2)))*factory->rotateFactor;
			
			/* Compose new navigation transformation: */
			NavTrackerState navigation=NavTrackerState::rotateAround(getDisplayCenter(),NavTrackerState::Rotation::rotateAxis(axis,angle));
			navigation*=getNavigationTransformation();
			
			/* Update Vrui's navigation transformation: */
			setNavigationTransformation(navigation);
			}
		
		lastDragPosition=dragPosition;
		}
	}

}
