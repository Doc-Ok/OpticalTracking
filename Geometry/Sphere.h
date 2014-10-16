/***********************************************************************
Sphere - Class for n-dimensional spheres.
Copyright (c) 2002-2014 Oliver Kreylos

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

#ifndef GEOMETRY_SPHERE_INCLUDED
#define GEOMETRY_SPHERE_INCLUDED

#include <Math/Math.h>
#include <Geometry/Vector.h>
#include <Geometry/Point.h>
#include <Geometry/OrthonormalTransformation.h>
#include <Geometry/OrthogonalTransformation.h>
#include <Geometry/Ray.h>
#include <Geometry/SolidHitResult.h>

namespace Geometry {

template <class ScalarParam,int dimensionParam>
class Sphere
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar; // The underlying scalar type
	static const int dimension=dimensionParam; // The sphere's dimension
	typedef Geometry::Vector<ScalarParam,dimensionParam> Vector; // The type for vectors
	typedef Geometry::Point<ScalarParam,dimensionParam> Point; // The type for points
	typedef Geometry::Ray<ScalarParam,dimensionParam> Ray; // Compatible ray type
	typedef Geometry::SolidHitResult<ScalarParam> HitResult; // Hit result type
	
	/* Elements: */
	private:
	Point center; // Sphere's center
	Scalar radius,sqrRadius; // Sphere's radius and squared radius
	
	/* Constructors and destructors: */
	public:
	Sphere(const Point& sCenter,Scalar sRadius) // Elementwise
		:center(sCenter),radius(sRadius),sqrRadius(Math::sqr(radius))
		{
		}
	
	/* Methods: */
	const Point& getCenter(void) const // Returns sphere's center
		{
		return center;
		}
	Sphere& setCenter(const Point& newCenter) // Sets a new center
		{
		center=newCenter;
		return *this;
		}
	Scalar getRadius(void) const // Returns sphere's radius
		{
		return radius;
		}
	Sphere& setRadius(Scalar newRadius) // Sets a new radius
		{
		radius=newRadius;
		sqrRadius=Math::sqr(radius);
		return *this;
		}
	Sphere& transform(const OrthonormalTransformation<Scalar,3>& t) // Transforms the sphere
		{
		center=t.transform(center);
		return *this;
		}
	Sphere& transform(const OrthogonalTransformation<Scalar,3>& t) // Transforms the sphere
		{
		center=t.transform(center);
		radius*=t.getScaling();
		sqrRadius=Math::sqr(radius);
		return *this;
		}
	bool contains(const Point& p) const // Returns true if given point is inside sphere or on the boundary
		{
		return Geometry::sqrDist(p,center)<=sqrRadius;
		}
	HitResult intersectRay(const Ray& ray) const // Intersects sphere with ray
		{
		Scalar d2=Geometry::sqr(ray.getDirection());
		Vector oc=ray.getOrigin()-center;
		Scalar ph=(oc*ray.getDirection());
		Scalar det=Math::sqr(ph)-(Geometry::sqr(oc)-sqrRadius)*d2;
		if(det<Scalar(0))
			return HitResult();
		
		det=Math::sqrt(det);
		Scalar lambda=(-ph-det)/d2; // First intersection
		if(lambda>=Scalar(0))
			return HitResult(lambda,HitResult::ENTRY);
		
		lambda=(-ph+det)/d2; // Second intersection
		if(lambda>=Scalar(0))
			return HitResult(lambda,HitResult::EXIT);
		
		return HitResult();
		}
	};

}

#endif
