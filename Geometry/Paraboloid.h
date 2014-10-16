/***********************************************************************
Paraboloid - Class for n-dimensional circular paraboloids.
Copyright (c) 2014 Oliver Kreylos

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

#ifndef GEOMETRY_PARABOLOID_INCLUDED
#define GEOMETRY_PARABOLOID_INCLUDED

#include <Math/Math.h>
#include <Geometry/Vector.h>
#include <Geometry/Point.h>
#include <Geometry/OrthonormalTransformation.h>
#include <Geometry/OrthogonalTransformation.h>
#include <Geometry/Ray.h>
#include <Geometry/SolidHitResult.h>

namespace Geometry {

template <class ScalarParam,int dimensionParam>
class Paraboloid
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar; // The underlying scalar type
	static const int dimension=dimensionParam; // The paraboloid's dimension
	typedef Geometry::Vector<ScalarParam,dimensionParam> Vector; // The type for vectors
	typedef Geometry::Point<ScalarParam,dimensionParam> Point; // The type for points
	typedef Geometry::Ray<ScalarParam,dimensionParam> Ray; // Compatible ray type
	typedef Geometry::SolidHitResult<ScalarParam> HitResult; // Hit result type
	
	/* Elements: */
	private:
	Point apex; // Paraboloid's apex
	Vector axis; // Paraboloid's normalized axis direction
	Scalar factor; // Paraboloid's shape factor
	
	/* Constructors and destructors: */
	public:
	Paraboloid(const Point& sApex,const Vector& sAxis,Scalar sFactor) // Elementwise
		:apex(sApex),axis(sAxis),factor(sFactor)
		{
		/* Normalize the axis direction: */
		axis.normalize();
		}
	
	/* Methods: */
	const Point& getApex(void) const // Returns paraboloid's apex
		{
		return apex;
		}
	Paraboloid& setApex(const Point& newApex) // Sets a new apex
		{
		apex=newApex;
		return *this;
		}
	const Vector& getAxis(void) const // Returns paraboloid's axis
		{
		return axis;
		}
	Paraboloid& setAxis(const Vector& newAxis) // Sets a new axis
		{
		axis=newAxis;
		axis.normalize();
		return *this;
		}
	Scalar getFactor(void) const // Returns paraboloid's shape factor
		{
		return factor;
		}
	Paraboloid& setFactor(Scalar newFactor) // Sets a new shape factor
		{
		factor=newFactor;
		return *this;
		}
	Paraboloid& transform(const OrthonormalTransformation<Scalar,3>& t) // Transforms the paraboloid
		{
		apex=t.transform(apex);
		axis=t.transform(axis);
		return *this;
		}
	Paraboloid& transform(const OrthogonalTransformation<Scalar,3>& t) // Transforms the paraboloid
		{
		apex=t.transform(apex);
		axis=t.transform(axis);
		factor/=t.getScaling();
		return *this;
		}
	bool contains(const Point& p) const // Returns true if given point is inside paraboloid or on the boundary
		{
		Scalar a=(p-apex)*axis;
		Scalar b2=Geometry::sqrDist(p,apex)-a*a;
		return a>=factor*b2;
		}
	HitResult intersectRay(const Ray& ray) const // Intersects paraboloid with ray
		{
		/* Transform ray to paraboloid's local coordinate system: */
		Vector op=ray.getOrigin()-apex;
		Scalar op2=Geometry::sqr(op);
		Scalar opa=op*axis;
		Scalar opd=op*ray.getDirection();
		Scalar d2=Geometry::sqr(ray.getDirection());
		Scalar da=ray.getDirection()*axis;
		
		Scalar ph=factor*(opa*da-opd)+Math::div2(da);
		Scalar q=factor*(opa*opa-op2)+opa;
		Scalar denom=factor*(da*da-d2);
		if(denom!=Scalar(0))
			{
			/* Solve quadratic equation: */
			Scalar det=Math::sqr(ph)-q*denom;
			if(det<Scalar(0))
				return HitResult();
			
			det=Math::sqrt(det);
			Scalar lambda=(-ph-Math::copysign(det,denom))/denom; // First intersection
			if(lambda>=Scalar(0))
				return HitResult(lambda,HitResult::ENTRY);
			
			lambda=(-ph+Math::copysign(det,denom))/denom; // Second intersection
			if(lambda>=Scalar(0))
				return HitResult(lambda,HitResult::EXIT);
			
			return HitResult();
			}
		else
			{
			/* Ray is parallel to paraboloid's axis or paraboloid is flat; solve linear equation: */
			if(ph!=Scalar(0))
				{
				Scalar lambda=-Math::div2(q/ph);
				if(lambda>=Scalar(0))
					return HitResult(lambda,da>=Scalar(0)?HitResult::ENTRY:HitResult::EXIT);
				}
			
			return HitResult();
			}
		}
	Vector calcNormal(const Point& p) const // Returns normal vector at the given point, assuming that it lies on the paraboloid
		{
		Vector pa=p-apex;
		Scalar y=pa*axis;
		return Geometry::normalize(pa*(Scalar(2)*factor)-axis*(y*Scalar(2)*factor+Scalar(1)));
		}
	};

}

#endif
