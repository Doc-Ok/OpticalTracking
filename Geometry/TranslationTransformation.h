/***********************************************************************
TranslationTransformation - Class for n-dimensional translations.
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

#ifndef GEOMETRY_TRANSLATIONTRANSFORMATION_INCLUDED
#define GEOMETRY_TRANSLATIONTRANSFORMATION_INCLUDED

#include <Geometry/Vector.h>
#include <Geometry/Point.h>
#include <Geometry/HVector.h>

namespace Geometry {

/* Forward declarations for friend functions: */
template <class ScalarParam,int dimensionParam>
class TranslationTransformation;
template <class ScalarParam,int dimensionParam>
bool operator==(const TranslationTransformation<ScalarParam,dimensionParam>& t1,const TranslationTransformation<ScalarParam,dimensionParam>& t2);
template <class ScalarParam,int dimensionParam>
bool operator!=(const TranslationTransformation<ScalarParam,dimensionParam>& t1,const TranslationTransformation<ScalarParam,dimensionParam>& t2);
template <class ScalarParam,int dimensionParam>
TranslationTransformation<ScalarParam,dimensionParam> operator*(const TranslationTransformation<ScalarParam,dimensionParam>&,const TranslationTransformation<ScalarParam,dimensionParam>&);
template <class ScalarParam,int dimensionParam>
TranslationTransformation<ScalarParam,dimensionParam> invert(const TranslationTransformation<ScalarParam,dimensionParam>&);

template <class ScalarParam,int dimensionParam>
class TranslationTransformation
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar; // The underlying scalar type
	static const int dimension=dimensionParam; // The translation's dimension
	typedef Geometry::Vector<ScalarParam,dimensionParam> Vector; // Compatible vector type
	typedef Geometry::Point<ScalarParam,dimensionParam> Point; // Compatible point type
	typedef Geometry::HVector<ScalarParam,dimensionParam> HVector; // Compatible homogenuous vector type
	
	/* Elements: */
	private:
	Vector translation; // Translation vector
	
	/* Constructors and destructors: */
	public:
	TranslationTransformation(void) // Creates identity translation
		:translation(Scalar(0))
		{
		}
	TranslationTransformation(const Vector& sTranslation) // Creates translation by given offset vector
		:translation(sTranslation)
		{
		}
	template <class SourceScalarParam>
	TranslationTransformation(const TranslationTransformation<SourceScalarParam,dimensionParam>& source) // Copy constructor with type conversion
		:translation(source.translation)
		{
		}
	
	/* Pseudo-constructors: */
	static const TranslationTransformation identity; // The identity transformation
	static TranslationTransformation translate(const Vector& sTranslation) // Returns translation
		{
		return TranslationTransformation(sTranslation);
		}
	static TranslationTransformation translateFromOriginTo(const Point& sP) // Returns translation from origin
		{
		Vector t;
		for(int i=0;i<dimension;++i)
			t[i]=sP[i];
		return TranslationTransformation(t);
		}
	static TranslationTransformation translateToOriginFrom(const Point& sP) // Returns translation to origin
		{
		Vector t;
		for(int i=0;i<dimension;++i)
			t[i]=-sP[i];
		return TranslationTransformation(t);
		}
	
	/* Comparison operators: */
	friend bool operator==<>(const TranslationTransformation& t1,const TranslationTransformation& t2);
	friend bool operator!=<>(const TranslationTransformation& t1,const TranslationTransformation& t2);
	
	/* Low-level manipulation functions: */
	const Vector& getTranslation(void) const // Returns the translation vector
		{
		return translation;
		}
	Vector& getTranslation(void) // Ditto
		{
		return translation;
		}
	
	/* Coordinate system methods: */
	Point getOrigin(void) const // Returns origin of coordinate system
		{
		return Point::origin+translation;
		}
	Vector getDirection(int j) const // Gets j-th direction vector of coordinate system
		{
		Vector result(Scalar(0));
		result[j]=Scalar(1);
		return result;
		}
	template <class MatrixParam>
	MatrixParam& writeMatrix(MatrixParam& destMatrix) const // Writes translation into upper-right dimension x 1 submatrix
		{
		for(int i=0;i<dimension;++i)
			destMatrix(i,dimension)=typename MatrixParam::Scalar(translation[i]);
		return destMatrix;
		}
	
	/* Concatenation methods: */
	TranslationTransformation& operator*=(const TranslationTransformation& other) // Multiplication assignment (from the right)
		{
		translation+=other.translation;
		return *this;
		}
	TranslationTransformation& leftMultiply(const TranslationTransformation& other) // Ditto (from the left)
		{
		translation=other.translation+translation;
		return *this;
		}
	friend TranslationTransformation operator*<>(const TranslationTransformation& t1,const TranslationTransformation& t2); // Multiplies two transformations
	TranslationTransformation& doInvert(void) // Inverts the transformation
		{
		translation=-translation;
		return *this;
		}
	friend TranslationTransformation invert<>(const TranslationTransformation& t); // Inverts a transformation
	
	/* Transformation methods: */
	Vector transform(const Vector& v) const // Transforms a vector
		{
		return v;
		}
	Point transform(const Point& p) const // Transforms a point
		{
		return p+translation;
		}
	HVector transform(const HVector& v) const // Transforms a homogenuous vector
		{
		HVector result=v;
		for(int i=0;i<dimension;++i)
			result[i]+=translation[i]*v[dimension];
		return result;
		}
	Vector inverseTransform(const Vector& v) const // Transforms a vector with the inverse transformation
		{
		return v;
		}
	Point inverseTransform(const Point& p) const // Transforms a point with the inverse transformation
		{
		return p-translation;
		}
	HVector inverseTransform(const HVector& v) const // Transforms a homogenuous vector with the inverse transformation
		{
		HVector result=v;
		for(int i=0;i<dimension;++i)
			result[i]-=translation[i]*v[dimension];
		return result;
		}
	};

/* Friend functions of class TranslationTransformation: */
template <class ScalarParam,int dimensionParam>
inline bool operator==(const TranslationTransformation<ScalarParam,dimensionParam>& t1,const TranslationTransformation<ScalarParam,dimensionParam>& t2)
	{
	return t1.translation==t2.translation;
	}
template <class ScalarParam,int dimensionParam>
inline bool operator!=(const TranslationTransformation<ScalarParam,dimensionParam>& t1,const TranslationTransformation<ScalarParam,dimensionParam>& t2)
	{
	return t1.translation!=t2.translation;
	}
template <class ScalarParam,int dimensionParam>
inline TranslationTransformation<ScalarParam,dimensionParam> operator*(const TranslationTransformation<ScalarParam,dimensionParam>& t1,const TranslationTransformation<ScalarParam,dimensionParam>& t2)
	{
	return TranslationTransformation<ScalarParam,dimensionParam>(t1.translation+t2.translation);
	}
template <class ScalarParam,int dimensionParam>
inline TranslationTransformation<ScalarParam,dimensionParam> invert(const TranslationTransformation<ScalarParam,dimensionParam>& t)
	{
	return TranslationTransformation<ScalarParam,dimensionParam>(-t.translation);
	}

}

#if defined(GEOMETRY_NONSTANDARD_TEMPLATES) && !defined(GEOMETRY_TRANSLATIONTRANSFORMATION_IMPLEMENTATION)
#include <Geometry/TranslationTransformation.icpp>
#endif

#endif
