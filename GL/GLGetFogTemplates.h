/***********************************************************************
GLGetFogTemplates - Templatized versions of glGetFog function.
The 'GLenum pname' parameter of all glFog functions has been replaced by
a 'GLFogEnums::Pname pname' parameter.
The 'GLenum pname' parameter of the original functions has been replaced
by suffixing the function name with the pname value.
The 'GLenum mode' value has been replaced by a 'GLFogEnums::Mode' value.
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

#ifndef GLGETFOGTEMPLATES_INCLUDED
#define GLGETFOGTEMPLATES_INCLUDED

#include <GL/gl.h>
#include <GL/GLColor.h>
#include <GL/GLGetTemplates.h>
#include <GL/GLFogEnums.h>

/*************************************
Generic versions of glGetFog... calls:
*************************************/

inline GLFogEnums::Mode glGetFogMode(void)
	{
	return GLFogEnums::Mode(glGet<GLint>(GLFogEnums::MODE));
	}

inline void glGetFogMode(GLFogEnums::Mode& param)
	{
	param=GLFogEnums::Mode(glGet<GLint>(GLFogEnums::MODE));
	}

template <class ScalarParam>
inline ScalarParam glGetFogDensity(void)
	{
	return glGet<ScalarParam>(GLFogEnums::DENSITY);
	}

template <class ScalarParam>
inline void glGetFogDensity(ScalarParam& param)
	{
	glGet(GLFogEnums::DENSITY,&param);
	}

template <class ScalarParam>
inline ScalarParam glGetFogStart(void)
	{
	return glGet<ScalarParam>(GLFogEnums::START);
	}

template <class ScalarParam>
inline void glGetFogStart(ScalarParam& param)
	{
	glGet(GLFogEnums::START,&param);
	}

template <class ScalarParam>
inline ScalarParam glGetFogEnd(void)
	{
	return glGet<ScalarParam>(GLFogEnums::END);
	}

template <class ScalarParam>
inline void glGetFogEnd(ScalarParam& param)
	{
	glGet(GLFogEnums::END,&param);
	}

template <class ScalarParam>
inline void glGetFogColor(ScalarParam params[4])
	{
	glGet(GLFogEnums::COLOR,params);
	}

template <class ScalarParam>
inline GLColor<ScalarParam,4> glGetFogColor(void)
	{
	GLColor<ScalarParam,4> result;
	glGet(GLFogEnums::COLOR,result.getRgba());
	return result;
	}

template <class ScalarParam>
inline void glGetFogColor(GLColor<ScalarParam,4>& param)
	{
	glGet(GLFogEnums::COLOR,param.getRgba());
	}

#endif
