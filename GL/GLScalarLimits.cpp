/***********************************************************************
GLScalarLimits - Helper class to store limit values of integral scalar
data types used in OpenGL to enable automatic type conversion for range-
limited scalar values.
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

#include <GL/GLScalarLimits.h>

/***************************************
Static elements of class GLScalarLimits:
***************************************/

const GLfloat GLScalarLimits<GLfloat>::min=0.0f;
const GLfloat GLScalarLimits<GLfloat>::max=1.0f;
const GLdouble GLScalarLimits<GLdouble>::min=0.0;
const GLdouble GLScalarLimits<GLdouble>::max=1.0;
