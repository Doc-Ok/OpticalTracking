/***********************************************************************
OrthonormalTransformation - Class for transformations constructed from
only translations and rotations.
Copyright (c) 2002-2011 Oliver Kreylos

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

#ifndef GEOMETRY_ORTHONORMALTRANSFORMATION_INCLUDED
#define GEOMETRY_ORTHONORMALTRANSFORMATION_INCLUDED

#include <Geometry/Vector.h>
#include <Geometry/Point.h>
#include <Geometry/HVector.h>
#include <Geometry/Rotation.h>
#include <Geometry/Matrix.h>

namespace Geometry {

/* Forward declarations: */
template <class ScalarParam,int dimensionParam>
class TranslationTransformation;
template <class ScalarParam,int dimensionParam>
class RotationTransformation;

/* Forward declarations for friend functions: */
template <class ScalarParam,int dimensionParam>
class OrthonormalTransformation;
template <class ScalarParam,int dimensionParam>
bool operator==(const OrthonormalTransformation<ScalarParam,dimensionParam>& t1,const OrthonormalTransformation<ScalarParam,dimensionParam>& t2);
template <class ScalarParam,int dimensionParam>
bool operator!=(const OrthonormalTransformation<ScalarParam,dimensionParam>& t1,const OrthonormalTransformation<ScalarParam,dimensionParam>& t2);
template <class ScalarParam,int dimensionParam>
OrthonormalTransformation<ScalarParam,dimensionParam> operator*(const OrthonormalTransformation<ScalarParam,dimensionParam>&,const OrthonormalTransformation<ScalarParam,dimensionParam>&);
template <class ScalarParam,int dimensionParam>
OrthonormalTransformation<ScalarParam,dimensionParam> invert(const OrthonormalTransformation<ScalarParam,dimensionParam>&);

template <class ScalarParam,int dimensionParam>
class OrthonormalTransformation
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar; // The underlying scalar type
	static const int dimension=dimensionParam; // The transformation's dimension
	typedef Geometry::Vector<ScalarParam,dimensionParam> Vector; // Compatible vector type
	typedef Geometry::Point<ScalarParam,dimensionParam> Point; // Compatible point type
	typedef Geometry::HVector<ScalarParam,dimensionParam> HVector; // Compatible homogenuous vector type
	typedef Geometry::Rotation<ScalarParam,dimensionParam> Rotation; // Compatible rotation type
	
	/* Elements: */
	private:
	Vector translation; // The transformation's translation part
	Rotation rotation; // The transformation's rotation part
	
	/* Constructors and destructors: */
	public:
	OrthonormalTransformation(void) // Creates identity transformation
		:translation(Scalar(0))
		{
		}
	OrthonormalTransformation(const Vector& sTranslation,const Rotation& sRotation) // Elementwise initialization
		:translation(sTranslation),rotation(sRotation)
		{
		}
	template <class SourceScalarParam>
	OrthonormalTransformation(const OrthonormalTransformation<SourceScalarParam,dimensionParam>& source) // Copy constructor with type conversion
		:translation(source.getTranslation()),rotation(source.getRotation())
		{
		}
	template <class SourceScalarParam>
	OrthonormalTransformation(const TranslationTransformation<SourceScalarParam,dimensionParam>& source); // Conversion from translation with type conversion
	template <class SourceScalarParam>
	OrthonormalTransformation(const Geometry::RotationTransformation<SourceScalarParam,dimensionParam>& source); // Conversion from rotation with type conversion
	
	/* Pseudo-constructors: */
	static const OrthonormalTransformation identity; // The identity transformation
	static OrthonormalTransformation translate(const Vector& sTranslation) // Returns translation
		{
		return OrthonormalTransformation(sTranslation,Rotation::identity);
		}
	static OrthonormalTransformation translateFromOriginTo(const Point& sP) // Returns translation from origin
		{
		Vector t;
		for(int i=0;i<dimension;++i)
			t[i]=sP[i];
		return OrthonormalTransformation(t,Rotation::identity);
		}
	static OrthonormalTransformation translateToOriginFrom(const Point& sP) // Returns translation to origin
		{
		Vector t;
		for(int i=0;i<dimension;++i)
			t[i]=-sP[i];
		return OrthonormalTransformation(t,Rotation::identity);
		}
	static OrthonormalTransformation rotate(const Rotation& sRotation) // Returns rotation
		{
		return OrthonormalTransformation(Vector::zero,sRotation);
		}
	static OrthonormalTransformation rotateAround(const Point& pivot,const Rotation& sRotation) // Returns rotation about given pivot point
		{
		return OrthonormalTransformation((pivot-Point::origin)+sRotation.transform(Point::origin-pivot),sRotation);
		}
	template<class MatrixParam>
	static OrthonormalTransformation fromMatrix(const MatrixParam& matrix) // Creates transformation from upper-left 3x4 submatrix; assumed to be orthonormal
		{
		return OrthonormalTransformation(Vector(matrix(0,3),matrix(1,3),matrix(2,3)),Rotation::fromMatrix(matrix));
		}
	
	/* Comparison operators: */
	friend bool operator==<>(const OrthonormalTransformation& t1,const OrthonormalTransformation& t2);
	friend bool operator!=<>(const OrthonormalTransformation& t1,const OrthonormalTransformation& t2);
	
	/* Low-level manipulation functions: */
	const Vector& getTranslation(void) const // Returns the translation part
		{
		return translation;
		}
	Vector& getTranslation(void) // Ditto
		{
		return translation;
		}
	const Rotation& getRotation(void) const // Returns the rotation part
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
	Point getOrigin(void) const // Returns origin of coordinate system
		{
		return Point::origin+translation;
		}
	Vector getDirection(int j) const // Gets j-th direction vector of coordinate system
		{
		return rotation.getDirection(j);
		}
	template <class MatrixParam>
	MatrixParam& writeMatrix(MatrixParam& destMatrix) const // Writes transformation into upper-left dimension x (dimension+1) submatrix
		{
		/* Write translation part: */
		for(int i=0;i<dimension;++i)
			destMatrix(i,dimension)=typename MatrixParam::Scalar(translation[i]);
		
		/* Write rotation part: */
		rotation.writeMatrix(destMatrix);
		
		return destMatrix;
		}
	
	/* Concatenation methods: */
	OrthonormalTransformation& operator*=(const OrthonormalTransformation& other)  // Multiplication assignment (from the right)
		{
		translation+=rotation.transform(other.translation);
		rotation*=other.rotation;
		return *this;
		}
	OrthonormalTransformation& leftMultiply(const OrthonormalTransformation& other)  // Ditto (from the left)
		{
		translation=other.translation+other.rotation.transform(translation);
		rotation.leftMultiply(other.rotation);
		return *this;
		}
	friend OrthonormalTransformation operator*<>(const OrthonormalTransformation& t1,const OrthonormalTransformation& t2); // Multiplies two transformations
	OrthonormalTransformation& doInvert(void) // Inverts the transformation
		{
		rotation.doInvert();
		translation=-rotation.transform(translation);
		return *this;
		}
	friend OrthonormalTransformation invert<>(const OrthonormalTransformation& t); // Inverts a transformation
	
	/* Transformation methods: */
	Vector transform(const Vector& v) const // Transforms a vector
		{
		return rotation.transform(v);
		}
	Point transform(const Point& p) const // Transforms a point
		{
		return translation+rotation.transform(p);
		}
	HVector transform(const HVector& v) const // Transforms a homogenuous vector
		{
		HVector result=rotation.transform(v);
		for(int i=0;i<dimension;++i)
			result[i]+=translation[i]*result[dimension];
		return result;
		}
	Vector inverseTransform(const Vector& v) const // Transforms a vector with the inverse transformation
		{
		return rotation.inverseTransform(v);
		}
	Point inverseTransform(const Point& p) const // Transforms a point with the inverse transformation
		{
		return rotation.inverseTransform(p-translation);
		}
	HVector inverseTransform(const HVector& v) const // Transforms a homogenuous vector with the inverse transformation
		{
		HVector result=v;
		for(int i=0;i<dimension;++i)
			result[i]-=translation[i]*result[dimension];
		return rotation.inverseTransform(result);
		}
	};

/* Friend functions of class OrthonormalTransformation: */
template <class ScalarParam,int dimensionParam>
inline bool operator==(const OrthonormalTransformation<ScalarParam,dimensionParam>& t1,const OrthonormalTransformation<ScalarParam,dimensionParam>& t2)
	{
	return t1.translation==t2.translation&&t1.rotation==t2.rotation;
	}
template <class ScalarParam,int dimensionParam>
inline bool operator!=(const OrthonormalTransformation<ScalarParam,dimensionParam>& t1,const OrthonormalTransformation<ScalarParam,dimensionParam>& t2)
	{
	return t1.translation!=t2.translation||t1.rotation!=t2.rotation;
	}
template <class ScalarParam,int dimensionParam>
inline OrthonormalTransformation<ScalarParam,dimensionParam> operator*(const OrthonormalTransformation<ScalarParam,dimensionParam>& t1,const OrthonormalTransformation<ScalarParam,dimensionParam>& t2)
	{
	return OrthonormalTransformation<ScalarParam,dimensionParam>(t1.translation+t1.rotation.transform(t2.translation),t1.rotation*t2.rotation);
	}
template <class ScalarParam,int dimensionParam>
inline OrthonormalTransformation<ScalarParam,dimensionParam> invert(const OrthonormalTransformation<ScalarParam,dimensionParam>& t)
	{
	return OrthonormalTransformation<ScalarParam,dimensionParam>(-t.rotation.inverseTransform(t.translation),invert(t.rotation));
	}

/* Concatenation operators with built-in generalization: */
template <class ScalarParam,int dimensionParam>
OrthonormalTransformation<ScalarParam,dimensionParam> operator*(const TranslationTransformation<ScalarParam,dimensionParam>& t1,const RotationTransformation<ScalarParam,dimensionParam>& t2);
template <class ScalarParam,int dimensionParam>
OrthonormalTransformation<ScalarParam,dimensionParam> operator*(const RotationTransformation<ScalarParam,dimensionParam>& t1,const TranslationTransformation<ScalarParam,dimensionParam>& t2);

}

#if defined(GEOMETRY_NONSTANDARD_TEMPLATES) && !defined(GEOMETRY_ORTHONORMALTRANSFORMATION_IMPLEMENTATION)
#include <Geometry/OrthonormalTransformation.icpp>
#endif

#endif
