/***********************************************************************
GLMarshallers - Marshaller classes for OpenGL abstraction classes.
Copyright (c) 2010 Oliver Kreylos

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

#include <GL/GLMarshallers.h>

#include <Misc/StandardMarshallers.h>
#include <GL/gl.h>
#include <GL/GLColor.h>
#include <GL/GLMaterial.h>

namespace Misc {

/***************************************
Methods of class Marshaller<GLMaterial>:
***************************************/

size_t Marshaller<GLMaterial>::getSize(const GLMaterial& value)
	{
	return Marshaller<Color>::getSize(value.ambient)+Marshaller<Color>::getSize(value.diffuse)+Marshaller<Color>::getSize(value.specular)+Marshaller<Scalar>::getSize(value.shininess)+Marshaller<Color>::getSize(value.emission);
	}

}
