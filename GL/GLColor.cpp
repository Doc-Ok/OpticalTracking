/***********************************************************************
GLColor - Class to represent color values in RGBA format.
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

#include <GL/GLScalarConverter.h>

#include <GL/GLColor.h>

/********************************************************************
Static elements of class GLColorBase<ScalarParam,numComponentsParam>:
********************************************************************/

template <class ScalarParam,GLsizei numComponentsParam>
const GLsizei GLColorBase<ScalarParam,numComponentsParam>::numComponents;

/************************************************************
Methods of class GLColorBase<ScalarParam,numComponentsParam>:
************************************************************/

template <class ScalarParam,GLsizei numComponentsParam>
void
GLColorBase<ScalarParam,numComponentsParam>::copy(
	GLsizei sNumComponents,
	const typename GLColorBase<ScalarParam,numComponentsParam>::Scalar* sRgba)
	{
	for(GLsizei i=0;i<sNumComponents;++i)
		rgba[i]=sRgba[i];
	}

template <class ScalarParam,GLsizei numComponentsParam>
template <class SourceScalarParam>
void
GLColorBase<ScalarParam,numComponentsParam>::copy(
	GLsizei sNumComponents,
	const SourceScalarParam* sRgba)
	{
	for(GLsizei i=0;i<sNumComponents;++i)
		rgba[i]=glConvertScalar<Scalar>(sRgba[i]);
	}

/******************************************
Force instantiation of all GLColor classes:
******************************************/

template class GLColorBase<GLbyte,3>;
template void GLColorBase<GLbyte,3>::copy<GLubyte>(GLsizei,const GLubyte*);
template void GLColorBase<GLbyte,3>::copy<GLshort>(GLsizei,const GLshort*);
template void GLColorBase<GLbyte,3>::copy<GLushort>(GLsizei,const GLushort*);
template void GLColorBase<GLbyte,3>::copy<GLint>(GLsizei,const GLint*);
template void GLColorBase<GLbyte,3>::copy<GLuint>(GLsizei,const GLuint*);
template void GLColorBase<GLbyte,3>::copy<GLfloat>(GLsizei,const GLfloat*);
template void GLColorBase<GLbyte,3>::copy<GLdouble>(GLsizei,const GLdouble*);

template class GLColorBase<GLubyte,3>;
template void GLColorBase<GLubyte,3>::copy<GLbyte>(GLsizei,const GLbyte*);
template void GLColorBase<GLubyte,3>::copy<GLshort>(GLsizei,const GLshort*);
template void GLColorBase<GLubyte,3>::copy<GLushort>(GLsizei,const GLushort*);
template void GLColorBase<GLubyte,3>::copy<GLint>(GLsizei,const GLint*);
template void GLColorBase<GLubyte,3>::copy<GLuint>(GLsizei,const GLuint*);
template void GLColorBase<GLubyte,3>::copy<GLfloat>(GLsizei,const GLfloat*);
template void GLColorBase<GLubyte,3>::copy<GLdouble>(GLsizei,const GLdouble*);

template class GLColorBase<GLshort,3>;
template void GLColorBase<GLshort,3>::copy<GLbyte>(GLsizei,const GLbyte*);
template void GLColorBase<GLshort,3>::copy<GLubyte>(GLsizei,const GLubyte*);
template void GLColorBase<GLshort,3>::copy<GLushort>(GLsizei,const GLushort*);
template void GLColorBase<GLshort,3>::copy<GLint>(GLsizei,const GLint*);
template void GLColorBase<GLshort,3>::copy<GLuint>(GLsizei,const GLuint*);
template void GLColorBase<GLshort,3>::copy<GLfloat>(GLsizei,const GLfloat*);
template void GLColorBase<GLshort,3>::copy<GLdouble>(GLsizei,const GLdouble*);

template class GLColorBase<GLushort,3>;
template void GLColorBase<GLushort,3>::copy<GLbyte>(GLsizei,const GLbyte*);
template void GLColorBase<GLushort,3>::copy<GLubyte>(GLsizei,const GLubyte*);
template void GLColorBase<GLushort,3>::copy<GLshort>(GLsizei,const GLshort*);
template void GLColorBase<GLushort,3>::copy<GLint>(GLsizei,const GLint*);
template void GLColorBase<GLushort,3>::copy<GLuint>(GLsizei,const GLuint*);
template void GLColorBase<GLushort,3>::copy<GLfloat>(GLsizei,const GLfloat*);
template void GLColorBase<GLushort,3>::copy<GLdouble>(GLsizei,const GLdouble*);

template class GLColorBase<GLint,3>;
template void GLColorBase<GLint,3>::copy<GLbyte>(GLsizei,const GLbyte*);
template void GLColorBase<GLint,3>::copy<GLubyte>(GLsizei,const GLubyte*);
template void GLColorBase<GLint,3>::copy<GLshort>(GLsizei,const GLshort*);
template void GLColorBase<GLint,3>::copy<GLushort>(GLsizei,const GLushort*);
template void GLColorBase<GLint,3>::copy<GLuint>(GLsizei,const GLuint*);
template void GLColorBase<GLint,3>::copy<GLfloat>(GLsizei,const GLfloat*);
template void GLColorBase<GLint,3>::copy<GLdouble>(GLsizei,const GLdouble*);

template class GLColorBase<GLuint,3>;
template void GLColorBase<GLuint,3>::copy<GLbyte>(GLsizei,const GLbyte*);
template void GLColorBase<GLuint,3>::copy<GLubyte>(GLsizei,const GLubyte*);
template void GLColorBase<GLuint,3>::copy<GLshort>(GLsizei,const GLshort*);
template void GLColorBase<GLuint,3>::copy<GLushort>(GLsizei,const GLushort*);
template void GLColorBase<GLuint,3>::copy<GLint>(GLsizei,const GLint*);
template void GLColorBase<GLuint,3>::copy<GLfloat>(GLsizei,const GLfloat*);
template void GLColorBase<GLuint,3>::copy<GLdouble>(GLsizei,const GLdouble*);

template class GLColorBase<GLfloat,3>;
template void GLColorBase<GLfloat,3>::copy<GLbyte>(GLsizei,const GLbyte*);
template void GLColorBase<GLfloat,3>::copy<GLubyte>(GLsizei,const GLubyte*);
template void GLColorBase<GLfloat,3>::copy<GLshort>(GLsizei,const GLshort*);
template void GLColorBase<GLfloat,3>::copy<GLushort>(GLsizei,const GLushort*);
template void GLColorBase<GLfloat,3>::copy<GLint>(GLsizei,const GLint*);
template void GLColorBase<GLfloat,3>::copy<GLuint>(GLsizei,const GLuint*);
template void GLColorBase<GLfloat,3>::copy<GLdouble>(GLsizei,const GLdouble*);

template class GLColorBase<GLdouble,3>;
template void GLColorBase<GLdouble,3>::copy<GLbyte>(GLsizei,const GLbyte*);
template void GLColorBase<GLdouble,3>::copy<GLubyte>(GLsizei,const GLubyte*);
template void GLColorBase<GLdouble,3>::copy<GLshort>(GLsizei,const GLshort*);
template void GLColorBase<GLdouble,3>::copy<GLushort>(GLsizei,const GLushort*);
template void GLColorBase<GLdouble,3>::copy<GLint>(GLsizei,const GLint*);
template void GLColorBase<GLdouble,3>::copy<GLuint>(GLsizei,const GLuint*);
template void GLColorBase<GLdouble,3>::copy<GLfloat>(GLsizei,const GLfloat*);

template class GLColorBase<GLbyte,4>;
template void GLColorBase<GLbyte,4>::copy<GLubyte>(GLsizei,const GLubyte*);
template void GLColorBase<GLbyte,4>::copy<GLshort>(GLsizei,const GLshort*);
template void GLColorBase<GLbyte,4>::copy<GLushort>(GLsizei,const GLushort*);
template void GLColorBase<GLbyte,4>::copy<GLint>(GLsizei,const GLint*);
template void GLColorBase<GLbyte,4>::copy<GLuint>(GLsizei,const GLuint*);
template void GLColorBase<GLbyte,4>::copy<GLfloat>(GLsizei,const GLfloat*);
template void GLColorBase<GLbyte,4>::copy<GLdouble>(GLsizei,const GLdouble*);

template class GLColorBase<GLubyte,4>;
template void GLColorBase<GLubyte,4>::copy<GLbyte>(GLsizei,const GLbyte*);
template void GLColorBase<GLubyte,4>::copy<GLshort>(GLsizei,const GLshort*);
template void GLColorBase<GLubyte,4>::copy<GLushort>(GLsizei,const GLushort*);
template void GLColorBase<GLubyte,4>::copy<GLint>(GLsizei,const GLint*);
template void GLColorBase<GLubyte,4>::copy<GLuint>(GLsizei,const GLuint*);
template void GLColorBase<GLubyte,4>::copy<GLfloat>(GLsizei,const GLfloat*);
template void GLColorBase<GLubyte,4>::copy<GLdouble>(GLsizei,const GLdouble*);

template class GLColorBase<GLshort,4>;
template void GLColorBase<GLshort,4>::copy<GLbyte>(GLsizei,const GLbyte*);
template void GLColorBase<GLshort,4>::copy<GLubyte>(GLsizei,const GLubyte*);
template void GLColorBase<GLshort,4>::copy<GLushort>(GLsizei,const GLushort*);
template void GLColorBase<GLshort,4>::copy<GLint>(GLsizei,const GLint*);
template void GLColorBase<GLshort,4>::copy<GLuint>(GLsizei,const GLuint*);
template void GLColorBase<GLshort,4>::copy<GLfloat>(GLsizei,const GLfloat*);
template void GLColorBase<GLshort,4>::copy<GLdouble>(GLsizei,const GLdouble*);

template class GLColorBase<GLushort,4>;
template void GLColorBase<GLushort,4>::copy<GLbyte>(GLsizei,const GLbyte*);
template void GLColorBase<GLushort,4>::copy<GLubyte>(GLsizei,const GLubyte*);
template void GLColorBase<GLushort,4>::copy<GLshort>(GLsizei,const GLshort*);
template void GLColorBase<GLushort,4>::copy<GLint>(GLsizei,const GLint*);
template void GLColorBase<GLushort,4>::copy<GLuint>(GLsizei,const GLuint*);
template void GLColorBase<GLushort,4>::copy<GLfloat>(GLsizei,const GLfloat*);
template void GLColorBase<GLushort,4>::copy<GLdouble>(GLsizei,const GLdouble*);

template class GLColorBase<GLint,4>;
template void GLColorBase<GLint,4>::copy<GLbyte>(GLsizei,const GLbyte*);
template void GLColorBase<GLint,4>::copy<GLubyte>(GLsizei,const GLubyte*);
template void GLColorBase<GLint,4>::copy<GLshort>(GLsizei,const GLshort*);
template void GLColorBase<GLint,4>::copy<GLushort>(GLsizei,const GLushort*);
template void GLColorBase<GLint,4>::copy<GLuint>(GLsizei,const GLuint*);
template void GLColorBase<GLint,4>::copy<GLfloat>(GLsizei,const GLfloat*);
template void GLColorBase<GLint,4>::copy<GLdouble>(GLsizei,const GLdouble*);

template class GLColorBase<GLuint,4>;
template void GLColorBase<GLuint,4>::copy<GLbyte>(GLsizei,const GLbyte*);
template void GLColorBase<GLuint,4>::copy<GLubyte>(GLsizei,const GLubyte*);
template void GLColorBase<GLuint,4>::copy<GLshort>(GLsizei,const GLshort*);
template void GLColorBase<GLuint,4>::copy<GLushort>(GLsizei,const GLushort*);
template void GLColorBase<GLuint,4>::copy<GLint>(GLsizei,const GLint*);
template void GLColorBase<GLuint,4>::copy<GLfloat>(GLsizei,const GLfloat*);
template void GLColorBase<GLuint,4>::copy<GLdouble>(GLsizei,const GLdouble*);

template class GLColorBase<GLfloat,4>;
template void GLColorBase<GLfloat,4>::copy<GLbyte>(GLsizei,const GLbyte*);
template void GLColorBase<GLfloat,4>::copy<GLubyte>(GLsizei,const GLubyte*);
template void GLColorBase<GLfloat,4>::copy<GLshort>(GLsizei,const GLshort*);
template void GLColorBase<GLfloat,4>::copy<GLushort>(GLsizei,const GLushort*);
template void GLColorBase<GLfloat,4>::copy<GLint>(GLsizei,const GLint*);
template void GLColorBase<GLfloat,4>::copy<GLuint>(GLsizei,const GLuint*);
template void GLColorBase<GLfloat,4>::copy<GLdouble>(GLsizei,const GLdouble*);

template class GLColorBase<GLdouble,4>;
template void GLColorBase<GLdouble,4>::copy<GLbyte>(GLsizei,const GLbyte*);
template void GLColorBase<GLdouble,4>::copy<GLubyte>(GLsizei,const GLubyte*);
template void GLColorBase<GLdouble,4>::copy<GLshort>(GLsizei,const GLshort*);
template void GLColorBase<GLdouble,4>::copy<GLushort>(GLsizei,const GLushort*);
template void GLColorBase<GLdouble,4>::copy<GLint>(GLsizei,const GLint*);
template void GLColorBase<GLdouble,4>::copy<GLuint>(GLsizei,const GLuint*);
template void GLColorBase<GLdouble,4>::copy<GLfloat>(GLsizei,const GLfloat*);
