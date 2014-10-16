/***********************************************************************
GLFogEnums - Helper class containing the enumerated values used by the
OpenGL fog API.
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

#ifndef GLFOGENUMS_INCLUDED
#define GLFOGENUMS_INCLUDED

struct GLFogEnums
	{
	/* Embedded classes: */
	public:
	enum Pname // Enumerated type for fog pname parameters
		{
		MODE=GL_FOG_MODE,
		DENSITY=GL_FOG_DENSITY,
		START=GL_FOG_START,
		END=GL_FOG_END,
		INDEX=GL_FOG_INDEX,
		COLOR=GL_FOG_COLOR
		};
	
	enum Mode // Enumerated type for fog modes
		{
		EXP=GL_EXP,
		EXP2=GL_EXP2,
		LINEAR=GL_LINEAR
		};
	};

#endif
