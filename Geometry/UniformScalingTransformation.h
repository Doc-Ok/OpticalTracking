/***********************************************************************
UniformScalingTransformation - Class for n-dimensional uniform scaling
transformations.
Copyright (c) 2003-2013 Oliver Kreylos

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

#ifndef GEOMETRY_UNIFORMSCALINGTRANSFORMATION_INCLUDED
#define GEOMETRY_UNIFORMSCALINGTRANSFORMATION_INCLUDED

#include <Geometry/Vector.h>
#include <Geometry/Point.h>
#include <Geometry/HVector.h>

namespace Geometry {

/* Forward declarations for friend functions: */
template <class ScalarParam,int dimensionParam>
class UniformScalingTransformation;
template <class ScalarParam,int dimensionParam>
bool operator==(const UniformScalingTransformation<ScalarParam,dimensionParam>& t1,const UniformScalingTransformation<ScalarParam,dimensionParam>& t2);
template <class ScalarParam,int dimensionParam>
bool operator!=(const UniformScalingTransformation<ScalarParam,dimensionParam>& t1,const UniformScalingTransformation<ScalarParam,dimensionParam>& t2);
template <class ScalarParam,int dimensionParam>
UniformScalingTransformation<ScalarParam,dimensionParam> operator*(const UniformScalingTransformation<ScalarParam,dimensionParam>&,const UniformScalingTransformation<ScalarParam,dimensionParam>&);
template <class ScalarParam,int dimensionParam>
UniformScalingTransformation<ScalarParam,dimensionParam> invert(const UniformScalingTransformation<ScalarParam,dimensionParam>&);

template <class ScalarParam,int dimensionParam>
class UniformScalingTransformation
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar; // The underlying scalar type
	static const int dimension=dimensionParam; // The uniform scaling's dimension
	typedef Geometry::Vector<ScalarParam,dimensionParam> Vector; // Compatible vector type
	typedef Geometry::Point<ScalarParam,dimensionParam> Point; // Compatible point type
	typedef Geometry::HVector<ScalarParam,dimensionParam> HVector; // Compatible homogenuous vector type
	
	/* Elements: */
	private:
	Scalar scaling; // Uniform scaling factor
	
	/* Constructors and destructors: */
	public:
	UniformScalingTransformation(void) // Creates identity scaling
		:scaling(Scalar(1))
		{
		}
	UniformScalingTransformation(Scalar sScaling) // Creates scaling by given factor
		:scaling(sScaling)
		{
		}
	
	/* Pseudo-constructors: */
	static const UniformScalingTransformation identity; // The identity transformation
	static UniformScalingTransformation scale(Scalar sScaling) // Returns scaling by given factor
		{
		return UniformScalingTransformation(sScaling);
		}
	
	/* Comparison operators: */
	friend bool operator==<>(const UniformScalingTransformation& t1,const UniformScalingTransformation& t2);
	friend bool operator!=<>(const UniformScalingTransformation& t1,const UniformScalingTransformation& t2);
	
	/* Low-level manipulation functions: */
	Scalar getScaling(void) const // Returns the scaling factor
		{
		return scaling;
		}
	void setScaling(Scalar newScaling) // Sets scaling factor
		{
		scaling=newScaling;
		}
	
	/* Coordinate system methods: */
	Point getOrigin(void) const // Returns origin of coordinate system
		{
		return Point::origin;
		}
	Vector getDirection(int j) const // Gets j-th direction vector of coordinate system
		{
		Vector result(Scalar(0));
		result[j]=scaling;
		return result;
		}
	template <class MatrixParam>
	MatrixParam& writeMatrix(MatrixParam& destMatrix) const // Writes scaling into upper-left dimension x dimension submatrix
		{
		for(int i=0;i<dimension;++i)
			for(int j=0;j<dimension;++j)
				destMatrix(i,j)=i==j?scaling:Scalar(0);
		return destMatrix;
		}
	
	/* Concatenation methods: */
	UniformScalingTransformation& operator*=(const UniformScalingTransformation& other) // Multiplication assignment (from the right)
		{
		scaling*=other.scaling;
		return *this;
		}
	UniformScalingTransformation& leftMultiply(const UniformScalingTransformation& other) // Ditto (from the left)
		{
		scaling=other.scaling*scaling;
		return *this;
		}
	friend UniformScalingTransformation operator*<>(const UniformScalingTransformation& t1,const UniformScalingTransformation& t2); // Multiplies two transformations
	UniformScalingTransformation& doInvert(void) // Inverts the transformation
		{
		scaling=Scalar(1)/scaling;
		return *this;
		}
	friend UniformScalingTransformation invert<>(const UniformScalingTransformation& t); // Inverts a transformation
	
	/* Transformation methods: */
	Vector transform(const Vector& v) const // Transforms a vector
		{
		return v*scaling;
		}
	Point transform(const Point& p) const // Transforms a point
		{
		Point result=p;
		for(int i=0;i<dimension;++i)
			result[i]*=scaling;
		return result;
		}
	HVector transform(const HVector& v) const // Transforms a homogenuous vector
		{
		HVector result=v;
		for(int i=0;i<dimension;++i)
			result[i]*=scaling;
		return result;
		}
	Vector inverseTransform(const Vector& v) const // Transforms a vector with the inverse transformation
		{
		return v/scaling;
		}
	Point inverseTransform(const Point& p) const // Transforms a point with the inverse transformation
		{
		Point result=p;
		for(int i=0;i<dimension;++i)
			result[i]/=scaling;
		return result;
		}
	HVector inverseTransform(const HVector& v) const // Transforms a homogenuous vector with the inverse transformation
		{
		HVector result=v;
		for(int i=0;i<dimension;++i)
			result[i]/=scaling;
		return result;
		}
	};

/* Friend functions of class UniformScalingTransformation: */
template <class ScalarParam,int dimensionParam>
inline bool operator==(const UniformScalingTransformation<ScalarParam,dimensionParam>& t1,const UniformScalingTransformation<ScalarParam,dimensionParam>& t2)
	{
	return t1.scaling==t2.scaling;
	}
template <class ScalarParam,int dimensionParam>
inline bool operator!=(const UniformScalingTransformation<ScalarParam,dimensionParam>& t1,const UniformScalingTransformation<ScalarParam,dimensionParam>& t2)
	{
	return t1.scaling!=t2.scaling;
	}
template <class ScalarParam,int dimensionParam>
inline UniformScalingTransformation<ScalarParam,dimensionParam> operator*(const UniformScalingTransformation<ScalarParam,dimensionParam>& t1,const UniformScalingTransformation<ScalarParam,dimensionParam>& t2)
	{
	return UniformScalingTransformation<ScalarParam,dimensionParam>(t1.scaling*t2.scaling);
	}
template <class ScalarParam,int dimensionParam>
inline UniformScalingTransformation<ScalarParam,dimensionParam> invert(const UniformScalingTransformation<ScalarParam,dimensionParam>& t)
	{
	return UniformScalingTransformation<ScalarParam,dimensionParam>(ScalarParam(1)/t.scaling);
	}

}

#if defined(GEOMETRY_NONSTANDARD_TEMPLATES) && !defined(GEOMETRY_UNIFORMSCALINGTRANSFORMATION_IMPLEMENTATION)
#include <Geometry/UniformScalingTransformation.icpp>
#endif

#endif
