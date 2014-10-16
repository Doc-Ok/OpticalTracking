/***********************************************************************
GLFog - Class to encapsulate OpenGL fog properties.
Copyright (c) 2004-2005 Oliver Kreylos

This file is part of the OpenGL C++ Wrapper Library (GLWrappers).

The OpenGL C++ Wrapper Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The OpenGL C++ Wrapper Library is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the OpenGL C++ Wrapper Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef GLFOG_INCLUDED
#define GLFOG_INCLUDED

#include <GL/gl.h>
#include <GL/GLColor.h>
#include <GL/GLFogEnums.h>

struct GLFog
	{
	/* Embedded classes: */
	public:
	typedef GLFogEnums::Mode Mode; // Enumerated type for fog modes
	typedef GLfloat Scalar; // Type for scalar values
	typedef GLColor<Scalar,4> Color; // Type for colors used in fog
	
	/* Elements: */
	Mode mode; // Fog mode
	Scalar density; // Fog density in EXP and EXP2 fog modes
	Scalar start,end; // Fog near and far distances in LINEAR fog mode
	Color color; // Fog color
	
	/* Constructors and destructors: */
	GLFog(void); // Constructs default fog
	GLFog(const Color& sColor); // Constructs default fog of given color
	GLFog(Mode sMode,const Color& sColor); // Constructs default fog of given mode and color
	GLFog(Mode sMode,Scalar sDensity,const Color& sColor); // Constructs fog of mode EXP or EXP2 with given density and color
	GLFog(Mode sMode,Scalar sStart,Scalar sEnd,const Color& sColor); // Constructs fog of mode LINEAR with given start and end and color
	GLFog(Mode sMode,Scalar sDensity,Scalar sStart,Scalar sEnd,const Color& sColor); // Full initialization
	
	/* Methods: */
	friend void glFog(const GLFog& fog); // Sets fog properties
	friend void glGetFog(GLFog& fog); // Stores current fog properties in passed object
	friend GLFog glGetFog(void); // Returns current fog properties
	};

#endif
