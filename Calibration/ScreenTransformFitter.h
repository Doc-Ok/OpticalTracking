/***********************************************************************
ScreenTransformFitter - Functor plug-in to find the best orthonormal
transformation with independent scaling in x and y direction
transforming a source point set into a target point set.
Copyright (c) 2009-2010 Oliver Kreylos

This file is part of the Vrui calibration utility package.

The Vrui calibration utility package is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The Vrui calibration utility package is distributed in the hope that it
will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Vrui calibration utility package; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#ifndef SCREENTRANSFORMFITTER_INCLUDED
#define SCREENTRANSFORMFITTER_INCLUDED

#include <Geometry/ComponentArray.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/OrthonormalTransformation.h>

class ScreenTransformFitter
	{
	/* Embedded classes: */
	public:
	typedef double Scalar; // Scalar type
	typedef Geometry::Point<Scalar,3> Point;
	typedef Geometry::Vector<Scalar,3> Vector;
	typedef Geometry::OrthonormalTransformation<Scalar,3> Transform;
	static const int dimension=9; // Dimension of the optimization space
	typedef Geometry::ComponentArray<Scalar,dimension> Derivative; // Type for distance function derivatives
	
	/* Elements: */
	private:
	size_t numPoints; // Number of source and target points
	const Point* sp; // Array of source points
	const Point* tp; // Array of target points
	
	/* Transient optimization state: */
	Transform transform; // The current base transformation estimate
	Scalar size[2]; // The screen width and height
	const Vector& t; // Shortcut to base transformation's translation vector
	const Scalar* q; // Shortcut to base transformation's rotation quaternion
	Transform transformSave;
	Scalar sizeSave[2];
	
	/* Constructors and destructors: */
	public:
	ScreenTransformFitter(size_t sNumPoints,const Point* sSp,const Point* sTp)
		:numPoints(sNumPoints),sp(sSp),tp(sTp),
		 transform(Transform::identity),
		 t(transform.getTranslation()),
		 q(transform.getRotation().getQuaternion())
		{
		size[0]=size[1]=Scalar(1);
		}
	
	/* Methods: */
	const Transform& getTransform(void) const // Returns the current transformation estimate
		{
		return transform;
		}
	Scalar getSize(int dim) const // Returns the current size estimate in the given dimension (width or height)
		{
		return size[dim];
		}
	void setTransform(const Transform& newTransform) // Sets the current transformation estimate
		{
		transform=newTransform;
		};
	void SetSize(int dim,Scalar newSize) // Sets the current size estimate in the given dimension (width or height)
		{
		size[dim]=newSize;
		}
	
	/* Methods required by Levenberg-Marquardt optimizer: */
	void save(void) // Saves the current estimate
		{
		transformSave=transform;
		sizeSave[0]=size[0];
		sizeSave[1]=size[1];
		};
	void restore(void) // Restores the last saved estimate
		{
		transform=transformSave;
		size[0]=sizeSave[0];
		size[1]=sizeSave[1];
		};
	size_t getNumPoints(void) const // Returns the number of distance functions to minimize
		{
		return numPoints;
		}
	Scalar calcDistance(size_t index) const // Calculates the distance value for the current estimate and the given distance function index
		{
		Point s=sp[index];
		s[0]*=size[0];
		s[1]*=size[1];
		return Geometry::dist(transform.transform(s),tp[index]);
		}
	Derivative calcDistanceDerivative(size_t index) const // Calculates the derivative of the distance value for the current estimate and the given distance function index
		{
		Point s=sp[index];
		
		/*******************************************************************
		Calculate the distance vector between the transformed source point
		and the target point. The transformation is spelled out in order to
		reuse the intermediate results for the derivative calculation.
		*******************************************************************/
		
		/* Calculate the non-uniform scaling: */
		s[0]*=size[0];
		s[1]*=size[1];
		
		/* Calculate the first rotation part: */
		Scalar rX=q[1]*s[2]-q[2]*s[1]+q[3]*s[0];
		Scalar rY=q[2]*s[0]-q[0]*s[2]+q[3]*s[1];
		Scalar rZ=q[0]*s[1]-q[1]*s[0]+q[3]*s[2];
		Scalar rW=q[0]*s[0]+q[1]*s[1]+q[2]*s[2];
		
		/* Calculate the second rotation part and the translation and difference: */
		Vector d;
		d[0]=rZ*q[1]-rY*q[2]+rW*q[0]+rX*q[3]+t[0]-tp[index][0];
		d[1]=rX*q[2]-rZ*q[0]+rW*q[1]+rY*q[3]+t[1]-tp[index][1];
		d[2]=rY*q[0]-rX*q[1]+rW*q[2]+rZ*q[3]+t[2]-tp[index][2];
		
		/* Calculate the difference magnitude: */
		Scalar dist=Geometry::mag(d);
		
		Derivative result;
		
		/* Calculate the translational partial derivatives: */
		result[0]=d[0]/dist;
		result[1]=d[1]/dist;
		result[2]=d[2]/dist;
		
		/* Calculate the rotational partial derivatives: */
		result[3]=Scalar(2)*(+d[0]*rW-d[1]*rZ+d[2]*rY)/dist;
		result[4]=Scalar(2)*(+d[0]*rZ+d[1]*rW-d[2]*rX)/dist;
		result[5]=Scalar(2)*(-d[0]*rY+d[1]*rX+d[2]*rW)/dist;
		result[6]=Scalar(2)*(+d[0]*rX+d[1]*rY+d[2]*rZ)/dist;
		
		/* Calculate the scaling partial derivatives: */
		result[7]=((+q[0]*q[0]-q[1]*q[1]-q[2]*q[2]+q[3]*q[3])*d[0]
		          +(+q[0]*q[1]+q[1]*q[0]+q[2]*q[3]+q[3]*q[2])*d[1]
		          +(+q[0]*q[2]-q[1]*q[3]+q[2]*q[0]-q[3]*q[1])*d[2])*s[0]/dist;
		result[8]=((+q[0]*q[1]+q[1]*q[0]-q[2]*q[3]-q[3]*q[2])*d[0]
		          +(-q[0]*q[0]+q[1]*q[1]-q[2]*q[2]+q[3]*q[3])*d[1]
		          +(+q[0]*q[3]+q[1]*q[2]+q[2]*q[1]+q[3]*q[0])*d[2])*s[1]/dist;
		
		return result;
		}
	Scalar calcMag(void) const // Returns the magnitude of the current estimate
		{
		return Math::sqrt(Geometry::sqr(t)+Scalar(1)+Math::sqr(size[0])+Math::sqr(size[1]));
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
		size[0]-=increment[7];
		size[1]-=increment[8];
		}
	void normalize(void) // Normalizes the current estimate
		{
		/* Transform constructor already normalized the quaternion */
		
		/* Normalize the sizes: */
		if(size[0]<Scalar(0))
			size[0]=-size[0];
		if(size[1]<Scalar(0))
			size[1]=-size[1];
		}
	};

#endif
