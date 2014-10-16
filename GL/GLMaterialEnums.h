/***********************************************************************
GLMaterialEnums - Helper class containing the enumerated values used by
the OpenGL material API.
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

#ifndef GLMATERIALENUMS_INCLUDED
#define GLMATERIALENUMS_INCLUDED

struct GLMaterialEnums
	{
	/* Embedded classes: */
	public:
	enum Pname // Enumerated type for material pname parameters
		{
		AMBIENT=GL_AMBIENT,
		DIFFUSE=GL_DIFFUSE,
		AMBIENT_AND_DIFFUSE=GL_AMBIENT_AND_DIFFUSE,
		SPECULAR=GL_SPECULAR,
		SHININESS=GL_SHININESS,
		EMISSION=GL_EMISSION,
		COLOR_INDEXES=GL_COLOR_INDEXES
		};
	
	enum Face // Enumerated type for material faces
		{
		FRONT=GL_FRONT,
		BACK=GL_BACK,
		FRONT_AND_BACK=GL_FRONT_AND_BACK
		};
	};

#endif
