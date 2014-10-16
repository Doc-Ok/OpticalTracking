/***********************************************************************
HVector - Class for homogeneous vectors.
Copyright (c) 2001-2010 Oliver Kreylos

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

#ifndef GEOMETRY_HVECTOR_INCLUDED
#define GEOMETRY_HVECTOR_INCLUDED

#include <Math/Math.h>
#include <Geometry/ComponentArray.h>
#include <Geometry/Vector.h>
#include <Geometry/Point.h>

namespace Geometry {

template <class ScalarParam,int dimensionParam>
class HVector:public ComponentArray<ScalarParam,dimensionParam+1>
	{
	/* Declarations of inherited types/elements: */
	public:
	using ComponentArray<ScalarParam,dimensionParam+1>::dimension;
	using ComponentArray<ScalarParam,dimensionParam+1>::components;
	
	/* Embedded classes: */
	public:
	static const int affineDimension=dimensionParam; // The homogeneous vector's affine dimension
	typedef Geometry::Vector<ScalarParam,dimensionParam> Vector; // Compatible vector type
	typedef Geometry::Point<ScalarParam,dimensionParam> Point; // Compatible point type
	
	/* Constructors and destructors: */
	static const HVector zero; // The zero vector (illegal vector with all zero components)
	static const HVector origin; // The zero point (origin of local coordinate system)
	HVector(void) // No initialization
		{
		}
	explicit HVector(ScalarParam filler,ScalarParam sW) // Fills the component array with a single value and given homogeneous weight
		:ComponentArray<ScalarParam,dimensionParam+1>(filler)
		{
		components[affineDimension]=sW;
		}
	HVector(ScalarParam sX,ScalarParam sY,ScalarParam sW) // Constructor for 2D (Euklidean) vector
		:ComponentArray<ScalarParam,dimensionParam+1>(sX,sY,sW)
		{
		}
	HVector(ScalarParam sX,ScalarParam sY,ScalarParam sZ,ScalarParam sW) // Constructor for 3D (Euklidean) vector
		:ComponentArray<ScalarParam,dimensionParam+1>(sX,sY,sZ,sW)
		{
		}
	template <class SourceScalarParam>
	HVector(const SourceScalarParam array[dimensionParam+1]) // Construction from C-style array
		:ComponentArray<ScalarParam,dimensionParam+1>(array+1)
		{
		}
	template <class SourceScalarParam,int sourceDimensionParam>
	explicit HVector(const ComponentArray<SourceScalarParam,sourceDimensionParam>& source) // Constructs a vector from a component array
		:ComponentArray<ScalarParam,dimensionParam+1>(source)
		{
		}
	template <class SourceScalarParam,int sourceDimensionParam>
	HVector(const HVector<SourceScalarParam,sourceDimensionParam>& source) // Copy constructor with type conversion and dimension change
		:ComponentArray<ScalarParam,dimensionParam+1>(source)
		{
		}
	HVector(const Vector& v) // Constructs homogeneous vector from Euclidean vector
		{
		for(int i=0;i<affineDimension;++i)
			components[i]=v[i];
		components[affineDimension]=ScalarParam(0);
		}
	HVector(const Point& p) // Constructs homogeneous vector from affine point
		{
		for(int i=0;i<affineDimension;++i)
			components[i]=p[i];
		components[affineDimension]=ScalarParam(1);
		}
	
	/* Methods: */
	bool isVector(void) const // Tests whether a homogeneous vector represents a Euclidean vector
		{
		return components[affineDimension]==ScalarParam(0);
		}
	bool isPoint(void) const // Tests whether a homogeneous vector represents an affine point
		{
		return components[affineDimension]!=ScalarParam(0);
		}
	Vector toVector(void) const // Converts homogeneous vector to Euklidean vector
		{
		Vector result;
		for(int i=0;i<affineDimension;++i)
			result[i]=components[i];
		return result;
		}
	Point toPoint(void) const // Converts homogeneous vector to affine point
		{
		Point result;
		for(int i=0;i<affineDimension;++i)
			result[i]=components[i]/components[affineDimension];
		return result;
		}
	HVector operator+(void) const // Unary plus operator; returns copy of vector
		{
		return *this;
		}
	HVector operator-(void) const // Negation operator
		{
		HVector result;
		for(int i=0;i<dimension;++i)
			result.components[i]=-components[i];
		return result;
		}
	HVector& operator+=(const HVector& other) // Addition assignment
		{
		for(int i=0;i<dimension;++i)
			components[i]+=other.components[i];
		return *this;
		}
	HVector& operator-=(const HVector& other) // Subtraction assignment
		{
		for(int i=0;i<dimension;++i)
			components[i]-=other.components[i];
		return *this;
		}
	HVector& operator*=(ScalarParam scalar) // Scalar multiplication assignment
		{
		for(int i=0;i<dimension;++i)
			components[i]*=scalar;
		return *this;
		}
	HVector& operator/=(ScalarParam scalar) // Scalar division assignment
		{
		for(int i=0;i<dimension;++i)
			components[i]/=scalar;
		return *this;
		}
	HVector& project(void) // Projects a vector into affine space by dividing by the homogeneous weight
		{
		for(int i=0;i<affineDimension;++i)
			components[i]/=components[affineDimension];
		components[affineDimension]=ScalarParam(1);
		return *this;
		}
	HVector& normalize(void) // Scales a vector to unit length
		{
		double norm=0.0;
		for(int i=0;i<dimension;++i)
			norm+=Math::sqr(double(components[i]));
		norm=Math::sqrt(norm);
		for(int i=0;i<dimension;++i)
			components[i]/=norm;
		return *this;
		}
	};

/**************************************
Operations on objects of class HVector:
**************************************/

template <class ScalarParam,int dimensionParam>
inline HVector<ScalarParam,dimensionParam> operator+(const HVector<ScalarParam,dimensionParam>& v1,const HVector<ScalarParam,dimensionParam>& v2) // Addition
	{
	HVector<ScalarParam,dimensionParam> result;
	for(int i=0;i<=dimensionParam;++i)
		result[i]=v1[i]+v2[i];
	return result;
	}

template <class ScalarParam>
inline HVector<ScalarParam,2> operator+(const HVector<ScalarParam,2>& v1,const HVector<ScalarParam,2>& v2)
	{
	return HVector<ScalarParam,2>(v1[0]+v2[0],v1[1]+v2[1],v1[2]+v2[2]);
	}

template <class ScalarParam>
inline HVector<ScalarParam,3> operator+(const HVector<ScalarParam,3>& v1,const HVector<ScalarParam,3>& v2)
	{
	return HVector<ScalarParam,3>(v1[0]+v2[0],v1[1]+v2[1],v1[2]+v2[2],v1[3]+v2[3]);
	}

template <class ScalarParam,int dimensionParam>
inline HVector<ScalarParam,dimensionParam> operator-(const HVector<ScalarParam,dimensionParam>& v1,const HVector<ScalarParam,dimensionParam>& v2) // Subtraction
	{
	HVector<ScalarParam,dimensionParam> result;
	for(int i=0;i<=dimensionParam;++i)
		result[i]=v1[i]+v2[i];
	return result;
	}

template <class ScalarParam>
inline HVector<ScalarParam,2> operator-(const HVector<ScalarParam,2>& v1,const HVector<ScalarParam,2>& v2)
	{
	return HVector<ScalarParam,2>(v1[0]-v2[0],v1[1]-v2[1],v1[2]-v2[2]);
	}

template <class ScalarParam>
inline HVector<ScalarParam,3> operator-(const HVector<ScalarParam,3>& v1,const HVector<ScalarParam,3>& v2)
	{
	return HVector<ScalarParam,3>(v1[0]-v2[0],v1[1]-v2[1],v1[2]-v2[2],v1[3]-v2[3]);
	}

template <class ScalarParam,int dimensionParam>
inline HVector<ScalarParam,dimensionParam> operator*(const HVector<ScalarParam,dimensionParam>& v,ScalarParam scalar) // Scalar multiplication (from the right)
	{
	HVector<ScalarParam,dimensionParam> result;
	for(int i=0;i<=dimensionParam;++i)
		result[i]=v[i]*scalar;
	return result;
	}

template <class ScalarParam>
inline HVector<ScalarParam,2> operator*(const HVector<ScalarParam,2>& v,ScalarParam scalar)
	{
	return HVector<ScalarParam,2>(v[0]*scalar,v[1]*scalar,v[2]*scalar);
	}

template <class ScalarParam>
inline HVector<ScalarParam,3> operator*(const HVector<ScalarParam,3>& v,ScalarParam scalar)
	{
	return HVector<ScalarParam,3>(v[0]*scalar,v[1]*scalar,v[2]*scalar,v[3]*scalar);
	}

template <class ScalarParam,int dimensionParam>
inline HVector<ScalarParam,dimensionParam> operator*(ScalarParam scalar,const HVector<ScalarParam,dimensionParam>& v1) // Ditto (from the left)
	{
	HVector<ScalarParam,dimensionParam> result;
	for(int i=0;i<=dimensionParam;++i)
		result[i]=scalar*v1[i];
	return result;
	}

template <class ScalarParam>
inline HVector<ScalarParam,2> operator*(ScalarParam scalar,const HVector<ScalarParam,2>& v)
	{
	return HVector<ScalarParam,2>(scalar*v[0],scalar*v[1],scalar*v[2]);
	}

template <class ScalarParam>
inline HVector<ScalarParam,3> operator*(ScalarParam scalar,const HVector<ScalarParam,3>& v)
	{
	return HVector<ScalarParam,3>(scalar*v[0],scalar*v[1],scalar*v[2],scalar*v[3]);
	}

template <class ScalarParam,int dimensionParam>
inline HVector<ScalarParam,dimensionParam> operator/(const HVector<ScalarParam,dimensionParam>& v1,ScalarParam scalar) // Scalar division
	{
	HVector<ScalarParam,dimensionParam> result;
	for(int i=0;i<=dimensionParam;++i)
		result[i]=v1[i]/scalar;
	return result;
	}

template <class ScalarParam>
inline HVector<ScalarParam,2> operator/(const HVector<ScalarParam,2>& v,ScalarParam scalar)
	{
	return HVector<ScalarParam,2>(v[0]/scalar,v[1]/scalar,v[2]/scalar);
	}

template <class ScalarParam>
inline HVector<ScalarParam,3> operator/(const HVector<ScalarParam,3>& v,ScalarParam scalar)
	{
	return HVector<ScalarParam,3>(v[0]/scalar,v[1]/scalar,v[2]/scalar,v[3]/scalar);
	}

template <class ScalarParam,int dimensionParam>
inline ScalarParam operator*(const HVector<ScalarParam,dimensionParam>& v1,const HVector<ScalarParam,dimensionParam>& v2) // Scalar product of two vectors
	{
	ScalarParam result(0);
	for(int i=0;i<=dimensionParam;++i)
		result+=v1[i]*v2[i];
	return result;
	}

template <class ScalarParam>
inline ScalarParam operator*(const HVector<ScalarParam,2>& v1,const HVector<ScalarParam,2>& v2)
	{
	return v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2];
	}

template <class ScalarParam>
inline ScalarParam operator*(const HVector<ScalarParam,3>& v1,const HVector<ScalarParam,3>& v2)
	{
	return v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2]+v1[3]*v2[3];
	}

template <class ScalarParam,int dimensionParam>
HVector<ScalarParam,dimensionParam> project(const HVector<ScalarParam,dimensionParam>& v) // Projects a vector into affine space by dividing by the homogeneous weight
	{
	HVector<ScalarParam,dimensionParam> result;
	for(int i=0;i<dimensionParam;++i)
		result[i]=v[i]/v[dimensionParam];
	result[dimensionParam]=ScalarParam(1);
	return result;
	}

template <class ScalarParam,int dimensionParam>
HVector<ScalarParam,dimensionParam> normalize(const HVector<ScalarParam,dimensionParam>& v) // Returns a collinear vector of unit length
	{
	double norm=0.0;
	for(int i=0;i<dimensionParam;++i)
		norm+=Math::sqr(double(v[i]));
	norm=Math::sqrt(norm);
	HVector<ScalarParam,dimensionParam> result;
	for(int i=0;i<dimensionParam;++i)
		result[i]=ScalarParam(v[i]/norm);
	return result;
	}

}

#if defined(GEOMETRY_NONSTANDARD_TEMPLATES) && !defined(GEOMETRY_HVECTOR_IMPLEMENTATION)
#include <Geometry/HVector.icpp>
#endif

#endif
