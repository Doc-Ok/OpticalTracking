/***********************************************************************
GLColorTemplates - Overloaded versions of glColor...() functions.
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

#ifndef GLCOLORTEMPLATES_INCLUDED
#define GLCOLORTEMPLATES_INCLUDED

#include <GL/gl.h>
#include <GL/GLColor.h>

/****************************************************
Overloaded versions of component-based glColor calls:
****************************************************/

inline void glColor(GLbyte r,GLbyte g,GLbyte b)
	{
	glColor3b(r,g,b);
	}

inline void glColor(GLubyte r,GLubyte g,GLubyte b)
	{
	glColor3ub(r,g,b);
	}

inline void glColor(GLshort r,GLshort g,GLshort b)
	{
	glColor3s(r,g,b);
	}

inline void glColor(GLushort r,GLushort g,GLushort b)
	{
	glColor3us(r,g,b);
	}

inline void glColor(GLint r,GLint g,GLint b)
	{
	glColor3i(r,g,b);
	}

inline void glColor(GLuint r,GLuint g,GLuint b)
	{
	glColor3ui(r,g,b);
	}

inline void glColor(GLfloat r,GLfloat g,GLfloat b)
	{
	glColor3f(r,g,b);
	}

inline void glColor(GLdouble r,GLdouble g,GLdouble b)
	{
	glColor3d(r,g,b);
	}

inline void glColor(GLbyte r,GLbyte g,GLbyte b,GLbyte a)
	{
	glColor4b(r,g,b,a);
	}

inline void glColor(GLubyte r,GLubyte g,GLubyte b,GLubyte a)
	{
	glColor4ub(r,g,b,a);
	}

inline void glColor(GLshort r,GLshort g,GLshort b,GLshort a)
	{
	glColor4s(r,g,b,a);
	}

inline void glColor(GLushort r,GLushort g,GLushort b,GLushort a)
	{
	glColor4us(r,g,b,a);
	}

inline void glColor(GLint r,GLint g,GLint b,GLint a)
	{
	glColor4i(r,g,b,a);
	}

inline void glColor(GLuint r,GLuint g,GLuint b,GLuint a)
	{
	glColor4ui(r,g,b,a);
	}

inline void glColor(GLfloat r,GLfloat g,GLfloat b,GLfloat a)
	{
	glColor4f(r,g,b,a);
	}

inline void glColor(GLdouble r,GLdouble g,GLdouble b,GLdouble a)
	{
	glColor4d(r,g,b,a);
	}

/*************************************************
Dummy generic version of array-based glColor call:
*************************************************/

template <GLsizei numComponents,class ScalarParam>
void glColor(const ScalarParam components[numComponents]);

/************************************************
Specialized versions of array-based glColor call:
************************************************/

template <>
inline void glColor<3,GLbyte>(const GLbyte components[3])
	{
	glColor3bv(components);
	}

template <>
inline void glColor<3,GLubyte>(const GLubyte components[3])
	{
	glColor3ubv(components);
	}

template <>
inline void glColor<3,GLshort>(const GLshort components[3])
	{
	glColor3sv(components);
	}

template <>
inline void glColor<3,GLushort>(const GLushort components[3])
	{
	glColor3usv(components);
	}

template <>
inline void glColor<3,GLint>(const GLint components[3])
	{
	glColor3iv(components);
	}

template <>
inline void glColor<3,GLuint>(const GLuint components[3])
	{
	glColor3uiv(components);
	}

template <>
inline void glColor<3,GLfloat>(const GLfloat components[3])
	{
	glColor3fv(components);
	}

template <>
inline void glColor<3,GLdouble>(const GLdouble components[3])
	{
	glColor3dv(components);
	}

template <>
inline void glColor<4,GLbyte>(const GLbyte components[4])
	{
	glColor4bv(components);
	}

template <>
inline void glColor<4,GLubyte>(const GLubyte components[4])
	{
	glColor4ubv(components);
	}

template <>
inline void glColor<4,GLshort>(const GLshort components[4])
	{
	glColor4sv(components);
	}

template <>
inline void glColor<4,GLushort>(const GLushort components[4])
	{
	glColor4usv(components);
	}

template <>
inline void glColor<4,GLint>(const GLint components[4])
	{
	glColor4iv(components);
	}

template <>
inline void glColor<4,GLuint>(const GLuint components[4])
	{
	glColor4uiv(components);
	}

template <>
inline void glColor<4,GLfloat>(const GLfloat components[4])
	{
	glColor4fv(components);
	}

template <>
inline void glColor<4,GLdouble>(const GLdouble components[4])
	{
	glColor4dv(components);
	}

/***********************************
Overloaded versions of glColor call:
***********************************/

template <class ScalarParam,GLsizei numComponentsParam>
inline void glColor(const GLColor<ScalarParam,numComponentsParam>& param)
	{
	glColor<numComponentsParam>(param.getRgba());
	}

inline void glClearColor(const GLColor<GLfloat,4>& param)
	{
	glClearColor(param[0],param[1],param[2],param[3]);
	}

#endif
