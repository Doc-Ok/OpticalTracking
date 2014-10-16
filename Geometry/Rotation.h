/***********************************************************************
Rotation - Class for 2D and 3D rotations.
Copyright (c) 2002-2013 Oliver Kreylos

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

#ifndef GEOMETRY_ROTATION_INCLUDED
#define GEOMETRY_ROTATION_INCLUDED

#include <Math/Math.h>
#include <Geometry/Vector.h>
#include <Geometry/Point.h>
#include <Geometry/HVector.h>

namespace Geometry {

/*************************************************
"Dummy" generic class for n-dimensional rotations.
*************************************************/

template <class ScalarParam,int dimensionParam>
class Rotation
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar; // The underlying scalar type
	static const int dimension=dimensionParam; // The rotation's dimension
	typedef Geometry::Vector<ScalarParam,dimensionParam> Vector; // Compatible vector type
	typedef Geometry::Point<ScalarParam,dimensionParam> Point; // Compatible point type
	typedef Geometry::HVector<ScalarParam,dimensionParam> HVector; // Compatible homogenuous vector type
	
	/* General rotations are not implemented */
	};

/**********************************
Specialized class for 2D rotations:
**********************************/

/* Forward declarations for friend functions: */
template <class ScalarParam>
class Rotation<ScalarParam,2>;
template <class ScalarParam>
bool operator==(const Rotation<ScalarParam,2>& r1,const Rotation<ScalarParam,2>& r2);
template <class ScalarParam>
bool operator!=(const Rotation<ScalarParam,2>& r1,const Rotation<ScalarParam,2>& r2);
template <class ScalarParam>
Rotation<ScalarParam,2> operator*(const Rotation<ScalarParam,2>& r1,const Rotation<ScalarParam,2>& r2);
template <class ScalarParam>
Rotation<ScalarParam,2> invert(const Rotation<ScalarParam,2>& r);
template <class ScalarParam>
Rotation<ScalarParam,2> operator/(const Rotation<ScalarParam,2>& r1,const Rotation<ScalarParam,2>& r2);

template <class ScalarParam>
class Rotation<ScalarParam,2>
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar; // The underlying scalar type
	static const int dimension=2; // The rotation's dimension
	typedef Geometry::Vector<ScalarParam,2> Vector; // Compatible vector type
	typedef Geometry::Point<ScalarParam,2> Point; // Compatible point type
	typedef Geometry::HVector<ScalarParam,2> HVector; // Compatible homogenuous vector type
	
	/* Elements: */
	private:
	Scalar angle; // Rotation angle in radians wrapped to [0, 2*pi)
	
	/* Constructors and destructors: */
	public:
	Rotation(void) // Creates identity rotation
		:angle(Scalar(0))
		{
		}
	Rotation(Scalar sAngle) // Creates rotation with angle in radians
		:angle(Math::wrapRad(sAngle))
		{
		}
	template <class SourceScalarParam>
	Rotation(const Rotation<SourceScalarParam,2>& source) // Copy constructor with type conversion
		:angle(Scalar(source.getAngle()))
		{
		}
	
	/* Pseudo-constructors: */
	static const Rotation identity; // Identity rotation
	static Rotation rotate(Scalar angle) // Returns rotation
		{
		return Rotation(angle);
		}
	
	/* Comparison operators: */
	friend bool operator==<>(const Rotation& r1,const Rotation& r2);
	friend bool operator!=<>(const Rotation& r1,const Rotation& r2);
	
	/* Low-level manipulation methods: */
	Scalar getAngle(void) const // Returns rotation angle
		{
		return angle;
		}
	void setAngle(Scalar newAngle) // Sets rotation angle
		{
		angle=Math::wrapRad(newAngle);
		}
	void renormalize(void) // Renormalizes the rotation
		{
		/* Unnecessary for 2D rotations! */
		}
	
	/* Coordinate system methods: */
	Vector getDirection(int j) const // Gets j-th direction vector of coordinate system
		{
		Vector result;
		switch(j)
			{
			case 0:
				result[0]=Math::cos(angle);
				result[1]=Math::sin(angle);
				break;
			
			default:
				result[0]=-Math::sin(angle);
				result[1]=Math::cos(angle);
				break;
			}
		
		return result;
		}
	template <class MatrixParam>
	MatrixParam& writeMatrix(MatrixParam& destMatrix) const // Writes rotation into upper-left 2x2 submatrix
		{
		Scalar c=Math::cos(angle);
		Scalar s=Math::sin(angle);
		destMatrix(0,0)=typename MatrixParam::Scalar(c);
		destMatrix(1,0)=typename MatrixParam::Scalar(s);
		destMatrix(0,1)=typename MatrixParam::Scalar(-s);
		destMatrix(1,1)=typename MatrixParam::Scalar(c);
		return destMatrix;
		}
	
	/* Concatenation methods: */
	Rotation& operator*=(const Rotation& other) // Multiplication assignment (from the right)
		{
		setAngle(angle+other.angle);
		return *this;
		}
	Rotation& leftMultiply(const Rotation& other) // Ditto (from the left)
		{
		setAngle(other.angle+angle);
		return *this;
		}
	friend Rotation operator*<ScalarParam>(const Rotation& r1,const Rotation& r2); // Multiplies two rotations
	Rotation& doInvert(void) // Inverts the rotation
		{
		setAngle(-angle);
		return *this;
		}
	friend Rotation invert<>(const Rotation& r); // Inverts a rotation
	Rotation& operator/=(const Rotation& other) // Division assignment (from the right)
		{
		setAngle(angle-other.angle);
		return *this;
		}
	friend Rotation operator/<ScalarParam>(const Rotation& r1,const Rotation& r2); // Divides two rotations
	
	/* Transformation methods: */
	Vector transform(const Vector& v) const // Transforms a vector
		{
		Scalar c=Math::cos(angle);
		Scalar s=Math::sin(angle);
		return Vector(c*v[0]-s*v[1],s*v[0]+c*v[1]);
		}
	Point transform(const Point& p) const // Transforms a point
		{
		Scalar c=Math::cos(angle);
		Scalar s=Math::sin(angle);
		return Point(c*p[0]-s*p[1],s*p[0]+c*p[1]);
		}
	HVector transform(const HVector& v) const // Transforms a homogenuous vector
		{
		Scalar c=Math::cos(angle);
		Scalar s=Math::sin(angle);
		return HVector(c*v[0]-s*v[1],s*v[0]+c*v[1],v[2]);
		}
	Vector inverseTransform(const Vector& v) const // Transforms a vector with the inverse transformation
		{
		Scalar c=Math::cos(angle);
		Scalar s=-Math::sin(angle);
		return Vector(c*v[0]-s*v[1],s*v[0]+c*v[1]);
		}
	Point inverseTransform(const Point& p) const // Transforms a point with the inverse transformation
		{
		Scalar c=Math::cos(angle);
		Scalar s=-Math::sin(angle);
		return Point(c*p[0]-s*p[1],s*p[0]+c*p[1]);
		}
	HVector inverseTransform(const HVector& v) const // Transforms a homogenuous vector with the inverse transformation
		{
		Scalar c=Math::cos(angle);
		Scalar s=-Math::sin(angle);
		return HVector(c*v[0]-s*v[1],s*v[0]+c*v[1],v[2]);
		}
	};

/* Friend functions of Rotation<ScalarParam,2>: */
template <class ScalarParam>
bool operator==(const Rotation<ScalarParam,2>& r1,const Rotation<ScalarParam,2>& r2)
	{
	return r1.angle==r2.angle;
	}
template <class ScalarParam>
bool operator!=(const Rotation<ScalarParam,2>& r1,const Rotation<ScalarParam,2>& r2)
	{
	return r1.angle!=r2.angle;
	}
template <class ScalarParam>
Rotation<ScalarParam,2> operator*(const Rotation<ScalarParam,2>& r1,const Rotation<ScalarParam,2>& r2)
	{
	return Rotation<ScalarParam,2>(r1.angle+r2.angle);
	}
template <class ScalarParam>
Rotation<ScalarParam,2> invert(const Rotation<ScalarParam,2>& r)
	{
	return Rotation<ScalarParam,2>(-r.angle);
	}
template <class ScalarParam>
Rotation<ScalarParam,2> operator/(const Rotation<ScalarParam,2>& r1,const Rotation<ScalarParam,2>& r2)
	{
	return Rotation<ScalarParam,2>(r1.angle-r2.angle);
	}

/**********************************
Specialized class for 3D rotations:
**********************************/

/* Forward declarations for friend functions: */
template <class ScalarParam>
class Rotation<ScalarParam,3>;
template <class ScalarParam>
bool operator==(const Rotation<ScalarParam,3>& r1,const Rotation<ScalarParam,3>& r2);
template <class ScalarParam>
bool operator!=(const Rotation<ScalarParam,3>& r1,const Rotation<ScalarParam,3>& r2);
template <class ScalarParam>
Rotation<ScalarParam,3> operator*(const Rotation<ScalarParam,3>& r1,const Rotation<ScalarParam,3>& r2);
template <class ScalarParam>
Rotation<ScalarParam,3> invert(const Rotation<ScalarParam,3>& r);
template <class ScalarParam>
Rotation<ScalarParam,3> operator/(const Rotation<ScalarParam,3>& r1,const Rotation<ScalarParam,3>& r2);

template <class ScalarParam>
class Rotation<ScalarParam,3>
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar; // The underlying scalar type
	static const int dimension=3; // The rotation's dimension
	typedef Geometry::Vector<ScalarParam,3> Vector; // Compatible vector type
	typedef Geometry::Point<ScalarParam,3> Point; // Compatible point type
	typedef Geometry::HVector<ScalarParam,3> HVector; // Compatible homogenuous vector type
	
	/* Elements: */
	private:
	Scalar q[4]; // Internal representation as unit quaternion, q[0]*q[0]+q[1]*q[1]+q[2]*q[2]+q[3]*q[3] = 1
	
	/* Constructors and destructors: */
	public:
	Rotation(void) // Creates identity rotation
		{
		q[0]=q[1]=q[2]=Scalar(0);
		q[3]=Scalar(1);
		}
	Rotation(Scalar sX,Scalar sY,Scalar sZ,Scalar sW) // Elementwise initialization; does not normalize
		{
		q[0]=sX;
		q[1]=sY;
		q[2]=sZ;
		q[3]=sW;
		}
	template <class SourceScalarParam>
	Rotation(const SourceScalarParam sQ[4]) // Elementwise initialization with type conversion; does not normalize
		{
		for(int i=0;i<4;++i)
			q[i]=Scalar(sQ[i]);
		}
	Rotation(const Vector& sAxis,Scalar sAngle) // Creates rotation of given angle around given axis
		{
		sAngle=Math::div2(sAngle);
		Scalar axisLen;
		if(sAngle!=Scalar(0)&&(axisLen=Scalar(sAxis.mag()))>Scalar(0))
			{
			/* Create proper rotation: */
			Scalar factor=Math::sin(sAngle)/axisLen;
			for(int i=0;i<3;++i)
				q[i]=sAxis[i]*factor;
			q[3]=Math::cos(sAngle);
			}
		else
			{
			/* Create identity rotation if either angle or axis of rotation are zero: */
			for(int i=0;i<3;++i)
				q[i]=Scalar(0);
			q[3]=Scalar(1);
			}
		}
	Rotation(const Vector& sScaledAxis) // Creates rotation around given axis (length of axis is angle of rotation in radians)
		{
		Scalar axisLen=Scalar(sScaledAxis.mag());
		Scalar angle=Math::div2(axisLen);
		if(angle!=Scalar(0))
			{
			/* Create proper rotation: */
			Scalar factor=Math::sin(angle)/axisLen;
			for(int i=0;i<3;++i)
				q[i]=sScaledAxis[i]*factor;
			q[3]=Math::cos(angle);
			}
		else
			{
			/* Create identity rotation if either angle is zero: */
			for(int i=0;i<3;++i)
				q[i]=Scalar(0);
			q[3]=Scalar(1);
			}
		}
	template <class SourceScalarParam>
	Rotation(const Rotation<SourceScalarParam,3>& source) // Copy constructor with type conversion
		{
		const SourceScalarParam* sQ=source.getQuaternion();
		for(int i=0;i<4;++i)
			q[i]=Scalar(sQ[i]);
		}
	
	/* Pseudo-constructors: */	
	static const Rotation identity; // The identity rotation
	static Rotation rotateX(Scalar sAngle) // Returns rotation around x axis
		{
		sAngle=Math::div2(sAngle);
		return Rotation(Math::sin(sAngle),Scalar(0),Scalar(0),Math::cos(sAngle));
		}
	static Rotation rotateY(Scalar sAngle) // Returns rotation around y axis
		{
		sAngle=Math::div2(sAngle);
		return Rotation(Scalar(0),Math::sin(sAngle),Scalar(0),Math::cos(sAngle));
		}
	static Rotation rotateZ(Scalar sAngle) // Returns rotation around z axis
		{
		sAngle=Math::div2(sAngle);
		return Rotation(Scalar(0),Scalar(0),Math::sin(sAngle),Math::cos(sAngle));
		}
	static Rotation rotateAxis(const Vector& sAxis,Scalar sAngle) // Returns rotation around arbitrary axis
		{
		return Rotation(sAxis,sAngle);
		}
	static Rotation rotateScaledAxis(const Vector& sScaledAxis) // Returns rotation around arbitrary axis (angle encoded as axis' length)
		{
		return Rotation(sScaledAxis);
		}
	static Rotation rotateFromTo(const Vector& from,const Vector& to); // Returns rotation that rotates the from vector into the to vector
	static Rotation fromQuaternion(Scalar sX,Scalar sY,Scalar sZ,Scalar sW) // Creates rotation from quaternion
		{
		/* Normalize given quaternion, just to be sure: */
		Scalar l=Math::sqrt(sX*sX+sY*sY+sZ*sZ+sW*sW);
		return Rotation(sX/l,sY/l,sZ/l,sW/l);
		}
	template <class SourceScalarParam>
	static Rotation fromQuaternion(const SourceScalarParam sQ[4]) // Ditto, from array with type conversion
		{
		/* Normalize given quaternion, just to be sure: */
		Scalar l=Scalar(Math::sqrt(sQ[0]*sQ[0]+sQ[1]*sQ[1]+sQ[2]*sQ[2]+sQ[3]*sQ[3]));
		return Rotation(Scalar(sQ[0])/l,Scalar(sQ[1])/l,Scalar(sQ[2])/l,Scalar(sQ[3])/l);
		}
	static Rotation fromEulerAngles(Scalar angleX,Scalar angleY,Scalar angleZ) // Creates rotation from Euler angles
		{
		return rotateX(angleX)*rotateY(angleY)*rotateZ(angleZ);
		}
	template <class SourceScalarParam>
	static Rotation fromEulerAngles(const SourceScalarParam angles[3]) // Ditto, from array with type conversion
		{
		return rotateX(Scalar(angles[0]))*rotateY(Scalar(angles[1]))*rotateZ(Scalar(angles[2]));
		}
	static Rotation fromBaseVectors(const Vector& xAxis,const Vector& yAxis); // Creates rotation from orthonormal coordinate frame based on given vectors
	template <class MatrixParam>
	static Rotation fromMatrix(const MatrixParam& matrix); // Creates rotation from upper-left 3x3 submatrix; assumed to be orthonormal
	
	/* Comparison operators: */
	friend bool operator==<>(const Rotation& r1,const Rotation& r2);
	friend bool operator!=<>(const Rotation& r1,const Rotation& r2);
	
	/* Low-level manipulation methods: */
	const Scalar* getQuaternion(void) const // Returns quaternion as array
		{
		return q;
		}
	void renormalize(void) // Renormalizes the quaternion
		{
		Scalar l=Math::sqrt(q[0]*q[0]+q[1]*q[1]+q[2]*q[2]+q[3]*q[3]);
		for(int i=0;i<4;++i)
			q[i]/=l;
		}
	Vector getAxis(void) const // Returns rotation axis
		{
		Scalar factor=Math::sqrt(q[0]*q[0]+q[1]*q[1]+q[2]*q[2]);
		if(factor==Scalar(0))
			return Vector(Scalar(1),Scalar(0),Scalar(0)); // Return "default" axis for identity rotation
		else
			return Vector(q[0]/factor,q[1]/factor,q[2]/factor);
		}
	Scalar getAngle(void) const // Returns rotation angle in radians
		{
		Scalar ac=Math::acos(q[3]);
		if(Math::isNan(ac))
			return Scalar(0);
		else
			return ac*Scalar(2);
		}
	Vector getScaledAxis(void) const // Returns rotation axis and angle as scaled axis
		{
		/* Calculate rotation angle in radians: */
		Scalar ac=Math::acos(q[3]);
		if(Math::isNan(ac))
			return Vector::zero;
		Scalar angle=ac*Scalar(2);
		if(angle<-Math::rad(Scalar(180)))
			angle+=Math::rad(Scalar(360));
		else if(angle>Math::rad(Scalar(180)))
			angle-=Math::rad(Scalar(360));
		
		/* Return normalized rotation axis scaled by rotation angle: */
		Scalar factor=Math::sqrt(q[0]*q[0]+q[1]*q[1]+q[2]*q[2]);
		if(angle==Scalar(0)||factor==Scalar(0))
			return Vector::zero;
		
		return Vector((q[0]*angle)/factor,(q[1]*angle)/factor,(q[2]*angle)/factor);
		}
	
	/* Coordinate system methods: */
	Vector getDirection(int j) const; // Gets j-th direction vector of coordinate system
	template <class MatrixParam>
	MatrixParam& writeMatrix(MatrixParam& destMatrix) const; // Writes rotation into upper-left 3x3 submatrix
	
	/* Concatenation methods: */
	Rotation& operator*=(const Rotation& other); // Multiplication assignment (from the right)
	Rotation& leftMultiply(const Rotation& other); // Ditto (from the left)
	friend Rotation operator*<ScalarParam>(const Rotation& r1,const Rotation& r2); // Multiplies two rotations
	Rotation& doInvert(void) // Inverts the rotation
		{
		q[3]=-q[3];
		return *this;
		}
	friend Rotation invert<>(const Rotation& r); // Inverts a rotation
	Rotation& operator/=(const Rotation& other); // Division assignment (from the right)
	friend Rotation operator/<ScalarParam>(const Rotation& r1,const Rotation& r2); // Divides two rotations
	
	/* Transformation methods: */
	Vector transform(const Vector& v) const // Transforms a vector
		{
		Scalar wxvx=q[1]*v[2]-q[2]*v[1]+v[0]*q[3];
		Scalar wxvy=q[2]*v[0]-q[0]*v[2]+v[1]*q[3];
		Scalar wxvz=q[0]*v[1]-q[1]*v[0]+v[2]*q[3];
		Vector result;
		result[0]=v[0]+Scalar(2)*(q[1]*wxvz-q[2]*wxvy);
		result[1]=v[1]+Scalar(2)*(q[2]*wxvx-q[0]*wxvz);
		result[2]=v[2]+Scalar(2)*(q[0]*wxvy-q[1]*wxvx);
		return result;
		}
	Point transform(const Point& p) const // Transforms a point
		{
		Scalar wxvx=q[1]*p[2]-q[2]*p[1]+p[0]*q[3];
		Scalar wxvy=q[2]*p[0]-q[0]*p[2]+p[1]*q[3];
		Scalar wxvz=q[0]*p[1]-q[1]*p[0]+p[2]*q[3];
		Point result;
		result[0]=p[0]+Scalar(2)*(q[1]*wxvz-q[2]*wxvy);
		result[1]=p[1]+Scalar(2)*(q[2]*wxvx-q[0]*wxvz);
		result[2]=p[2]+Scalar(2)*(q[0]*wxvy-q[1]*wxvx);
		return result;
		}
	HVector transform(const HVector& v) const // Transforms a homogenuous vector
		{
		Scalar wxvx=q[1]*v[2]-q[2]*v[1]+v[0]*q[3];
		Scalar wxvy=q[2]*v[0]-q[0]*v[2]+v[1]*q[3];
		Scalar wxvz=q[0]*v[1]-q[1]*v[0]+v[2]*q[3];
		HVector result;
		result[0]=v[0]+Scalar(2)*(q[1]*wxvz-q[2]*wxvy);
		result[1]=v[1]+Scalar(2)*(q[2]*wxvx-q[0]*wxvz);
		result[2]=v[2]+Scalar(2)*(q[0]*wxvy-q[1]*wxvx);
		result[3]=v[3];
		return result;
		}
	Vector inverseTransform(const Vector& v) const // Transforms a vector with the inverse transformation
		{
		Scalar wxvx=q[1]*v[2]-q[2]*v[1]-v[0]*q[3];
		Scalar wxvy=q[2]*v[0]-q[0]*v[2]-v[1]*q[3];
		Scalar wxvz=q[0]*v[1]-q[1]*v[0]-v[2]*q[3];
		Vector result;
		result[0]=v[0]+Scalar(2)*(q[1]*wxvz-q[2]*wxvy);
		result[1]=v[1]+Scalar(2)*(q[2]*wxvx-q[0]*wxvz);
		result[2]=v[2]+Scalar(2)*(q[0]*wxvy-q[1]*wxvx);
		return result;
		}
	Point inverseTransform(const Point& p) const // Transforms a point with the inverse transformation
		{
		Scalar wxvx=q[1]*p[2]-q[2]*p[1]-p[0]*q[3];
		Scalar wxvy=q[2]*p[0]-q[0]*p[2]-p[1]*q[3];
		Scalar wxvz=q[0]*p[1]-q[1]*p[0]-p[2]*q[3];
		Point result;
		result[0]=p[0]+Scalar(2)*(q[1]*wxvz-q[2]*wxvy);
		result[1]=p[1]+Scalar(2)*(q[2]*wxvx-q[0]*wxvz);
		result[2]=p[2]+Scalar(2)*(q[0]*wxvy-q[1]*wxvx);
		return result;
		}
	HVector inverseTransform(const HVector& v) const // Transforms a homogenuous vector with the inverse transformation
		{
		Scalar wxvx=q[1]*v[2]-q[2]*v[1]-v[0]*q[3];
		Scalar wxvy=q[2]*v[0]-q[0]*v[2]-v[1]*q[3];
		Scalar wxvz=q[0]*v[1]-q[1]*v[0]-v[2]*q[3];
		HVector result;
		result[0]=v[0]+Scalar(2)*(q[1]*wxvz-q[2]*wxvy);
		result[1]=v[1]+Scalar(2)*(q[2]*wxvx-q[0]*wxvz);
		result[2]=v[2]+Scalar(2)*(q[0]*wxvy-q[1]*wxvx);
		result[3]=v[3];
		return result;
		}
	};

/* Friend functions of Rotation<ScalarParam,3>: */
template <class ScalarParam>
bool operator==(const Rotation<ScalarParam,3>& r1,const Rotation<ScalarParam,3>& r2)
	{
	return r1.q[0]==r2.q[0]&&r1.q[1]==r2.q[1]&&r1.q[2]==r2.q[2]&&r1.q[3]==r2.q[3];
	}
template <class ScalarParam>
bool operator!=(const Rotation<ScalarParam,3>& r1,const Rotation<ScalarParam,3>& r2)
	{
	return r1.q[0]!=r2.q[0]||r1.q[1]!=r2.q[1]||r1.q[2]!=r2.q[2]||r1.q[3]!=r2.q[3];
	}
template <class ScalarParam>
Rotation<ScalarParam,3> invert(const Rotation<ScalarParam,3>& r)
	{
	return Rotation<ScalarParam,3>(r.q[0],r.q[1],r.q[2],-r.q[3]);
	}

}

#if defined(GEOMETRY_NONSTANDARD_TEMPLATES) && !defined(GEOMETRY_ROTATION_IMPLEMENTATION)
#include <Geometry/Rotation.icpp>
#endif

#endif
