/***********************************************************************
SplinePatch - Class for n-dimensional non-uniform, non-rational tensor-
product B-spline patches.
Copyright (c) 2003-2010 Oliver Kreylos

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

#ifndef GEOMETRY_SPLINEPATCH_INCLUDED
#define GEOMETRY_SPLINEPATCH_INCLUDED

#include <Geometry/ComponentArray.h>
#include <Geometry/Vector.h>
#include <Geometry/Point.h>

namespace Geometry {

/***********************************************************************
The indexing scheme for control points and knots is the same as used in
class SplineCurve (see note there). The only difference is that B-Spline
patches have two knot vectors (the first in u-, the second in v-
direction) and a 2D grid of control points.
Sorry about the confusion.
***********************************************************************/

template <class ScalarParam,int dimensionParam>
class SplinePatch
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar;
	static const int dimension=dimensionParam;
	typedef Geometry::Vector<ScalarParam,dimensionParam> Vector;
	typedef Geometry::Point<ScalarParam,dimensionParam> Point;
	typedef Geometry::ComponentArray<int,2> Size; // Type for array sizes and such
	typedef Geometry::ComponentArray<int,2> Index; // Type for indices in control point and knot arrays
	typedef Geometry::ComponentArray<ScalarParam,2> Parameter; // Type for spline patch parameters
	
	class EvaluationCache // Class to speed up subsequent spline patch evaluations
		{
		friend class SplinePatch;
		
		/* Elements: */
		private:
		Point* points; // 2D Array of intermediate points
		
		/* Constructors and destructors: */
		EvaluationCache(const Size& sDegree)
			:points(new Point[(sDegree[0]+1)*(sDegree[1]+1)])
			{
			}
		public:
		~EvaluationCache(void)
			{
			delete[] points;
			}
		};
	
	/* Elements: */
	private:
	Size degree; // Polynomial degrees of spline patch, >=0
	Size numPoints; // Number of control points
	Scalar* knots[2]; // Knot arrays (only the numPoints[i]+degree[i]-1 interior knots are stored)
	Point* points; // 2D Control point array
	
	/* Private methods: */
	Index findInterval(const Parameter& u) const; // Localizes the knot interval containing the given value (knots[direction][i]<=knot<knots[direction][i+1])
	void deBoorStage0(const Parameter& u,EvaluationCache* cache,const Index& iv,const Size& subDegree) const; // Performs a single stage of deBoor's algorithm along minor patch direction
	void deBoorStage1(const Parameter& u,EvaluationCache* cache,const Index& iv,const Size& subDegree) const; // Performs a single stage of deBoor's algorithm along major patch direction
	
	/* Constructors and destructors: */
	public:
	SplinePatch(const Size& sDegree,const Size& sNumPoints,const Scalar* const sKnots[2],const Point* sPoints =0); // Creates spline patch using the given degrees, node numbers, knot vectors, and node point array
	~SplinePatch(void);
	
	/* Methods: */
	const Size& getDegree(void) const // Returns degrees of polynomial patch
		{
		return degree;
		}
	Size getNumKnots(void) const // Returns number of interior knots (the ones stored in the knot array)
		{
		return Size(numPoints[0]+degree[0]-1,numPoints[1]+degree[1]-1);
		}
	const Size& getNumPoints(void) const // Returns number of control points
		{
		return numPoints;
		}
	Size getNumSegments(void) const // Returns number of polynomial segments
		{
		return Size(numPoints[0]-degree[0],numPoints[1]-degree[1]);
		}
	Parameter getUMin(void) const // Returns lower bound of valid parameter interval
		{
		return Parameter(knots[0][degree[0]-1],knots[1][degree[1]-1]);
		}
	Parameter getUMax(void) const // Returns upper bound of valid parameter interval
		{
		return Parameter(knots[0][numPoints[0]-1],knots[1][numPoints[1]-1]);
		}
	Scalar getKnot(int direction,int index) const // Returns a knot value
		{
		return knots[direction][index];
		}
	const Point& getPoint(const Index& index) const // Returns a control point
		{
		return points[index[1]*numPoints[0]+index[0]];
		}
	EvaluationCache* createEvaluationCache(void) const // Creates an evaluation cache
		{
		return new EvaluationCache(degree);
		}
	Point evaluate(const Parameter& u,EvaluationCache* cache) const; // Evaluates the spline curve for a parameter
	Point evaluate(const Parameter& u,EvaluationCache* cache,Vector& deriv0,Vector& deriv1) const; // Calculates point and first derivative
	void setKnot(int direction,int index,Scalar newKnot) // Sets an interior knot value
		{
		knots[direction][index]=newKnot;
		}
	void setPoint(const Index& index,const Point& newPoint) // Sets a control point
		{
		points[index[1]*numPoints[0]+index[0]]=newPoint;
		}
	template <class TransformationParam>
	SplinePatch& transform(const TransformationParam& t) // Transforms entire spline patch
		{
		/* Transform each control point: */
		for(int i=0;i<numPoints[0]*numPoints[1];++i)
			points[i]=t.transform(points[i]);
		return *this;
		}
	};

}

#if defined(GEOMETRY_NONSTANDARD_TEMPLATES) && !defined(GEOMETRY_SPLINEPATCH_IMPLEMENTATION)
#include <Geometry/SplinePatch.icpp>
#endif

#endif
