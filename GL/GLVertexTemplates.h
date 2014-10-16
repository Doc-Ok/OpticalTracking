/***********************************************************************
GLVertexTemplates - Overloaded versions of glVertex...() functions.
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

#ifndef GLVERTEXTEMPLATES_INCLUDED
#define GLVERTEXTEMPLATES_INCLUDED

#include <GL/gl.h>
#include <GL/GLVector.h>

/*****************************************************
Overloaded versions of component-based glVertex calls:
*****************************************************/

inline void glVertex(GLshort x,GLshort y)
	{
	glVertex2s(x,y);
	}

inline void glVertex(GLint x,GLint y)
	{
	glVertex2i(x,y);
	}

inline void glVertex(GLfloat x,GLfloat y)
	{
	glVertex2f(x,y);
	}

inline void glVertex(GLdouble x,GLdouble y)
	{
	glVertex2d(x,y);
	}

inline void glVertex(GLshort x,GLshort y,GLshort z)
	{
	glVertex3s(x,y,z);
	}

inline void glVertex(GLint x,GLint y,GLint z)
	{
	glVertex3i(x,y,z);
	}

inline void glVertex(GLfloat x,GLfloat y,GLfloat z)
	{
	glVertex3f(x,y,z);
	}

inline void glVertex(GLdouble x,GLdouble y,GLdouble z)
	{
	glVertex3d(x,y,z);
	}

inline void glVertex(GLshort x,GLshort y,GLshort z,GLshort w)
	{
	glVertex4s(x,y,z,w);
	}

inline void glVertex(GLint x,GLint y,GLint z,GLint w)
	{
	glVertex4i(x,y,z,w);
	}

inline void glVertex(GLfloat x,GLfloat y,GLfloat z,GLfloat w)
	{
	glVertex4f(x,y,z,w);
	}

inline void glVertex(GLdouble x,GLdouble y,GLdouble z,GLdouble w)
	{
	glVertex4d(x,y,z,w);
	}

/**************************************************
Dummy generic version of array-based glVertex call:
**************************************************/

template <GLsizei numComponents,class ScalarParam>
void glVertex(const ScalarParam components[numComponents]);

/**************************************************
Specialized versions of array-based glVertex calls:
**************************************************/

template <>
inline void glVertex<2,GLshort>(const GLshort components[2])
	{
	glVertex2sv(components);
	}

template <>
inline void glVertex<2,GLint>(const GLint components[2])
	{
	glVertex2iv(components);
	}

template <>
inline void glVertex<2,GLfloat>(const GLfloat components[2])
	{
	glVertex2fv(components);
	}

template <>
inline void glVertex<2,GLdouble>(const GLdouble components[2])
	{
	glVertex2dv(components);
	}

template <>
inline void glVertex<3,GLshort>(const GLshort components[3])
	{
	glVertex3sv(components);
	}

template <>
inline void glVertex<3,GLint>(const GLint components[3])
	{
	glVertex3iv(components);
	}

template <>
inline void glVertex<3,GLfloat>(const GLfloat components[3])
	{
	glVertex3fv(components);
	}

template <>
inline void glVertex<3,GLdouble>(const GLdouble components[3])
	{
	glVertex3dv(components);
	}

template <>
inline void glVertex<4,GLshort>(const GLshort components[4])
	{
	glVertex4sv(components);
	}

template <>
inline void glVertex<4,GLint>(const GLint components[4])
	{
	glVertex4iv(components);
	}

template <>
inline void glVertex<4,GLfloat>(const GLfloat components[4])
	{
	glVertex4fv(components);
	}

template <>
inline void glVertex<4,GLdouble>(const GLdouble components[4])
	{
	glVertex4dv(components);
	}

/************************************
Overloaded versions of glVertex call:
************************************/

template <class ScalarParam,GLsizei numComponentsParam>
inline void glVertex(const GLVector<ScalarParam,numComponentsParam>& param)
	{
	glVertex<numComponentsParam>(param.getXyzw());
	}

#endif
