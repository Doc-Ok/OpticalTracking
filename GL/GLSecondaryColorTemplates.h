/***********************************************************************
GLSecondaryColorTemplates - Overloaded versions of OpenGL API calls
related to secondary colors.
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

#ifndef GLSECONDARYCOLORTEMPLATES_INCLUDED
#define GLSECONDARYCOLORTEMPLATES_INCLUDED

#include <GL/gl.h>
#include <GL/GLColor.h>

/*************************************************************
Overloaded versions of component-based glSecondaryColor calls:
*************************************************************/

inline void glSecondaryColor(GLbyte r,GLbyte g,GLbyte b)
	{
	glSecondaryColor3b(r,g,b);
	}

inline void glSecondaryColor(GLubyte r,GLubyte g,GLubyte b)
	{
	glSecondaryColor3ub(r,g,b);
	}

inline void glSecondaryColor(GLshort r,GLshort g,GLshort b)
	{
	glSecondaryColor3s(r,g,b);
	}

inline void glSecondaryColor(GLushort r,GLushort g,GLushort b)
	{
	glSecondaryColor3us(r,g,b);
	}

inline void glSecondaryColor(GLint r,GLint g,GLint b)
	{
	glSecondaryColor3i(r,g,b);
	}

inline void glSecondaryColor(GLuint r,GLuint g,GLuint b)
	{
	glSecondaryColor3ui(r,g,b);
	}

inline void glSecondaryColor(GLfloat r,GLfloat g,GLfloat b)
	{
	glSecondaryColor3f(r,g,b);
	}

inline void glSecondaryColor(GLdouble r,GLdouble g,GLdouble b)
	{
	glSecondaryColor3d(r,g,b);
	}

/**********************************************************
Dummy generic version of array-based glSecondaryColor call:
**********************************************************/

template <GLsizei numComponents,class ScalarParam>
void glSecondaryColor(const ScalarParam components[numComponents]);

/*********************************************************
Specialized versions of array-based glSecondaryColor call:
*********************************************************/

template <>
inline void glSecondaryColor<3,GLbyte>(const GLbyte components[3])
	{
	glSecondaryColor3bv(components);
	}

template <>
inline void glSecondaryColor<3,GLubyte>(const GLubyte components[3])
	{
	glSecondaryColor3ubv(components);
	}

template <>
inline void glSecondaryColor<3,GLshort>(const GLshort components[3])
	{
	glSecondaryColor3sv(components);
	}

template <>
inline void glSecondaryColor<3,GLushort>(const GLushort components[3])
	{
	glSecondaryColor3usv(components);
	}

template <>
inline void glSecondaryColor<3,GLint>(const GLint components[3])
	{
	glSecondaryColor3iv(components);
	}

template <>
inline void glSecondaryColor<3,GLuint>(const GLuint components[3])
	{
	glSecondaryColor3uiv(components);
	}

template <>
inline void glSecondaryColor<3,GLfloat>(const GLfloat components[3])
	{
	glSecondaryColor3fv(components);
	}

template <>
inline void glSecondaryColor<3,GLdouble>(const GLdouble components[3])
	{
	glSecondaryColor3dv(components);
	}

/********************************************
Overloaded versions of glSecondaryColor call:
********************************************/

template <class ScalarParam>
inline void glSecondaryColor(const GLColor<ScalarParam,3>& param)
	{
	glSecondaryColor<3>(param.getRgba());
	}

#endif
