/***********************************************************************
GLGetTemplates - Overloaded versions of glGet...() functions.
Copyright (c) 2003-2005 Oliver Kreylos

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

#ifndef GLGETTEMPLATES_INCLUDED
#define GLGETTEMPLATES_INCLUDED

/*********************************
Overloaded versions of glGet call:
*********************************/

inline void glGet(GLenum pname,GLboolean* components)
	{
	glGetBooleanv(pname,components);
	}

inline void glGet(GLenum pname,GLint* components)
	{
	glGetIntegerv(pname,components);
	}

inline void glGet(GLenum pname,GLfloat* components)
	{
	glGetFloatv(pname,components);
	}

inline void glGet(GLenum pname,GLdouble* components)
	{
	glGetDoublev(pname,components);
	}

/******************************************
Generic version of single-value glGet call:
******************************************/

template <class ScalarParam>
inline ScalarParam glGet(GLenum pname)
	{
	ScalarParam result[1];
	glGet(pname,result);
	return result[0];
	}

#endif
