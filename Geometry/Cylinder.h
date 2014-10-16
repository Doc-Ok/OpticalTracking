/***********************************************************************
Cylinder - Class for n-dimensional upright cylinders.
Copyright (c) 2002-2005 Oliver Kreylos

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

#ifndef GEOMETRY_CYLINDER_INCLUDED
#define GEOMETRY_CYLINDER_INCLUDED

#include <Math/Math.h>
#include <Math/Constants.h>
#include <Geometry/Vector.h>
#include <Geometry/Point.h>
#include <Geometry/OrthonormalTransformation.h>
#include <Geometry/OrthogonalTransformation.h>
#include <Geometry/Ray.h>
#include <Geometry/SolidHitResult.h>

namespace Geometry {

template <class ScalarParam,int dimensionParam>
class Cylinder
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar; // The underlying scalar type
	static const int dimension=dimensionParam; // The sphere's dimension
	typedef Geometry::Vector<ScalarParam,dimensionParam> Vector; // The type for vectors
	typedef Geometry::Point<ScalarParam,dimensionParam> Point; // The type for points
	typedef Geometry::Ray<ScalarParam,dimensionParam> Ray; // Compatible ray type
	
	class HitResult:public SolidHitResult<ScalarParam> // Type for hit results
		{
		/* Embedded classes: */
		public:
		enum Part // Enumerated type to report which part of the cylinder was hit
			{
			INVALID_PART,MANTEL,BOTTOMCAP,TOPCAP
			};
		
		/* Elements: */
		private:
		Part part; // Part of cylinder that was hit
		
		/* Constructors and destructors: */
		public:
		HitResult(void) // Constructs invalid hit result
			:part(INVALID_PART)
			{
			}
		HitResult(ScalarParam sLambda,typename SolidHitResult<ScalarParam>::Direction sDirection,Part sPart)
			:SolidHitResult<ScalarParam>(sLambda,sDirection),part(sPart)
			{
			}
		
		/* Methods: */
		Part getPart(void) const // Returns part of cylinder that was intersected
			{
			return part;
			}
		};
	
	/* Elements: */
	private:
	Point p1,p2; // The cylinder's two endpoints
	Vector axis; // The (normalized) cylinder axis, from p1 to p2
	Scalar height; // Height of cylinder
	Scalar radius,sqrRadius; // The cylinder's radius and squared radius
	
	/* Constructors and destructors: */
	public:
	Cylinder(const Point& sP1,const Point& sP2,Scalar sRadius) // Creates upright cylinder between two points
		:p1(sP1),p2(sP2),axis(p2-p1),height(axis.mag()),radius(sRadius),sqrRadius(Math::sqr(radius))
		{
		axis/=height;
		}
	
	/* Methods: */
	const Point& getP1(void) const // Returns first point
		{
		return p1;
		}
	Cylinder& setP1(const Point& newP1) // Sets first point
		{
		p1=newP1;
		axis=p2-p1;
		height=axis.mag();
		axis/=height;
		return *this;
		}
	const Point& getP2(void) const // Returns second point
		{
		return p2;
		}
	Cylinder& setP2(const Point& newP2) // Sets second point
		{
		p2=newP2;
		axis=p2-p1;
		height=axis.mag();
		axis/=height;
		return *this;
		}
	Cylinder& setPoints(const Point& newP1,const Point& newP2) // Sets both points at once
		{
		p1=newP1;
		p2=newP2;
		axis=p2-p1;
		height=axis.mag();
		axis/=height;
		return *this;
		}
	const Vector& getAxis(void) const // Returns cylinder's axis
		{
		return axis;
		}
	Scalar getHeight(void) const // Returns height
		{
		return height;
		}
	Scalar getRadius(void) const // Returns radius
		{
		return radius;
		}
	Cylinder& setRadius(Scalar newRadius) // Sets radius
		{
		radius=newRadius;
		sqrRadius=Math::sqr(radius);
		return *this;
		}
	Cylinder& transform(const OrthonormalTransformation<Scalar,dimension>& t) // Transforms the cylinder
		{
		p1=t.transform(p1);
		p2=t.transform(p2);
		axis=p2-p1;
		height=axis.mag();
		axis/=height;
		return *this;
		}
	Cylinder& transform(const OrthogonalTransformation<Scalar,dimension>& t) // Transforms the cylinder
		{
		p1=t.transform(p1);
		p2=t.transform(p2);
		axis=p2-p1;
		height=axis.mag();
		axis/=height;
		radius*=t.getScaling();
		sqrRadius=Math::sqr(radius);
		return *this;
		}
	bool contains(const Point& p) const // Returns true if given point is inside cylinder or on boundary
		{
		Vector pp1=p-p1;
		Scalar pp1a=pp1*axis;
		if(pp1a>=Scalar(0)&&pp1a<=height)
			return sqr(pp1)-Math::sqr(pp1a)<=sqrRadius;
		else
			return false;
		}
	HitResult intersectRay(const Ray& ray) const // Intersects cylinder with ray
		{
		Scalar resultLambda=Math::Constants<Scalar>::max;
		typename HitResult::Direction resultDirection=HitResult::INVALID_DIRECTION;
		typename HitResult::Part resultPart=HitResult::INVALID_PART;
		bool checkCaps=true;
		Scalar lambda;
		
		Vector op1=ray.getOrigin()-p1;
		Scalar dira=ray.getDirection()*axis;
		Scalar op1a=op1*axis;
		
		/* Intersect ray with cylinder mantel: */
		Scalar a=sqr(ray.getDirection())-Math::sqr(dira);
		if(a!=Scalar(0))
			{
			Scalar b=op1*ray.getDirection()-op1a*dira;
			Scalar c=sqr(op1)-Math::sqr(op1a)-sqrRadius;
			Scalar det=Math::sqr(b)-c*a;
			if(det>=Scalar(0))
				{
				det=Math::sqrt(det);
				if((lambda=(-b-det)/a)>=Scalar(0))
					{
					/* Check if the intersection is valid: */
					Scalar beta=op1a+dira*lambda;
					if(beta>=Scalar(0)&&beta<=height)
						{
						resultLambda=lambda;
						resultDirection=HitResult::ENTRY;
						resultPart=HitResult::MANTEL;
						}
					}
				else if((lambda=(-b+det)/a)>=Scalar(0))
					{
					/* Check if the intersection is valid: */
					Scalar beta=op1a+dira*lambda;
					if(beta>=Scalar(0)&&beta<=height)
						{
						resultLambda=lambda;
						resultDirection=HitResult::EXIT;
						resultPart=HitResult::MANTEL;
						}
					}
				}
			else
				checkCaps=false; // If the ray does not intersect the cylinder, it won't intersect the endcaps either
			}
		
		if(checkCaps&&dira!=Scalar(0))
			{
			/* Intersect ray with bottom cap (around p1): */
			lambda=-op1a/dira;
			if(lambda>=Scalar(0)&&lambda<resultLambda&&sqrDist(ray(lambda),p1)<=sqrRadius)
				{
				resultLambda=lambda;
				resultDirection=dira>Scalar(0)?HitResult::ENTRY:HitResult::EXIT;
				resultPart=HitResult::BOTTOMCAP;
				}
			
			/* Intersect ray with top cap (around p2): */
			lambda=(height-op1a)/dira;
			if(lambda>=Scalar(0)&&lambda<resultLambda&&sqrDist(ray(lambda),p2)<=sqrRadius)
				{
				resultDirection=dira<Scalar(0)?HitResult::ENTRY:HitResult::EXIT;
				resultPart=HitResult::TOPCAP;
				}
			}
		
		return HitResult(resultLambda,resultDirection,resultPart);
		}
	};

}

#endif
