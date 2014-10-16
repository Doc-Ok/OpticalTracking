/***********************************************************************
Cone - Class for n-dimensional upright conical frustums.
Copyright (c) 2005 Oliver Kreylos

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

#ifndef GEOMETRY_CONE_INCLUDED
#define GEOMETRY_CONE_INCLUDED

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
class Cone
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
		enum Part // Enumerated type to report which part of the conical frustum was hit
			{
			INVALID_PART,MANTEL,BOTTOMCAP,TOPCAP
			}
		
		/* Elements: */
		private:
		Part part; // Part of conical frustum that was hit
		
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
		Part getPart(void) const // Returns part of conical frustum that was intersected
			{
			return part;
			}
		}
	
	/* Elements: */
	private:
	Point p1,p2; // The conical frustum's two endpoints (centers of bottom and top circles, respectively)
	Vector axis; // The (normalized) conical frustum axis, from p1 to p2
	Scalar height; // Height of conical frustum
	Scalar radius1,sqrRadius1; // The conical frustum's radius and squared radius at p1
	Scalar radius2,sqrRadius2; // The conical frustum's radius and squared radius at p2
	Scalar slant,sqrSlantPlus1; // The conical frustum's mantle slant and its square plus 1
	
	/* Constructors and destructors: */
	public:
	Cone(const Point& sP1,Scalar sRadius1,const Point& sP2,Scalar sRadius2) // Creates upright conical frustum between two points
		:p1(sP1),p2(sP2),axis(p2-p1),height(axis.mag()),
		 radius1(sRadius1),sqrRadius1(Math::sqr(radius1)),
		 radius2(sRadius2),sqrRadius2(Math::sqr(radius2)),
		 slant((radius2-radius1)/height),
		 sqrSlantPlus1(Scalar(1)+Math::sqr(slant))
		{
		axis/=height;
		}
	
	/* Methods: */
	const Point& getP1(void) const // Returns first point
		{
		return p1;
		}
	Cone& setP1(const Point& newP1) // Sets first point
		{
		p1=newP1;
		axis=p2-p1;
		height=axis.mag();
		axis/=height;
		slant=(radius2-radius1)/height;
		sqrSlantPlus1=Scalar(1)+Math::sqr(slant);
		return *this;
		}
	const Point& getP2(void) const // Returns second point
		{
		return p2;
		}
	Cone& setP2(const Point& newP2) // Sets second point
		{
		p2=newP2;
		axis=p2-p1;
		height=axis.mag();
		axis/=height;
		slant=(radius2-radius1)/height;
		sqrSlantPlus1=Scalar(1)+Math::sqr(slant);
		return *this;
		}
	Cone& setPoints(const Point& newP1,const Point& newP2) // Sets both points at once
		{
		p1=newP1;
		p2=newP2;
		axis=p2-p1;
		height=axis.mag();
		axis/=height;
		slant=(radius2-radius1)/height;
		sqrSlantPlus1=Scalar(1)+Math::sqr(slant);
		return *this;
		}
	const Vector& getAxis(void) const // Returns conical frustum's axis
		{
		return axis;
		}
	Scalar getHeight(void) const // Returns height
		{
		return height;
		}
	Scalar getRadius1(void) const // Returns radius at first point
		{
		return radius1;
		}
	Cone& setRadius1(Scalar newRadius1) // Sets radius at first point
		{
		radius1=newRadius1;
		sqrRadius1=Math::sqr(radius1);
		slant=(radius2-radius1)/height;
		sqrSlantPlus1=Scalar(1)+Math::sqr(slant);
		return *this;
		}
	Scalar getRadius2(void) const // Returns radius at second point
		{
		return radius2;
		}
	Cone& setRadius2(Scalar newRadius2) // Sets radius at second point
		{
		radius2=newRadius2;
		sqrRadius2=Math::sqr(radius2);
		slant=(radius2-radius1)/height;
		sqrSlantPlus1=Scalar(1)+Math::sqr(slant);
		return *this;
		}
	Cone& setRadii(Scalar newRadius1,Scalar newRadius2) // Sets both radii at once
		{
		radius1=newRadius1;
		sqrRadius1=Math::sqr(radius1);
		radius2=newRadius2;
		sqrRadius2=Math::sqr(radius2);
		slant=(radius2-radius1)/height;
		sqrSlantPlus1=Scalar(1)+Math::sqr(slant);
		return *this;
		}
	Cone& transform(const OrthonormalTransformation<Scalar,dimension>& t) // Transforms the conical frustum
		{
		p1=t.transform(p1);
		p2=t.transform(p2);
		axis=p2-p1;
		height=axis.mag();
		axis/=height;
		slant=(radius2-radius1)/height;
		sqrSlantPlus1=Scalar(1)+Math::sqr(slant);
		return *this;
		}
	Cone& transform(const OrthogonalTransformation<Scalar,dimension>& t) // Transforms the conical frustum
		{
		p1=t.transform(p1);
		p2=t.transform(p2);
		axis=p2-p1;
		height=axis.mag();
		axis/=height;
		radius1*=t.getScaling();
		sqrRadius1=Math::sqr(radius1);
		radius2*=t.getScaling();
		sqrRadius2=Math::sqr(radius2);
		slant=(radius2-radius1)/height;
		sqrSlantPlus1=Scalar(1)+Math::sqr(slant);
		return *this;
		}
	bool contains(const Point& p) const // Returns true if given point is inside conical frustum or on boundary
		{
		Vector pp1=p-p1;
		Scalar pp1a=pp1*axis;
		if(pp1a>=Scalar(0)&&pp1a<=height)
			{
			Scalar r=radius1+slant*ppa1;
			return sqr(pp1)-Math::sqr(pp1a)<=Math::sqr(r);
			}
		else
			return false;
		}
	HitResult intersectRay(const Ray& ray) const // Intersects conical frustum with ray
		{
		Scalar resultLambda=Math::Constants<Scalar>::max;
		typename HitResult::Direction resultDirection=HitResult::INVALID_DIRECTION;
		typename HitResult::Part resultPart=HitResult::INVALID_PART;
		bool checkCaps=true;
		Scalar lambda;
		
		Vector op1=ray.getOrigin()-p1;
		Scalar dira=ray.getDirection()*axis;
		Scalar op1a=op1*axis;
		
		/* Intersect ray with cone mantel: */
		Scalar a=sqr(ray.getDirection())-sqrSlantPlus1*Math::sqr(dira);
		if(a!=Scalar(0))
			{
			Scalar b=op1*ray.getDirection()-(sqrSlantPlus1*op1a+radius1*slant)*dira;
			Scalar c=sqr(op1)-Math::sqr(op1a)-Math::sqr(radius1+slant*op1a);
			Scalar det=Math::sqr(b)-c*a;
			if(det>=Scalar(0))
				{
				det=Math::sqrt(det);
				bool needLambda=true;
				if((lambda=a>Scalar(0)?(-b-det)/a:(-b+det)/a)>=Scalar(0))
					{
					/* Check if the intersection is valid: */
					Scalar beta=op1a+dira*lambda;
					if(beta>=Scalar(0)&&beta<=height)
						{
						needLambda=false;
						resultLambda=lambda;
						resultDirection=HitResult::ENTRY;
						resultPart=HitResult::MANTEL;
						}
					}
				if(needLambda&&(lambda=a>Scalar(0)?(-b+det)/a:(-b-det)/a)>=Scalar(0))
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
				checkCaps=false; // If the ray does not intersect the cone, it won't intersect the endcaps either
			}
		
		if(checkCaps&&dira!=Scalar(0))
			{
			/* Intersect ray with bottom cap (around p1): */
			lambda=-op1a/dira;
			if(lambda>=Scalar(0)&&lambda<resultLambda&&sqrDist(ray(lambda),p1)<=sqrRadius1)
				{
				resultLambda=lambda;
				resultDirection=dira>Scalar(0)?HitResult::ENTRY:HitResult::EXIT;
				resultPart=HitResult::BOTTOMCAP;
				}
			
			/* Intersect ray with top cap (around p2): */
			lambda=(height-op1a)/dira;
			if(lambda>=Scalar(0)&&lambda<resultLambda&&sqrDist(ray(lambda),p2)<=sqrRadius2)
				{
				resultDirection=dira<Scalar(0)?HitResult::ENTRY:HitResult::EXIT;
				resultPart=HitResult::TOPCAP;
				}
			}
		
		return HitResult(resultLambda,resultDirection,resultPart);
		}
	}

}

#endif
