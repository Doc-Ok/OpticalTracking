/***********************************************************************
EarthquakeTool - Vrui tool class to snap a virtual input device to
events in an earthquake data set.
Copyright (c) 2009-2013 Oliver Kreylos

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include "EarthquakeTool.h"

#include <Math/Constants.h>
#include <Geometry/Ray.h>
#include <Geometry/OrthogonalTransformation.h>
#include <Vrui/Vrui.h>
#include <Vrui/InputDevice.h>
#include <Vrui/ToolManager.h>
#include <Vrui/InputGraphManager.h>

#include "EarthquakeSet.h"

/**************************************
Methods of class EarthquakeToolFactory:
**************************************/

EarthquakeToolFactory::EarthquakeToolFactory(Vrui::ToolManager& toolManager,const std::vector<EarthquakeSet*>& sEarthquakeSets)
	:Vrui::ToolFactory("EarthquakeTool",toolManager),
	 earthquakeSets(sEarthquakeSets)
	{
	/* Insert class into class hierarchy: */
	Vrui::TransformToolFactory* transformToolFactory=dynamic_cast<Vrui::TransformToolFactory*>(toolManager.loadClass("TransformTool"));
	transformToolFactory->addChildClass(this);
	addParentClass(transformToolFactory);
	
	/* Initialize tool layout: */
	layout.setNumButtons(0,true);
	layout.setNumValuators(0,true);
	
	/* Set the custom tool class' factory pointer: */
	EarthquakeTool::factory=this;
	}

EarthquakeToolFactory::~EarthquakeToolFactory(void)
	{
	/* Reset the custom tool class' factory pointer: */
	EarthquakeTool::factory=0;
	}

const char* EarthquakeToolFactory::getName(void) const
	{
	return "Earthquake Projector";
	}

Vrui::Tool* EarthquakeToolFactory::createTool(const Vrui::ToolInputAssignment& inputAssignment) const
	{
	/* Create a new object of the custom tool class: */
	EarthquakeTool* newTool=new EarthquakeTool(this,inputAssignment);
	
	return newTool;
	}

void EarthquakeToolFactory::destroyTool(Vrui::Tool* tool) const
	{
	/* Cast the tool pointer to the Earthquake tool class (not really necessary): */
	EarthquakeTool* earthquakeTool=dynamic_cast<EarthquakeTool*>(tool);
	
	/* Destroy the tool: */
	delete earthquakeTool;
	}

/***************************************
Static elements of class EarthquakeTool:
***************************************/

EarthquakeToolFactory* EarthquakeTool::factory=0;

/*******************************
Methods of class EarthquakeTool:
*******************************/

EarthquakeTool::EarthquakeTool(const Vrui::ToolFactory* factory,const Vrui::ToolInputAssignment& inputAssignment)
	:Vrui::TransformTool(factory,inputAssignment),
	 lastRayParameter(0)
	{
	/* Set the source device: */
	if(input.getNumButtonSlots()>0)
		sourceDevice=getButtonDevice(0);
	else
		sourceDevice=getValuatorDevice(0);
	}

void EarthquakeTool::initialize(void)
	{
	/* Initialize the base tool: */
	TransformTool::initialize();
	
	/* Disable the transformed device's glyph: */
	Vrui::getInputGraphManager()->getInputDeviceGlyph(transformedDevice).disable();
	}

const Vrui::ToolFactory* EarthquakeTool::getFactory(void) const
	{
	return factory;
	}

void EarthquakeTool::frame(void)
	{
	const EarthquakeSet::Event* event=0;
	
	if(sourceDevice->is6DOFDevice())
		{
		/* Snap the device's position to the closest earthquake event: */
		EarthquakeSet::Point position=EarthquakeSet::Point(Vrui::getNavigationTransformation().inverseTransform(sourceDevice->getPosition()));
		float maxDistance=float(Vrui::getPointPickDistance());
		for(std::vector<EarthquakeSet*>::const_iterator eqsIt=factory->earthquakeSets.begin();eqsIt!=factory->earthquakeSets.end();++eqsIt)
			{
			const EarthquakeSet::Event* e=(*eqsIt)->selectEvent(position,maxDistance);
			if(e!=0)
				{
				event=e;
				maxDistance=Geometry::dist(position,event->position);
				}
			}
		}
	else
		{
		/* Snap the device's position to the closest earthquake event along a ray: */
		Vrui::Ray ray=sourceDevice->getRay();
		ray.transform(Vrui::getInverseNavigationTransformation());
		Vrui::Scalar rayLength=Geometry::mag(ray.getDirection());
		ray.normalizeDirection();
		EarthquakeSet::Ray eqRay(ray);
		Vrui::Scalar rayParameter=Math::Constants<Vrui::Scalar>::max;
		for(std::vector<EarthquakeSet*>::const_iterator eqsIt=factory->earthquakeSets.begin();eqsIt!=factory->earthquakeSets.end();++eqsIt)
			{
			const EarthquakeSet::Event* e=(*eqsIt)->selectEvent(ray,float(Vrui::getRayPickCosine()));
			if(e!=0)
				{
				/* Calculate the test event's ray parameter: */
				Vrui::Scalar rp=((Vrui::Point(e->position)-ray.getOrigin())*ray.getDirection())/rayLength;
				if(rayParameter>rp)
					{
					event=e;
					rayParameter=rp;
					}
				}
			}
		
		if(event!=0)
			{
			/* Update the last ray parameter: */
			lastRayParameter=rayParameter;
			}
		}
	
	transformedDevice->setDeviceRay(sourceDevice->getDeviceRayDirection(),Vrui::Scalar(0));
	if(event!=0)
		{
		/* Set the virtual device to the event's position: */
		Vrui::Point eventPos=Vrui::Point(event->position);
		Vrui::TrackerState ts(Vrui::getNavigationTransformation().transform(eventPos)-Vrui::Point::origin,sourceDevice->getOrientation());
		transformedDevice->setTransformation(ts);
		}
	else if(sourceDevice->is6DOFDevice())
		transformedDevice->setTransformation(sourceDevice->getTransformation());
	else
		{
		/* Position the virtual device at the same ray parameter as the last successful intersection: */
		Vrui::Point pos=sourceDevice->getRay()(lastRayParameter);
		Vrui::TrackerState ts(pos-Vrui::Point::origin,sourceDevice->getOrientation());
		transformedDevice->setTransformation(ts);
		}
	}
