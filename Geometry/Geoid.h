/***********************************************************************
Geoid - Class to represent geoids, actually reference ellipsoids, to
support coordinate system transformations between several spherical or
ellipsoidal coordinate systems commonly used in geodesy.
Copyright (c) 2009-2012 Oliver Kreylos

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

#ifndef GEOMETRY_GEOID_INCLUDED
#define GEOMETRY_GEOID_INCLUDED

#include <Math/Math.h>
#include <Math/Constants.h>
#include <Geometry/Point.h>
#include <Geometry/Matrix.h>
#include <Geometry/Rotation.h>
#include <Geometry/OrthonormalTransformation.h>

namespace Geometry {

template <class ScalarParam>
class Geoid
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar; // Scalar type for input/output geometry objects
	static const int dimension=3; // Geoids are always three-dimensional
	typedef Geometry::Point<Scalar,dimension> Point; // Type for points
	typedef Geometry::Matrix<Scalar,dimension,dimension> Derivative; // Type for transformation derivatives
	typedef Geometry::Rotation<Scalar,dimension> Orientation; // Type for coordinate orientations
	typedef Geometry::OrthonormalTransformation<Scalar,dimension> Frame; // Type for coordinate frames
	
	/* Elements: */
	protected:
	double radius; // Geoid's radius (semi-major axis) in whatever unit is convenient
	double flatteningFactor; // Geoid's flattening factor
	double b; // Geoid's semi-minor axis
	double e2; // Geoid's squared eccentricity, derived from flattening factor
	double ep2; // Geoid's squared second eccentricity
	
	/* Constructors and destructors: */
	public:
	static double getDefaultRadius(void); // Returns radius of default geoid (WGS84)
	static double getDefaultFlatteningFactor(void); // Returns flattening factor of default geoid (WGS84)
	Geoid(void); // Creates a default geoid (WGS84)
	Geoid(double sRadius,double sFlatteningFactor); // Creates a geoid with the given parameters
	
	/* Methods: */
	double getRadius(void) const // Returns the geoid's radius (semi-major axis)
		{
		return radius;
		}
	double getFlatteningFactor(void) const // Returns the geoid's flattening factor
		{
		return flatteningFactor;
		}
	
	/*********************************************************************
	Conversions between Geoid-centered Geoid-fixed Cartesian and geodetic
	(longitude/latitude/elevation) coordinates:
	Geodetic points are in (longitude, latitude, elevation) in radians and
	whatever unit was used for the Geoid's radius.
	Cartesian points are (x, y, z) where (0, 0, 0) is the Geoid's
	centroid, the north pole is at (0, 0, +b), and the 0 meridian is in
	the y=0 plane.
	Cartesian frames have their z axes normal to the ellipsoid pointing to
	the outside (away from the centroid), and their y axes pointing north
	along a meridian.
	*********************************************************************/
	
	Point geodeticToCartesian(const Point& geodetic) const // Transforms a point
		{
		double sLon=Math::sin(double(geodetic[0]));
		double cLon=Math::cos(double(geodetic[0]));
		double sLat=Math::sin(double(geodetic[1]));
		double cLat=Math::cos(double(geodetic[1]));
		double elev=double(geodetic[2]);
		double chi=Math::sqrt(1.0-e2*sLat*sLat);
		return Point(Scalar((radius/chi+elev)*cLat*cLon),Scalar((radius/chi+elev)*cLat*sLon),Scalar((radius*(1.0-e2)/chi+elev)*sLat));
		}
	Derivative geodeticToCartesianDerivative(const Point& geodeticBase) const; // Returns the derivative of the point transformation at the given base point in geodetic coordinates
	Orientation geodeticToCartesianOrientation(const Point& geodeticBase) const // Returns a geoid-tangential coordinate orientation at the given base point in geodetic coordinates
		{
		Orientation o=Orientation::rotateZ(Scalar(0.5*Math::Constants<double>::pi+double(geodeticBase[0])));
		o*=Orientation::rotateX(Scalar(0.5*Math::Constants<double>::pi-double(geodeticBase[1])));
		return o;
		}
	Frame geodeticToCartesianFrame(const Point& geodeticBase) const; // Returns a geoid-tangential coordinate frame at the given base point in geodetic coordinates
	Point cartesianToGeodetic(const Point& cartesian) const; // Transforms a point
	};

}

#if defined(GEOMETRY_NONSTANDARD_TEMPLATES) && !defined(GEOMETRY_GEOID_IMPLEMENTATION)
#include <Geometry/Geoid.icpp>
#endif

#endif
