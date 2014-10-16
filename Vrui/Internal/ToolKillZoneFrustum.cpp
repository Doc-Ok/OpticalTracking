/***********************************************************************
ToolKillZoneFrustum - Class for "kill zones" for tools and input devices
that use projection onto a plane to detect whether tools are inside the
zone.
Copyright (c) 2007-2014 Oliver Kreylos

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

#include <Vrui/Internal/ToolKillZoneFrustum.h>

#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Misc/ThrowStdErr.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/Ray.h>
#include <Geometry/GeometryValueCoders.h>
#include <GL/gl.h>
#include <GL/GLGeometryWrappers.h>
#include <GL/GLTransformationWrappers.h>
#include <Vrui/InputDevice.h>
#include <Vrui/Viewer.h>
#include <Vrui/VRScreen.h>
#include <Vrui/Vrui.h>

namespace Vrui {

/************************************
Methods of class ToolKillZoneFrustum:
************************************/

void ToolKillZoneFrustum::renderModel(void) const
	{
	/* Draw a screen-aligned quad: */
	glPushMatrix();
	glMultMatrix(screen->getScreenTransformation());
	glBegin(GL_QUADS);
	glNormal3f(0.0f,0.0f,1.0f);
	glVertex(box.min[0],box.min[1]);
	glVertex(box.max[0],box.min[1]);
	glVertex(box.max[0],box.max[1]);
	glVertex(box.min[0],box.max[1]);
	glEnd();
	glPopMatrix();
	}

ToolKillZoneFrustum::ToolKillZoneFrustum(const Misc::ConfigurationFileSection& configFileSection)
	:ToolKillZone(configFileSection),
	 viewer(0),screen(0)
	{
	/* Get the viewer: */
	std::string viewerName=configFileSection.retrieveString("./killZoneViewerName");
	viewer=findViewer(viewerName.c_str());
	if(viewer==0)
		Misc::throwStdErr("ToolKillZoneFrustum::ToolKillZoneFrustum: Viewer %s not found",viewerName.c_str());
	
	/* Get the screen: */
	std::string screenName=configFileSection.retrieveString("./killZoneScreenName");
	screen=findScreen(screenName.c_str());
	if(screen==0)
		Misc::throwStdErr("ToolKillZoneFrustum::ToolKillZoneFrustum: Screen %s not found",screenName.c_str());
	
	/* Read box' center and size: */
	Point boxCenter=configFileSection.retrieveValue<Point>("./killZoneCenter");
	Vector boxSize=configFileSection.retrieveValue<Vector>("./killZoneSize");
	
	/* Transform box center and size to screen coordinates: */
	ONTransform screenT=screen->getScreenTransformation();
	boxCenter=screenT.inverseTransform(boxCenter);
	boxCenter[2]=Scalar(0);
	boxSize=screenT.inverseTransform(boxSize);
	boxSize[2]=Scalar(0);
	box=Box(boxCenter-boxSize*Scalar(0.5),boxCenter+boxSize*Scalar(0.5));
	}

Size ToolKillZoneFrustum::getSize(void) const
	{
	return screen->getScreenTransformation().transform(Vector(box.getSize()));
	}

Point ToolKillZoneFrustum::getCenter(void) const
	{
	/* Return the kill zone's center position in physical coordinates: */
	ONTransform screenT=screen->getScreenTransformation();
	return screenT.transform(box.getOrigin()+Vector(box.getSize())*Scalar(0.5));
	}

void ToolKillZoneFrustum::setCenter(const Point& newCenter)
	{
	/* Transform the new center to screen coordinates: */
	ONTransform screenT=screen->getScreenTransformation();
	Point newBoxCenter=screenT.inverseTransform(newCenter);
	newBoxCenter[2]=Scalar(0);
	
	/* Move the box to the new center position: */
	box.setOrigin(newBoxCenter-Vector(box.getSize())*Scalar(0.5));
	
	/* Update the model: */
	updateModel();
	}

bool ToolKillZoneFrustum::isDeviceIn(const InputDevice* device) const
	{
	/* Project the device's position into the screen plane: */
	ONTransform screenT=screen->getScreenTransformation();
	Point screenHead=screenT.inverseTransform(viewer->getHeadPosition());
	Point screenDevice=screenT.inverseTransform(device->getPosition());
	Vector d=screenDevice-screenHead;
	
	if(d[2]!=Scalar(0))
		{
		Scalar lambda=-screenHead[2]/d[2];
		Point p=screenHead+d*lambda;
		p[2]=Scalar(0);
		return box.contains(p);
		}
	else
		return false; 
	}

}
