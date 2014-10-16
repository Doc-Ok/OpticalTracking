/***********************************************************************
PTransformFitter - Functor plug-in to find the best projective
transformation transforming a source point set into a target point set.
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

#ifndef PTRANSFORMFITTER_INCLUDED
#define PTRANSFORMFITTER_INCLUDED

#include <Geometry/ComponentArray.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/HVector.h>
#include <Geometry/ProjectiveTransformation.h>

class PTransformFitter
	{
	/* Embedded classes: */
	public:
	typedef double Scalar; // Scalar type
	typedef Geometry::Point<Scalar,3> Point;
	typedef Geometry::Vector<Scalar,3> Vector;
	typedef Geometry::HVector<Scalar,3> HVector;
	typedef Geometry::ProjectiveTransformation<Scalar,3> Transform;
	static const int dimension=16; // Dimension of the optimization space
	typedef Geometry::ComponentArray<Scalar,dimension> Derivative; // Type for distance function derivatives
	
	/* Elements: */
	private:
	size_t numPoints; // Number of source and target points
	const Point* sp; // Array of source points
	const Point* tp; // Array of target points
	
	/* Transient optimization state: */
	Transform transform; // The current transformation estimate
	Transform::Matrix& m; // Shortcut to transformation's matrix
	Transform transformSave;
	
	/* Constructors and destructors: */
	public:
	PTransformFitter(size_t sNumPoints,const Point* sSp,const Point* sTp)
		:numPoints(sNumPoints),sp(sSp),tp(sTp),
		 transform(Transform::identity),
		 m(transform.getMatrix())
		{
		}
	
	/* Methods: */
	const Transform& getTransform(void) const // Returns the current transformation estimate
		{
		return transform;
		}
	void setTransform(const Transform& newTransform) // Sets the current transformation estimate
		{
		transform=newTransform;
		};
	
	/* Methods required by Levenberg-Marquardt optimizer: */
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
		return numPoints;
		}
	Scalar calcDistance(size_t index) const // Calculates the distance value for the current estimate and the given distance function index
		{
		return Geometry::dist(transform.transform(sp[index]),tp[index]);
		}
	Derivative calcDistanceDerivative(size_t index) const // Calculates the derivative of the distance value for the current estimate and the given distance function index
		{
		/* Calculate the distance vector between the transformed source point and the target point: */
		HVector s(sp[index]);
		HVector ms=transform.transform(s);
		Vector d=ms.toPoint()-tp[index];
		
		/* Calculate the difference magnitude: */
		Scalar dist=Geometry::mag(d);
		
		Derivative result;
		
		/* Calculate the partial derivatives: */
		for(int i=0;i<3;++i)
			{
			Scalar factor=d[i]/(ms[3]*dist);
			for(int j=0;j<4;++j)
				result[i*4+j]=factor*s[j];
			}
		Scalar factor=-(d[0]*ms[0]+d[1]*ms[1]+d[2]*ms[2])/(ms[3]*ms[3]*dist);
		for(int j=0;j<4;++j)
			result[12+j]=factor*s[j];
		
		return result;
		}
	Scalar calcMag(void) const // Returns the magnitude of the current estimate
		{
		/* Return the L2 norm of the matrix: */
		Scalar sum2=Scalar(0);
		for(int i=0;i<4;++i)
			for(int j=0;j<4;++j)
				sum2+=Math::sqr(m(i,j));
		return Math::sqrt(sum2);
		}
	void increment(const Derivative& increment) // Increments the current estimate by the given difference vector
		{
		/* Apply the increment vector directly to the matrix: */
		for(int i=0;i<4;++i)
			for(int j=0;j<4;++j)
				m(i,j)-=increment[i*4+j];
		}
	void normalize(void) // Normalizes the current estimate
		{
		/* Normalize the homogeneous entry to 1: */
		for(int i=0;i<4;++i)
			for(int j=0;j<4;++j)
				m(i,j)/=m(3,3);
		}
	};

#endif
