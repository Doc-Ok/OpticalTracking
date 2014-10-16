/***********************************************************************
SurfaceNavigationTool - Base class for navigation tools that are limited
to navigate along an application-defined surface.
Copyright (c) 2009-2013 Oliver Kreylos

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

#include <Vrui/SurfaceNavigationTool.h>

#include <Misc/FunctionCalls.h>
#include <Geometry/Vector.h>
#include <Geometry/Rotation.h>
#include <Geometry/Plane.h>
#include <Geometry/OrthogonalTransformation.h>
#include <Vrui/Vrui.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/*********************************************
Methods of class SurfaceNavigationToolFactory:
*********************************************/

SurfaceNavigationToolFactory::SurfaceNavigationToolFactory(ToolManager& toolManager)
	:ToolFactory("SurfaceNavigationTool",toolManager)
	{
	/* Insert class into class hierarchy: */
	ToolFactory* toolFactory=toolManager.loadClass("NavigationTool");
	toolFactory->addChildClass(this);
	addParentClass(toolFactory);
	}

const char* SurfaceNavigationToolFactory::getName(void) const
	{
	return "Surface-Aligned Navigation";
	}

/******************************************************
Methods of class SurfaceNavigationTool::AlignmentState:
******************************************************/

SurfaceNavigationTool::AlignmentState::~AlignmentState(void)
	{
	}

/**************************************
Methods of class SurfaceNavigationTool:
**************************************/

Point SurfaceNavigationTool::projectToFloor(const Point& p)
	{
	/* Project the given point onto the floor plane along the up direction: */
	const Vector& normal=getFloorPlane().getNormal();
	Scalar lambda=(getFloorPlane().getOffset()-p*normal)/(getUpDirection()*normal);
	return p+getUpDirection()*lambda;
	}

NavTransform& SurfaceNavigationTool::calcPhysicalFrame(const Point& basePoint)
	{
	/* Center the physical frame at the base point: */
	physicalFrame=NavTransform::translateFromOriginTo(basePoint);
	
	/* Align the physical frame with the forward and up directions: */
	Vector x=getForwardDirection()^getUpDirection();
	Vector y=getUpDirection()^x;
	physicalFrame*=NavTransform::rotate(Rotation::fromBaseVectors(x,y));
	
	return physicalFrame;
	}

Scalar SurfaceNavigationTool::calcAzimuth(const Rotation& orientation)
	{
	Vector y=Geometry::invert(orientation).getDirection(1);
	if(Math::abs(y[2])>=Scalar(1)-Math::Constants<Scalar>::epsilon)
		{
		/* Gimbal lock: */
		Vector x=orientation.getDirection(0);
		return -Math::atan2(x[1],x[0]);
		}
	else
		return -Math::atan2(-y[0],y[1]);
	}

void SurfaceNavigationTool::calcEulerAngles(const Rotation& orientation,Scalar angles[3])
	{
	Rotation rot=Geometry::invert(orientation);
	
	/* Calculate the elevation and azimuth angles: */
	Vector y=rot.getDirection(1);
	if(y[2]>=Scalar(1)-Math::Constants<Scalar>::epsilon)
		{
		/* Positive gimbal lock: */
		angles[1]=-Math::div2(Math::Constants<Scalar>::pi);
		Vector x=rot.getDirection(0);
		angles[0]=-Math::atan2(x[1],x[0]);
		angles[2]=Scalar(0);
		}
	else if(y[2]<=Scalar(-1)+Math::Constants<Scalar>::epsilon)
		{
		/* Negative gimbal lock: */
		angles[1]=Math::div2(Math::Constants<Scalar>::pi);
		Vector x=rot.getDirection(0);
		angles[0]=-Math::atan2(x[1],x[0]);
		angles[2]=Scalar(0);
		}
	else
		{
		angles[1]=-Math::asin(y[2]);
		angles[0]=-Math::atan2(-y[0],y[1]);
		
		/* Calculate the roll angle: */
		Scalar x0Len=Math::sqrt(Math::sqr(y[0])+Math::sqr(y[1]));
		Vector z=rot.getDirection(2);
		Scalar x=(y[1]*z[0]-y[0]*z[1])/x0Len;
		if(x>=Scalar(1))
			angles[2]=-Math::Constants<Scalar>::pi;
		else if(x<=Scalar(-1))
			angles[2]=Math::Constants<Scalar>::pi;
		else
			angles[2]=-Math::asin(x);
		if(z[2]<Scalar(0))
			{
			if(angles[2]>=Scalar(0))
				angles[2]=Math::Constants<Scalar>::pi-angles[2];
			else
				angles[2]=-Math::Constants<Scalar>::pi-angles[2];
			}
		}
	}

void SurfaceNavigationTool::align(SurfaceNavigationTool::AlignmentData& alignmentData)
	{
	/* Align the initial surface frame: */
	if(alignFunction!=0)
		{
		/* Put the alignment state into the alignment data structure: */
		alignmentData.alignmentState=alignmentState;
		
		/* Call the alignment function: */
		(*alignFunction)(alignmentData);
		
		/* Store the returned alignment state: */
		if(alignmentState!=alignmentData.alignmentState)
			delete alignmentState;
		alignmentState=alignmentData.alignmentState;
		}
	else
		{
		/* Default behavior: Snap frame to z=0 plane and align it with identity transformation: */
		Vector translation=alignmentData.surfaceFrame.getTranslation();
		translation[2]=Scalar(0);
		Scalar scaling=alignmentData.surfaceFrame.getScaling();
		alignmentData.surfaceFrame=NavTransform(translation,Rotation::identity,scaling);
		}
	}

void SurfaceNavigationTool::align(SurfaceNavigationTool::AlignmentData& alignmentData,Scalar& azimuth,Scalar& elevation,Scalar& roll)
	{
	/* Copy the initial surface frame: */
	NavTransform initialSurfaceFrame=alignmentData.surfaceFrame;
	
	/* Align the initial surface frame: */
	if(alignFunction!=0)
		{
		/* Put the alignment state into the alignment data structure: */
		alignmentData.alignmentState=alignmentState;
		
		/* Call the alignment function: */
		(*alignFunction)(alignmentData);
		
		/* Store the returned alignment state: */
		if(alignmentState!=alignmentData.alignmentState)
			delete alignmentState;
		alignmentState=alignmentData.alignmentState;
		}
	else
		{
		/* Default behavior: Snap frame to z=0 plane and align it with identity transformation: */
		Vector translation=alignmentData.surfaceFrame.getTranslation();
		translation[2]=Scalar(0);
		Scalar scaling=alignmentData.surfaceFrame.getScaling();
		alignmentData.surfaceFrame=NavTransform(translation,Rotation::identity,scaling);
		}
	
	/* Calculate rotation of initial frame relative to aligned frame: */
	Rotation rot=Geometry::invert(alignmentData.surfaceFrame.getRotation())*initialSurfaceFrame.getRotation();
	
	/* Calculate the elevation and azimuth angles: */
	Vector y=rot.getDirection(1);
	if(y[2]>=Scalar(1)-Math::Constants<Scalar>::epsilon)
		{
		/* Positive gimbal lock: */
		elevation=-Math::div2(Math::Constants<Scalar>::pi);
		Vector x=rot.getDirection(0);
		azimuth=-Math::atan2(x[1],x[0]);
		roll=Scalar(0);
		}
	else if(y[2]<=Scalar(-1)+Math::Constants<Scalar>::epsilon)
		{
		/* Negative gimbal lock: */
		elevation=Math::div2(Math::Constants<Scalar>::pi);
		Vector x=rot.getDirection(0);
		azimuth=-Math::atan2(x[1],x[0]);
		roll=Scalar(0);
		}
	else
		{
		elevation=-Math::asin(y[2]);
		azimuth=-Math::atan2(-y[0],y[1]);
		
		/* Calculate the roll angle: */
		Scalar x0Len=Math::sqrt(Math::sqr(y[0])+Math::sqr(y[1]));
		Vector z=rot.getDirection(2);
		Scalar x=(y[1]*z[0]-y[0]*z[1])/x0Len;
		if(x>=Scalar(1))
			roll=-Math::Constants<Scalar>::pi;
		else if(x<=Scalar(-1))
			roll=Math::Constants<Scalar>::pi;
		else
			roll=-Math::asin(x);
		if(z[2]<Scalar(0))
			{
			if(roll>=Scalar(0))
				roll=Math::Constants<Scalar>::pi-roll;
			else
				roll=-Math::Constants<Scalar>::pi-roll;
			}
		}
	}

SurfaceNavigationTool::SurfaceNavigationTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment)
	:NavigationTool(factory,inputAssignment),
	 alignFunction(0),alignmentState(0)
	{
	}

SurfaceNavigationTool::~SurfaceNavigationTool(void)
	{
	/* Delete the alignment object's state: */
	delete alignmentState;
	
	/* Delete the alignment function call object: */
	delete alignFunction;
	}

void SurfaceNavigationTool::deactivate(void)
	{
	/* Delete the alignment state object: */
	delete alignmentState;
	alignmentState=0;
	
	/* Call the base class method: */
	NavigationTool::deactivate();
	}

void SurfaceNavigationTool::setAlignFunction(SurfaceNavigationTool::AlignFunction* newAlignFunction)
	{
	/* Delete the current alignment object's state: */
	delete alignmentState;
	
	/* Delete the current alignment function call object: */
	delete alignFunction;
	
	/* Install the new alignment function call object: */
	alignFunction=newAlignFunction;
	alignmentState=0;
	}

}
