/***********************************************************************
GLGetPrimitiveTemplates - Overloaded versions of all OpenGL functions
related to querying current state of primitive specification.
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

#ifndef GLGETPRIMITIVETEMPLATES_INCLUDED
#define GLGETPRIMITIVETEMPLATES_INCLUDED

#include <GL/gl.h>
#include <GL/GLColor.h>
#include <GL/GLVector.h>
#include <GL/GLGetTemplates.h>

/****************************************
Generic versions of glGetCurrentTexCoord:
****************************************/

template <class ScalarParam>
inline void glGetCurrentTexCoord(ScalarParam params[4])
	{
	glGet(GL_CURRENT_TEXTURE_COORDS,params);
	}

template <class ScalarParam>
inline GLVector<ScalarParam,4> glGetCurrentTexCoord(void)
	{
	GLVector<ScalarParam,4> result;
	glGet(GL_CURRENT_TEXTURE_COORDS,result.getXyzw());
	return result;
	}

template <class ScalarParam>
inline void glGetCurrentTexCoord(GLVector<ScalarParam,4>& param)
	{
	glGet(GL_CURRENT_TEXTURE_COORDS,param.getXyzw());
	}

template <class ScalarParam>
inline void glGetCurrentNormal(ScalarParam params[3])
	{
	glGet(GL_CURRENT_NORMAL,params);
	}

template <class ScalarParam>
inline GLVector<ScalarParam,3> glGetCurrentNormal(void)
	{
	GLVector<ScalarParam,3> result;
	glGet(GL_CURRENT_NORMAL,result.getXyzw());
	return result;
	}

template <class ScalarParam>
inline void glGetCurrentNormal(GLVector<ScalarParam,3>& param)
	{
	glGet(GL_CURRENT_NORMAL,param.getXyzw());
	}

template <class ScalarParam>
inline void glGetCurrentColor(ScalarParam params[4])
	{
	glGet(GL_CURRENT_COLOR,params);
	}

template <class ScalarParam>
inline GLColor<ScalarParam,4> glGetCurrentColor(void)
	{
	GLColor<ScalarParam,4> result;
	glGet(GL_CURRENT_COLOR,result.getRgba());
	return result;
	}

template <class ScalarParam>
inline void glGetCurrentColor(GLColor<ScalarParam,4>& param)
	{
	glGet(GL_CURRENT_COLOR,param.getRgba());
	}

template <class ScalarParam>
inline void glGetCurrentSecondaryColor(ScalarParam params[3])
	{
	glGet(GL_CURRENT_SECONDARY_COLOR,params);
	}

template <class ScalarParam>
inline GLColor<ScalarParam,3> glGetCurrentSecondaryColor(void)
	{
	GLColor<ScalarParam,3> result;
	glGet(GL_CURRENT_SECONDARY_COLOR,result.getRgba());
	return result;
	}

template <class ScalarParam>
inline void glGetCurrentSecondaryColor(GLColor<ScalarParam,3>& param)
	{
	glGet(GL_CURRENT_SECONDARY_COLOR,param.getRgba());
	}

template <class ScalarParam>
inline ScalarParam glGetCurrentIndex(void)
	{
	return glGet<ScalarParam>(GL_CURRENT_INDEX);
	}

template <class ScalarParam>
inline void glGetCurrentIndex(ScalarParam& param)
	{
	glGet(GL_CURRENT_INDEX,&param);
	}

#endif
