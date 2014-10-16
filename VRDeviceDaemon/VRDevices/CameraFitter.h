/***********************************************************************
CameraFitter - Functor plug-in to fit the extrinsic parameters of a
camera to a set of observed point projections using a Levenberg-
Marquardt algorithm.
Copyright (c) 2007-2014 Oliver Kreylos

This file is part of the Vrui VR Device Driver Daemon (VRDeviceDaemon).

The Vrui VR Device Driver Daemon is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Vrui VR Device Driver Daemon is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Vrui VR Device Driver Daemon; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#ifndef CAMERAFITTER_INCLUDED
#define CAMERAFITTER_INCLUDED

#include <Math/Math.h>
#include <Geometry/ComponentArray.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/Rotation.h>
#include <Geometry/OrthonormalTransformation.h>

class CameraFitter
	{
	/* Embedded classes: */
	public:
	typedef double Scalar; // Scalar type
	typedef Geometry::Point<Scalar,3> Point;
	typedef Geometry::Point<Scalar,2> Pixel;
	typedef Geometry::Vector<Scalar,3> Vector;
	typedef Geometry::OrthonormalTransformation<Scalar,3> Transform;
	static const int dimension=7; // Dimension of the optimization space
	typedef Geometry::ComponentArray<Scalar,dimension> Derivative; // Type for distance function derivatives
	
	/* Elements: */
	private:
	
	/* Intrinsic camera parameters: */
	Pixel center; // Center of projection in CCD pixel coordinates
	Scalar dist; // Distance from lens center to CCD (focal distance)
	
	/* Extrinsic camera parameters: */
	Transform transform; // Position and orientation of the camera in world space
	const Vector& t; // Shortcut to transformation's translation vector
	const Scalar* q; // Shortcut to transformation's rotation quaternion
	
	/* State of calibration target: */
	Point points[4]; // Positions of the four points in world space
	
	/* Transient optimization state: */
	bool pixelValids[4]; // Valid flags for the observed pixels
	Pixel pixels[4]; // Positions of the observed pixels in CCD pixel coordinates
	Transform transformSave;
	
	/* Constructors and destructors: */
	public:
	CameraFitter(const Pixel& sCenter,Scalar sDist)
		:center(sCenter),dist(sDist),
		 transform(Transform::identity),
		 t(transform.getTranslation()),
		 q(transform.getRotation().getQuaternion())
		{
		/* Initialize the pixel state array: */
		for(int i=0;i<4;++i)
			pixelValids[i]=false;
		}
	
	/* Methods: */
	void setTargetPoint(int index,const Point& newPoint) // Sets the world position of a target point
		{
		points[index]=newPoint;
		}
	void setTransform(const Transform& newTransform) // Sets the extrinsic camera parameters
		{
		transform=newTransform;
		}
	Pixel project(const Point& point) const // Returns CCD pixel coordinates of point in world coordinates
		{
		/* Transform the point into camera coordinates: */
		Point cPoint=transform.inverseTransform(point);
		
		/* Project the point onto the CCD: */
		return Pixel(cPoint[0]*dist/cPoint[1]+center[0],cPoint[2]*dist/cPoint[1]+center[1]);
		}
	Pixel project(int index) const // Returns CCD pixel coordinates of one of the target points
		{
		/* Transform the point into camera coordinates: */
		Point cPoint=transform.inverseTransform(points[index]);
		
		/* Project the point onto the CCD: */
		return Pixel(cPoint[0]*dist/cPoint[1]+center[0],cPoint[2]*dist/cPoint[1]+center[1]);
		}
	void invalidatePixel(int index) // Marks the given observed pixel as invalid
		{
		pixelValids[index]=false;
		}
	void setPixel(int index,const Pixel& newPosition) // Sets the position of the observed pixel and marks it valid
		{
		pixelValids[index]=true;
		pixels[index]=newPosition;
		}
	void save(void) // Saves the current estimate
		{
		transformSave=transform;
		};
	void restore(void) // Restores the last saved estimate
		{
		transform=transformSave;
		};
	size_t getNumPoints(void) const // Returns the number of distance functions to minimize
		{
		return 8; // 4 pixels with two coordinates each
		}
	Scalar calcDistance(size_t index) const // Calculates the distance value for the current estimate and the given distance function index
		{
		if(pixelValids[index>>1])
			{
			Point p=points[index>>1]-t;
			Scalar camY=(q[1]*q[1]-q[0]*q[0]-q[2]*q[2]+q[3]*q[3])*p[1]+Scalar(2)*((q[0]*q[1]-q[2]*q[3])*p[0]+(q[1]*q[2]+q[0]*q[3])*p[2]);
			if(index&0x1)
				{
				/* Calculate distance in CCD's Y direction: */
				Scalar camZ=(q[2]*q[2]-q[0]*q[0]-q[1]*q[1]+q[3]*q[3])*p[2]+Scalar(2)*((q[0]*q[2]+q[1]*q[3])*p[0]+(q[1]*q[2]-q[0]*q[3])*p[1]);
				Scalar ccdY=camZ*dist/camY+center[1];
				return ccdY-pixels[index>>1][1];
				}
			else
				{
				/* Calculate distance in CCD's X direction: */
				Scalar camX=(q[0]*q[0]-q[1]*q[1]-q[2]*q[2]+q[3]*q[3])*p[0]+Scalar(2)*((q[0]*q[1]+q[2]*q[3])*p[1]+(q[0]*q[2]-q[1]*q[3])*p[2]);
				Scalar ccdX=camX*dist/camY+center[0];
				return ccdX-pixels[index>>1][0];
				}
			}
		else
			return Scalar(0);
		}
	Derivative calcDistanceDerivative(size_t index) const // Calculates the derivative of the distance value for the current estimate and the given distance function index
		{
		if(pixelValids[index>>1])
			{
			Point p=points[index>>1]-t;
			Scalar camY=(q[1]*q[1]-q[0]*q[0]-q[2]*q[2]+q[3]*q[3])*p[1]+Scalar(2)*((q[0]*q[1]-q[2]*q[3])*p[0]+(q[1]*q[2]+q[0]*q[3])*p[2]);
			Scalar factor=dist/Math::sqr(camY);
			if(index&0x1)
				{
				/* Calculate distance in CCD's Y direction: */
				Scalar camZ=(q[2]*q[2]-q[0]*q[0]-q[1]*q[1]+q[3]*q[3])*p[2]+Scalar(2)*((q[0]*q[2]+q[1]*q[3])*p[0]+(q[1]*q[2]-q[0]*q[3])*p[1]);
				
				Derivative result;
				result[0]=Scalar(2)*((q[0]*q[1]-q[2]*q[3])*camZ-(q[0]*q[2]+q[1]*q[3])*camY)*factor;
				result[1]=((q[1]*q[1]-q[0]*q[0]-q[2]*q[2]+q[3]*q[3])*camZ-Scalar(2)*(q[1]*q[2]-q[0]*q[3])*camY)*factor;
				result[2]=(Scalar(2)*(q[1]*q[2]+q[0]*q[3])*camZ-(q[2]*q[2]-q[0]*q[0]-q[1]*q[1]+q[3]*q[3])*camY)*factor;
				
				Scalar dq[4];
				dq[0]=Scalar(2)*(( q[2]*p[0]-q[3]*p[1]-q[0]*p[2])*camY-( q[1]*p[0]-q[0]*p[1]+q[3]*p[2])*camZ)*factor;
				dq[1]=Scalar(2)*(( q[3]*p[0]+q[2]*p[1]-q[1]*p[2])*camY-( q[0]*p[0]+q[1]*p[1]+q[2]*p[2])*camZ)*factor;
				dq[2]=Scalar(2)*(( q[0]*p[0]+q[1]*p[1]+q[2]*p[2])*camY-(-q[3]*p[0]-q[2]*p[1]+q[1]*p[2])*camZ)*factor;
				dq[3]=Scalar(2)*(( q[1]*p[0]-q[0]*p[1]+q[3]*p[2])*camY-(-q[2]*p[0]+q[3]*p[1]+q[0]*p[2])*camZ)*factor;
				
				for(int i=0;i<4;++i)
					result[3+i]=dq[i]-(dq[0]*q[0]+dq[1]*q[1]+dq[2]*q[2]+dq[2]*q[2])*q[i];
				
				return result;
				}
			else
				{
				/* Calculate distance in CCD's X direction: */
				Scalar camX=(q[0]*q[0]-q[1]*q[1]-q[2]*q[2]+q[3]*q[3])*p[0]+Scalar(2)*((q[0]*q[1]+q[2]*q[3])*p[1]+(q[0]*q[2]-q[1]*q[3])*p[2]);
				
				Derivative result;
				result[0]=(Scalar(2)*(q[0]*q[1]-q[2]*q[3])*camX-(q[0]*q[0]-q[1]*q[1]-q[2]*q[2]+q[3]*q[3])*camY)*factor;
				result[1]=((q[1]*q[1]-q[0]*q[0]-q[2]*q[2]+q[3]*q[3])*camX-Scalar(2)*(q[0]*q[1]+q[2]*q[3])*camY)*factor;
				result[2]=Scalar(2)*((q[1]*q[2]+q[0]*q[3])*camX-(q[0]*q[2]-q[1]*q[3])*camY)*factor;
				
				Scalar dq[4];
				dq[0]=Scalar(2)*(( q[0]*p[0]+q[1]*p[1]+q[2]*p[2])*camY-( q[1]*p[0]-q[0]*p[1]+q[3]*p[2])*camX)*factor;
				dq[1]=Scalar(2)*((-q[1]*p[0]+q[0]*p[1]-q[3]*p[2])*camY-( q[0]*p[0]+q[1]*p[1]+q[2]*p[2])*camX)*factor;
				dq[2]=Scalar(2)*((-q[2]*p[0]+q[3]*p[1]+q[0]*p[2])*camY-(-q[3]*p[0]-q[2]*p[1]+q[1]*p[2])*camX)*factor;
				dq[3]=Scalar(2)*(( q[3]*p[0]+q[2]*p[1]-q[1]*p[2])*camY-(-q[2]*p[0]+q[3]*p[1]+q[0]*p[2])*camX)*factor;
				
				for(int i=0;i<4;++i)
					result[3+i]=dq[i]-(dq[0]*q[0]+dq[1]*q[1]+dq[2]*q[2]+dq[2]*q[2])*q[i];
				
				return result;
				}
			}
		else
			return Derivative(0);
		}
	Scalar calcMag(void) const // Returns the magnitude of the current estimate
		{
		return Math::sqrt(Geometry::sqr(t)+Scalar(1));
		}
	void increment(const Derivative& increment) // Increments the current estimate by the given difference vector
		{
		Vector newT;
		for(int i=0;i<3;++i)
			newT[i]=t[i]-increment[i];
		Scalar newQ[4];
		for(int i=0;i<4;++i)
			newQ[i]=q[i]-increment[3+i];
		transform=Transform(newT,Transform::Rotation::fromQuaternion(newQ));
		}
	void normalize(void) // Normalizes the current estimate
		{
		/* Nothing to do; Transform constructor automatically normalizes quaternion */
		}
	const Transform& getTransform(void) const // Returns the camera's transformation
		{
		return transform;
		}
	};

#endif
