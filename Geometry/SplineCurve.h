/***********************************************************************
SplineCurve - Class for n-dimensional non-uniform, non-rational B-spline
curves.
Copyright (c) 2001-2014 Oliver Kreylos

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

#ifndef GEOMETRY_SPLINECURVE_INCLUDED
#define GEOMETRY_SPLINECURVE_INCLUDED

#include <Geometry/Vector.h>
#include <Geometry/Point.h>

namespace Geometry {

/***********************************************************************
The indexing scheme for control points and knots is the one used in
Gerald Farin's book "Curves and Surfaces for CAGD." That is, the domain
of a single B-spline basis function of degree n, N_i^n(u), is the half-
open interval dom(N_i^n(u)) = [u_{i-1}, u_{i+n}). Then a B-spline of
degree n with p control points has a control point vector C = (c_0, c_1,
..., c_{p-1}) and a knot vector U = (u_{-1}, u_0, ..., u_{p-1+n}). Since
the first and last knots, u_{-1} and u_{p-1+n}, are never referenced in
deBoor's algorithm, they are not passed to the SplineCurve constructor
and not stored in the knot array. In other words, the knot array only
contains the p-1+n knots from u_0 to u_{p-2+n}.
Sorry about the confusion.
***********************************************************************/

template <class ScalarParam,int dimensionParam>
class SplineCurve
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar; // Scalar data type
	static const int dimension=dimensionParam; // Dimension of spline curves
	typedef Geometry::Vector<ScalarParam,dimensionParam> Vector; // Compatible vector type
	typedef Geometry::Point<ScalarParam,dimensionParam> Point; // Compatible point type
	
	enum KnotVectorType // Enumerated type for types of automatically generated knot vectors
		{
		UNIFORM, // Uniform knot vector [0, 1, ..., numPoints+degree-2]
		NATURAL // Natural knot vector; interpolates first and last control points
		};
	
	class EvaluationCache // Class to speed up subsequent spline curve evaluations
		{
		friend class SplineCurve;
		
		/* Elements: */
		private:
		Point* points; // Array of intermediate points
		
		/* Constructors and destructors: */
		EvaluationCache(int sDegree)
			:points(new Point[sDegree+1])
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
	int degree; // Polynomial degree of spline curve, >=0
	int numPoints; // Number of control points
	Scalar* knots; // Knot array (only the numPoints+degree-1 interior knots are stored)
	Point* points; // Control point array
	
	/* Private methods: */
	int findInterval(Scalar u) const // Localizes the knot interval containing u (knots[i]<=u<knots[i+1])
		{
		/* Perform binary search on the valid range of the knot vector: */
		int l=degree-1;
		int r=numPoints-1;
		while(r-l>1)
			{
			int t=(l+r)>>1;
			if(u<knots[t])
				r=t;
			else
				l=t;
			}

		return l;
		}
	void deBoorStage(Scalar u,EvaluationCache* cache,int iv,int k) const // Performs a single stage of deBoor's algorithm
		{
		int subDegree=degree-k;
		const Scalar* knotBase=&knots[iv-subDegree+1];
		for(int i=0;i<subDegree;++i,++knotBase)
			{
			Scalar alpha=(u-knotBase[0])/(knotBase[subDegree]-knotBase[0]);
			cache->points[i]=affineCombination(cache->points[i],cache->points[i+1],alpha);
			}
		}
	
	/* Constructors and destructors: */
	public:
	SplineCurve(int sDegree,int sNumPoints); // Creates spline curve with uninitialized knot and control point arrays
	SplineCurve(int sDegree,int sNumPoints,const Point* sPoints,KnotVectorType knotVectorType =UNIFORM); // Creates spline curve from control point array
	SplineCurve(int sDegree,int sNumPoints,const Scalar* sKnots,const Point* sPoints); // Creates spline curve from knot and control point arrays
	SplineCurve(const SplineCurve& source); // Copy constructor
	~SplineCurve(void);
	
	/* Methods: */
	SplineCurve& operator=(const SplineCurve& source); // Assignment operator
	int getDegree(void) const // Returns degree of polynomial curve
		{
		return degree;
		}
	int getNumKnots(void) const // Returns number of interior knots (the ones stored in the knot array)
		{
		return numPoints+degree-1;
		}
	int getNumPoints(void) const // Returns number of control points
		{
		return numPoints;
		}
	int getNumSegments(void) const // Returns number of polynomial segments
		{
		return numPoints-degree;
		}
	Scalar getUMin(void) const // Returns lower bound of valid parameter interval
		{
		return knots[degree-1];
		}
	Scalar getUMax(void) const // Returns upper bound of valid parameter interval
		{
		return knots[numPoints-1];
		}
	Scalar getKnot(int index) const // Returns a knot value
		{
		return knots[index];
		}
	const Point& getPoint(int index) const // Returns a control point
		{
		return points[index];
		}
	EvaluationCache* createEvaluationCache(void) const // Creates an evaluation cache
		{
		return new EvaluationCache(degree);
		}
	Point evaluate(Scalar u,EvaluationCache* cache) const; // Evaluates the spline curve for a parameter
	Point evaluate(Scalar u,EvaluationCache* cache,Vector& deriv1) const; // Calculates point and first derivative
	Point evaluate(Scalar u,EvaluationCache* cache,Vector& deriv1,Vector& deriv2) const; // Calculates point and first and second derivatives
	void setKnot(int index,Scalar newKnot) // Sets an interior knot value
		{
		knots[index]=newKnot;
		}
	void setPoint(int index,const Point& newPoint) // Sets a control point
		{
		points[index]=newPoint;
		}
	template <class TransformationParam>
	SplineCurve& transform(const TransformationParam& t) // Transforms entire spline curve
		{
		/* Transform each control point: */
		for(int i=0;i<numPoints;++i)
			points[i]=t.transform(points[i]);
		return *this;
		}
	SplineCurve& insertKnot(Scalar newKnot); // Inserts new knot at given parameter value without changing curve's shape
	SplineCurve& elevateDegree(void); // Elevates polynomial degree of spline curve by one without changing curve's shape
	};

}

#if defined(GEOMETRY_NONSTANDARD_TEMPLATES) && !defined(GEOMETRY_SPLINECURVE_IMPLEMENTATION)
#include <Geometry/SplineCurve.icpp>
#endif

#endif
