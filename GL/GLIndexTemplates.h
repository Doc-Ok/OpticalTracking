/***********************************************************************
GLIndexTemplates - Overloaded versions of glIndex...() functions.
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

#ifndef GLINDEXTEMPLATES_INCLUDED
#define GLINDEXTEMPLATES_INCLUDED

#include <GL/gl.h>

/****************************************************
Overloaded versions of component-based glIndex calls:
****************************************************/

inline void glIndex(GLubyte i)
	{
	glIndexub(i);
	}

inline void glIndex(GLshort i)
	{
	glIndexs(i);
	}

inline void glIndex(GLint i)
	{
	glIndexi(i);
	}

inline void glIndex(GLfloat i)
	{
	glIndexf(i);
	}

inline void glIndex(GLdouble i)
	{
	glIndexd(i);
	}

/****************************************************
Dummy generic version of array-based glIndex call:
****************************************************/

template <GLsizei numComponents,class ScalarParam>
void glIndex(const ScalarParam components[numComponents]);

/****************************************************
Specialized versions of array-based glIndex calls:
****************************************************/

template <>
inline void glIndex<1,GLubyte>(const GLubyte components[1])
	{
	glIndexubv(components);
	}

template <>
inline void glIndex<1,GLshort>(const GLshort components[1])
	{
	glIndexsv(components);
	}

template <>
inline void glIndex<1,GLint>(const GLint components[1])
	{
	glIndexiv(components);
	}

template <>
inline void glIndex<1,GLfloat>(const GLfloat components[1])
	{
	glIndexfv(components);
	}

template <>
inline void glIndex<1,GLdouble>(const GLdouble components[1])
	{
	glIndexdv(components);
	}

#endif
