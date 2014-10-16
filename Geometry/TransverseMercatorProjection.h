/***********************************************************************
TransverseMercatorProjection - Class to represent transverse Mercator
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

#ifndef GEOMETRY_TRANSVERSEMERCATORPROJECTION_INCLUDED
#define GEOMETRY_TRANSVERSEMERCATORPROJECTION_INCLUDED

#include <Math/Math.h>
#include <Geometry/Point.h>
#include <Geometry/Box.h>
#include <Geometry/Geoid.h>

namespace Geometry {

template <class ScalarParam>
class TransverseMercatorProjection:public Geoid<ScalarParam>
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
	using Geoid<ScalarParam>::flatteningFactor;
	using Geoid<ScalarParam>::e2;
	using Geoid<ScalarParam>::ep2;
	
	private:
	double lng0; // The projection's central meridian
	double lat0; // The projection's latitude of origin
	double k0; // Stretching factor at center parallel
	double offset[2]; // The projection's false easting and northing
	
	/* Derived projection and unprojection constants: */
	double Mc1,Mc2,Mc3,Mc4,M0; // Constants of the projection (geodetic -> UTM) formula
	double e1; // Derived inverse flattening factor
	double IMc0,IMc1,IMc2,IMc3,IMc4; // Constants of the unprojection (UTM -> geodetic) formula
	
	/* Private methods: */
	void calcProjectionConstants(void); // Calculates UTM projection constants based on the current reference ellipsoid and parameters
	
	/* Constructors and destructors: */
	public:
	TransverseMercatorProjection(double sLng0,double sLat0); // Creates a default transverse Mercator projection with the given central longitude and latitude on the WGS84 reference ellipsoid
	TransverseMercatorProjection(double sLng0,double sLat0,double sRadius,double sFlatteningFactor); // Creates a default transverse Mercator projection with the given central longitude and latitude on the given reference ellipsoid
	
	/* Methods: */
	double getStretching(void) const // Returns the stretching factor at the central parallel
		{
		return k0;
		}
	void setStretching(double newStretching); // Sets the stretching factor at the central parallel
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
		double sphi=Math::sin(double(geodetic[1]));
		double sphi2=Math::sqr(sphi);
		double cphi=Math::cos(double(geodetic[1]));
		double cphi2=Math::sqr(cphi);
		double N=radius/Math::sqrt((1.0-e2*sphi2));
		double T=sphi2/cphi2;
		double C=ep2*cphi2;
		double A=(double(geodetic[0])-lng0)*cphi;
		double M=(Mc1*double(geodetic[1])-Mc2*Math::sin(2.0*double(geodetic[1]))+Mc3*Math::sin(4.0*double(geodetic[1]))-Mc4*Math::sin(6.0*double(geodetic[1])))*radius;
		
		/* Calculate the transverse Mercator coordinates: */
		double A2=Math::sqr(A);
		return PPoint(Scalar(((1.0+((1.0-T+C)+(5.0-18.0*T+T*T+72.0*C-58.0*ep2)*A2/20.0)*A2/6.0)*A)*k0*N+offset[0]),
		              Scalar((M-M0+((1.0+((5.0-T+9.0*C+4.0*C*C)+(61.0-58.0*T+T*T+600.0*C-330.0*ep2)*A2/30.0)*A2/12.0)*A2/2.0)*N*sphi/cphi)*k0+offset[1]));
		}
	PBox geodeticToMap(const PBox& geodetic) const; // Conservatively converts a 2D bounding box in geodetic space to map space
	PPoint mapToGeodetic(const PPoint& map) const // Converts a 2D point in map coordinates to geodetic (longitude, latitude) coordinates
		{
		/*******************************************************************
		These formulae are from the literature. Don't ask me to explain
		them.
		*******************************************************************/
		
		/* Calculate the reverse projection coefficients: */
		double M=M0+(double(map[1])-offset[1])/k0;
		double mu=M/IMc0;
		double phi=mu+IMc1*Math::sin(2.0*mu)+IMc2*Math::sin(4.0*mu)+IMc3*Math::sin(6.0*mu)+IMc4*Math::sin(8.0*mu);
		double sphi=Math::sin(phi);
		double sphi2=Math::sqr(sphi);
		double cphi=Math::cos(phi);
		double cphi2=Math::sqr(cphi);
		double kappa=1.0-e2*sphi2;
		double N=radius/Math::sqrt(kappa);
		double NbyR=kappa/(1.0-e2);
		double T=sphi2/cphi2;
		double C=ep2*cphi2;
		double D=(double(map[0])-offset[0])/(N*k0);
		
		/* Calculate the geodetic coordinates: */
		double D2=Math::sqr(D);
		return PPoint(Scalar(lng0+((((5.0+(-3.0*C-2.0)*C+(24.0*T+28.0)*T+8.0*ep2)/120.0*D2-(1.0+C+2.0*T)/6.0)*D2+1.0)*D)/cphi),
		              Scalar(phi-NbyR*sphi/cphi*(((61.0+(-3.0*C+298.0)*C+(45.0*T+90.0)*T-252.0*ep2)/720.0*D2-(5.0+(-4.0*C+10.0)*C+3.0*T-9.0*ep2)/24.0)*D2+1.0/2.0)*D2));
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

#if defined(GEOMETRY_NONSTANDARD_TEMPLATES) && !defined(GEOMETRY_TRANSVERSEMERCATORPROJECTION_IMPLEMENTATION)
#include <Geometry/TransverseMercatorProjection.icpp>
#endif

#endif
