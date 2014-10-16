/***********************************************************************
Plane - Class for n-dimensional planes.
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

#ifndef GEOMETRY_PLANE_INCLUDED
#define GEOMETRY_PLANE_INCLUDED

#include <Math/Math.h>
#include <Geometry/ComponentArray.h>
#include <Geometry/Vector.h>
#include <Geometry/Point.h>
#include <Geometry/Matrix.h>
#include <Geometry/Ray.h>
#include <Geometry/SolidHitResult.h>

namespace Geometry {

template <class ScalarParam,int dimensionParam>
class Plane
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar; // The underlying scalar type
	static const int dimension=dimensionParam; // The plane's dimension
	typedef Geometry::Vector<ScalarParam,dimensionParam> Vector; // The type for vectors
	typedef Geometry::Point<ScalarParam,dimensionParam> Point; // The type for points
	typedef Geometry::Ray<ScalarParam,dimensionParam> Ray; // Compatible ray type
	typedef SolidHitResult<ScalarParam> HitResult; // Hit result type
	
	/* Elements: */
	private:
	Vector normal; // Normal vector of plane (not normalized)
	Scalar offset; // Offset of plane from origin
	
	/* Constructors and destructors: */
	public:
	Plane(void) // Dummy constructor
		{
		}
	Plane(const Vector& sNormal,Scalar sOffset) // Elementwise constructor
		:normal(sNormal),offset(sOffset)
		{
		}
	Plane(const Vector& sNormal,const Point& p) // Constructs plane from normal vector and point
		:normal(sNormal),offset(normal*p)
		{
		}
	Plane(const Plane& source) // Copy constructor
		:normal(source.normal),offset(source.offset)
		{
		}
	template <class SourceScalarParam>
	Plane(const Plane<SourceScalarParam,dimensionParam>& source) // Ditto, with scalar type conversion
		:normal(source.getNormal()),offset(source.getOffset())
		{
		}
	
	/* Methods: */
	const Vector& getNormal(void) const // Returns plane's normal vector
		{
		return normal;
		}
	Plane& setNormal(const Vector& newNormal) // Sets a new normal vector
		{
		normal=newNormal;
		return *this;
		}
	Scalar getOffset(void) const // Returns plane's offset from origin
		{
		return offset;
		}
	Plane& setOffset(Scalar newOffset) // Sets a new offset from origin
		{
		offset=newOffset;
		return *this;
		}
	Plane& setPoint(const Point& p) // Sets the plane to contain given point
		{
		offset=normal*p;
		return *this;
		}
	Plane& normalize(void) // Normalizes the plane's normal vector
		{
		Scalar normalLen=normal.mag();
		normal/=normalLen;
		offset/=normalLen;
		return *this;
		}
	template <class TransformationParam>
	Plane& transform(const TransformationParam& t) // Transforms the plane by the given transformation
		{
		/* Convert plane equation to homogenuous format: */
		ComponentArray<Scalar,dimension+1> hn;
		for(int i=0;i<dimension;++i)
			hn[i]=normal[i];
		hn[dimension]=-offset;
		
		/* Convert transformation to projective transformation: */
		Matrix<Scalar,dimension+1,dimension+1> m=Matrix<Scalar,dimension+1,dimension+1>::one;
		t.writeMatrix(m);
		
		/* Transform the homogenuous plane equation by the normal transformation: */
		m=transpose(m);
		hn=hn/m;
		
		/* Convert the homogenuous plane equation back to (normal, offset) format: */
		for(int i=0;i<dimension;++i)
			normal[i]=hn[i];
		offset=-hn[dimension];
		
		return *this;
		}
	Scalar calcDistance(const Point& p) const // Returns non-normalized signed distance between plane and given point
		{
		return normal*p-offset;
		}
	bool contains(const Point& p) const // Returns true if given point is on the plane or in the halfspace behind the plane
		{
		return normal*p<=offset;
		}
	Vector project(const Vector& v) const // Projects a vector orthogonally into the plane
		{
		return v-normal*((v*normal)/normal.sqr());
		}
	Point project(const Point& p) const // Projects a point orthogonally into the plane
		{
		return p-normal*((p*normal-offset)/normal.sqr());
		}
	HitResult intersectRay(const Ray& ray) const // Intersects plane with ray
		{
		Scalar divisor=normal*ray.getDirection();
		if(divisor==Scalar(0))
			return HitResult();
		
		Scalar lambda=(offset-normal*ray.getOrigin())/divisor;
		if(lambda<Scalar(0))
			return HitResult();
		
		return HitResult(lambda,divisor<Scalar(0)?HitResult::ENTRY:HitResult::EXIT);
		}
	};

/*********************************************
Operations on objects of class Plane:
*********************************************/

template <class ScalarParam,int dimensionParam>
inline bool operator==(const Plane<ScalarParam,dimensionParam>& p1,const Plane<ScalarParam,dimensionParam>& p2) // Equality operator
	{
	return p1.getOffset()==p2.getOffset()&&p1.getNormal()==p2.getNormal();
	}

template <class ScalarParam,int dimensionParam>
inline bool operator!=(const Plane<ScalarParam,dimensionParam>& p1,const Plane<ScalarParam,dimensionParam>& p2) // Inequality operator
	{
	return p1.getOffset()!=p2.getOffset()||p1.getNormal()!=p2.getNormal();
	}

}

#endif
