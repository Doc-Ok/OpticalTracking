/***********************************************************************
PointPicker - Functor class to pick points from a point set based on
distance to a query point.
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

#ifndef GEOMETRY_POINTPICKER_INCLUDED
#define GEOMETRY_POINTPICKER_INCLUDED

#include <Math/Math.h>
#include <Math/Constants.h>
#include <Geometry/Point.h>

namespace Geometry {

template <class ScalarParam,int dimensionParam>
class PointPicker
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar; // The underlying scalar type
	static const int dimension=dimensionParam; // The dimension of the picker's affine space
	typedef Geometry::Point<ScalarParam,dimensionParam> Point; // The type for points
	typedef unsigned int Index; // Index type for points and pick results
	
	/* Elements: */
	private:
	Point queryPoint; // The query point
	Scalar maxDist2; // The squared maximum picking distance; adjusted as points are processed
	Index pointIndex; // The index of the next point to be processed
	Index pickIndex; // The index of the currently picked point
	
	/* Constructors and destructors: */
	public:
	PointPicker(const Point& sQueryPoint,Scalar sMaxDist) // Creates a point picker for the given query point and maximum picking distance
		:queryPoint(sQueryPoint),maxDist2(Math::sqr(sMaxDist)),
		 pointIndex(0),pickIndex(~Index(0))
		{
		}
	PointPicker(const Point& sQueryPoint) // Ditto, with "infinite" maximum picking distance
		:queryPoint(sQueryPoint),maxDist2(Math::Constants<Scalar>::max),
		 pointIndex(0),pickIndex(~Index(0))
		{
		}
	
	/* Methods: */
	bool operator()(const Point& p) // Checks if the given point is closer than the previously picked point; returns true if point was picked
		{
		bool result=false;
		
		/* Check if the point is closer: */
		Scalar dist2=Geometry::sqrDist(queryPoint,p);
		if(maxDist2>dist2)
			{
			maxDist2=dist2;
			pickIndex=pointIndex;
			result=true;
			}
		
		/* Prepare to process the next point: */
		++pointIndex;
		
		return result;
		}
	Scalar getMaxDist2(void) const // Returns the squared distance to the currently picked point
		{
		return maxDist2;
		}
	Index getNumPoints(void) const // Returns the total number of processed points
		{
		return pointIndex;
		}
	bool havePickedPoint(void) const // Returns true if a point was picked
		{
		return pickIndex!=~Index(0);
		}
	Index getPickIndex(void) const // Returns the index of the picked point, or ~0x0U if no point was picked
		{
		return pickIndex;
		}
	};

}

#endif
