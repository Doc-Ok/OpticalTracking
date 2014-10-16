/***********************************************************************
GeoCoordinateSystem - Abstract base class for projected, geographic, or
geocentric coordinate systems used in geodesy.
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

#ifndef GEOMETRY_GEOCOORDINATESYSTEM_INCLUDED
#define GEOMETRY_GEOCOORDINATESYSTEM_INCLUDED

#include <Misc/RefCounted.h>
#include <Misc/Autopointer.h>
#include <IO/Directory.h>
#include <Geometry/Point.h>
#include <Geometry/Box.h>

namespace Geometry {

class GeoCoordinateSystem:public Misc::RefCounted
	{
	/* Embedded classes: */
	public:
	typedef double Scalar; // Scalar type
	typedef Geometry::Point<Scalar,3> Point; // Type for points
	
	/* Methods: */
	virtual Point toCartesian(const Point& system) const =0; // Transforms a point from this object's coordinate system to geocentric Cartesian coordinates
	virtual Point fromCartesian(const Point& cartesian) const =0; // Transforms a point from geocentric Cartesian coordinates to this object's coordinate system
	};

typedef Misc::Autopointer<GeoCoordinateSystem> GeoCoordinateSystemPtr; // Type for autopointers to geodetic coordinate systems

class GeoReprojector:public Misc::RefCounted // Abstract base class to reproject points from a source to a destination coordinate system
	{
	/* Embedded classes: */
	public:
	typedef double Scalar; // Scalar type
	typedef Geometry::Point<Scalar,3> Point; // Type for points
	typedef Geometry::Box<Scalar,3> Box; // Type for axis-aligned boxes
	
	/* Methods: */
	virtual Point convert(const Point& source) const =0; // Transforms a point from the source to the destination coordinate system
	virtual Box convert(const Box& source) const =0; // Conservatively transforms an axis-aligned box from the source to the destination coordinate system
	};

typedef Misc::Autopointer<GeoReprojector> GeoReprojectorPtr; // Type for autopointers to coordinate system reprojectors

/**************************
Namespace-global functions:
**************************/

GeoCoordinateSystemPtr parseProjectionFile(IO::DirectoryPtr directory,const char* projectionFileName); // Parses a projection file of the give name inside the given directory and returns a geodetic coordinate system object
GeoReprojectorPtr createReprojector(GeoCoordinateSystemPtr source,GeoCoordinateSystemPtr dest); // Creates a reprojector object between the two given coordinate systems

}

#endif
