/***********************************************************************
GLFrustum - Class describing an OpenGL view frustum in model coordinates
to perform software-based frustum culling and LOD calculation.
Copyright (c) 2007-2013 Oliver Kreylos

This file is part of the OpenGL Wrapper Library for the Templatized
Geometry Library (GLGeometry).

The OpenGL Wrapper Library for the Templatized Geometry Library is free
software; you can redistribute it and/or modify it under the terms of
the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any
later version.

The OpenGL Wrapper Library for the Templatized Geometry Library is
distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License along
with the OpenGL Wrapper Library for the Templatized Geometry Library; if
not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite
330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <GL/GLFrustum.icpp>

/*****************************************************
Force instantiation of all standard GLFrustum classes:
*****************************************************/

template class GLFrustum<GLfloat>;
template class GLFrustum<GLdouble>;
