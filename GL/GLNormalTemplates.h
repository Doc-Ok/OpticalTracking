/***********************************************************************
GLNormalTemplates - Overloaded versions of glNormal...() functions.
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

#ifndef GLNORMALTEMPLATES_INCLUDED
#define GLNORMALTEMPLATES_INCLUDED

#include <GL/gl.h>
#include <GL/GLVector.h>

/*****************************************************
Overloaded versions of component-based glNormal calls:
*****************************************************/

inline void glNormal(GLbyte x,GLbyte y,GLbyte z)
	{
	glNormal3b(x,y,z);
	}

inline void glNormal(GLshort x,GLshort y,GLshort z)
	{
	glNormal3s(x,y,z);
	}

inline void glNormal(GLint x,GLint y,GLint z)
	{
	glNormal3i(x,y,z);
	}

inline void glNormal(GLfloat x,GLfloat y,GLfloat z)
	{
	glNormal3f(x,y,z);
	}

inline void glNormal(GLdouble x,GLdouble y,GLdouble z)
	{
	glNormal3d(x,y,z);
	}

/**************************************************
Dummy generic version of array-based glNormal call:
**************************************************/

template <GLsizei numComponents,class ScalarParam>
void glNormal(const ScalarParam components[numComponents]);

/**************************************************
Specialized versions of array-based glNormal calls:
**************************************************/

template <>
inline void glNormal<3,GLbyte>(const GLbyte components[3])
	{
	glNormal3bv(components);
	}

template <>
inline void glNormal<3,GLshort>(const GLshort components[3])
	{
	glNormal3sv(components);
	}

template <>
inline void glNormal<3,GLint>(const GLint components[3])
	{
	glNormal3iv(components);
	}

template <>
inline void glNormal<3,GLfloat>(const GLfloat components[3])
	{
	glNormal3fv(components);
	}

template <>
inline void glNormal<3,GLdouble>(const GLdouble components[3])
	{
	glNormal3dv(components);
	}

/************************************
Overloaded versions of glNormal call:
************************************/

template <class ScalarParam>
inline void glNormal(const GLVector<ScalarParam,3>& param)
	{
	glNormal<3>(param.getXyzw());
	}

#endif
