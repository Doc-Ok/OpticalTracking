/***********************************************************************
GLVertexArrayTemplates - Overloaded versions of OpenGL API calls related
to vertex arrays.
Copyright (c) 2004-2013 Oliver Kreylos

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

#ifndef GLVERTEXARRAYTEMPLATES_INCLUDED
#define GLVERTEXARRAYTEMPLATES_INCLUDED

#include <GL/gl.h>
#include <GL/GLColor.h>
#include <GL/GLVector.h>

/**********************************************
Overloaded versions of glTexCoordPointer calls:
**********************************************/

inline void glTexCoordPointer(GLint numComponents,GLsizei stride,const GLshort* pointer)
	{
	glTexCoordPointer(numComponents,GL_SHORT,stride,pointer);
	}

inline void glTexCoordPointer(GLint numComponents,GLsizei stride,const GLint* pointer)
	{
	glTexCoordPointer(numComponents,GL_INT,stride,pointer);
	}

inline void glTexCoordPointer(GLint numComponents,GLsizei stride,const GLfloat* pointer)
	{
	glTexCoordPointer(numComponents,GL_FLOAT,stride,pointer);
	}

inline void glTexCoordPointer(GLint numComponents,GLsizei stride,const GLdouble* pointer)
	{
	glTexCoordPointer(numComponents,GL_DOUBLE,stride,pointer);
	}

template <class ScalarParam,GLsizei numComponentsParam>
inline void glTexCoordPointer(GLsizei stride,const GLVector<ScalarParam,numComponentsParam>* pointer)
	{
	glTexCoordPointer(numComponentsParam,stride,pointer[0].getXyzw());
	}

/********************************************
Overloaded versions of glNormalPointer calls:
********************************************/

inline void glNormalPointer(GLsizei stride,const GLbyte* pointer)
	{
	glNormalPointer(GL_BYTE,stride,pointer);
	}

inline void glNormalPointer(GLsizei stride,const GLshort* pointer)
	{
	glNormalPointer(GL_SHORT,stride,pointer);
	}

inline void glNormalPointer(GLsizei stride,const GLint* pointer)
	{
	glNormalPointer(GL_INT,stride,pointer);
	}

inline void glNormalPointer(GLsizei stride,const GLfloat* pointer)
	{
	glNormalPointer(GL_FLOAT,stride,pointer);
	}

inline void glNormalPointer(GLsizei stride,const GLdouble* pointer)
	{
	glNormalPointer(GL_DOUBLE,stride,pointer);
	}

template <class ScalarParam>
inline void glNormalPointer(GLsizei stride,const GLVector<ScalarParam,3>* pointer)
	{
	glNormalPointer(stride,pointer[0].getXyzw());
	}

/*******************************************
Overloaded versions of glColorPointer calls:
*******************************************/

inline void glColorPointer(GLint numComponents,GLsizei stride,const GLbyte* pointer)
	{
	glColorPointer(numComponents,GL_BYTE,stride,pointer);
	}

inline void glColorPointer(GLint numComponents,GLsizei stride,const GLubyte* pointer)
	{
	glColorPointer(numComponents,GL_UNSIGNED_BYTE,stride,pointer);
	}

inline void glColorPointer(GLint numComponents,GLsizei stride,const GLshort* pointer)
	{
	glColorPointer(numComponents,GL_SHORT,stride,pointer);
	}

inline void glColorPointer(GLint numComponents,GLsizei stride,const GLushort* pointer)
	{
	glColorPointer(numComponents,GL_UNSIGNED_SHORT,stride,pointer);
	}

inline void glColorPointer(GLint numComponents,GLsizei stride,const GLint* pointer)
	{
	glColorPointer(numComponents,GL_INT,stride,pointer);
	}

inline void glColorPointer(GLint numComponents,GLsizei stride,const GLuint* pointer)
	{
	glColorPointer(numComponents,GL_UNSIGNED_INT,stride,pointer);
	}

inline void glColorPointer(GLint numComponents,GLsizei stride,const GLfloat* pointer)
	{
	glColorPointer(numComponents,GL_FLOAT,stride,pointer);
	}

inline void glColorPointer(GLint numComponents,GLsizei stride,const GLdouble* pointer)
	{
	glColorPointer(numComponents,GL_DOUBLE,stride,pointer);
	}

template <class ScalarParam,GLsizei numComponentsParam>
inline void glColorPointer(GLsizei stride,const GLColor<ScalarParam,numComponentsParam>* pointer)
	{
	glColorPointer(numComponentsParam,stride,pointer[0].getRgba());
	}

#if 0

/****************************************************
Overloaded versions of glSecondaryColorPointer calls:
****************************************************/

inline void glSecondaryColorPointer(GLint numComponents,GLsizei stride,const GLbyte* pointer)
	{
	glSecondaryColorPointer(numComponents,GL_BYTE,stride,pointer);
	}

inline void glSecondaryColorPointer(GLint numComponents,GLsizei stride,const GLubyte* pointer)
	{
	glSecondaryColorPointer(numComponents,GL_UNSIGNED_BYTE,stride,pointer);
	}

inline void glSecondaryColorPointer(GLint numComponents,GLsizei stride,const GLshort* pointer)
	{
	glSecondaryColorPointer(numComponents,GL_SHORT,stride,pointer);
	}

inline void glSecondaryColorPointer(GLint numComponents,GLsizei stride,const GLushort* pointer)
	{
	glSecondaryColorPointer(numComponents,GL_UNSIGNED_SHORT,stride,pointer);
	}

inline void glSecondaryColorPointer(GLint numComponents,GLsizei stride,const GLint* pointer)
	{
	glSecondaryColorPointer(numComponents,GL_INT,stride,pointer);
	}

inline void glSecondaryColorPointer(GLint numComponents,GLsizei stride,const GLuint* pointer)
	{
	glSecondaryColorPointer(numComponents,GL_UNSIGNED_INT,stride,pointer);
	}

inline void glSecondaryColorPointer(GLint numComponents,GLsizei stride,const GLfloat* pointer)
	{
	glSecondaryColorPointer(numComponents,GL_FLOAT,stride,pointer);
	}

inline void glSecondaryColorPointer(GLint numComponents,GLsizei stride,const GLdouble* pointer)
	{
	glSecondaryColorPointer(numComponents,GL_DOUBLE,stride,pointer);
	}

template <class ScalarParam,GLsizei numComponentsParam>
inline void glSecondaryColorPointer(GLsizei stride,const GLColor<ScalarParam,numComponentsParam>* pointer)
	{
	glSecondaryColorPointer(numComponentsParam,stride,pointer[0].getRgba());
	}

#endif

/*******************************************
Overloaded versions of glIndexPointer calls:
*******************************************/

inline void glIndexPointer(GLsizei stride,const GLubyte* pointer)
	{
	glIndexPointer(GL_UNSIGNED_BYTE,stride,pointer);
	}

inline void glIndexPointer(GLsizei stride,const GLshort* pointer)
	{
	glIndexPointer(GL_SHORT,stride,pointer);
	}

inline void glIndexPointer(GLsizei stride,const GLint* pointer)
	{
	glIndexPointer(GL_INT,stride,pointer);
	}

inline void glIndexPointer(GLsizei stride,const GLfloat* pointer)
	{
	glIndexPointer(GL_FLOAT,stride,pointer);
	}

inline void glIndexPointer(GLsizei stride,const GLdouble* pointer)
	{
	glIndexPointer(GL_DOUBLE,stride,pointer);
	}

/********************************************
Overloaded versions of glVertexPointer calls:
********************************************/

inline void glVertexPointer(GLint numComponents,GLsizei stride,const GLshort* pointer)
	{
	glVertexPointer(numComponents,GL_SHORT,stride,pointer);
	}

inline void glVertexPointer(GLint numComponents,GLsizei stride,const GLint* pointer)
	{
	glVertexPointer(numComponents,GL_INT,stride,pointer);
	}

inline void glVertexPointer(GLint numComponents,GLsizei stride,const GLfloat* pointer)
	{
	glVertexPointer(numComponents,GL_FLOAT,stride,pointer);
	}

inline void glVertexPointer(GLint numComponents,GLsizei stride,const GLdouble* pointer)
	{
	glVertexPointer(numComponents,GL_DOUBLE,stride,pointer);
	}

template <class ScalarParam,GLsizei numComponentsParam>
inline void glVertexPointer(GLsizei stride,const GLVector<ScalarParam,numComponentsParam>* pointer)
	{
	glVertexPointer(numComponentsParam,stride,pointer[0].getXyzw());
	}

#endif
