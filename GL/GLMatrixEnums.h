/***********************************************************************
GLMatrixEnums - Helper class containing the enumerated values used by
the OpenGL matrix API.
Copyright (c) 2005 Oliver Kreylos

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

#ifndef GLMATRIXENUMS_INCLUDED
#define GLMATRIXENUMS_INCLUDED

#include <GL/gl.h>
#include <GL/glext.h>

struct GLMatrixEnums
	{
	/* Embedded classes: */
	public:
	enum Pname // Enumerated type for matrix pname parameters
		{
		COLOR=GL_COLOR_MATRIX,
		MODELVIEW=GL_MODELVIEW_MATRIX,
		PROJECTION=GL_PROJECTION_MATRIX,
		TEXTURE=GL_TEXTURE_MATRIX
		};
	};

#endif
