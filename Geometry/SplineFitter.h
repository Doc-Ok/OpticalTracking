/***********************************************************************
SplineFitter - Class to fit a parametric B-spline of degree n to a set
of sample positions with given parameter values.
Copyright (c) 2009 Oliver Kreylos

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

#ifndef GEOMETRY_SPLINEFITTER_INCLUDED
#define GEOMETRY_SPLINEFITTER_INCLUDED

#include <vector>
#include <Geometry/Point.h>

namespace Geometry {

template <class ScalarParam,int dimensionParam>
class SplineFitter
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar; // Scalar type
	static const int dimension=dimensionParam; // Dimension of samples and B-spline
	typedef Geometry::Point<ScalarParam,dimensionParam> Point; // Type for points
	
	struct Sample // Structure for input samples
		{
		/* Elements: */
		public:
		Scalar t; // Sample parameter value
		Point pos; // Sample position
		
		/* Constructors and destructors: */
		Sample(Scalar sT,const Point& sPos)
			:t(sT),pos(sPos)
			{
			}
		};
	
	typedef std::vector<Scalar> KnotVector; // Type for spline knot vectors, sorted in non-decreasing order
	typedef std::vector<Sample> SampleList; // Type for lists of input samples
	typedef std::vector<Point> ControlPointVector; // Type for spline control point vectors
	
	/* Methods: */
	static ControlPointVector fitSpline(const KnotVector& knots,int degree,const SampleList& samples); // Fits a B-spline with the given knot vector to the given sample list
	};

}

#ifndef GEOMETRY_SPLINEFITTER_IMPLEMENTATION
#include <Geometry/SplineFitter.cpp>
#endif

#endif
