/***********************************************************************
ClipPlane - Class to describe clipping planes in virtual environments.
Since clipping planes in OpenGL are limited resources, Vrui contains an
abstraction layer that allows users to create clipping planes as needed,
and then maps enabled clipping planes to OpenGL clipping planes as
available.
Copyright (c) 2009 Oliver Kreylos

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

#ifndef VRUI_CLIPPLANE_INCLUDED
#define VRUI_CLIPPLANE_INCLUDED

#include <Geometry/Plane.h>
#include <Vrui/Geometry.h>

namespace Vrui {

class ClipPlane
	{
	/* Elements: */
	private:
	bool enabled; // Flag if the light source is enabled
	Plane plane; // Plane equation of the clipping plane
	
	/* Constructors and destructors: */
	public:
	ClipPlane(void) // Creates a disabled clipping plane
		:enabled(false)
		{
		}
	ClipPlane(const Plane& sPlane) // Creates an enabled clipping plane with the given plane equation
		:enabled(true),plane(sPlane)
		{
		}
	
	/* Methods: */
	bool isEnabled(void) const // Returns the enabled flag
		{
		return enabled;
		}
	void enable(void) // Enables the clipping plane
		{
		enabled=true;
		}
	void disable(void) // Disables the clipping plane
		{
		enabled=false;
		}
	const Plane& getPlane(void) const // Returns the clipping plane's plane equation
		{
		return plane;
		}
	Plane& getPlane(void) // Ditto
		{
		return plane;
		}
	};

}

#endif
