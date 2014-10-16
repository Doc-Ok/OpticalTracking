/***********************************************************************
GLGetMatrixTemplates - Overloaded versions of glGet function related to
matrix manipulation.
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

#ifndef GLGETMATRIXTEMPLATES_INCLUDED
#define GLGETMATRIXTEMPLATES_INCLUDED

#include <GL/gl.h>
#include <GL/GLGetTemplates.h>
#include <GL/GLMatrixEnums.h>

/***********************************
Generic version of glGetMatrix call:
***********************************/

template <class ScalarParam>
void glGetMatrix(GLMatrixEnums::Pname pname,ScalarParam params[16])
	{
	glGet(pname,params);
	}

/**************************************
Generic versions of glGet... functions:
**************************************/

template <class ScalarParam>
void glGetColorMatrix(ScalarParam params[16])
	{
	glGet(GL_COLOR_MATRIX,params);
	}

template <class ScalarParam>
void glGetModelviewMatrix(ScalarParam params[16])
	{
	glGet(GL_MODELVIEW_MATRIX,params);
	}

template <class ScalarParam>
void glGetProjectionMatrix(ScalarParam params[16])
	{
	glGet(GL_PROJECTION_MATRIX,params);
	}

template <class ScalarParam>
void glGetTextureMatrix(ScalarParam params[16])
	{
	glGet(GL_TEXTURE_MATRIX,params);
	}

#endif
