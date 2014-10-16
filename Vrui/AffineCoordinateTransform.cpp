/***********************************************************************
AffineCoordinateTransform - Coordinate transformation class for
arbitrary affine transformations.
Copyright (c) 2012 Oliver Kreylos

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

#include <Vrui/AffineCoordinateTransform.h>

namespace Vrui {

/******************************************
Methods of class AffineCoordinateTransform:
******************************************/

AffineCoordinateTransform::AffineCoordinateTransform(const ATransform& sATransform)
	:aTransform(sATransform),inverseATransform(Geometry::invert(aTransform))
	{
	}

Point AffineCoordinateTransform::transform(const Point& navigationPoint) const
	{
	return inverseATransform.transform(navigationPoint);
	}

Point AffineCoordinateTransform::inverseTransform(const Point& userPoint) const
	{
	return aTransform.transform(userPoint);
	}

void AffineCoordinateTransform::setTransform(const ATransform& newATransform)
	{
	aTransform=newATransform;
	inverseATransform=Geometry::invert(aTransform);
	}

}
