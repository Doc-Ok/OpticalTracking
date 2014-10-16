/***********************************************************************
GeodeticCoordinateTransform - Coordinate transformation class to be used
when navigation space is geocentric Cartesian space, and users are
interested in geodetic coordinates (latitude, longitude, elevation).
Copyright (c) 2008-2012 Oliver Kreylos

This file is part of the Virtual Reality User Interface Library (Vrui).

The Virtual Reality User Interface Library is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The Virtual Reality User Interface Library is distributed in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Virtual Reality User Interface Library; if not, write to the
Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#ifndef VRUI_GEODETICCOORDINATETRANSFORM_INCLUDED
#define VRUI_GEODETICCOORDINATETRANSFORM_INCLUDED

#include <Geometry/Geoid.h>
#include <Vrui/CoordinateTransform.h>

namespace Vrui {

class GeodeticCoordinateTransform:public CoordinateTransform
	{
	/* Embedded classes: */
	private:
	typedef Geometry::Geoid<Scalar> Geoid; // Type for reference ellipsoids
	
	/* Elements: */
	private:
	Geoid geoid; // The reference ellipsoid underlying this geodetic transformation
	bool colatitude; // Flag whether to return colatitude instead of latitude
	bool radians; // Flag whether to return angles in radians instead of degrees
	bool depth; // Flag whether to return depths below geoid instead of heights above geoid
	
	/* Constructors and destructors: */
	public:
	GeodeticCoordinateTransform(double scaleFactor); // Creates transformation for WGS84 geoid using given scale factor
	
	/* Methods from CoordinateTransform: */
	virtual const char* getComponentName(int componentIndex) const;
	virtual Point transform(const Point& navigationPoint) const;
	virtual Point inverseTransform(const Point& userPoint) const;
	
	/* New methods: */
	void setGeoid(double newRadius,double newFlatteningFactor); // Sets the geoid parameters
	void setColatitude(bool newColatitude); // Sets the colatitude flag
	void setRadians(bool newRadians); // Sets the radians flag
	void setDepth(bool newDepth); // Sets the depth flag
	};

}

#endif
