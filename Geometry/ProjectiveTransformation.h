/***********************************************************************
ProjectiveTransformation - Class for n-dimensional projective
transformations.
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

#ifndef GEOMETRY_PROJECTIVETRANSFORMATION_INCLUDED
#define GEOMETRY_PROJECTIVETRANSFORMATION_INCLUDED

#include <Geometry/Vector.h>
#include <Geometry/Point.h>
#include <Geometry/HVector.h>
#include <Geometry/Matrix.h>
#include <Geometry/Rotation.h>

namespace Geometry {

/* Helper class to specialize transformation operations: */

template <class ScalarParam,int dimensionParam>
class ProjectiveTransformationOperationsBase
	{
	/* Methods: */
	public:
	static
	Vector<ScalarParam,dimensionParam>
	transformV(
		const Matrix<ScalarParam,dimensionParam+1,dimensionParam+1>& m,
		const Vector<ScalarParam,dimensionParam>& v);
	static
	Point<ScalarParam,dimensionParam>
	transformP(
		const Matrix<ScalarParam,dimensionParam+1,dimensionParam+1>& m,
		const Point<ScalarParam,dimensionParam>& p);
	inline
	static
	HVector<ScalarParam,dimensionParam>
	transformHV(
		const Matrix<ScalarParam,dimensionParam+1,dimensionParam+1>& m,
		const HVector<ScalarParam,dimensionParam>& hv)
		{
		return HVector<ScalarParam,dimensionParam>(m*hv);
		}
	static
	Vector<ScalarParam,dimensionParam>
	inverseTransformV(
		const Matrix<ScalarParam,dimensionParam+1,dimensionParam+1>& m,
		const Vector<ScalarParam,dimensionParam>& v);
	static
	Point<ScalarParam,dimensionParam>
	inverseTransformP(
		const Matrix<ScalarParam,dimensionParam+1,dimensionParam+1>& m,
		const Point<ScalarParam,dimensionParam>& p);
	inline
	static
	HVector<ScalarParam,dimensionParam>
	inverseTransformHV(
		const Matrix<ScalarParam,dimensionParam+1,dimensionParam+1>& m,
		const HVector<ScalarParam,dimensionParam>& hv)
		{
		return HVector<ScalarParam,dimensionParam>(hv/m);
		}
	};

template <class ScalarParam,int dimensionParam>
class ProjectiveTransformationOperations:public ProjectiveTransformationOperationsBase<ScalarParam,dimensionParam>
	{
	};

template <class ScalarParam>
class ProjectiveTransformationOperations<ScalarParam,2>:public ProjectiveTransformationOperationsBase<ScalarParam,2>
	{
	/* Methods: */
	public:
	inline
	static
	Vector<ScalarParam,2>
	transformV(
		const Matrix<ScalarParam,3,3>& m,
		const Vector<ScalarParam,2>& v)
		{
		return Vector<ScalarParam,2>(m(0,0)*v[0]+m(0,1)*v[1],
	                               m(1,0)*v[0]+m(1,1)*v[1]);
		}
	inline
	static
	Point<ScalarParam,2>
	transformP(
		const Matrix<ScalarParam,3,3>& m,
		const Point<ScalarParam,2>& p)
		{
		ScalarParam weight=m(2,0)*p[0]+m(2,1)*p[1]+m(2,2);
		return Point<ScalarParam,2>((m(0,0)*p[0]+m(0,1)*p[1]+m(0,2))/weight,
		                            (m(1,0)*p[0]+m(1,1)*p[1]+m(1,2))/weight);
		}
	inline
	static
	HVector<ScalarParam,2>
	transformHV(
		const Matrix<ScalarParam,3,3>& m,
		const HVector<ScalarParam,2>& hv)
		{
		return HVector<ScalarParam,2>(m(0,0)*hv[0]+m(0,1)*hv[1]+m(0,2)*hv[2],
		                              m(1,0)*hv[0]+m(1,1)*hv[1]+m(1,2)*hv[2],
		                              m(2,0)*hv[0]+m(2,1)*hv[1]+m(2,2)*hv[2]);
		}
	inline
	static
	Vector<ScalarParam,2>
	inverseTransformV(
		const Matrix<ScalarParam,3,3>& m,
		const Vector<ScalarParam,2>& v)
		{
		double sub[3][2];
		sub[0][0]=double(m(1,1))*double(m(2,2))-double(m(2,1))*double(m(1,2));
		sub[0][1]=double(m(1,2))*double(m(2,0))-double(m(2,2))*double(m(1,0));
		sub[1][0]=double(m(2,1))*double(m(0,2))-double(m(0,1))*double(m(2,2));
		sub[1][1]=double(m(2,2))*double(m(0,0))-double(m(0,2))*double(m(2,0));
		sub[2][0]=double(m(0,1))*double(m(1,2))-double(m(1,1))*double(m(0,2));
		double det=double(m(0,0))*sub[0][0]+double(m(1,0))*sub[1][0]+double(m(2,0))*sub[2][0];
		
		return Vector<ScalarParam,2>(ScalarParam((sub[0][0]*double(v[0])+sub[1][0]*double(v[1]))/det),
		                             ScalarParam((sub[0][1]*double(v[0])+sub[1][1]*double(v[1]))/det));
		}
	inline
	static
	Point<ScalarParam,2>
	inverseTransformP(
		const Matrix<ScalarParam,3,3>& m,
		const Point<ScalarParam,2>& p)
		{
		double sub[3][3];
		sub[0][0]=double(m(1,1))*double(m(2,2))-double(m(2,1))*double(m(1,2));
		sub[0][1]=double(m(1,2))*double(m(2,0))-double(m(2,2))*double(m(1,0));
		sub[0][2]=double(m(1,0))*double(m(2,1))-double(m(2,0))*double(m(1,1));
		sub[1][0]=double(m(2,1))*double(m(0,2))-double(m(0,1))*double(m(2,2));
		sub[1][1]=double(m(2,2))*double(m(0,0))-double(m(0,2))*double(m(2,0));
		sub[1][2]=double(m(2,0))*double(m(0,1))-double(m(0,0))*double(m(2,1));
		sub[2][0]=double(m(0,1))*double(m(1,2))-double(m(1,1))*double(m(0,2));
		sub[2][1]=double(m(0,2))*double(m(1,0))-double(m(1,2))*double(m(0,0));
		sub[2][2]=double(m(0,0))*double(m(1,1))-double(m(1,0))*double(m(0,1));
		double weight=sub[0][2]*double(p[0])+sub[1][2]*double(p[1])+sub[2][2];
		return Point<ScalarParam,2>(ScalarParam((sub[0][0]*double(p[0])+sub[1][0]*double(p[1])+sub[2][0])/weight),
		                            ScalarParam((sub[0][1]*double(p[0])+sub[1][1]*double(p[1])+sub[2][1])/weight));
		}
	inline
	static
	HVector<ScalarParam,2>
	inverseTransformHV(
		const Matrix<ScalarParam,3,3>& m,
		const HVector<ScalarParam,2>& hv)
		{
		double sub[3][3];
		sub[0][0]=double(m(1,1))*double(m(2,2))-double(m(2,1))*double(m(1,2));
		sub[0][1]=double(m(1,2))*double(m(2,0))-double(m(2,2))*double(m(1,0));
		sub[0][2]=double(m(1,0))*double(m(2,1))-double(m(2,0))*double(m(1,1));
		sub[1][0]=double(m(2,1))*double(m(0,2))-double(m(0,1))*double(m(2,2));
		sub[1][1]=double(m(2,2))*double(m(0,0))-double(m(0,2))*double(m(2,0));
		sub[1][2]=double(m(2,0))*double(m(0,1))-double(m(0,0))*double(m(2,1));
		sub[2][0]=double(m(0,1))*double(m(1,2))-double(m(1,1))*double(m(0,2));
		sub[2][1]=double(m(0,2))*double(m(1,0))-double(m(1,2))*double(m(0,0));
		sub[2][2]=double(m(0,0))*double(m(1,1))-double(m(1,0))*double(m(0,1));
		double det=double(m(0,0))*sub[0][0]+double(m(1,0))*sub[1][0]+double(m(2,0))*sub[2][0];
		return HVector<ScalarParam,2>(ScalarParam((sub[0][0]*double(hv[0])+sub[1][0]*double(hv[1])+sub[2][0]*double(hv[2]))/det),
		                              ScalarParam((sub[0][1]*double(hv[0])+sub[1][1]*double(hv[1])+sub[2][1]*double(hv[2]))/det),
		                              ScalarParam((sub[0][2]*double(hv[0])+sub[1][2]*double(hv[1])+sub[2][2]*double(hv[2]))/det));
		}
	};

template <class ScalarParam>
class ProjectiveTransformationOperations<ScalarParam,3>:public ProjectiveTransformationOperationsBase<ScalarParam,3>
	{
	/* Methods: */
	public:
	inline
	static
	Vector<ScalarParam,3>
	transformV(
		const Matrix<ScalarParam,4,4>& m,
		const Vector<ScalarParam,3>& v)
		{
		return Vector<ScalarParam,3>(m(0,0)*v[0]+m(0,1)*v[1]+m(0,2)*v[2],
		                             m(1,0)*v[0]+m(1,1)+v[1]+m(1,2)*v[2],
		                             m(2,0)*v[0]+m(2,1)+v[1]+m(2,2)*v[2]);
		}
	inline
	static
	Point<ScalarParam,3>
	transformP(
		const Matrix<ScalarParam,4,4>& m,
		const Point<ScalarParam,3>& p)
		{
		ScalarParam weight=m(3,0)*p[0]+m(3,1)*p[1]+m(3,2)*p[2]+m(3,3);
		return Point<ScalarParam,3>((m(0,0)*p[0]+m(0,1)*p[1]+m(0,2)*p[2]+m(0,3))/weight,
		                            (m(1,0)*p[0]+m(1,1)*p[1]+m(1,2)*p[2]+m(1,3))/weight,
		                            (m(2,0)*p[0]+m(2,1)*p[1]+m(2,2)*p[2]+m(2,3))/weight);
		}
	inline
	static
	HVector<ScalarParam,3>
	transformHV(
		const Matrix<ScalarParam,4,4>& m,
		const HVector<ScalarParam,3>& hv)
		{
		return HVector<ScalarParam,3>(m(0,0)*hv[0]+m(0,1)*hv[1]+m(0,2)*hv[2]+m(0,3)*hv[3],
		                              m(1,0)*hv[0]+m(1,1)*hv[1]+m(1,2)*hv[2]+m(1,3)*hv[3],
		                              m(2,0)*hv[0]+m(2,1)*hv[1]+m(2,2)*hv[2]+m(2,3)*hv[3],
		                              m(3,0)*hv[0]+m(3,1)*hv[1]+m(3,2)*hv[2]+m(3,3)*hv[3]);
		}
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
template <class ScalarParam,int dimensionParam>
class AffineTransformation;

/* Forward declarations for friend functions: */
template <class ScalarParam,int dimensionParam>
class ProjectiveTransformation;
template <class ScalarParam,int dimensionParam>
bool operator==(const ProjectiveTransformation<ScalarParam,dimensionParam>& t1,const ProjectiveTransformation<ScalarParam,dimensionParam>& t2);
template <class ScalarParam,int dimensionParam>
bool operator!=(const ProjectiveTransformation<ScalarParam,dimensionParam>& t1,const ProjectiveTransformation<ScalarParam,dimensionParam>& t2);
template <class ScalarParam,int dimensionParam>
ProjectiveTransformation<ScalarParam,dimensionParam> operator*(const ProjectiveTransformation<ScalarParam,dimensionParam>&,const ProjectiveTransformation<ScalarParam,dimensionParam>&);
template <class ScalarParam,int dimensionParam>
ProjectiveTransformation<ScalarParam,dimensionParam> invert(const ProjectiveTransformation<ScalarParam,dimensionParam>&);

template <class ScalarParam,int dimensionParam>
class ProjectiveTransformation // Class for projective transformations in n-space
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
	typedef Geometry::Matrix<ScalarParam,dimensionParam+1,dimensionParam+1> Matrix; // Compatible matrix type
	private:
	typedef ProjectiveTransformationOperations<ScalarParam,dimensionParam> PTO;
	
	/* Elements: */
	private:
	Matrix matrix; // Matrix representing the transformation
	
	/* Constructors and destructors: */
	public:
	ProjectiveTransformation(void) // Creates identity transformation
		:matrix(Scalar(1))
		{
		}
	ProjectiveTransformation(const Matrix& sMatrix) // Elementwise initialization
		:matrix(sMatrix)
		{
		}
	template <class SourceScalarParam>
	ProjectiveTransformation(const ProjectiveTransformation<SourceScalarParam,dimensionParam>& source) // Copy constructor with type conversion
		:matrix(source.getMatrix())
		{
		}
	template <class SourceScalarParam>
	ProjectiveTransformation(const TranslationTransformation<SourceScalarParam,dimensionParam>& source); // Conversion from translation with type conversion
	template <class SourceScalarParam>
	ProjectiveTransformation(const RotationTransformation<SourceScalarParam,dimensionParam>& source); // Conversion from rotation with type conversion
	template <class SourceScalarParam>
	ProjectiveTransformation(const OrthonormalTransformation<SourceScalarParam,dimensionParam>& source); // Conversion from orthonormal transformation with type conversion
	template <class SourceScalarParam>
	ProjectiveTransformation(const UniformScalingTransformation<SourceScalarParam,dimensionParam>& source); // Conversion from uniform scaling with type conversion
	template <class SourceScalarParam>
	ProjectiveTransformation(const OrthogonalTransformation<SourceScalarParam,dimensionParam>& source); // Conversion from orthogonal transformation with type conversion
	template <class SourceScalarParam>
	ProjectiveTransformation(const ScalingTransformation<SourceScalarParam,dimensionParam>& source); // Conversion from non-uniform scaling with type conversion
	template <class SourceScalarParam>
	ProjectiveTransformation(const AffineTransformation<SourceScalarParam,dimensionParam>& source); // Conversion from affine transformation with type conversion
	
	/* Pseudo-constructors: */
	static const ProjectiveTransformation identity; // The identity transformation
	static ProjectiveTransformation translate(const Vector& sTranslation) // Returns translation
		{
		Matrix sMatrix=Matrix::one;
		for(int i=0;i<dimension;++i)
			sMatrix(i,dimension)=sTranslation[i];
		return ProjectiveTransformation(sMatrix);
		}
	static ProjectiveTransformation translateFromOriginTo(const Point& sP) // Returns translation from origin
		{
		Matrix sMatrix=Matrix::one;
		for(int i=0;i<dimension;++i)
			sMatrix(i,dimension)=sP[i];
		return ProjectiveTransformation(sMatrix);
		}
	static ProjectiveTransformation translateToOriginFrom(const Point& sP) // Returns translation to origin
		{
		Matrix sMatrix=Matrix::one;
		for(int i=0;i<dimension;++i)
			sMatrix(i,dimension)=-sP[i];
		return ProjectiveTransformation(sMatrix);
		}
	static ProjectiveTransformation rotate(const Rotation& sRotation) // Returns rotation
		{
		Matrix sMatrix=Matrix::one;
		sRotation.writeMatrix(sMatrix);
		return ProjectiveTransformation(sMatrix);
		}
	static ProjectiveTransformation rotateAround(const Point& pivot,const Rotation& sRotation); // Returns rotation about given pivot point
	static ProjectiveTransformation scale(Scalar sScaling) // Returns scaling
		{
		Matrix sMatrix=Matrix::one;
		for(int i=0;i<dimension;++i)
			sMatrix(i,i)=sScaling;
		return ProjectiveTransformation(sMatrix);
		}
	static ProjectiveTransformation scaleAround(const Point& pivot,Scalar sScaling); // Returns scaling about given pivot point
	static ProjectiveTransformation scale(const Scale& sScaling) // Returns non-uniform scaling
		{
		Matrix sMatrix=Matrix::one;
		for(int i=0;i<dimension;++i)
			sMatrix(i,i)=sScaling[i];
		return ProjectiveTransformation(sMatrix);
		}
	static ProjectiveTransformation scaleAround(const Point& pivot,const Scale& sScaling); // Returns non-uniform scaling about given pivot point
	template <class SourceScalarParam>
	static ProjectiveTransformation fromRowMajor(const SourceScalarParam* components) // Constructs transformation from array in row-major order with type conversion
		{
		return ProjectiveTransformation(Matrix::fromRowMajor(components));
		}
	template <class SourceScalarParam>
	static ProjectiveTransformation fromColumnMajor(const SourceScalarParam* components) // Constructs transformation from array in column-major order with type conversion
		{
		return ProjectiveTransformation(Matrix::fromColumnMajor(components));
		}
	
	/* Comparison operators: */
	friend bool operator==<>(const ProjectiveTransformation& t1,const ProjectiveTransformation& t2);
	friend bool operator!=<>(const ProjectiveTransformation& t1,const ProjectiveTransformation& t2);
	
	/* Low-level manipulation functions: */
	const Matrix& getMatrix(void) const // Returns matrix
		{
		return matrix;
		}
	Matrix& getMatrix(void) // Ditto
		{
		return matrix;
		}
	void setOrigin(const Point& newOrigin) // Sets origin of coordinate system to point
		{
		for(int i=0;i<dimension;++i)
			matrix(i,dimension)=newOrigin[i];
		matrix(dimension,dimension)=Scalar(1);
		}
	void setOrigin(const HVector& newOrigin) // Sets origin of coordinate system to homogenuous vector
		{
		for(int i=0;i<=dimension;++i)
			matrix(i,dimension)=newOrigin[i];
		}
	void setDirection(int j,const HVector& newDirection) // Sets j-th direction vector of coordinate system to homogenuous vector
		{
		for(int i=0;i<=dimension;++i)
			matrix(i,j)=newDirection[i];
		}
	
	/* Coordinate system methods: */
	Point getOrigin(void) const // Gets origin of coordinate system
		{
		Scalar weight=matrix(dimension,dimension);
		Point result;
		for(int i=0;i<dimension;++i)
			result[i]=matrix(i,dimension)/weight;
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
	MatrixParam& writeMatrix(MatrixParam& destMatrix) const // Writes transformation into upper-left (dimension+1) x (dimension+1) submatrix
		{
		/* Copy all matrix entries: */
		for(int i=0;i<=dimension;++i)
			for(int j=0;j<=dimension;++j)
				destMatrix(i,j)=typename MatrixParam::Scalar(matrix(i,j));
		
		return destMatrix;
		}
	
	/* Concatenation methods: */
	ProjectiveTransformation& operator*=(const ProjectiveTransformation& other) // Multiplication assignment (from the right)
		{
		matrix*=other.matrix;
		return *this;
		}
	ProjectiveTransformation& leftMultiply(const ProjectiveTransformation& other) // Ditto (from the left)
		{
		matrix.leftMultiply(other.matrix);
		return *this;
		}
	friend ProjectiveTransformation operator*<>(const ProjectiveTransformation& t1,const ProjectiveTransformation& t2); // Multiplies two transformations
	ProjectiveTransformation& doInvert(void) // Inverts the transformation
		{
		matrix=invert(matrix);
		return *this;
		}
	friend ProjectiveTransformation invert<>(const ProjectiveTransformation& t); // Inverts a transformation
	
	/* ProjectiveTransformation methods: */
	Vector transform(const Vector& v) const // Transforms a vector
		{
		return PTO::transformV(matrix,v);
		}
	Point transform(const Point& p) const // Transforms a point
		{
		return PTO::transformP(matrix,p);
		}
	HVector transform(const HVector& v) const // Transforms a homogenuous vector
		{
		return PTO::transformHV(matrix,v);
		}
	Vector inverseTransform(const Vector& v) const // Transforms a vector with the inverse transformation
		{
		return PTO::inverseTransformV(matrix,v);
		}
	Point inverseTransform(const Point& p) const // Transforms a point with the inverse transformation
		{
		return PTO::inverseTransformP(matrix,p);
		}
	HVector inverseTransform(const HVector& v) const // Transforms a homogenuous vector with the inverse transformation
		{
		return PTO::inverseTransformHV(matrix,v);
		}
	};

/* Friend functions of class ProjectiveTransformation: */
template <class ScalarParam,int dimensionParam>
inline bool operator==(const ProjectiveTransformation<ScalarParam,dimensionParam>& t1,const ProjectiveTransformation<ScalarParam,dimensionParam>& t2)
	{
	return t1.matrix==t2.matrix;
	}
template <class ScalarParam,int dimensionParam>
inline bool operator!=(const ProjectiveTransformation<ScalarParam,dimensionParam>& t1,const ProjectiveTransformation<ScalarParam,dimensionParam>& t2)
	{
	return t1.matrix!=t2.matrix;
	}

template <class ScalarParam,int dimensionParam>
inline ProjectiveTransformation<ScalarParam,dimensionParam> operator*(const ProjectiveTransformation<ScalarParam,dimensionParam>& t1,const ProjectiveTransformation<ScalarParam,dimensionParam>& t2)
	{
	return ProjectiveTransformation<ScalarParam,dimensionParam>(t1.matrix*t2.matrix);
	}
template <class ScalarParam,int dimensionParam>
inline ProjectiveTransformation<ScalarParam,dimensionParam> invert(const ProjectiveTransformation<ScalarParam,dimensionParam>& t)
	{
	return ProjectiveTransformation<ScalarParam,dimensionParam>(invert(t.matrix));
	}

}

#if defined(GEOMETRY_NONSTANDARD_TEMPLATES) && !defined(GEOMETRY_PROJECTIVETRANSFORMATION_IMPLEMENTATION)
#include <Geometry/ProjectiveTransformation.icpp>
#endif

#endif
