/***********************************************************************
GLLightModelEnums - Helper class containing the enumerated values used
by the OpenGL light model API.
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

#ifndef GLLIGHTMODELENUMS_INCLUDED
#define GLLIGHTMODELENUMS_INCLUDED

struct GLLightModelEnums
	{
	/* Embedded classes: */
	public:
	enum Pname // Enumerated type for light model pname parameters
		{
		AMBIENT=GL_LIGHT_MODEL_AMBIENT,
		COLOR_CONTROL=GL_LIGHT_MODEL_COLOR_CONTROL,
		LOCAL_VIEWER=GL_LIGHT_MODEL_LOCAL_VIEWER,
		TWO_SIDE=GL_LIGHT_MODEL_TWO_SIDE
		};
	
	enum ColorControl // Enumerated type for light model color control values
		{
		SINGLE_COLOR=GL_SINGLE_COLOR,
		SEPARATE_SPECULAR_COLOR=GL_SEPARATE_SPECULAR_COLOR
		};
	};

#endif
