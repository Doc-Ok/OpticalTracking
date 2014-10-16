/***********************************************************************
GLMatrixTemplates - Overloaded versions of OpenGL API calls related to
matrix manipulation.
Copyright (c) 2003-2012 Oliver Kreylos

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

#ifndef GLMATRIXTEMPLATES_INCLUDED
#define GLMATRIXTEMPLATES_INCLUDED

#include <GL/gl.h>
#include <GL/GLVector.h>

inline void glTranslate(GLfloat x,GLfloat y,GLfloat z)
	{
	glTranslatef(x,y,z);
	}

inline void glTranslate(GLdouble x,GLdouble y,GLdouble z)
	{
	glTranslated(x,y,z);
	}

inline void glTranslate(const GLfloat t[3])
	{
	glTranslatef(t[0],t[1],t[2]);
	}

inline void glTranslate(const GLdouble t[3])
	{
	glTranslated(t[0],t[1],t[2]);
	}

template <class ScalarParam>
inline void glTranslate(const GLVector<ScalarParam,3>& t)
	{
	glTranslate(t.getXyzw());
	}

inline void glRotate(GLfloat angle,GLfloat axisX,GLfloat axisY,GLfloat axisZ)
	{
	glRotatef(angle,axisX,axisY,axisZ);
	}

inline void glRotate(GLdouble angle,GLdouble axisX,GLdouble axisY,GLdouble axisZ)
	{
	glRotated(angle,axisX,axisY,axisZ);
	}

inline void glRotate(GLfloat angle,const GLfloat axis[3])
	{
	glRotatef(angle,axis[0],axis[1],axis[2]);
	}

inline void glRotate(GLdouble angle,const GLdouble axis[3])
	{
	glRotated(angle,axis[0],axis[1],axis[2]);
	}

template <class ScalarParam>
inline void glRotate(ScalarParam angle,const GLVector<ScalarParam,3>& axis)
	{
	glRotate(angle,axis.getXyzw());
	}

inline void glScale(GLfloat scaleXYZ)
	{
	glScalef(scaleXYZ,scaleXYZ,scaleXYZ);
	}

inline void glScale(GLdouble scaleXYZ)
	{
	glScaled(scaleXYZ,scaleXYZ,scaleXYZ);
	}

inline void glScale(GLfloat scaleX,GLfloat scaleY,GLfloat scaleZ)
	{
	glScalef(scaleX,scaleY,scaleZ);
	}

inline void glScale(GLdouble scaleX,GLdouble scaleY,GLdouble scaleZ)
	{
	glScaled(scaleX,scaleY,scaleZ);
	}

inline void glScale(const GLfloat scale[3])
	{
	glScalef(scale[0],scale[1],scale[2]);
	}

inline void glScale(const GLdouble scale[3])
	{
	glScaled(scale[0],scale[1],scale[2]);
	}

template <class ScalarParam>
inline void glScale(const GLVector<ScalarParam,3>& scale)
	{
	glScale(scale.getXyzw());
	}

template <class ScalarParam>
inline void glScale(ScalarParam uniformScale)
	{
	glScale(uniformScale,uniformScale,uniformScale);
	}

inline void glLoadMatrix(const GLfloat matrix[16])
	{
	glLoadMatrixf(matrix);
	}

inline void glLoadMatrix(const GLdouble matrix[16])
	{
	glLoadMatrixd(matrix);
	}

inline void glMultMatrix(const GLfloat matrix[16])
	{
	glMultMatrixf(matrix);
	}

inline void glMultMatrix(const GLdouble matrix[16])
	{
	glMultMatrixd(matrix);
	}

#endif
