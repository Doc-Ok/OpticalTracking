/***********************************************************************
CameraFitter - Functor plug-in to fit the extrinsic parameters of a
camera to a set of observed point projections using a Levenberg-
Marquardt algorithm.
Copyright (c) 2007-2014 Oliver Kreylos

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
	Scalar fu,sk,cu,fv,cv; // Scale factor in u, skew factor, center in u, scale factor in v, center in v
	
	/* State of the tracked object: */
	unsigned int numPoints; // Number of 3D points defining the tracked object
	Point* points; // Array of 3D points defining the tracked object
	
	/* Current tracked object pose estimate: */
	mutable Transform transform; // Position and orientation of the camera in world space
	const Vector& t; // Shortcut to transformation's translation vector
	const Scalar* q; // Shortcut to transformation's rotation quaternion
	
	/* Current observation state of tracked object: */
	Pixel* pixels; // Array of 2D pixel positions associated with the tracked object's model points
	
	/* Transient optimization state: */
	Transform transformSave;
	
	/* Constructors and destructors: */
	public:
	CameraFitter(Scalar sFu,Scalar sSk,Scalar sCu,Scalar sFv,Scalar sCv)
		:fu(sFu),sk(sSk),cu(sCu),fv(sFv),cv(sCv),
		 numPoints(0),points(0),
		 transform(Transform::identity),
		 t(transform.getTranslation()),
		 q(transform.getRotation().getQuaternion()),
		 pixels(0)
		{
		}
	~CameraFitter(void)
		{
		delete[] points;
		delete[] pixels;
		}
	
	/* Methods: */
	void setTrackedObject(unsigned int newNumPoints,const Point newPoints[]) // Sets the 3D point positions defining the tracked object
		{
		/* Reallocate the state arrays: */
		if(numPoints!=newNumPoints)
			{
			delete[] points;
			delete[] pixels;
			numPoints=newNumPoints;
			points=new Point[numPoints];
			pixels=new Pixel[numPoints];
			}
		
		/* Copy the new point positions: */
		for(unsigned int i=0;i<numPoints;++i)
			points[i]=newPoints[i];
		}
	void setTransform(const Transform& newTransform) // Sets the current tracked object pose estimate
		{
		transform=newTransform;
		}
	Pixel project(const Point& point) const // Returns CCD pixel coordinates of point in world coordinates
		{
		/* Transform the point into camera coordinates: */
		Point cPoint=transform.transform(point);
		
		/* Project the point onto the CCD: */
		return Pixel((cPoint[0]*fu+cPoint[1]*sk)/cPoint[2]+cu,cPoint[1]*fv/cPoint[2]+cv);
		}
	Pixel project(unsigned int index) const // Returns CCD pixel coordinates of one of the target points
		{
		/* Transform the point into camera coordinates: */
		Point cPoint=transform.transform(points[index]);
		
		/* Project the point onto the CCD: */
		return Pixel((cPoint[0]*fu+cPoint[1]*sk)/cPoint[2]+cu,cPoint[1]*fv/cPoint[2]+cv);
		}
	void setPixel(unsigned int index,const Pixel& newPosition) // Sets the observed position of the pixel associated with the given tracked object point
		{
		pixels[index]=newPosition;
		}
	const Transform& getTransform(void) const // Returns the current tracked object pose estimate
		{
		return transform;
		}
	
	/* Levenberg-Marquardt optimization interface: */
	void save(void) // Saves the current tracked object pose estimate
		{
		transformSave=transform;
		}
	void restore(void) // Restores the last saved tracked object pose estimate
		{
		transform=transformSave;
		}
	unsigned int getNumPoints(void) const // Returns the number of distance functions to minimize
		{
		return numPoints*2; // Two pixel coordinates per tracked object point
		}
	Scalar calcDistance(unsigned int index) const // Calculates the distance value for the current estimate and the given distance function index
		{
		Point p=points[index>>1];
		Scalar camZ=(q[2]*q[2]-q[0]*q[0]-q[1]*q[1]+q[3]*q[3])*p[2]+Scalar(2)*((q[0]*q[2]-q[1]*q[3])*p[0]+(q[1]*q[2]+q[0]*q[3])*p[1])+t[2];
		if(index&0x1)
			{
			/* Calculate distance in v direction: */
			Scalar camY=(q[1]*q[1]-q[0]*q[0]-q[2]*q[2]+q[3]*q[3])*p[1]+Scalar(2)*((q[0]*q[1]+q[2]*q[3])*p[0]+(q[1]*q[2]-q[0]*q[3])*p[2])+t[1];
			Scalar v=camY*fv/camZ+cv;
			return v-pixels[index>>1][1];
			}
		else
			{
			/* Calculate distance in u direction: */
			Scalar camX=(q[0]*q[0]-q[1]*q[1]-q[2]*q[2]+q[3]*q[3])*p[0]+Scalar(2)*((q[0]*q[1]-q[2]*q[3])*p[1]+(q[0]*q[2]+q[1]*q[3])*p[2])+t[0];
			Scalar camY=(q[1]*q[1]-q[0]*q[0]-q[2]*q[2]+q[3]*q[3])*p[1]+Scalar(2)*((q[0]*q[1]+q[2]*q[3])*p[0]+(q[1]*q[2]-q[0]*q[3])*p[2])+t[1];
			Scalar u=(camX*fu+camY*sk)/camZ+cu;
			return u-pixels[index>>1][0];
			}
		}
	Derivative calcDistanceDerivativeFoo(unsigned int index) const // Calculates the derivative of the distance value for the current estimate and the given distance function index
		{
		Derivative result;
		
		for(int i=0;i<3;++i)
			{
			Transform oldTransform=transform;
			Vector tp=t;
			tp[i]=t[i]+0.0001;
			transform=Transform(tp,q);
			Scalar r1=calcDistance(index);
			tp[i]=t[i]-0.0001;
			transform=Transform(tp,q);
			Scalar r0=calcDistance(index);
			result[i]=(r1-r0)/0.0002;
			transform=oldTransform;
			}
		
		for(int i=0;i<4;++i)
			{
			Transform oldTransform=transform;
			Scalar qp[4];
			for(int j=0;j<4;++j)
				qp[j]=q[j];
			qp[i]=q[i]+0.0001;
			transform=Transform(t,Transform::Rotation(qp));
			Scalar r1=calcDistance(index);
			qp[i]=q[i]-0.0001;
			transform=Transform(t,Transform::Rotation(qp));
			Scalar r0=calcDistance(index);
			result[3+i]=(r1-r0)/0.0002;
			transform=oldTransform;
			}
		
		return result;
		}
	Derivative calcDistanceDerivative(unsigned int index) const // Calculates the derivative of the distance value for the current estimate and the given distance function index
		{
		Point p=points[index>>1];
		Scalar camZ=(q[2]*q[2]-q[0]*q[0]-q[1]*q[1]+q[3]*q[3])*p[2]+Scalar(2)*((q[0]*q[2]-q[1]*q[3])*p[0]+(q[1]*q[2]+q[0]*q[3])*p[1])+t[2];
		if(index&0x1)
			{
			/* Calculate distance derivative in v direction: */
			Scalar camY=(q[1]*q[1]-q[0]*q[0]-q[2]*q[2]+q[3]*q[3])*p[1]+Scalar(2)*((q[0]*q[1]+q[2]*q[3])*p[0]+(q[1]*q[2]-q[0]*q[3])*p[2])+t[1];
			
			Derivative result;
			result[0]=Scalar(0);
			result[1]=fv/camZ;
			result[2]=(-fv*camY)/Math::sqr(camZ);
			
			result[3]=Scalar(2)*fv*(( q[1]*p[0]-q[0]*p[1]-q[3]*p[2])*camZ-camY*( q[2]*p[0]+q[3]*p[1]-q[0]*p[2]))/Math::sqr(camZ);
			result[4]=Scalar(2)*fv*(( q[0]*p[0]+q[1]*p[1]+q[2]*p[2])*camZ-camY*(-q[3]*p[0]+q[2]*p[1]-q[1]*p[2]))/Math::sqr(camZ);
			result[5]=Scalar(2)*fv*(( q[3]*p[0]-q[2]*p[1]+q[1]*p[2])*camZ-camY*( q[0]*p[0]+q[1]*p[1]+q[2]*p[2]))/Math::sqr(camZ);
			result[6]=Scalar(2)*fv*(( q[2]*p[0]+q[3]*p[1]-q[0]*p[2])*camZ-camY*(-q[1]*p[0]+q[0]*p[1]+q[3]*p[2]))/Math::sqr(camZ);
			
			return result;
			}
		else
			{
			/* Calculate distance derivative in u direction: */
			Scalar camX=(q[0]*q[0]-q[1]*q[1]-q[2]*q[2]+q[3]*q[3])*p[0]+Scalar(2)*((q[0]*q[1]-q[2]*q[3])*p[1]+(q[0]*q[2]+q[1]*q[3])*p[2])+t[0];
			Scalar camY=(q[1]*q[1]-q[0]*q[0]-q[2]*q[2]+q[3]*q[3])*p[1]+Scalar(2)*((q[0]*q[1]+q[2]*q[3])*p[0]+(q[1]*q[2]-q[0]*q[3])*p[2])+t[1];
			
			Derivative result;
			result[0]=fu/camZ;
			result[1]=sk/camZ;
			result[2]=(-fu*camX-sk*camY)/Math::sqr(camZ);
			
			result[3]=Scalar(2)*((fu*( q[0]*p[0]+q[1]*p[1]+q[2]*p[2])+sk*( q[1]*p[0]-q[0]*p[1]-q[3]*p[2]))*camZ-(fu*camX+sk*camY)*( q[2]*p[0]+q[3]*p[1]-q[0]*p[2]))/Math::sqr(camZ);
			result[4]=Scalar(2)*((fu*(-q[1]*p[0]+q[0]*p[1]+q[3]*p[2])+sk*( q[0]*p[0]+q[1]*p[1]+q[2]*p[2]))*camZ-(fu*camX+sk*camY)*(-q[3]*p[0]+q[2]*p[1]-q[1]*p[2]))/Math::sqr(camZ);
			result[5]=Scalar(2)*((fu*(-q[2]*p[0]-q[3]*p[1]+q[0]*p[2])+sk*( q[3]*p[0]-q[2]*p[1]+q[1]*p[2]))*camZ-(fu*camX+sk*camY)*( q[0]*p[0]+q[1]*p[1]+q[2]*p[2]))/Math::sqr(camZ);
			result[6]=Scalar(2)*((fu*( q[3]*p[0]-q[2]*p[1]+q[1]*p[2])+sk*( q[2]*p[0]+q[3]*p[1]-q[0]*p[2]))*camZ-(fu*camX+sk*camY)*(-q[1]*p[0]+q[0]*p[1]+q[3]*p[2]))/Math::sqr(camZ);
			
			return result;
			}
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
	};

#endif
