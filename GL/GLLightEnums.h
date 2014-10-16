/***********************************************************************
GLLightEnums - Helper class containing the enumerated values used by the
OpenGL light source API.
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

#ifndef GLLIGHTENUMS_INCLUDED
#define GLLIGHTENUMS_INCLUDED

struct GLLightEnums
	{
	/* Embedded classes: */
	public:
	enum Pname // Enumerated type for light pname parameters
		{
		AMBIENT=GL_AMBIENT,
		DIFFUSE=GL_DIFFUSE,
		SPECULAR=GL_SPECULAR,
		POSITION=GL_POSITION,
		SPOT_DIRECTION=GL_SPOT_DIRECTION,
		SPOT_EXPONENT=GL_SPOT_EXPONENT,
		SPOT_CUTOFF=GL_SPOT_CUTOFF,
		CONSTANT_ATTENUATION=GL_CONSTANT_ATTENUATION,
		LINEAR_ATTENUATION=GL_LINEAR_ATTENUATION,
		QUADRATIC_ATTENUATION=GL_QUADRATIC_ATTENUATION
		};
	
	enum Light // Enumerated type for light source names
		{
		LIGHT0=GL_LIGHT0,
		LIGHT1=GL_LIGHT1,
		LIGHT2=GL_LIGHT2,
		LIGHT3=GL_LIGHT3,
		LIGHT4=GL_LIGHT4,
		LIGHT5=GL_LIGHT5,
		LIGHT6=GL_LIGHT6,
		LIGHT7=GL_LIGHT7
		};
	};

#endif
