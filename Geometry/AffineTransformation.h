/***********************************************************************
AffineTransformation - Class for general affine transformations.
Copyright (c) 2001-2011 Oliver Kreylos

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

#ifndef GEOMETRY_AFFINETRANSFORMATION_INCLUDED
#define GEOMETRY_AFFINETRANSFORMATION_INCLUDED

#include <Geometry/Vector.h>
#include <Geometry/Point.h>
#include <Geometry/HVector.h>
#include <Geometry/Matrix.h>
#include <Geometry/Rotation.h>

namespace Geometry {

/* Helper class to specialize transformation operations: */

template <class ScalarParam,int dimensionParam>
class AffineTransformationOperations
	{
	/* Embedded classes: */
	public:
	typedef Geometry::Matrix<ScalarParam,dimensionParam,dimensionParam+1> M;
	typedef Geometry::Vector<ScalarParam,dimensionParam> V;
	typedef Geometry::Point<ScalarParam,dimensionParam> P;
	typedef Geometry::HVector<ScalarParam,dimensionParam> HV;
	
	/* Methods: */
	static V transform(const M& m,const V& v);
	static P transform(const M& m,const P& p);
	static HV transform(const M& m,const HV& hv);
	static V inverseTransform(const M& m,const V& v);
	static P inverseTransform(const M& m,const P& p);
	static HV inverseTransform(const M& m,const HV& hv);
	};

template <class ScalarParam>
class AffineTransformationOperations<ScalarParam,2>
	{
	/* Embedded classes: */
	public:
	typedef Geometry::Matrix<ScalarParam,2,3> M;
	typedef Geometry::Vector<ScalarParam,2> V;
	typedef Geometry::Point<ScalarParam,2> P;
	typedef Geometry::HVector<ScalarParam,2> HV;
	
	/* Methods: */
	inline static V transform(const M& m,const V& v)
		{
		return V(m(0,0)*v[0]+m(0,1)*v[1],
		         m(1,0)*v[0]+m(1,1)*v[1]);
		}
	inline static P transform(const M& m,const P& p)
		{
		return P(m(0,0)*p[0]+m(0,1)*p[1]+m(0,2),
		         m(1,0)*p[0]+m(1,1)*p[1]+m(1,2));
		}
	inline static HV transform(const M& m,const HV& hv)
		{
		return HV(m(0,0)*hv[0]+m(0,1)*hv[1]+m(0,2)*hv[2],
		          m(1,0)*hv[0]+m(1,1)*hv[1]+m(1,2)*hv[2],
		          hv[2]);
		}
	inline static V inverseTransform(const M& m,const V& v)
		{
		double det=double(m(0,0))*double(m(1,1))-double(m(1,0))*double(m(0,1));
		return V(ScalarParam((double(m(1,1))*double(v[0])-double(m(0,1))*double(v[1]))/det),
		         ScalarParam((double(m(0,0))*double(v[1])-double(m(1,0))*double(v[0]))/det));
		}
	inline static P inverseTransform(const M& m,const P& p)
		{
		double det=double(m(0,0))*double(m(1,1))-double(m(1,0))*double(m(0,1));
		return P(ScalarParam((double(m(1,1))*(double(p[0])-double(m(0,2)))-double(m(0,1))*(double(p[1])-double(m(1,2))))/det),
		         ScalarParam((double(m(0,0))*(double(p[1])-double(m(1,2)))-double(m(1,0))*(double(p[0])-double(m(0,2))))/det));
		}
	inline static HV inverseTransform(const M& m,const HV& hv)
		{
		double det=double(m(0,0))*double(m(1,1))-double(m(1,0))*double(m(0,1));
		return HV(ScalarParam((double(m(1,1))*(double(hv[0])-double(m(0,2))*double(hv[2]))-double(m(0,1))*(double(hv[1])-double(m(1,2))*double(hv[2])))/det),
		          ScalarParam((double(m(0,0))*(double(hv[1])-double(m(1,2))*double(hv[2]))-double(m(1,0))*(double(hv[0])-double(m(0,2))*double(hv[2])))/det),
		          hv[2]);
		}
	};

template <class ScalarParam>
class AffineTransformationOperations<ScalarParam,3>
	{
	/* Embedded classes: */
	public:
	typedef Geometry::Matrix<ScalarParam,3,4> M;
	typedef Geometry::Vector<ScalarParam,3> V;
	typedef Geometry::Point<ScalarParam,3> P;
	typedef Geometry::HVector<ScalarParam,3> HV;
	
	/* Methods: */
	inline static V transform(const M& m,const V& v)
		{
		return V(m(0,0)*v[0]+m(0,1)*v[1]+m(0,2)*v[2],
		         m(1,0)*v[0]+m(1,1)*v[1]+m(1,2)*v[2],
		         m(2,0)*v[0]+m(2,1)*v[1]+m(2,2)*v[2]);
		}
	inline static P transform(const M& m,const P& p)
		{
		return P(m(0,0)*p[0]+m(0,1)*p[1]+m(0,2)*p[2]+m(0,3),
		         m(1,0)*p[0]+m(1,1)*p[1]+m(1,2)*p[2]+m(1,3),
		         m(2,0)*p[0]+m(2,1)*p[1]+m(2,2)*p[2]+m(2,3));
		}
	inline static HV transform(const M& m,const HV& hv)
		{
		return HV(m(0,0)*hv[0]+m(0,1)*hv[1]+m(0,2)*hv[2]+m(0,3)*hv[3],
		          m(1,0)*hv[0]+m(1,1)*hv[1]+m(1,2)*hv[2]+m(1,3)*hv[3],
		          m(2,0)*hv[0]+m(2,1)*hv[1]+m(2,2)*hv[2]+m(2,3)*hv[3],
		          hv[3]);
		}
	static V inverseTransform(const M& m,const V& v);
	static P inverseTransform(const M& m,const P& p);
	static HV inverseTransform(const M& m,const HV& hv);
	};

/* Forward declarations: */
template <class ScalarParam,int dimensionParam>
class TranslationTransformation;
template <class ScalarParam,int dimensionParam>
class RotationTransformation;
template <class ScalarParam,int dimensionParam>
class OrthonormalTransformation;
template <class ScalarParam,int dimensionParam>
class UniformScalingTransformation;
template <class ScalarParam,int dimensionParam>
class OrthogonalTransformation;
template <class ScalarParam,int dimensionParam>
class ScalingTransformation;

/* Forward declarations for friend functions: */
template <class ScalarParam,int dimensionParam>
class AffineTransformation;
template <class ScalarParam,int dimensionParam>
bool operator==(const AffineTransformation<ScalarParam,dimensionParam>& t1,const AffineTransformation<ScalarParam,dimensionParam>& t2);
template <class ScalarParam,int dimensionParam>
bool operator!=(const AffineTransformation<ScalarParam,dimensionParam>& t1,const AffineTransformation<ScalarParam,dimensionParam>& t2);
template <class ScalarParam,int dimensionParam>
AffineTransformation<ScalarParam,dimensionParam> operator*(const AffineTransformation<ScalarParam,dimensionParam>&,const AffineTransformation<ScalarParam,dimensionParam>&);
template <class ScalarParam,int dimensionParam>
AffineTransformation<ScalarParam,dimensionParam> invert(const AffineTransformation<ScalarParam,dimensionParam>&);

template <class ScalarParam,int dimensionParam>
class AffineTransformation // Class for affine transformations
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar; // Scalar data type
	static const int dimension=dimensionParam; // Dimension of transformations
	typedef Geometry::Vector<ScalarParam,dimensionParam> Vector; // Compatible vector type
	typedef Geometry::Point<ScalarParam,dimensionParam> Point; // Compatible point type
	typedef Geometry::HVector<ScalarParam,dimensionParam> HVector; // Compatible homogenuous vector type
	typedef Geometry::Rotation<ScalarParam,dimensionParam> Rotation; // Compatible rotation type
	typedef Geometry::ComponentArray<ScalarParam,dimensionParam> Scale; // Type for scale factors
	typedef Geometry::Matrix<ScalarParam,dimensionParam,dimensionParam+1> Matrix; // Compatible matrix type
	private:
	typedef AffineTransformationOperations<ScalarParam,dimensionParam> ATO;
	
	/* Elements: */
	private:
	Matrix matrix; // Reduced matrix representing the affine transformation; last row assumed to be (0, ...,0, 1)
	
	/* Constructors and destructors: */
	public:
	AffineTransformation(void) // Creates identity transformation
		:matrix(Scalar(1))
		{
		}
	AffineTransformation(const Matrix& sMatrix) // Elementwise initialization
		:matrix(sMatrix)
		{
		}
	template <class SourceScalarParam>
	AffineTransformation(const AffineTransformation<SourceScalarParam,dimensionParam>& source) // Copy constructor with type conversion
		:matrix(source.getMatrix())
		{
		}
	template <class SourceScalarParam>
	AffineTransformation(const TranslationTransformation<SourceScalarParam,dimensionParam>& source); // Conversion from translation with type conversion
	template <class SourceScalarParam>
	AffineTransformation(const RotationTransformation<SourceScalarParam,dimensionParam>& source); // Conversion from rotation with type conversion
	template <class SourceScalarParam>
	AffineTransformation(const OrthonormalTransformation<SourceScalarParam,dimensionParam>& source); // Conversion from orthonormal transformation with type conversion
	template <class SourceScalarParam>
	AffineTransformation(const UniformScalingTransformation<SourceScalarParam,dimensionParam>& source); // Conversion from uniform scaling with type conversion
	template <class SourceScalarParam>
	AffineTransformation(const OrthogonalTransformation<SourceScalarParam,dimensionParam>& source); // Conversion from orthogonal transformation with type conversion
	template <class SourceScalarParam>
	AffineTransformation(const ScalingTransformation<SourceScalarParam,dimensionParam>& source); // Conversion from non-uniform scaling with type conversion
	
	/* Pseudo-constructors: */
	static const AffineTransformation identity; // The identity transformation
	static AffineTransformation translate(const Vector& sTranslation) // Returns translation
		{
		Matrix sMatrix=Matrix::one;
		for(int i=0;i<dimension;++i)
			sMatrix(i,dimension)=sTranslation[i];
		return AffineTransformation(sMatrix);
		}
	static AffineTransformation translateFromOriginTo(const Point& sP) // Returns translation from origin
		{
		Matrix sMatrix=Matrix::one;
		for(int i=0;i<dimension;++i)
			sMatrix(i,dimension)=sP[i];
		return AffineTransformation(sMatrix);
		}
	static AffineTransformation translateToOriginFrom(const Point& sP) // Returns translation to origin
		{
		Matrix sMatrix=Matrix::one;
		for(int i=0;i<dimension;++i)
			sMatrix(i,dimension)=-sP[i];
		return AffineTransformation(sMatrix);
		}
	static AffineTransformation rotate(const Rotation& sRotation) // Returns rotation
		{
		Matrix sMatrix=Matrix::one;
		sRotation.writeMatrix(sMatrix);
		return AffineTransformation(sMatrix);
		}
	static AffineTransformation rotateAround(const Point& pivot,const Rotation& sRotation); // Returns rotation about given pivot point
	static AffineTransformation scale(Scalar sScaling) // Returns scaling
		{
		Matrix sMatrix=Matrix::one;
		for(int i=0;i<dimension;++i)
			sMatrix(i,i)=sScaling;
		return AffineTransformation(sMatrix);
		}
	static AffineTransformation scaleAround(const Point& pivot,Scalar sScaling); // Returns scaling about given pivot point
	static AffineTransformation scale(const Scale& sScaling) // Returns non-uniform scaling
		{
		Matrix sMatrix=Matrix::one;
		for(int i=0;i<dimension;++i)
			sMatrix(i,i)=sScaling[i];
		return AffineTransformation(sMatrix);
		}
	static AffineTransformation scaleAround(const Point& pivot,const Scale& sScaling); // Returns non-uniform scaling about given pivot point
	template <class SourceScalarParam>
	static AffineTransformation fromRowMajor(const SourceScalarParam* components) // Constructs transformation from array in row-major order with type conversion
		{
		return AffineTransformation(Matrix::fromRowMajor(components));
		}
	template <class SourceScalarParam>
	static AffineTransformation fromColumnMajor(const SourceScalarParam* components) // Constructs transformation from array in column-major order with type conversion
		{
		return AffineTransformation(Matrix::fromColumnMajor(components));
		}
	
	/* Comparison operators: */
	friend bool operator==<>(const AffineTransformation& t1,const AffineTransformation& t2);
	friend bool operator!=<>(const AffineTransformation& t1,const AffineTransformation& t2);
	
	/* Low-level manipulation functions: */
	const Matrix& getMatrix(void) const // Returns reduced matrix
		{
		return matrix;
		}
	Matrix& getMatrix(void) // Ditto
		{
		return matrix;
		}
	void setOrigin(const Point& newOrigin) // Sets origin of coordinate system
		{
		for(int i=0;i<dimension;++i)
			matrix(i,dimension)=newOrigin[i];
		}
	void setDirection(int j,const Vector& newDirection) // Sets j-th direction vector of coordinate system
		{
		for(int i=0;i<dimension;++i)
			matrix(i,j)=newDirection[i];
		}
	
	/* Coordinate system methods: */
	Point getOrigin(void) const // Gets origin of coordinate system
		{
		Point result;
		for(int i=0;i<dimension;++i)
			result[i]=matrix(i,dimension);
		return result;
		}
	Vector getDirection(int j) const // Gets j-th direction vector of coordinate system
		{
		Vector result;
		for(int i=0;i<dimension;++i)
			result[i]=matrix(i,j);
		return result;
		}
	template <class MatrixParam>
	MatrixParam& writeMatrix(MatrixParam& destMatrix) const // Writes transformation into upper-left dimension x (dimension+1) submatrix
		{
		/* Copy all matrix entries: */
		for(int i=0;i<dimension;++i)
			for(int j=0;j<=dimension;++j)
				destMatrix(i,j)=typename MatrixParam::Scalar(matrix(i,j));
		
		return destMatrix;
		}
	
	/* Concatenation methods: */
	AffineTransformation& operator*=(const AffineTransformation& other); // Multiplication assignment (from the right)
	AffineTransformation& leftMultiply(const AffineTransformation& other); // Ditto (from the left)
	friend AffineTransformation operator*<>(const AffineTransformation& t1,const AffineTransformation& t2); // Multiplies two transformations
	AffineTransformation& doInvert(void); // Inverts the transformation
	friend AffineTransformation invert<>(const AffineTransformation& t); // Inverts a transformation
	
	/* Transformation methods: */
	Vector transform(const Vector& v) const // Transforms a vector
		{
		return ATO::transform(matrix,v);
		}
	Point transform(const Point& p) const // Transforms a point
		{
		return ATO::transform(matrix,p);
		}
	HVector transform(const HVector& v) const // Transforms a homogenuous vector
		{
		return ATO::transform(matrix,v);
		}
	Vector inverseTransform(const Vector& v) const // Transforms a vector with the inverse transformation
		{
		return ATO::inverseTransform(matrix,v);
		}
	Point inverseTransform(const Point& p) const // Transforms a point with the inverse transformation
		{
		return ATO::inverseTransform(matrix,p);
		}
	HVector inverseTransform(const HVector& v) const // Transforms a homogenuous vector with the inverse transformation
		{
		return ATO::inverseTransform(matrix,v);
		}
	};

/* Friend functions of class AffineTransformation: */
template <class ScalarParam,int dimensionParam>
inline bool operator==(const AffineTransformation<ScalarParam,dimensionParam>& t1,const AffineTransformation<ScalarParam,dimensionParam>& t2)
	{
	return t1.matrix==t2.matrix;
	}
template <class ScalarParam,int dimensionParam>
inline bool operator!=(const AffineTransformation<ScalarParam,dimensionParam>& t1,const AffineTransformation<ScalarParam,dimensionParam>& t2)
	{
	return t1.matrix!=t2.matrix;
	}

}

#if defined(GEOMETRY_NONSTANDARD_TEMPLATES) && !defined(GEOMETRY_AFFINETRANSFORMATION_IMPLEMENTATION)
#include <Geometry/AffineTransformation.icpp>
#endif

#endif
