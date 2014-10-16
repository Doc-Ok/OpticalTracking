/***********************************************************************
ScalingTransformation - Class for n-dimensional non-uniform scaling
transformations.
Copyright (c) 2003-2011 Oliver Kreylos

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

#ifndef GEOMETRY_SCALINGTRANSFORMATION_INCLUDED
#define GEOMETRY_SCALINGTRANSFORMATION_INCLUDED

#include <Geometry/ComponentArray.h>
#include <Geometry/Vector.h>
#include <Geometry/Point.h>
#include <Geometry/HVector.h>

namespace Geometry {

/* Forward declarations: */
template <class ScalarParam,int dimensionParam>
class UniformScalingTransformation;

/* Forward declarations for friend functions: */
template <class ScalarParam,int dimensionParam>
class ScalingTransformation;
template <class ScalarParam,int dimensionParam>
bool operator==(const ScalingTransformation<ScalarParam,dimensionParam>& t1,const ScalingTransformation<ScalarParam,dimensionParam>& t2);
template <class ScalarParam,int dimensionParam>
bool operator!=(const ScalingTransformation<ScalarParam,dimensionParam>& t1,const ScalingTransformation<ScalarParam,dimensionParam>& t2);
template <class ScalarParam,int dimensionParam>
ScalingTransformation<ScalarParam,dimensionParam> operator*(const ScalingTransformation<ScalarParam,dimensionParam>&,const ScalingTransformation<ScalarParam,dimensionParam>&);
template <class ScalarParam,int dimensionParam>
ScalingTransformation<ScalarParam,dimensionParam> invert(const ScalingTransformation<ScalarParam,dimensionParam>&);

template <class ScalarParam,int dimensionParam>
class ScalingTransformation
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar; // The underlying scalar type
	static const int dimension=dimensionParam; // The scaling's dimension
	typedef Geometry::ComponentArray<ScalarParam,dimensionParam> Scale; // Type for scaling factors
	typedef Geometry::Vector<ScalarParam,dimensionParam> Vector; // Compatible vector type
	typedef Geometry::Point<ScalarParam,dimensionParam> Point; // Compatible point type
	typedef Geometry::HVector<ScalarParam,dimensionParam> HVector; // Compatible homogenuous vector type
	
	/* Elements: */
	private:
	Scale scaling; // Scaling factor
	
	/* Constructors and destructors: */
	public:
	ScalingTransformation(void) // Creates identity scaling
		:scaling(Scalar(1))
		{
		}
	ScalingTransformation(Scalar sScaling) // Creates uniform scaling by given factor
		:scaling(sScaling)
		{
		}
	ScalingTransformation(const Scale& sScaling) // Creates scaling by given factor
		:scaling(sScaling)
		{
		}
	template <class SourceScalarParam>
	ScalingTransformation(const UniformScalingTransformation<SourceScalarParam,dimensionParam>& source); // Conversion from uniform scaling with type conversion
	
	/* Pseudo-constructors: */
	static const ScalingTransformation identity; // The identity transformation
	static ScalingTransformation scale(Scalar sScaling) // Returns uniform scaling by given factor
		{
		return ScalingTransformation(sScaling);
		}
	static ScalingTransformation scale(const Scale& sScaling) // Returns scaling by given factor
		{
		return ScalingTransformation(sScaling);
		}
	
	/* Comparison operators: */
	friend bool operator==<>(const ScalingTransformation& t1,const ScalingTransformation& t2);
	friend bool operator!=<>(const ScalingTransformation& t1,const ScalingTransformation& t2);
	
	/* Low-level manipulation functions: */
	const Scale& getScaling(void) const // Returns the scaling factor
		{
		return scaling;
		}
	void setScaling(const Scale& newScaling) // Sets scaling factor
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
		result[j]=scaling[j];
		return result;
		}
	template <class MatrixParam>
	MatrixParam& writeMatrix(MatrixParam& destMatrix) const // Writes scaling into upper-left dimension x dimension submatrix
		{
		for(int i=0;i<dimension;++i)
			for(int j=0;j<dimension;++j)
				destMatrix(i,j)=i==j?scaling[i]:Scalar(0);
		return destMatrix;
		}
	
	/* Concatenation methods: */
	ScalingTransformation& operator*=(const ScalingTransformation& other) // Multiplication assignment (from the right)
		{
		for(int i=0;i<dimension;++i)
			scaling[i]*=other.scaling[i];
		return *this;
		}
	ScalingTransformation& leftMultiply(const ScalingTransformation& other) // Ditto (from the left)
		{
		for(int i=0;i<dimension;++i)
			scaling[i]=other.scaling[i]*scaling[i];
		return *this;
		}
	friend ScalingTransformation operator*<>(const ScalingTransformation& t1,const ScalingTransformation& t2); // Multiplies two transformations
	ScalingTransformation& doInvert(void) // Inverts the transformation
		{
		for(int i=0;i<dimension;++i)
			scaling[i]=Scalar(1)/scaling[i];
		return *this;
		}
	friend ScalingTransformation invert<>(const ScalingTransformation& t); // Inverts a transformation
	
	/* Transformation methods: */
	Vector transform(const Vector& v) const // Transforms a vector
		{
		Vector result=v;
		for(int i=0;i<dimension;++i)
			result[i]*=scaling[i];
		return result;
		}
	Point transform(const Point& p) const // Transforms a point
		{
		Point result=p;
		for(int i=0;i<dimension;++i)
			result[i]*=scaling[i];
		return result;
		}
	HVector transform(const HVector& v) const // Transforms a homogenuous vector
		{
		HVector result=v;
		for(int i=0;i<dimension;++i)
			result[i]*=scaling[i];
		return result;
		}
	Vector inverseTransform(const Vector& v) const // Transforms a vector with the inverse transformation
		{
		Vector result=v;
		for(int i=0;i<dimension;++i)
			result[i]/=scaling[i];
		return result;
		}
	Point inverseTransform(const Point& p) const // Transforms a point with the inverse transformation
		{
		Point result=p;
		for(int i=0;i<dimension;++i)
			result[i]/=scaling[i];
		return result;
		}
	HVector inverseTransform(const HVector& v) const // Transforms a homogenuous vector with the inverse transformation
		{
		HVector result=v;
		for(int i=0;i<dimension;++i)
			result[i]/=scaling[i];
		return result;
		}
	};

/* Friend functions of class ScalingTransformation: */
template <class ScalarParam,int dimensionParam>
inline bool operator==(const ScalingTransformation<ScalarParam,dimensionParam>& t1,const ScalingTransformation<ScalarParam,dimensionParam>& t2)
	{
	return t1.scaling==t2.scaling;
	}
template <class ScalarParam,int dimensionParam>
inline bool operator!=(const ScalingTransformation<ScalarParam,dimensionParam>& t1,const ScalingTransformation<ScalarParam,dimensionParam>& t2)
	{
	return t1.scaling!=t2.scaling;
	}
template <class ScalarParam,int dimensionParam>
inline ScalingTransformation<ScalarParam,dimensionParam> operator*(const ScalingTransformation<ScalarParam,dimensionParam>& t1,const ScalingTransformation<ScalarParam,dimensionParam>& t2)
	{
	typename ScalingTransformation<ScalarParam,dimensionParam>::Scale scaling;
	for(int i=0;i<dimensionParam;++i)
		scaling[i]=t1.scaling[i]*t2.scaling[i];
	return ScalingTransformation<ScalarParam,dimensionParam>(scaling);
	}
template <class ScalarParam,int dimensionParam>
inline ScalingTransformation<ScalarParam,dimensionParam> invert(const ScalingTransformation<ScalarParam,dimensionParam>& t)
	{
	typename ScalingTransformation<ScalarParam,dimensionParam>::Scale scaling;
	for(int i=0;i<dimensionParam;++i)
		scaling[i]=ScalarParam(1)/t.scaling[i];
	return ScalingTransformation<ScalarParam,dimensionParam>(scaling);
	}

}

#if defined(GEOMETRY_NONSTANDARD_TEMPLATES) && !defined(GEOMETRY_SCALINGTRANSFORMATION_IMPLEMENTATION)
#include <Geometry/ScalingTransformation.icpp>
#endif

#endif
