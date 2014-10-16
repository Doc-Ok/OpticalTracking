/***********************************************************************
GLModels - Helper functions to render simple models using OpenGL.
Copyright (c) 2004-2013 Oliver Kreylos

This file is part of the OpenGL Support Library (GLSupport).

The OpenGL Support Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The OpenGL Support Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the OpenGL Support Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef GLMODELS_INCLUDED
#define GLMODELS_INCLUDED

#include <GL/gl.h>

void glDrawCube(GLfloat size);
void glDrawBox(const GLfloat min[3],const GLfloat max[3]);
void glDrawSphereMercator(GLfloat radius,GLsizei numStrips,GLsizei numQuads);
void glDrawSphereMercatorWithTexture(GLfloat radius,GLsizei numStrips,GLsizei numQuads);
void glDrawSphereIcosahedron(GLfloat radius,GLsizei numStrips);
void glDrawCylinder(GLfloat radius,GLfloat height,GLsizei numStrips);
void glDrawCone(GLfloat radius,GLfloat height,GLsizei numStrips);
void glDrawWireframeCube(GLfloat cubeSize,GLfloat edgeSize,GLfloat vertexSize);
void glDrawArrow(GLfloat shaftRadius,GLfloat tipRadius,GLfloat tipHeight,GLfloat totalHeight,GLsizei numStrips);

#endif
