/***********************************************************************
LambertConformalProjection - Class to represent Lambert conformal conic
projections as horizontal datums.
Copyright (c) 2013 Oliver Kreylos

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

#ifndef GEOMETRY_LAMBERTCONFORMALPROJECTION_INCLUDED
#define GEOMETRY_LAMBERTCONFORMALPROJECTION_INCLUDED

#include <Math/Math.h>
#include <Math/Constants.h>
#include <Geometry/Point.h>
#include <Geometry/Box.h>
#include <Geometry/Geoid.h>

namespace Geometry {

template <class ScalarParam>
class LambertConformalProjection:public Geoid<ScalarParam>
	{
	/* Embedded classes: */
	public:
	typedef typename Geoid<ScalarParam>::Scalar Scalar;
	typedef typename Geoid<ScalarParam>::Point Point;
	typedef typename Geoid<ScalarParam>::Derivative Derivative;
	typedef typename Geoid<ScalarParam>::Orientation Orientation;
	typedef typename Geoid<ScalarParam>::Frame Frame;
	typedef Geometry::Point<ScalarParam,2> PPoint; // Type for 2D projection points
	typedef Geometry::Box<ScalarParam,2> PBox; // Type for 2D boxes
	
	/* Elements: */
	protected:
	using Geoid<ScalarParam>::radius;
	using Geoid<ScalarParam>::e2;
	
	private:
	double lng0; // Central meridian in radians
	double lat0; // Central parallel in radians
	double standardLats[2]; // Lower and upper standard parallels in radians
	bool southern; // Flag whether the projection cone is centered on the south pole instead of the north pole
	double unitFactor; // Conversion factor from linear units to meters
	double offset[2]; // False easting and northing in linear units
	
	/* Derived projection and unprojection constants: */
	double e,n,f,rho0;
	
	/* Private methods: */
	void calcProjectionConstants(void); // Calculates Lambert conformal projection constants based on the current reference ellipsoid and parameters
	
	/* Constructors and destructors: */
	public:
	LambertConformalProjection(double sLng0,double sLat0,double sStandardLat0,double sStandardLat1); // Creates a default Albers projection with the given parameters on the WGS84 reference ellipsoid
	LambertConformalProjection(double sLng0,double sLat0,double sStandardLat0,double sStandardLat1,double sRadius,double sFlatteningFactor); // Creates a default Albers projection with the given parameters on the given reference ellipsoid
	
	/* Methods: */
	double getUnitFactor(void) const // Returns the conversion factor from Albers linear units to meters
		{
		return unitFactor;
		}
	void setUnitFactor(double newUnitFactor); // Sets the conversion factor from Albers linear units to meters
	double getFalseNorthing(void) const // Returns the projection's false northing in meters
		{
		return offset[1];
		}
	void setFalseNorthing(double newFalseNorthing); // Sets the projection's false northing (typically 0.0m for northern and 10.0e6m for southern hemisphere)
	double getFalseEasting(void) const // Returns the projection's false easting in meters
		{
		return offset[0];
		}
	void setFalseEasting(double newFalseEasting); // Sets the projection's false easting (typically 5.0e5m)
	
	/* Projection and unprojection methods: */
	PPoint geodeticToMap(const PPoint& geodetic) const // Converts a 2D point in geodetic (longitude, latitude) coordinates to map coordinates
		{
		/*******************************************************************
		These formulae are from the literature. Don't ask me to explain
		them.
		*******************************************************************/
		
		/* Calculate the projection coefficients: */
		double p=e*Math::sin(geodetic[1]);
		double t=Math::tan(Math::Constants<double>::pi*0.25-geodetic[1]*0.5)/Math::pow((1.0-p)/(1.0+p),0.5*e);
		double rho=radius*f*Math::pow(t,n);
		double theta=n*(geodetic[0]-lng0);
		
		/* Calculate the Lambert conformal map coordinates: */
		return PPoint(Scalar(rho*Math::sin(theta)/unitFactor+offset[0]),
		              Scalar((rho0-rho*Math::cos(theta))/unitFactor+offset[1]));
		}
	PBox geodeticToMap(const PBox& geodetic) const; // Conservatively converts a 2D bounding box in geodetic space to map space
	PPoint mapToGeodetic(const PPoint& map) const // Converts a 2D point in map coordinates to geodetic (longitude, latitude) coordinates
		{
		/*******************************************************************
		These formulae are from the literature. Don't ask me to explain
		them.
		*******************************************************************/
		
		/* Calculate the unprojection coefficients: */
		double x=(map[0]-offset[0])*unitFactor;
		double rho0y=rho0-(map[1]-offset[1])*unitFactor;
		double rho=Math::copysign(Math::sqrt(Math::sqr(x)+Math::sqr(rho0y)),n);
		double t=Math::pow(rho/(radius*f),1.0/n);
		double chi=Math::Constants<double>::pi*0.5-2.0*Math::atan(t);
		
		/* Calculate the geodetic coordinates: */
		return PPoint(Scalar(Math::atan(x/(rho0y))/n+lng0),
		              Scalar(chi+(e2*(1.0/2.0+e2*(5.0/24.0+e2*(1.0/12.0+e2*13.0/360.0))))*Math::sin(2.0*chi)
		                        +(e2*e2*(7.0/48.0+e2*(29.0/240.0+e2*811.0/11520.0)))*Math::sin(4.0*chi)
		                        +(e2*e2*e2*(7.0/120.0+e2*81.0/1120.0))*Math::sin(6.0*chi)
		                        +e2*e2*e2*e2*4279.0/161280.0*Math::sin(8.0*chi)));
		}
	PBox mapToGeodetic(const PBox& map) const; // Conservatively converts a 2D bounding box in map space to geodetic space
	
	/* Map coordinate versions of methods from Geoid: */
	Point mapToCartesian(const Point& map) const // Converts a 3D point in map coordinates with geodetic vertical datum to geoid-centered geoid-fixed Cartesian coordinates
		{
		/* Unproject the point's horizontal coordinate from map coordinates to geodetic: */
		PPoint geodetic=mapToGeodetic(PPoint(map[0],map[1]));
		
		/* Reattach geodetic (reference ellipsoid-relative) elevation to the geodetic point and transform it to Cartesian: */
		return this->geodeticToCartesian(Point(geodetic[0],geodetic[1],map[2]));
		}
	Orientation mapToCartesianOrientation(const Point& mapBase) const // Returns a geoid-tangential coordinate orientation at the given base point in geodetic coordinates
		{
		/* Unproject the point's horizontal coordinate from map coordinates to geodetic: */
		PPoint geodeticBase=mapToGeodetic(PPoint(mapBase[0],mapBase[1]));
		
		/* Reattach geodetic (reference ellipsoid-relative) elevation to the geodetic point and return the Cartesian orientation: */
		return this->geodeticToCartesianOrientation(Point(geodeticBase[0],geodeticBase[1],mapBase[2]));
		}
	Frame mapToCartesianFrame(const Point& mapBase) const // Returns a geoid-tangential coordinate frame at the given base point in map coordinates
		{
		/* Unproject the point's horizontal coordinate from map coordinates to geodetic: */
		PPoint geodeticBase=mapToGeodetic(PPoint(mapBase[0],mapBase[1]));
		
		/* Reattach geodetic (reference ellipsoid-relative) elevation to the geodetic point and return the Cartesian orientation: */
		return this->geodeticToCartesianFrame(Point(geodeticBase[0],geodeticBase[1],mapBase[2]));
		}
	Point cartesianToMap(const Point& cartesian) const // Converts a 3D point in geoid-centered geoid-fixed Cartesian coordinates to map coordinates with geodetic vertical datum
		{
		/* Transform the Cartesian point to geodetic coordinates: */
		Point geodetic=this->cartesianToGeodetic(cartesian);
		
		/* Transform the geodetic point's horizontal coordinates to map coordinates and re-attach the geodetic (reference-ellipsoid relative) elevation: */
		PPoint map=geodeticToMap(PPoint(geodetic[0],geodetic[1]));
		return Point(map[0],map[1],geodetic[2]);
		}
	};

}

#if defined(GEOMETRY_NONSTANDARD_TEMPLATES) && !defined(GEOMETRY_LAMBERTCONFORMALPROJECTION_IMPLEMENTATION)
#include <Geometry/LambertConformalProjection.icpp>
#endif

#endif
