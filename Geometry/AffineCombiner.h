/***********************************************************************
AffineCombiner - Class to create points from affine combinations of
arbitrary numbers of source points with arbitrary affine weights.
Copyright (c) 2002-2005 Oliver Kreylos

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

#ifndef GEOMETRY_AFFINECOMBINER_INCLUDED
#define GEOMETRY_AFFINECOMBINER_INCLUDED

#include <Geometry/ComponentArray.h>
#include <Geometry/Point.h>

namespace Geometry {

template <class ScalarParam,int dimensionParam>
class AffineCombiner
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar; // The underlying scalar type
	static const int dimension=dimensionParam; // The combiner's dimension
	typedef Geometry::ComponentArray<ScalarParam,dimensionParam> PointSum; // Type for sums of points
	typedef Geometry::Point<ScalarParam,dimensionParam> Point; // Type of created/used points
	
	/* Elements: */
	private:
	PointSum pointSum; // Sum of affine points used in combination
	Scalar weightSum; // Sum of affine weights used in combination
	
	/* Constructors and destructors: */
	public:
	AffineCombiner(void) // Creates "empty" combiner
		:pointSum(Scalar(0)),
		 weightSum(0)
		{
		}
	
	/* Methods: */
	Point getPoint(void) const // Returns the created point
		{
		Point result;
		for(int i=0;i<dimension;++i)
			result[i]=pointSum[i]/weightSum;
		return result;
		}
	AffineCombiner& reset(void) // Resets the combiner to "empty" state
		{
		for(int i=0;i<dimension;++i)
			pointSum[i]=Scalar(0);
		weightSum=Scalar(0);
		return *this;
		}
	AffineCombiner& addPoint(const Point& p) // Adds point with affine weight 1
		{
		for(int i=0;i<dimension;++i)
			pointSum[i]+=p[i];
		weightSum+=Scalar(1);
		return *this;
		}
	AffineCombiner& addPoint(const Point& p,Scalar weight) // Adds point with given affine weight
		{
		for(int i=0;i<dimension;++i)
			pointSum[i]+=p[i]*weight;
		weightSum+=weight;
		return *this;
		}
	};

}

#endif
