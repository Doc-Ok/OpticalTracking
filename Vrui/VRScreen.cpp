/***********************************************************************
VRScreen - Class for display screens (fixed and head-mounted) in VR
environments.
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

#include <Vrui/VRScreen.h>

#include <Misc/ThrowStdErr.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Math/Math.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/Ray.h>
#include <Geometry/GeometryValueCoders.h>
#include <GL/gl.h>
#include <GL/GLMatrixTemplates.h>
#include <GL/GLTransformationWrappers.h>
#include <Vrui/InputDevice.h>
#include <Vrui/Vrui.h>

namespace Vrui {

/*************************
Methods of class VRScreen:
*************************/

VRScreen::VRScreen(void)
	:screenName(0),
	 deviceMounted(false),device(0),
	 transform(ONTransform::identity),inverseTransform(ONTransform::identity),
	 offAxis(false),screenHomography(PTransform2::identity),inverseClipHomography(PTransform::identity),
	 intersect(true)
	{
	screenSize[0]=screenSize[1]=Scalar(0);
	}

VRScreen::~VRScreen(void)
	{
	delete[] screenName;
	}

void VRScreen::initialize(const Misc::ConfigurationFileSection& configFileSection)
	{
	/* Read the screen's name: */
	std::string name=configFileSection.retrieveString("./name");
	screenName=new char[name.size()+1];
	strcpy(screenName,name.c_str());
	
	/* Determine whether screen is device-mounted: */
	deviceMounted=configFileSection.retrieveValue<bool>("./deviceMounted",false);
	if(deviceMounted)
		{
		/* Retrieve the input device this screen is attached to: */
		std::string deviceName=configFileSection.retrieveString("./deviceName");
		device=findInputDevice(deviceName.c_str());
		if(device==0)
			Misc::throwStdErr("VRScreen: Mounting device \"%s\" not found",deviceName.c_str());
		}
	
	/* Retrieve screen position/orientation in physical or device coordinates: */
	try
		{
		/* Try reading the screen transformation directly: */
		transform=configFileSection.retrieveValue<ONTransform>("./transform");
		}
	catch(std::runtime_error)
		{
		/* Fall back to reading the screen's origin and axis directions: */
		Point origin=configFileSection.retrieveValue<Point>("./origin");
		Vector horizontalAxis=configFileSection.retrieveValue<Vector>("./horizontalAxis");
		Vector verticalAxis=configFileSection.retrieveValue<Vector>("./verticalAxis");
		ONTransform::Rotation rot=ONTransform::Rotation::fromBaseVectors(horizontalAxis,verticalAxis);
		transform=ONTransform(origin-Point::origin,rot);
		}
	
	/* Read the screen's size: */
	screenSize[0]=configFileSection.retrieveValue<Scalar>("./width");
	screenSize[1]=configFileSection.retrieveValue<Scalar>("./height");
	
	/* Apply a rotation around a single axis: */
	Point rotateCenter=configFileSection.retrieveValue<Point>("./rotateCenter",Point::origin);
	Vector rotateAxis=configFileSection.retrieveValue<Vector>("./rotateAxis",Vector(1,0,0));
	Scalar rotateAngle=configFileSection.retrieveValue<Scalar>("./rotateAngle",Scalar(0));
	if(rotateAngle!=Scalar(0))
		{
		ONTransform screenRotation=ONTransform::translateFromOriginTo(rotateCenter);
		screenRotation*=ONTransform::rotate(ONTransform::Rotation::rotateAxis(rotateAxis,Math::rad(rotateAngle)));
		screenRotation*=ONTransform::translateToOriginFrom(rotateCenter);
		transform.leftMultiply(screenRotation);
		}
	
	/* Apply an arbitrary pre-transformation: */
	ONTransform preTransform=configFileSection.retrieveValue<ONTransform>("./preTransform",ONTransform::identity);
	transform.leftMultiply(preTransform);
	
	/* Finalize the screen transformation: */
	transform.renormalize();
	inverseTransform=Geometry::invert(transform);
	
	/* Check if the screen is projected off-axis: */
	offAxis=configFileSection.retrieveValue<bool>("./offAxis",offAxis);
	if(offAxis)
		{
		/* Create the inverse of the 2D homography from clip space to rectified screen space in screen coordinates: */
		PTransform2 sHomInv=PTransform2::identity;
		sHomInv.getMatrix()(0,0)=Scalar(2)/screenSize[0];
		sHomInv.getMatrix()(0,2)=Scalar(-1);
		sHomInv.getMatrix()(1,1)=Scalar(2)/screenSize[1];
		sHomInv.getMatrix()(1,2)=Scalar(-1);
		sHomInv.getMatrix()(2,2)=Scalar(1);
		
		/* Retrieve the 2D homography from clip space to projected screen space in screen coordinates: */
		PTransform2 pHom=configFileSection.retrieveValue<PTransform2>("./homography");
		
		/* Calculate the screen space homography: */
		screenHomography=pHom*sHomInv;
		
		/* Calculate the clip space homography: */
		PTransform2 hom=sHomInv*pHom;
		for(int i=0;i<3;++i)
			for(int j=0;j<3;++j)
				inverseClipHomography.getMatrix()(i<2?i:3,j<2?j:3)=hom.getMatrix()(i,j);
		
		/* Put in correction factors to keep the frustum's far plane in the same position: */
		inverseClipHomography.getMatrix()(2,0)=inverseClipHomography.getMatrix()(3,0);
		inverseClipHomography.getMatrix()(2,1)=inverseClipHomography.getMatrix()(3,1);
		
		inverseClipHomography.doInvert();
		}
	
	/* Read the intersect flag: */
	intersect=configFileSection.retrieveValue<bool>("./intersect",intersect);
	}

void VRScreen::attachToDevice(const InputDevice* newDevice)
	{
	/* Set the device to which the screen is mounted, and update the mounted flag: */
	deviceMounted=newDevice!=0;
	device=newDevice;
	}

void VRScreen::setSize(Scalar newWidth,Scalar newHeight)
	{
	/* Adjust the screen's origin in its own coordinate system: */
	transform*=ONTransform::translate(Vector(Math::div2(screenSize[0]-newWidth),Math::div2(screenSize[1]-newHeight),0));
	inverseTransform=Geometry::invert(transform);
	
	/* Adjust the screen's sizes: */
	screenSize[0]=newWidth;
	screenSize[1]=newHeight;
	}

void VRScreen::setTransform(const ONTransform& newTransform)
	{
	/* Update the screen-to-physical/device transformation and its inverse: */
	transform=newTransform;
	inverseTransform=Geometry::invert(transform);
	}

ONTransform VRScreen::getScreenTransformation(void) const
	{
	ONTransform result=transform;
	if(deviceMounted)
		result.leftMultiply(device->getTransformation());
	return result;
	}

void VRScreen::setScreenTransform(void) const
	{
	/* Save the current matrix mode: */
	glPushAttrib(GL_TRANSFORM_BIT);
	
	/* Save the modelview matrix: */
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	
	/* Modify the modelview matrix: */
	if(deviceMounted)
		glMultMatrix(device->getTransformation());
	glMultMatrix(transform);
	
	/* Restore the current matrix mode: */
	glPopAttrib();
	}

void VRScreen::resetScreenTransform(void) const
	{
	/* Save the current matrix mode: */
	glPushAttrib(GL_TRANSFORM_BIT);
	
	/* Restore the modelview matrix: */
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	
	/* Restore the current matrix mode: */
	glPopAttrib();
	}

}
