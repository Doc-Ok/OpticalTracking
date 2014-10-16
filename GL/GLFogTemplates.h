/***********************************************************************
GLFogTemplates - Templatized versions of glFog function.
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

#ifndef GLFOGTEMPLATES_INCLUDED
#define GLFOGTEMPLATES_INCLUDED

#include <GL/gl.h>
#include <GL/GLColor.h>
#include <GL/GLFogEnums.h>

/**********************************************
Overloaded versions of single-value glFog call:
**********************************************/

inline void glFog(GLFogEnums::Pname pname,GLint param)
	{
	glFogi(pname,param);
	}

inline void glFog(GLFogEnums::Pname pname,GLfloat param)
	{
	glFogf(pname,param);
	}

inline void glFog(GLFogEnums::Pname pname,GLdouble param)
	{
	glFogf(pname,GLfloat(param));
	}

/***********************************************
Dummy generic version of array-based glFog call:
***********************************************/

template <GLsizei numComponentsParam,class ScalarParam>
void glFog(GLFogEnums::Pname pname,const ScalarParam components[numComponentsParam]);

/***********************************************
Specialized versions of array-based glFog calls:
***********************************************/

template <>
inline void glFog<1,GLint>(GLFogEnums::Pname pname,const GLint params[1])
	{
	glFogiv(pname,params);
	}

template <>
inline void glFog<1,GLfloat>(GLFogEnums::Pname pname,const GLfloat params[1])
	{
	glFogfv(pname,params);
	}

template <>
inline void glFog<1,GLdouble>(GLFogEnums::Pname pname,const GLdouble params[1])
	{
	GLfloat tempParams[1];
	for(GLsizei i=0;i<1;++i)
		tempParams[i]=GLfloat(params[i]);
	glFogfv(pname,tempParams);
	}

template <>
inline void glFog<4,GLint>(GLFogEnums::Pname pname,const GLint params[4])
	{
	glFogiv(pname,params);
	}

template <>
inline void glFog<4,GLfloat>(GLFogEnums::Pname pname,const GLfloat params[4])
	{
	glFogfv(pname,params);
	}

template <>
inline void glFog<4,GLdouble>(GLFogEnums::Pname pname,const GLdouble params[4])
	{
	GLfloat tempParams[4];
	for(GLsizei i=0;i<4;++i)
		tempParams[i]=GLfloat(params[i]);
	glFogfv(pname,tempParams);
	}

/**********************************
Generic versions of glFog... calls:
**********************************/

void glFogMode(GLFogEnums::Mode mode)
	{
	glFog(GLFogEnums::MODE,GLint(mode));
	}

template <class ScalarParam>
inline void glFogDensity(ScalarParam param)
	{
	glFog(GLFogEnums::DENSITY,param);
	}

template <class ScalarParam>
inline void glFogStart(ScalarParam param)
	{
	glFog(GLFogEnums::START,param);
	}

template <class ScalarParam>
inline void glFogEnd(ScalarParam param)
	{
	glFog(GLFogEnums::END,param);
	}

template <class ScalarParam>
inline void glFogIndex(ScalarParam param)
	{
	glFog(GLFogEnums::INDEX,param);
	}

template <class ScalarParam>
inline void glFogColor(const ScalarParam params[4])
	{
	glFog<4>(GLFogEnums::COLOR,params);
	}

template <class ScalarParam>
inline void glFogColor(const GLColor<ScalarParam,4>& color)
	{
	glFog<4>(GLFogEnums::COLOR,color.getRgba());
	}

#endif
