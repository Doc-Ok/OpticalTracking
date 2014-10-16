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

#include <Vrui/GeodeticCoordinateTransform.h>

#include <Misc/ThrowStdErr.h>
#include <Math/Math.h>

namespace Vrui {

/********************************************
Methods of class GeodeticCoordinateTransform:
********************************************/

GeodeticCoordinateTransform::GeodeticCoordinateTransform(double scaleFactor)
	:geoid(Geoid::getDefaultRadius()*scaleFactor,Geoid::getDefaultFlatteningFactor()),
	 colatitude(false),radians(false),depth(false)
	{
	}

const char* GeodeticCoordinateTransform::getComponentName(int componentIndex) const
	{
	switch(componentIndex)
		{
		case 0:
			return colatitude?"Colatitude":"Latitude";
			break;
		
		case 1:
			return "Longitude";
			break;
		
		case 2:
			return depth?"Depth":"Height";
			break;
		
		default:
			Misc::throwStdErr("GeodeticCoordinateTransform::getComponentName: Invalid component index %d",componentIndex);
			return ""; // Never reached; just to make compiler happy
		}
	}

Point GeodeticCoordinateTransform::transform(const Point& navigationPoint) const
	{
	#if 0
	
	/* Calculate the transformation using a not-so-great formula: */
	Point spherical;
	double xy=Math::sqrt(Math::sqr(double(navigationPoint[0]))+Math::sqr(double(navigationPoint[1])));
	spherical[0]=Scalar(Math::atan2(double(navigationPoint[2]),(1.0-e2)*xy));
	double lats=Math::sin(spherical[0]);
	double nu=radius/Math::sqrt(1.0-e2*Math::sqr(lats));
	for(int i=0;i<6;++i)
		{
		spherical[0]=Scalar(atan2(double(navigationPoint[2])+e2*nu*lats,xy));
		lats=Math::sin(spherical[0]);
		nu=radius/Math::sqrt(1.0-e2*Math::sqr(lats));
		}
	spherical[1]=Scalar(Math::atan2(double(navigationPoint[1]),double(navigationPoint[0])));
	if(Math::abs(spherical[0])<=Math::rad(Scalar(45)))
		spherical[2]=Scalar(xy/Math::cos(double(spherical[0]))-nu;
	else
		spherical[2]=Scalar(double(navigationPoint[2])/lats-(1.0-e2)*nu);
	
	#else
	
	/* Let the geoid do the transformation: */
	Point spherical=geoid.cartesianToGeodetic(navigationPoint);
	
	#endif
	
	/* Apply the secondary coordinate transformation: */
	if(colatitude)
		spherical[0]=Math::rad(Scalar(90))-spherical[0];
	if(!radians)
		{
		spherical[0]=Math::deg(spherical[0]);
		spherical[1]=Math::deg(spherical[1]);
		}
	if(depth)
		spherical[2]=-spherical[2];
	
	return spherical;
	}

Point GeodeticCoordinateTransform::inverseTransform(const Point& userPoint) const
	{
	/* Apply the inverse secondary coordinate transformation: */
	Point spherical=userPoint;
	if(!radians)
		{
		spherical[0]=Math::rad(spherical[0]);
		spherical[1]=Math::rad(spherical[1]);
		}
	if(colatitude)
		spherical[0]=Math::rad(Scalar(90))-spherical[0];
	if(depth)
		spherical[2]=-spherical[2];
	
	/* Let the geoid do the inverse transformation: */
	return geoid.geodeticToCartesian(spherical);
	}

void GeodeticCoordinateTransform::setGeoid(double newRadius,double newFlatteningFactor)
	{
	/* Set the geoid parameters: */
	geoid=Geoid(newRadius,newFlatteningFactor);
	}

void GeodeticCoordinateTransform::setColatitude(bool newColatitude)
	{
	colatitude=newColatitude;
	}

void GeodeticCoordinateTransform::setRadians(bool newRadians)
	{
	radians=newRadians;
	}

void GeodeticCoordinateTransform::setDepth(bool newDepth)
	{
	depth=newDepth;
	}

}
