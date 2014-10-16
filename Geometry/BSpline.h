/***********************************************************************
BSpline - Class for n-dimensional non-uniform B-spline curves or
surfaces.
Copyright (c) 2011 Oliver Kreylos

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

#ifndef GEOMETRY_BSPLINE_INCLUDED
#define GEOMETRY_BSPLINE_INCLUDED

#include <Misc/Array.h>
#include <Geometry/LinearInterpolator.h>

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

template <class ControlPointParam,int tensorDimensionParam,class InterpolatorParam =LinearInterpolator<ControlPointParam> >
class BSpline
	{
	/* Embedded classes: */
	public:
	typedef ControlPointParam ControlPoint; // Type for spline control points
	typedef typename ControlPoint::Scalar Scalar; // Scalar data type
	static const int dimension=ControlPoint::dimension; // Dimension of control point space
	static const int tensorDimension=tensorDimensionParam; // Dimension of tensor-product curve/surface/solid
	typedef InterpolatorParam Interpolator; // Type to interpolate between control points
	typedef Misc::Array<ControlPoint,tensorDimension> ControlPointArray; // Type for arrays of control points
	typedef typename ControlPointArray::Index Index; // Type for control point array indices
	typedef Geometry::Point<Scalar,tensorDimension> Parameter; // Type for spline evaluation parameters
	
	enum KnotVectorType // Enumerated type for types of automatically generated knot vectors
		{
		UNIFORM, // Uniform knot vector from 0 to numPoints-degree
		NATURAL // Natural knot vector; interpolates first and last control points
		};
	
	class EvaluationCache // Class to speed up subsequent spline evaluations
		{
		friend class BSpline;
		
		/* Elements: */
		private:
		ControlPointArray points; // Array of intermediate control points used by deBoor's algorithm
		
		/* Constructors and destructors: */
		EvaluationCache(const Index& degree)
			:points(degree+Index(1))
			{
			}
		};
	
	/* Elements: */
	private:
	Index degree; // Polynomial degree of spline, >=0
	ControlPointArray points; // Control point array
	Scalar* knots[tensorDimension]; // Knot arrays (only the numPoints+degree-1 interior knots are stored)
	
	/* Private methods: */
	Index findInterval(const Parameter& u) const; // Localizes the knot interval containing u (knots[i]<=u<knots[i+1])
	void deBoorStage(int tensorDirection,Scalar u,EvaluationCache* cache,int iv,const Index& subDegree) const; // Performs a single stage of deBoor's algorithm
	
	/* Constructors and destructors: */
	public:
	BSpline(const Index& sDegree,const Index& sNumPoints); // Creates B-spline with uninitialized knot and control point arrays
	BSpline(const BSpline& source); // Copy constructor
	BSpline& operator=(const BSpline& source); // Assignment operator
	~BSpline(void); // Destroys the B-spline
	
	/* Methods: */
	const Index& getDegree(void) const // Returns B-spline's polynomial degree
		{
		return degree;
		}
	int getDegree(int tensorDirection) const // Returns B-spline's polynomial degree in the given tensor direction
		{
		return degree[tensorDirection];
		}
	const Index& getNumPoints(void) const // Returns number of control points
		{
		return points.getSize();
		}
	int getNumPoints(int tensorDirection) const // Returns number of control points in the given tensor direction
		{
		return points.getSize(tensorDirection);
		}
	Index getNumKnots(void) const // Returns number of interior knots (the ones stored in the knot array)
		{
		Index result;
		for(int td=0;td<tensorDimension;++td)
			result[td]=points.getSize(td)+degree[td]-1;
		return result;
		}
	int getNumKnots(int tensorDirection) const // Returns number of interior knots (the ones stored in the knot array) in the given tensor direction
		{
		return points.getSize(tensorDirection)+degree[tensorDirection]-1;
		}
	Index getNumSegments(void) const // Returns number of polynomial segments
		{
		Index result;
		for(int td=0;td<tensorDimension;++td)
			result[td]=points.getSize(td)-degree[td];
		return result;
		}
	int getNumSegments(int tensorDirection) const // Returns number of polynomial segments in the given tensor direction
		{
		return points.getSize(tensorDirection)-degree[tensorDirection];
		}
	Parameter getUMin(void) const // Returns lower bound of valid parameter interval
		{
		Parameter result;
		for(int td=0;td<tensorDimension;++td)
			result[td]=knots[td][degree[td]-1];
		return result;
		}
	Scalar getUMin(int tensorDirection) const // Returns lower bound of valid parameter interval in the given tensor direction
		{
		return knots[tensorDirection][degree[tensorDirection]-1];
		}
	Parameter getUMax(void) const // Returns upper bound of valid parameter interval
		{
		Parameter result;
		for(int td=0;td<tensorDimension;++td)
			result[td]=knots[td][points.getSize(td)-1];
		return result;
		}
	Scalar getUMax(int tensorDirection) const // Returns upper bound of valid parameter interval in the given tensor direction
		{
		return knots[tensorDirection][points.getSize(tensorDirection)-1];
		}
	Parameter getKnot(const Index& index) const // Returns a tensor product knot value
		{
		Parameter result;
		for(int td=0;td<tensorDimension;++td)
			result[td]=knots[td][index[td]];
		return result;
		}
	Scalar getKnot(int tensorDirection,int index) const // Returns a knot value from the given tensor direction
		{
		return knots[tensorDirection][index];
		}
	const ControlPoint& getPoint(const Index& index) const // Returns a control point
		{
		return points[index];
		}
	EvaluationCache* createEvaluationCache(void) const // Creates an evaluation cache
		{
		return new EvaluationCache(degree);
		}
	ControlPoint evaluate(const Parameter& u,EvaluationCache* cache) const; // Evaluates the B-spline for the given parameter
	void setKnot(const Index& index,const Parameter& newKnot) // Sets an interior knot value
		{
		for(int td=0;td<tensorDimension;++td)
			knots[td][index[td]]=newKnot[td];
		}
	void setKnot(int tensorDirection,int index,Scalar newKnot) // Sets an interior knot value in the given tensor direction
		{
		knots[tensorDirection][index]=newKnot;
		}
	void setPoint(const Index& index,const ControlPoint& newPoint) // Sets a control point
		{
		points[index]=newPoint;
		}
	template <class TransformationParam>
	BSpline& transform(const TransformationParam& t) // Transforms entire B-spline
		{
		/* Transform each control point: */
		for(typename ControlPointArray::iterator pIt=points.begin();pIt!=points.end();++pIt)
			*pIt=t.transform(*pIt);
		return *this;
		}
	};

}

#if defined(GEOMETRY_NONSTANDARD_TEMPLATES) && !defined(GEOMETRY_BSPLINE_IMPLEMENTATION)
#include <Geometry/BSpline.icpp>
#endif

#endif
