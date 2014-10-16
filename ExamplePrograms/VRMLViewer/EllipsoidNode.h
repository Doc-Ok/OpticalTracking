/***********************************************************************
EllipsoidNode - Class to represent different ellipsoid shapes. These are
not meant for rendering, but to convert spherical coordinates into
geocentric Cartesian coordinates, also known as GPS coordinates.
Copyright (c) 2008 Oliver Kreylos

This file is part of the Virtual Reality VRML viewer (VRMLViewer).

The Virtual Reality VRML viewer is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Virtual Reality VRML viewer is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Virtual Reality VRML viewer; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef ELLIPSOIDNODE_INCLUDED
#define ELLIPSOIDNODE_INCLUDED

#include <Math/Math.h>

#include "Types.h"

#include "VRMLNode.h"

class EllipsoidNode:public VRMLNode
	{
	/* Elements: */
	private:
	double radius; // Equatorial radius of ellipsoid in meters
	double flatteningFactor; // Flattening factor of the ellipsoid
	double scaleFactor; // Scale factor from meters to model coordinates
	double modelRadius; // Scaled ellipsoid radius
	double e2; // Ellipsoid's eccentricity
	
	/* Constructors and destructors: */
	public:
	EllipsoidNode(VRMLParser& parser); // Creates ellipsoid by parsing VRML file
	
	/* New methods: */
	Point sphericalToCartesian(const double spherical[3]) const // Converts lat, long, elevation spherical coordinates in radians, radians, meters, to Cartesian in model coordinates
		{
		double sLat=Math::sin(spherical[0]);
		double cLat=Math::cos(spherical[0]);
		double r=radius/Math::sqrt(1.0-e2*sLat*sLat);
		double xy=(r+spherical[2])*cLat;
		return Point(float(xy*Math::cos(spherical[1])*scaleFactor),float(xy*Math::sin(spherical[1])*scaleFactor),float(((1.0-e2)*r+spherical[2])*sLat*scaleFactor));
		}
	};

#endif
