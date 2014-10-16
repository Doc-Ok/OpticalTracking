/***********************************************************************
ShowLEDs - Vrui application to extract and visualize 3D LED positions
and emission directions downloaded from the firmware of an Oculus Rift
DK2.
Copyright (c) 2014 Oliver Kreylos

This file is part of the optical/inertial sensor fusion tracking
package.

The optical/inertial sensor fusion tracking package is free software;
you can redistribute it and/or modify it under the terms of the GNU
General Public License as published by the Free Software Foundation;
either version 2 of the License, or (at your option) any later version.

The optical/inertial sensor fusion tracking package is distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the optical/inertial sensor fusion tracking package; if not, write
to the Free Software Foundation, Inc., 59 Temple Place, Suite 330,
Boston, MA 02111-1307 USA
***********************************************************************/

#include <iostream>
#include <RawHID/BusType.h>
#include <RawHID/Device.h>
#include <Geometry/Point.h>
#include <Geometry/Ray.h>
#include <Geometry/Sphere.h>
#include <Geometry/OutputOperators.h>
#include <GL/gl.h>
#include <GL/GLModels.h>
#include <GL/GLNumberRenderer.h>
#include <GL/GLGeometryWrappers.h>
#include <GL/GLTransformationWrappers.h>
#include <Vrui/Vrui.h>
#include <Vrui/Application.h>

#include "HMDModel.h"

class ShowLEDs:public Vrui::Application
	{
	/* Embedded classes: */
	private:
	HMDModel hmdModel; // The HMD model
	GLNumberRenderer numberRenderer; // A helper object to render marker IDs
	
	/* Constructors and destructors: */
	public:
	ShowLEDs(int& argc,char**& argv);
	
	/* Methods from Vrui::Application: */
	virtual void display(GLContextData& contextData) const;
	};

/*************************
Methods of class ShowLEDs:
*************************/

namespace {

/*************************************************
Helper class to enumerate Oculus Rift DK2 devices:
*************************************************/

class OculusRiftDK2Matcher:public RawHID::Device::DeviceMatcher
	{
	/* Methods from RawHID::Device::DeviceMatcher: */
	public:
	virtual bool operator()(int busType,unsigned short vendorId,unsigned short productId) const
		{
		return busType==RawHID::BUSTYPE_USB&&vendorId==0x2833U&&productId==0x0021U;
		}
	};

}

ShowLEDs::ShowLEDs(int& argc,char**& argv)
	:Vrui::Application(argc,argv),
	 numberRenderer(0.0035,false)
	{
	/* Open the first Oculus Rift DK2: */
	RawHID::Device rift(OculusRiftDK2Matcher(),0);
	
	/* Initialize the HMD model: */
	hmdModel.readFromRiftDK2(rift);
	
	for(unsigned int i=0;i<hmdModel.getNumMarkers();++i)
		std::cout<<i<<": "<<hmdModel.getMarkerPos(i)<<std::endl;
	}

void ShowLEDs::display(GLContextData& contextData) const
	{
	glPushAttrib(GL_ENABLE_BIT|GL_POINT_BIT);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT,GL_AMBIENT_AND_DIFFUSE);
	
	/* Draw the IMU as a sphere: */
	glPushMatrix();
	glTranslate(hmdModel.getIMU()-HMDModel::Point::origin);
	glColor3f(0.5f,0.5f,1.0f);
	glDrawSphereIcosahedron(0.005,4);
	glPopMatrix();
	
	/* Draw the markers as spheres: */
	for(unsigned int markerIndex=0;markerIndex<hmdModel.getNumMarkers();++markerIndex)
		{
		glPushMatrix();
		glTranslate(hmdModel.getMarkerPos(markerIndex)-HMDModel::Point::origin);
		glColor3f(1.0f,0.5f,0.5f);
		glDrawSphereIcosahedron(0.005,4);
		glPopMatrix();
		}
	
	glDisable(GL_LIGHTING);
	glPointSize(3.0f);
	glLineWidth(3.0f);
	
	/* Draw the markers emission direction vectors: */
	glColor3f(0.5f,1.0f,5.0f);
	glBegin(GL_LINES);
	for(unsigned int markerIndex=0;markerIndex<hmdModel.getNumMarkers();++markerIndex)
		{
		glVertex(hmdModel.getMarkerPos(markerIndex));
		glVertex(hmdModel.getMarkerPos(markerIndex)+hmdModel.getMarkerDir(markerIndex)*HMDModel::Scalar(0.03));
		}
	glEnd();
	
	/* Draw the markers' IDs: */
	glLineWidth(2.0f);
	glColor3f(1.0f,1.0f,1.0f);
	HMDModel::Point head=HMDModel::Point(Vrui::getHeadPosition());
	for(unsigned int markerIndex=0;markerIndex<hmdModel.getNumMarkers();++markerIndex)
		{
		/* Intersect a ray from the head position to the marker position with the marker sphere: */
		Geometry::Sphere<HMDModel::Scalar,3> marker(hmdModel.getMarkerPos(markerIndex),HMDModel::Scalar(0.0075));
		Geometry::Ray<HMDModel::Scalar,3> ray(head,marker.getCenter()-head);
		Geometry::Sphere<HMDModel::Scalar,3>::HitResult hr=marker.intersectRay(ray);
		HMDModel::Point labelPos=ray(hr.getParameter());
		glPushMatrix();
		Vrui::Vector up=Vrui::getInverseNavigationTransformation().transform(Vrui::getUpDirection());
		Vrui::Vector x=Vrui::Vector(ray.getDirection())^up;
		glMultMatrix(Vrui::ONTransform::rotateAround(labelPos,Vrui::Rotation::fromBaseVectors(x,up)));
		numberRenderer.drawNumber(GLNumberRenderer::Vector(labelPos.getComponents()),markerIndex,contextData,0,0);
		glPopMatrix();
		}
	
	glPopAttrib();
	}

VRUI_APPLICATION_RUN(ShowLEDs)
