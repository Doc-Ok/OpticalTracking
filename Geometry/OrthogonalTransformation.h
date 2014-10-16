/***********************************************************************
OrthogonalTransformation - Class for transformations constructed from
only translations, rotations and uniform scalings.
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

#ifndef GEOMETRY_ORTHOGONALTRANSFORMATION_INCLUDED
#define GEOMETRY_ORTHOGONALTRANSFORMATION_INCLUDED

#include <Geometry/Vector.h>
#include <Geometry/Point.h>
#include <Geometry/HVector.h>
#include <Geometry/Rotation.h>

namespace Geometry {

/* Forward declarations: */
template <class ScalarParam,int dimensionParam>
class TranslationTransformation;
template <class ScalarParam,int dimensionParam>
class RotationTransformation;
template <class ScalarParam,int dimensionParam>
class OrthonormalTransformation;
template <class ScalarParam,int dimensionParam>
class UniformScalingTransformation;

/* Forward declarations for friend functions: */
template <class ScalarParam,int dimensionParam>
class OrthogonalTransformation;
template <class ScalarParam,int dimensionParam>
bool operator==(const OrthogonalTransformation<ScalarParam,dimensionParam>& t1,const OrthogonalTransformation<ScalarParam,dimensionParam>& t2);
template <class ScalarParam,int dimensionParam>
bool operator!=(const OrthogonalTransformation<ScalarParam,dimensionParam>& t1,const OrthogonalTransformation<ScalarParam,dimensionParam>& t2);
template <class ScalarParam,int dimensionParam>
OrthogonalTransformation<ScalarParam,dimensionParam> operator*(const OrthogonalTransformation<ScalarParam,dimensionParam>&,const OrthogonalTransformation<ScalarParam,dimensionParam>&);
template <class ScalarParam,int dimensionParam>
OrthogonalTransformation<ScalarParam,dimensionParam> invert(const OrthogonalTransformation<ScalarParam,dimensionParam>&);

template <class ScalarParam,int dimensionParam>
class OrthogonalTransformation
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
	Scalar scaling; // The transformation's uniform scaling factor
	
	/* Constructors and destructors: */
	public:
	OrthogonalTransformation(void) // Creates identity transformation
		:translation(Scalar(0)),scaling(1)
		{
		}
	OrthogonalTransformation(const Vector& sTranslation,const Rotation& sRotation,Scalar sScaling) // Elementwise initialization
		:translation(sTranslation),rotation(sRotation),scaling(sScaling)
		{
		}
	template <class SourceScalarParam>
	OrthogonalTransformation(const OrthogonalTransformation<SourceScalarParam,dimensionParam>& source) // Copy constructor with type conversion
		:translation(source.getTranslation()),rotation(source.getRotation()),scaling(source.getScaling())
		{
		}
	template <class SourceScalarParam>
	OrthogonalTransformation(const TranslationTransformation<SourceScalarParam,dimensionParam>& source); // Conversion from translation with type conversion
	template <class SourceScalarParam>
	OrthogonalTransformation(const RotationTransformation<SourceScalarParam,dimensionParam>& source); // Conversion from rotation with type conversion
	template <class SourceScalarParam>
	OrthogonalTransformation(const OrthonormalTransformation<SourceScalarParam,dimensionParam>& source); // Conversion from orthonormal transformation with type conversion
	template <class SourceScalarParam>
	OrthogonalTransformation(const UniformScalingTransformation<SourceScalarParam,dimensionParam>& source); // Conversion from uniform scaling with type conversion
	
	/* Pseudo-constructors: */
	static const OrthogonalTransformation identity; // The identity transformation
	static OrthogonalTransformation translate(const Vector& sTranslation) // Returns translation
		{
		return OrthogonalTransformation(sTranslation,Rotation::identity,Scalar(1));
		}
	static OrthogonalTransformation translateFromOriginTo(const Point& sP) // Returns translation from origin
		{
		Vector t;
		for(int i=0;i<dimension;++i)
			t[i]=sP[i];
		return OrthogonalTransformation(t,Rotation::identity,Scalar(1));
		}
	static OrthogonalTransformation translateToOriginFrom(const Point& sP) // Returns translation to origin
		{
		Vector t;
		for(int i=0;i<dimension;++i)
			t[i]=-sP[i];
		return OrthogonalTransformation(t,Rotation::identity,Scalar(1));
		}
	static OrthogonalTransformation rotate(const Rotation& sRotation) // Returns rotation
		{
		return OrthogonalTransformation(Vector::zero,sRotation,Scalar(1));
		}
	static OrthogonalTransformation rotateAround(const Point& pivot,const Rotation& sRotation) // Returns rotation about given pivot point
		{
		return OrthogonalTransformation((pivot-Point::origin)+sRotation.transform(Point::origin-pivot),sRotation,Scalar(1));
		}
	static OrthogonalTransformation scale(Scalar sScaling) // Returns scaling
		{
		return OrthogonalTransformation(Vector::zero,Rotation::identity,sScaling);
		}
	static OrthogonalTransformation scaleAround(const Point& pivot,Scalar sScaling) // Returns scaling about given pivot point
		{
		return OrthogonalTransformation((pivot-Point::origin)+(Point::origin-pivot)*sScaling,Rotation::identity,sScaling);
		}
	
	/* Comparison operators: */
	friend bool operator==<>(const OrthogonalTransformation& t1,const OrthogonalTransformation& t2);
	friend bool operator!=<>(const OrthogonalTransformation& t1,const OrthogonalTransformation& t2);
	
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
	Scalar getScaling(void) const // Returns the scaling part
		{
		return scaling;
		}
	Scalar& getScaling(void) // Ditto
		{
		return scaling;
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
		return rotation.getDirection(j)*scaling;
		}
	template <class MatrixParam>
	MatrixParam& writeMatrix(MatrixParam& destMatrix) const // Writes transformation into upper-left dimension x (dimension+1) submatrix
		{
		/* Write translation part: */
		for(int i=0;i<dimension;++i)
			destMatrix(i,dimension)=typename MatrixParam::Scalar(translation[i]);
		
		/* Write rotation part: */
		rotation.writeMatrix(destMatrix);
		
		/* Write scaling part: */
		for(int i=0;i<dimension;++i)
			for(int j=0;j<dimension;++j)
				destMatrix(i,j)*=typename MatrixParam::Scalar(scaling);
		
		return destMatrix;
		}
	
	/* Concatenation methods: */
	OrthogonalTransformation& operator*=(const OrthogonalTransformation& other)  // Multiplication assignment (from the right)
		{
		translation+=rotation.transform(scaling*other.translation);
		rotation*=other.rotation;
		scaling*=other.scaling;
		return *this;
		}
	OrthogonalTransformation& leftMultiply(const OrthogonalTransformation& other)  // Ditto (from the left)
		{
		translation=other.translation+other.rotation.transform(other.scaling*translation);
		rotation.leftMultiply(other.rotation);
		scaling=other.scaling*scaling;
		return *this;
		}
	friend OrthogonalTransformation operator*<>(const OrthogonalTransformation& t1,const OrthogonalTransformation& t2); // Multiplies two transformations
	OrthogonalTransformation& doInvert(void) // Inverts the transformation
		{
		scaling=Scalar(1)/scaling;
		rotation.doInvert();
		translation=rotation.transform((-scaling)*translation);
		return *this;
		}
	friend OrthogonalTransformation invert<>(const OrthogonalTransformation& t); // Inverts a transformation
	
	/* Transformation methods: */
	Vector transform(const Vector& v) const // Transforms a vector
		{
		return rotation.transform(scaling*v);
		}
	Point transform(const Point& p) const // Transforms a point
		{
		Point sp=p;
		for(int i=0;i<dimension;++i)
			sp[i]*=scaling;
		return translation+rotation.transform(sp);
		}
	HVector transform(const HVector& v) const // Transforms a homogenuous vector
		{
		HVector result=rotation.transform(v);
		for(int i=0;i<dimension;++i)
			result[i]=(result[i]*scaling)+translation[i]*result[dimension];
		return result;
		}
	Vector inverseTransform(const Vector& v) const // Transforms a vector with the inverse transformation
		{
		return rotation.inverseTransform(v/scaling);
		}
	Point inverseTransform(const Point& p) const // Transforms a point with the inverse transformation
		{
		Point sp=p;
		sp-=translation;
		for(int i=0;i<dimension;++i)
			sp[i]/=scaling;
		return rotation.inverseTransform(sp);
		}
	HVector inverseTransform(const HVector& v) const // Transforms a homogenuous vector with the inverse transformation
		{
		HVector sv=v;
		for(int i=0;i<dimension;++i)
			sv[i]=(sv[i]-translation[i]*sv[dimension])/scaling;
		return rotation.inverseTransform(sv);
		}
	};

/* Friend functions of class OrthogonalTransformation: */
template <class ScalarParam,int dimensionParam>
inline bool operator==(const OrthogonalTransformation<ScalarParam,dimensionParam>& t1,const OrthogonalTransformation<ScalarParam,dimensionParam>& t2)
	{
	return t1.scaling==t2.scaling&&t1.translation==t2.translation&&t1.rotation==t2.rotation;
	}
template <class ScalarParam,int dimensionParam>
inline bool operator!=(const OrthogonalTransformation<ScalarParam,dimensionParam>& t1,const OrthogonalTransformation<ScalarParam,dimensionParam>& t2)
	{
	return t1.scaling!=t2.scaling||t1.translation!=t2.translation||t1.rotation!=t2.rotation;
	}
template <class ScalarParam,int dimensionParam>
inline OrthogonalTransformation<ScalarParam,dimensionParam> operator*(const OrthogonalTransformation<ScalarParam,dimensionParam>& t1,const OrthogonalTransformation<ScalarParam,dimensionParam>& t2)
	{
	return OrthogonalTransformation<ScalarParam,dimensionParam>(t1.translation+t1.rotation.transform(t1.scaling*t2.translation),t1.rotation*t2.rotation,t1.scaling*t2.scaling);
	}
template <class ScalarParam,int dimensionParam>
inline OrthogonalTransformation<ScalarParam,dimensionParam> invert(const OrthogonalTransformation<ScalarParam,dimensionParam>& t)
	{
	return OrthogonalTransformation<ScalarParam,dimensionParam>(t.rotation.inverseTransform((ScalarParam(-1)/t.scaling)*t.translation),invert(t.rotation),ScalarParam(1)/t.scaling);
	}

}

#if defined(GEOMETRY_NONSTANDARD_TEMPLATES) && !defined(GEOMETRY_ORTHOGONALTRANSFORMATION_IMPLEMENTATION)
#include <Geometry/OrthogonalTransformation.icpp>
#endif

#endif
