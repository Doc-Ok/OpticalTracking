/***********************************************************************
Lightsource - Class to describe light sources in virtual environments.
Since light sources in OpenGL are a limited resource, Vrui contains an
abstraction that allows users to create light sources as needed, and
maps created light sources to OpenGL light sources as needed.
Copyright (c) 2005 Oliver Kreylos

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

#ifndef VRUI_LIGHTSOURCE_INCLUDED
#define VRUI_LIGHTSOURCE_INCLUDED

#include <GL/GLLight.h>

namespace Vrui {

class Lightsource
	{
	/* Elements: */
	private:
	bool enabled; // Flag if the light source is enabled
	GLLight light; // OpenGL parameters defining the light source
	
	/* Constructors and destructors: */
	public:
	Lightsource(void) // Creates an enabled light source with standard OpenGL parameters
		:enabled(true)
		{
		}
	Lightsource(const GLLight& sLight) // Creates an enabled light source with the given OpenGL parameters
		:enabled(true),light(sLight)
		{
		}
	
	/* Methods: */
	bool isEnabled(void) const // Returns the enabled flag
		{
		return enabled;
		}
	void enable(void) // Enables the light source
		{
		enabled=true;
		}
	void disable(void) // Disables the light source
		{
		enabled=false;
		}
	const GLLight& getLight(void) const // Returns the light source's OpenGL parameters
		{
		return light;
		}
	GLLight& getLight(void) // Ditto
		{
		return light;
		}
	};

}

#endif
