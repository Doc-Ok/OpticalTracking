/***********************************************************************
GLTexEnvEnums - Helper class containing the enumerated values used by
the OpenGL texture environment API.
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

#ifndef GLTEXENVENUMS_INCLUDED
#define GLTEXENVENUMS_INCLUDED

struct GLTexEnvEnums
	{
	/* Embedded classes: */
	public:
	enum Target // Enumerated type for texture environment target parameters
		{
		TEXTURE_ENV=GL_TEXTURE_ENV
		};
	
	enum Pname // Enumerated type for texture environment pname parameters
		{
		MODE=GL_TEXTURE_ENV_MODE,
		COLOR=GL_TEXTURE_ENV_COLOR
		};
	
	enum Mode // Enumerated type for texture environment modes
		{
		MODULATE=GL_MODULATE,
		DECAL=GL_DECAL,
		BLEND=GL_BLEND,
		REPLACE=GL_REPLACE
		};
	};

#endif
