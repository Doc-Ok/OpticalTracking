/***********************************************************************
ToolKillZoneBox - Class for box-shaped "kill zones" for tools and input
devices.
Copyright (c) 2004-2014 Oliver Kreylos

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

#include <Vrui/Internal/ToolKillZoneBox.h>

#include <Misc/ConfigurationFile.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/Ray.h>
#include <Geometry/GeometryValueCoders.h>
#include <GL/gl.h>
#include <GL/GLModels.h>
#include <Vrui/InputDevice.h>

namespace Vrui {

/********************************
Methods of class ToolKillZoneBox:
********************************/

void ToolKillZoneBox::renderModel(void) const
	{
	GLfloat min[3],max[3];
	for(int i=0;i<3;++i)
		{
		min[i]=GLfloat(box.min[i]);
		max[i]=GLfloat(box.max[i]);
		}
	glDrawBox(min,max);
	}

ToolKillZoneBox::ToolKillZoneBox(const Misc::ConfigurationFileSection& configFileSection)
	:ToolKillZone(configFileSection)
	{
	/* Read box' center and size: */
	Point boxCenter=configFileSection.retrieveValue<Point>("./killZoneCenter");
	Vector boxSize=configFileSection.retrieveValue<Vector>("./killZoneSize");
	box=Box(boxCenter-boxSize*Scalar(0.5),boxCenter+boxSize*Scalar(0.5));
	}

Point ToolKillZoneBox::getCenter(void) const
	{
	/* Return the box's center point: */
	return box.getOrigin()+Vector(box.getSize())*Scalar(0.5);
	}

void ToolKillZoneBox::setCenter(const Point& newCenter)
	{
	/* Move the box to the new center position: */
	box.setOrigin(newCenter-Vector(box.getSize())*Scalar(0.5));
	
	/* Update the model: */
	updateModel();
	}

bool ToolKillZoneBox::isDeviceIn(const InputDevice* device) const
	{
	if(device->isRayDevice())
		{
		/* Check if the device's ray intersects the kill box: */
		Ray ray=device->getRay();
		if(baseDevice!=0)
			ray.inverseTransform(baseDevice->getTransformation());
		Box::HitResult hr=box.intersectRay(ray);
		return hr.isValid();
		}
	else
		{
		/* Check if the device's position is inside the kill box: */
		Point position=device->getPosition();
		if(baseDevice!=0)
			position=baseDevice->getTransformation().inverseTransform(position);
		return box.contains(position);
		}
	}

}
