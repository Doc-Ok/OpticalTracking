/***********************************************************************
GLTexCoordTemplates - Overloaded versions of glTexCoord...() functions.
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

#ifndef GLTEXCOORDTEMPLATES_INCLUDED
#define GLTEXCOORDTEMPLATES_INCLUDED

#include <GL/gl.h>
#include <GL/GLVector.h>

/*******************************************************
Overloaded versions of component-based glTexCoord calls:
*******************************************************/

inline void glTexCoord(GLshort s)
	{
	glTexCoord1s(s);
	}

inline void glTexCoord(GLint s)
	{
	glTexCoord1i(s);
	}

inline void glTexCoord(GLfloat s)
	{
	glTexCoord1f(s);
	}

inline void glTexCoord(GLdouble s)
	{
	glTexCoord1d(s);
	}

inline void glTexCoord(GLshort s,GLshort t)
	{
	glTexCoord2s(s,t);
	}

inline void glTexCoord(GLint s,GLint t)
	{
	glTexCoord2i(s,t);
	}

inline void glTexCoord(GLfloat s,GLfloat t)
	{
	glTexCoord2f(s,t);
	}

inline void glTexCoord(GLdouble s,GLdouble t)
	{
	glTexCoord2d(s,t);
	}

inline void glTexCoord(GLshort s,GLshort t,GLshort r)
	{
	glTexCoord3s(s,t,r);
	}

inline void glTexCoord(GLint s,GLint t,GLint r)
	{
	glTexCoord3i(s,t,r);
	}

inline void glTexCoord(GLfloat s,GLfloat t,GLfloat r)
	{
	glTexCoord3f(s,t,r);
	}

inline void glTexCoord(GLdouble s,GLdouble t,GLdouble r)
	{
	glTexCoord3d(s,t,r);
	}

inline void glTexCoord(GLshort s,GLshort t,GLshort r,GLshort q)
	{
	glTexCoord4s(s,t,r,q);
	}

inline void glTexCoord(GLint s,GLint t,GLint r,GLint q)
	{
	glTexCoord4i(s,t,r,q);
	}

inline void glTexCoord(GLfloat s,GLfloat t,GLfloat r,GLfloat q)
	{
	glTexCoord4f(s,t,r,q);
	}

inline void glTexCoord(GLdouble s,GLdouble t,GLdouble r,GLdouble q)
	{
	glTexCoord4d(s,t,r,q);
	}

/****************************************************
Dummy generic version of array-based glTexCoord call:
****************************************************/

template <GLsizei numComponents,class ScalarParam>
void glTexCoord(const ScalarParam components[numComponents]);

/****************************************************
Specialized versions of array-based glTexCoord calls:
****************************************************/

template <>
inline void glTexCoord<1,GLshort>(const GLshort components[1])
	{
	glTexCoord1sv(components);
	}

template <>
inline void glTexCoord<1,GLint>(const GLint components[1])
	{
	glTexCoord1iv(components);
	}

template <>
inline void glTexCoord<1,GLfloat>(const GLfloat components[1])
	{
	glTexCoord1fv(components);
	}

template <>
inline void glTexCoord<1,GLdouble>(const GLdouble components[1])
	{
	glTexCoord1dv(components);
	}

template <>
inline void glTexCoord<2,GLshort>(const GLshort components[2])
	{
	glTexCoord2sv(components);
	}

template <>
inline void glTexCoord<2,GLint>(const GLint components[2])
	{
	glTexCoord2iv(components);
	}

template <>
inline void glTexCoord<2,GLfloat>(const GLfloat components[2])
	{
	glTexCoord2fv(components);
	}

template <>
inline void glTexCoord<2,GLdouble>(const GLdouble components[2])
	{
	glTexCoord2dv(components);
	}

template <>
inline void glTexCoord<3,GLshort>(const GLshort components[3])
	{
	glTexCoord3sv(components);
	}

template <>
inline void glTexCoord<3,GLint>(const GLint components[3])
	{
	glTexCoord3iv(components);
	}

template <>
inline void glTexCoord<3,GLfloat>(const GLfloat components[3])
	{
	glTexCoord3fv(components);
	}

template <>
inline void glTexCoord<3,GLdouble>(const GLdouble components[3])
	{
	glTexCoord3dv(components);
	}

template <>
inline void glTexCoord<4,GLshort>(const GLshort components[4])
	{
	glTexCoord4sv(components);
	}

template <>
inline void glTexCoord<4,GLint>(const GLint components[4])
	{
	glTexCoord4iv(components);
	}

template <>
inline void glTexCoord<4,GLfloat>(const GLfloat components[4])
	{
	glTexCoord4fv(components);
	}

template <>
inline void glTexCoord<4,GLdouble>(const GLdouble components[4])
	{
	glTexCoord4dv(components);
	}

/**************************************
Overloaded versions of glTexCoord call:
**************************************/

template <class ScalarParam,GLsizei numComponentsParam>
inline void glTexCoord(const GLVector<ScalarParam,numComponentsParam>& param)
	{
	glTexCoord<numComponentsParam>(param.getXyzw());
	}

#endif
