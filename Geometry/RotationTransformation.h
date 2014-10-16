/***********************************************************************
RotationTransformation - Class for n-dimensional rotations.
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

#ifndef GEOMETRY_ROTATIONTRANSFORMATION_INCLUDED
#define GEOMETRY_ROTATIONTRANSFORMATION_INCLUDED

#include <Geometry/Vector.h>
#include <Geometry/Point.h>
#include <Geometry/HVector.h>
#include <Geometry/Rotation.h>

namespace Geometry {

/* Forward declarations for friend functions: */
template <class ScalarParam,int dimensionParam>
class RotationTransformation;
template <class ScalarParam,int dimensionParam>
bool operator==(const RotationTransformation<ScalarParam,dimensionParam>& t1,const RotationTransformation<ScalarParam,dimensionParam>& t2);
template <class ScalarParam,int dimensionParam>
bool operator!=(const RotationTransformation<ScalarParam,dimensionParam>& t1,const RotationTransformation<ScalarParam,dimensionParam>& t2);
template <class ScalarParam,int dimensionParam>
RotationTransformation<ScalarParam,dimensionParam> operator*(const RotationTransformation<ScalarParam,dimensionParam>&,const RotationTransformation<ScalarParam,dimensionParam>&);
template <class ScalarParam,int dimensionParam>
RotationTransformation<ScalarParam,dimensionParam> invert(const RotationTransformation<ScalarParam,dimensionParam>&);

template <class ScalarParam,int dimensionParam>
class RotationTransformation
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar; // The underlying scalar type
	static const int dimension=dimensionParam; // The rotation's dimension
	typedef Geometry::Vector<ScalarParam,dimensionParam> Vector; // Compatible vector type
	typedef Geometry::Point<ScalarParam,dimensionParam> Point; // Compatible point type
	typedef Geometry::HVector<ScalarParam,dimensionParam> HVector; // Compatible homogenuous vector type
	typedef Geometry::Rotation<ScalarParam,dimensionParam> Rotation; // The underlying rotation type
	
	/* Elements: */
	private:
	Rotation rotation; // Rotation
	
	/* Constructors and destructors: */
	public:
	RotationTransformation(void) // Creates identity rotation
		{
		}
	RotationTransformation(const Rotation& sRotation) // Creates rotation transformation from rotation
		:rotation(sRotation)
		{
		}
	template <class SourceScalarParam>
	RotationTransformation(const RotationTransformation<SourceScalarParam,dimensionParam>& source) // Copy constructor with type conversion
		:rotation(source.rotation)
		{
		}
	
	/* Pseudo-constructors: */
	static const RotationTransformation identity; // The identity transformation
	static RotationTransformation rotate(const Rotation& sRotation) // Returns rotation transformation
		{
		return RotationTransformation(sRotation);
		}
	
	/* Comparison operators: */
	friend bool operator==<>(const RotationTransformation& t1,const RotationTransformation& t2);
	friend bool operator!=<>(const RotationTransformation& t1,const RotationTransformation& t2);
	
	/* Low-level manipulation functions: */
	const Rotation& getRotation(void) const // Returns the underlying rotation
		{
		return rotation;
		}
	Rotation& getRotation(void) // Ditto
		{
		return rotation;
		}
	void renormalize(void) // Renormalizes the transformation
		{
		rotation.renormalize();
		}
	
	/* Coordinate system methods: */
	Point getOrigin(void) const // Returns origin of coordinate system
		{
		return Point::origin;
		}
	Vector getDirection(int j) const // Gets j-th direction vector of coordinate system
		{
		return rotation.getDirection(j);
		}
	template <class MatrixParam>
	MatrixParam& writeMatrix(MatrixParam& destMatrix) const // Writes translation into upper-right dimension x 1 submatrix
		{
		return rotation.writeMatrix(destMatrix);
		}
	
	/* Concatenation methods: */
	RotationTransformation& operator*=(const RotationTransformation& other) // Multiplication assignment (from the right)
		{
		rotation*=other.rotation;
		return *this;
		}
	RotationTransformation& leftMultiply(const RotationTransformation& other) // Ditto (from the left)
		{
		rotation=other.rotation*rotation;
		return *this;
		}
	friend RotationTransformation operator*<>(const RotationTransformation& t1,const RotationTransformation& t2); // Multiplies two transformations
	RotationTransformation& doInvert(void) // Inverts the transformation
		{
		rotation.doInvert();
		return *this;
		}
	friend RotationTransformation invert<>(const RotationTransformation& t); // Inverts a transformation
	
	/* Transformation methods: */
	Vector transform(const Vector& v) const // Transforms a vector
		{
		return rotation.transform(v);
		}
	Point transform(const Point& p) const // Transforms a point
		{
		return rotation.transform(p);
		}
	HVector transform(const HVector& v) const // Transforms a homogenuous vector
		{
		return rotation.transform(v);
		}
	Vector inverseTransform(const Vector& v) const // Transforms a vector with the inverse transformation
		{
		return rotation.inverseTransform(v);
		}
	Point inverseTransform(const Point& p) const // Transforms a point with the inverse transformation
		{
		return rotation.inverseTransform(p);
		}
	HVector inverseTransform(const HVector& v) const // Transforms a homogenuous vector with the inverse transformation
		{
		return rotation.inverseTransform(v);
		}
	};

/* Friend functions of class RotationTransformation: */
template <class ScalarParam,int dimensionParam>
inline bool operator==(const RotationTransformation<ScalarParam,dimensionParam>& t1,const RotationTransformation<ScalarParam,dimensionParam>& t2)
	{
	return t1.rotation==t2.rotation;
	}
template <class ScalarParam,int dimensionParam>
inline bool operator!=(const RotationTransformation<ScalarParam,dimensionParam>& t1,const RotationTransformation<ScalarParam,dimensionParam>& t2)
	{
	return t1.rotation!=t2.rotation;
	}
template <class ScalarParam,int dimensionParam>
inline RotationTransformation<ScalarParam,dimensionParam> operator*(const RotationTransformation<ScalarParam,dimensionParam>& t1,const RotationTransformation<ScalarParam,dimensionParam>& t2)
	{
	return RotationTransformation<ScalarParam,dimensionParam>(t1.rotation*t2.rotation);
	}
template <class ScalarParam,int dimensionParam>
inline RotationTransformation<ScalarParam,dimensionParam> invert(const RotationTransformation<ScalarParam,dimensionParam>& t)
	{
	return RotationTransformation<ScalarParam,dimensionParam>(invert(t.rotation));
	}

}

#if defined(GEOMETRY_NONSTANDARD_TEMPLATES) && !defined(GEOMETRY_ROTATIONTRANSFORMATION_IMPLEMENTATION)
#include <Geometry/RotationTransformation.icpp>
#endif

#endif
