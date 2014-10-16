/***********************************************************************
RayPicker - Functor class to pick points from a point set based on
distance along a ray, with a maximum deviation angle from the ray. In
other words, picks points that lie within a cone of given apex angle.
Copyright (c) 2010 Oliver Kreylos

This file is part of the Templatized Geometry Library (TGL).

The Templatized Geometry Library is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Templatized Geometry Library is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Templatized Geometry Library; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#ifndef GEOMETRY_RAYPICKER_INCLUDED
#define GEOMETRY_RAYPICKER_INCLUDED

#include <Math/Math.h>
#include <Math/Constants.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/Ray.h>

namespace Geometry {

template <class ScalarParam,int dimensionParam>
class RayPicker
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar; // The underlying scalar type
	static const int dimension=dimensionParam; // The dimension of the picker's affine space
	typedef Geometry::Point<ScalarParam,dimensionParam> Point; // The type for points
	typedef Geometry::Vector<ScalarParam,dimensionParam> Vector; // The type for vectors
	typedef Geometry::Ray<ScalarParam,dimensionParam> Ray; // The type for rays
	typedef unsigned int Index; // Index type for points and pick results
	
	/* Elements: */
	private:
	Ray queryRay; // The query ray; does not have to be normalized
	Scalar d2; // The squared length of the query ray's direction vector
	Scalar scaledMaxAngleCos2; // The squared cosine of the maximum deviation angle from the ray, measured from the apex, multiplied by d2
	Scalar minScaledLambda; // The scaled ray parameter of the currently picked point
	Index pointIndex; // The index of the next point to be processed
	Index pickIndex; // The index of the currently picked point
	
	/* Constructors and destructors: */
	public:
	RayPicker(const Ray& sQueryRay,Scalar sMaxAngleCos) // Creates a ray picker for the given query ray and cosine of maximum deviation angle
		:queryRay(sQueryRay),d2(Geometry::sqr(queryRay.getDirection())),
		 scaledMaxAngleCos2(Math::sqr(sMaxAngleCos)*d2),
		 minScaledLambda(Math::Constants<Scalar>::max),
		 pointIndex(0),pickIndex(~Index(0))
		{
		}
	
	/* Methods: */
	bool operator()(const Point& p) // Checks if the given point is closer than the previously picked point; returns true if point was picked
		{
		bool result=false;
		
		/* Calculate the scaled ray parameter of the given point: */
		Vector op=p-queryRay.getOrigin();
		Scalar scaledLambda=op*queryRay.getDirection();
		
		/* Check if the point is closer than the currently picked point: */
		if(scaledLambda>=Scalar(0)&&minScaledLambda>scaledLambda)
			{
			/* Check if the point is inside the cone: */
			if(Math::sqr(scaledLambda)>=scaledMaxAngleCos2*Geometry::sqr(op))
				{
				minScaledLambda=scaledLambda;
				pickIndex=pointIndex;
				result=true;
				}
			}
		
		/* Prepare to process the next point: */
		++pointIndex;
		
		return result;
		}
	Scalar getLambda(void) const // Returns the ray parameter of the currently picked point
		{
		return minScaledLambda/d2;
		}
	Index getNumPoints(void) const // Returns the total number of processed points
		{
		return pointIndex;
		}
	bool havePickedPoint(void) const // Returns true if a point was picked
		{
		return pickIndex!=~Index(0);
		}
	Index getPickIndex(void) const // Returns the index of the picked point, or ~0 if no point was picked
		{
		return pickIndex;
		}
	};

}

#endif
