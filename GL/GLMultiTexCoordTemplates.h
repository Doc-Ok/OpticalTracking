/***********************************************************************
GLMultiTexCoordTemplates - Overloaded versions of glMultiTexCoord...()
functions.
The 'GLenum texture" parameter of all glMultiTexCoord...() functions has
been replaced by a 'GLsizei textureIndex' parameter in the range from 0
to the number of texture units-1.
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

#ifndef GLMULTITEXCOORDTEMPLATES_INCLUDED
#define GLMULTITEXCOORDTEMPLATES_INCLUDED

#include <GL/gl.h>
#include <GL/GLVector.h>

/************************************************************
Overloaded versions of component-based glMultiTexCoord calls:
************************************************************/

inline void glMultiTexCoord(GLsizei textureIndex,GLshort s)
	{
	glMultiTexCoord1s(GL_TEXTURE0+textureIndex,s);
	}

inline void glMultiTexCoord(GLsizei textureIndex,GLint s)
	{
	glMultiTexCoord1i(GL_TEXTURE0+textureIndex,s);
	}

inline void glMultiTexCoord(GLsizei textureIndex,GLfloat s)
	{
	glMultiTexCoord1f(GL_TEXTURE0+textureIndex,s);
	}

inline void glMultiTexCoord(GLsizei textureIndex,GLdouble s)
	{
	glMultiTexCoord1d(GL_TEXTURE0+textureIndex,s);
	}

inline void glMultiTexCoord(GLsizei textureIndex,GLshort s,GLshort t)
	{
	glMultiTexCoord2s(GL_TEXTURE0+textureIndex,s,t);
	}

inline void glMultiTexCoord(GLsizei textureIndex,GLint s,GLint t)
	{
	glMultiTexCoord2i(GL_TEXTURE0+textureIndex,s,t);
	}

inline void glMultiTexCoord(GLsizei textureIndex,GLfloat s,GLfloat t)
	{
	glMultiTexCoord2f(GL_TEXTURE0+textureIndex,s,t);
	}

inline void glMultiTexCoord(GLsizei textureIndex,GLdouble s,GLdouble t)
	{
	glMultiTexCoord2d(GL_TEXTURE0+textureIndex,s,t);
	}

inline void glMultiTexCoord(GLsizei textureIndex,GLshort s,GLshort t,GLshort r)
	{
	glMultiTexCoord3s(GL_TEXTURE0+textureIndex,s,t,r);
	}

inline void glMultiTexCoord(GLsizei textureIndex,GLint s,GLint t,GLint r)
	{
	glMultiTexCoord3i(GL_TEXTURE0+textureIndex,s,t,r);
	}

inline void glMultiTexCoord(GLsizei textureIndex,GLfloat s,GLfloat t,GLfloat r)
	{
	glMultiTexCoord3f(GL_TEXTURE0+textureIndex,s,t,r);
	}

inline void glMultiTexCoord(GLsizei textureIndex,GLdouble s,GLdouble t,GLdouble r)
	{
	glMultiTexCoord3d(GL_TEXTURE0+textureIndex,s,t,r);
	}

inline void glMultiTexCoord(GLsizei textureIndex,GLshort s,GLshort t,GLshort r,GLshort q)
	{
	glMultiTexCoord4s(GL_TEXTURE0+textureIndex,s,t,r,q);
	}

inline void glMultiTexCoord(GLsizei textureIndex,GLint s,GLint t,GLint r,GLint q)
	{
	glMultiTexCoord4i(GL_TEXTURE0+textureIndex,s,t,r,q);
	}

inline void glMultiTexCoord(GLsizei textureIndex,GLfloat s,GLfloat t,GLfloat r,GLfloat q)
	{
	glMultiTexCoord4f(GL_TEXTURE0+textureIndex,s,t,r,q);
	}

inline void glMultiTexCoord(GLsizei textureIndex,GLdouble s,GLdouble t,GLdouble r,GLdouble q)
	{
	glMultiTexCoord4d(GL_TEXTURE0+textureIndex,s,t,r,q);
	}

/*********************************************************
Dummy generic version of array-based glMultiTexCoord call:
*********************************************************/

template <GLsizei numComponents,class ScalarParam>
void glMultiTexCoord(GLsizei textureIndex,const ScalarParam components[numComponents]);

/*********************************************************
Specialized versions of array-based glMultiTexCoord calls:
*********************************************************/

template <>
inline void glMultiTexCoord<1,GLshort>(GLsizei textureIndex,const GLshort components[1])
	{
	glMultiTexCoord1sv(GL_TEXTURE0+textureIndex,components);
	}

template <>
inline void glMultiTexCoord<1,GLint>(GLsizei textureIndex,const GLint components[1])
	{
	glMultiTexCoord1iv(GL_TEXTURE0+textureIndex,components);
	}

template <>
inline void glMultiTexCoord<1,GLfloat>(GLsizei textureIndex,const GLfloat components[1])
	{
	glMultiTexCoord1fv(GL_TEXTURE0+textureIndex,components);
	}

template <>
inline void glMultiTexCoord<1,GLdouble>(GLsizei textureIndex,const GLdouble components[1])
	{
	glMultiTexCoord1dv(GL_TEXTURE0+textureIndex,components);
	}

template <>
inline void glMultiTexCoord<2,GLshort>(GLsizei textureIndex,const GLshort components[2])
	{
	glMultiTexCoord2sv(GL_TEXTURE0+textureIndex,components);
	}

template <>
inline void glMultiTexCoord<2,GLint>(GLsizei textureIndex,const GLint components[2])
	{
	glMultiTexCoord2iv(GL_TEXTURE0+textureIndex,components);
	}

template <>
inline void glMultiTexCoord<2,GLfloat>(GLsizei textureIndex,const GLfloat components[2])
	{
	glMultiTexCoord2fv(GL_TEXTURE0+textureIndex,components);
	}

template <>
inline void glMultiTexCoord<2,GLdouble>(GLsizei textureIndex,const GLdouble components[2])
	{
	glMultiTexCoord2dv(GL_TEXTURE0+textureIndex,components);
	}

template <>
inline void glMultiTexCoord<3,GLshort>(GLsizei textureIndex,const GLshort components[3])
	{
	glMultiTexCoord3sv(GL_TEXTURE0+textureIndex,components);
	}

template <>
inline void glMultiTexCoord<3,GLint>(GLsizei textureIndex,const GLint components[3])
	{
	glMultiTexCoord3iv(GL_TEXTURE0+textureIndex,components);
	}

template <>
inline void glMultiTexCoord<3,GLfloat>(GLsizei textureIndex,const GLfloat components[3])
	{
	glMultiTexCoord3fv(GL_TEXTURE0+textureIndex,components);
	}

template <>
inline void glMultiTexCoord<3,GLdouble>(GLsizei textureIndex,const GLdouble components[3])
	{
	glMultiTexCoord3dv(GL_TEXTURE0+textureIndex,components);
	}

template <>
inline void glMultiTexCoord<4,GLshort>(GLsizei textureIndex,const GLshort components[4])
	{
	glMultiTexCoord4sv(GL_TEXTURE0+textureIndex,components);
	}

template <>
inline void glMultiTexCoord<4,GLint>(GLsizei textureIndex,const GLint components[4])
	{
	glMultiTexCoord4iv(GL_TEXTURE0+textureIndex,components);
	}

template <>
inline void glMultiTexCoord<4,GLfloat>(GLsizei textureIndex,const GLfloat components[4])
	{
	glMultiTexCoord4fv(GL_TEXTURE0+textureIndex,components);
	}

template <>
inline void glMultiTexCoord<4,GLdouble>(GLsizei textureIndex,const GLdouble components[4])
	{
	glMultiTexCoord4dv(GL_TEXTURE0+textureIndex,components);
	}

/*******************************************
Overloaded versions of glMultiTexCoord call:
*******************************************/

template <class ScalarParam,GLsizei numComponentsParam>
inline void glMultiTexCoord(GLsizei textureIndex,const GLVector<ScalarParam,numComponentsParam>& param)
	{
	glMultiTexCoord<numComponentsParam>(textureIndex,param.getXyzw());
	}

#endif
