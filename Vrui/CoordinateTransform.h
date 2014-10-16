/***********************************************************************
CoordinateTransform - Base class for application-defined, potentially
non-linear, coordinate transformations from "user interest space" to
Vrui's navigation space. Used by measurement tools to display
measurements in the coordinates and units expected by users of
particular applications.
Base class implements identity transformation.
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

#ifndef VRUI_COORDINATETRANSFORM_INCLUDED
#define VRUI_COORDINATETRANSFORM_INCLUDED

#include <Geometry/Point.h>
#include <Vrui/Geometry.h>

namespace Vrui {

class CoordinateTransform
	{
	/* Constructors and destructors: */
	public:
	virtual ~CoordinateTransform(void);
	
	/* Methods: */
	virtual int getNumComponents(void) const; // Returns the number of components in a user-space coordinate (usually 3)
	virtual const char* getComponentName(int componentIndex) const; // Returns the name of the given user-space coordinate component
	virtual const char* getUnitName(int componentIndex) const; // Returns the name of the unit used for the given user-space coordinate component
	virtual const char* getUnitAbbreviation(int componentIndex) const; // Returns the abbreviated name of the unit used for the given user-space coordinate component
	virtual Point transform(const Point& navigationPoint) const; // Transforms a point from navigation space to user space
	virtual Point inverseTransform(const Point& userPoint) const; // Transforms a point from user space to navigation space
	};

}

#endif
