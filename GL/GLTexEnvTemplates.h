/***********************************************************************
GLTexEnvTemplates - Templatized versions of glTexEnv function.
The 'GLenum target' parameter of all glTexEnv functions has been
replaced by a 'GLTexEnvEnums::Target target' parameter.
The 'GLenum pname' parameter of all glTexEnv functions has been replaced
by a 'GLTexEnvEnums::Pname pname' parameter.
The 'GLenum pname' parameter of the original functions has been replaced
by suffixing the function name with the pname value.
The 'GLenum mode' value has been replaced by a 'GLTexEnvEnums::Mode'
value.
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

#ifndef GLTEXENVTEMPLATES_INCLUDED
#define GLTEXENVTEMPLATES_INCLUDED

#include <GL/gl.h>
#include <GL/GLColor.h>
#include <GL/GLTexEnvEnums.h>

/*************************************************
Overloaded versions of single-value glTexEnv call:
*************************************************/

inline void glTexEnv(GLTexEnvEnums::Target target,GLTexEnvEnums::Pname pname,GLint param)
	{
	glTexEnvi(target,pname,param);
	}

inline void glTexEnv(GLTexEnvEnums::Target target,GLTexEnvEnums::Pname pname,GLfloat param)
	{
	glTexEnvf(target,pname,param);
	}

inline void glTexEnv(GLTexEnvEnums::Target target,GLTexEnvEnums::Pname pname,GLdouble param)
	{
	glTexEnvf(target,pname,GLfloat(param));
	}

/**************************************************
Dummy generic version of array-based glTexEnv call:
**************************************************/

template <GLsizei numComponentsParam,class ScalarParam>
void glTexEnv(GLTexEnvEnums::Target target,GLTexEnvEnums::Pname pname,const ScalarParam components[numComponentsParam]);

/**************************************************
Specialized versions of array-based glTexEnv calls:
**************************************************/

template <>
inline void glTexEnv<1,GLint>(GLTexEnvEnums::Target target,GLTexEnvEnums::Pname pname,const GLint params[1])
	{
	glTexEnviv(target,pname,params);
	}

template <>
inline void glTexEnv<1,GLfloat>(GLTexEnvEnums::Target target,GLTexEnvEnums::Pname pname,const GLfloat params[1])
	{
	glTexEnvfv(target,pname,params);
	}

template <>
inline void glTexEnv<1,GLdouble>(GLTexEnvEnums::Target target,GLTexEnvEnums::Pname pname,const GLdouble params[1])
	{
	GLfloat tempParams[1];
	for(GLsizei i=0;i<1;++i)
		tempParams[i]=GLfloat(params[i]);
	glTexEnvfv(target,pname,tempParams);
	}

template <>
inline void glTexEnv<4,GLint>(GLTexEnvEnums::Target target,GLTexEnvEnums::Pname pname,const GLint params[4])
	{
	glTexEnviv(target,pname,params);
	}

template <>
inline void glTexEnv<4,GLfloat>(GLTexEnvEnums::Target target,GLTexEnvEnums::Pname pname,const GLfloat params[4])
	{
	glTexEnvfv(target,pname,params);
	}

template <>
inline void glTexEnv<4,GLdouble>(GLTexEnvEnums::Target target,GLTexEnvEnums::Pname pname,const GLdouble params[4])
	{
	GLfloat tempParams[4];
	for(GLsizei i=0;i<4;++i)
		tempParams[i]=GLfloat(params[i]);
	glTexEnvfv(target,pname,tempParams);
	}

/*************************************
Generic versions of glTexEnv... calls:
*************************************/

inline void glTexEnvMode(GLTexEnvEnums::Target target,GLTexEnvEnums::Mode mode)
	{
	glTexEnv(target,GLTexEnvEnums::MODE,GLint(mode));
	}

template <class ScalarParam>
void glTexEnvColor(GLTexEnvEnums::Target target,const ScalarParam params[4])
	{
	glTexEnv<4>(target,GLTexEnvEnums::COLOR,params);
	}

template <class ScalarParam>
void glTexEnvColor(GLTexEnvEnums::Target target,const GLColor<ScalarParam,4>& param)
	{
	glTexEnv<4>(target,GLTexEnvEnums::COLOR,param.getRgba());
	}

#endif
