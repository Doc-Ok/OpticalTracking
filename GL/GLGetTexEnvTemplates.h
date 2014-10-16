/***********************************************************************
GLGetTexEnvTemplates - Templatized versions of glGetTexEnv function.
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

#ifndef GLGETTEXENVTEMPLATES_INCLUDED
#define GLGETTEXENVTEMPLATES_INCLUDED

#include <GL/gl.h>
#include <GL/GLColor.h>
#include <GL/GLTexEnvEnums.h>

/*****************************************************
Dummy generic version of array-based glGetTexEnv call:
*****************************************************/

template <GLsizei numComponentsParam,class ScalarParam>
void glGetTexEnv(GLTexEnvEnums::Target target,GLTexEnvEnums::Pname pname,ScalarParam components[numComponentsParam]);

/*****************************************************
Specialized versions of array-based glGetTexEnv calls:
*****************************************************/

template <>
inline void glGetTexEnv<1,GLint>(GLTexEnvEnums::Target target,GLTexEnvEnums::Pname pname,GLint components[1])
	{
	glGetTexEnviv(target,pname,components);
	}

template <>
inline void glGetTexEnv<1,GLfloat>(GLTexEnvEnums::Target target,GLTexEnvEnums::Pname pname,GLfloat components[1])
	{
	glGetTexEnvfv(target,pname,components);
	}

template <>
inline void glGetTexEnv<1,GLdouble>(GLTexEnvEnums::Target target,GLTexEnvEnums::Pname pname,GLdouble components[1])
	{
	GLfloat tempComponents[1];
	glGetTexEnvfv(target,pname,tempComponents);
	for(GLsizei i=0;i<1;++i)
		components[i]=GLdouble(tempComponents[i]);
	}

template <>
inline void glGetTexEnv<4,GLint>(GLTexEnvEnums::Target target,GLTexEnvEnums::Pname pname,GLint components[4])
	{
	glGetTexEnviv(target,pname,components);
	}

template <>
inline void glGetTexEnv<4,GLfloat>(GLTexEnvEnums::Target target,GLTexEnvEnums::Pname pname,GLfloat components[4])
	{
	glGetTexEnvfv(target,pname,components);
	}

template <>
inline void glGetTexEnv<4,GLdouble>(GLTexEnvEnums::Target target,GLTexEnvEnums::Pname pname,GLdouble components[4])
	{
	GLfloat tempComponents[4];
	glGetTexEnvfv(target,pname,tempComponents);
	for(GLsizei i=0;i<4;++i)
		components[i]=GLdouble(tempComponents[i]);
	}

/************************************************
Generic version of single-value glGetTexEnv call:
************************************************/

template <class ScalarParam>
inline ScalarParam glGetTexEnv(GLTexEnvEnums::Target target,GLTexEnvEnums::Pname pname)
	{
	ScalarParam result[1];
	glGetTexEnv<1>(target,pname,result);
	return result[0];
	}

/****************************************
Generic versions of glGetTexEnv... calls:
****************************************/

inline GLTexEnvEnums::Mode glGetTexEnvMode(GLTexEnvEnums::Target target)
	{
	return GLTexEnvEnums::Mode(glGetTexEnv<GLint>(target,GLTexEnvEnums::MODE));
	}

inline void glGetTexEnvMode(GLTexEnvEnums::Target target,GLTexEnvEnums::Mode& param)
	{
	param=GLTexEnvEnums::Mode(glGetTexEnv<GLint>(target,GLTexEnvEnums::MODE));
	}

template <class ScalarParam>
inline void glGetTexEnvColor(GLTexEnvEnums::Target target,ScalarParam params[4])
	{
	glGetTexEnv<4>(target,GLTexEnvEnums::COLOR,params);
	}

template <class ScalarParam>
inline GLColor<ScalarParam,4> glGetTexEnvColor(GLTexEnvEnums::Target target)
	{
	GLColor<ScalarParam,4> result;
	glGetTexEnv<4>(target,GLTexEnvEnums::COLOR,result.getRgba());
	return result;
	}

template <class ScalarParam>
inline void glGetTexEnvColor(GLTexEnvEnums::Target target,GLColor<ScalarParam,4>& param)
	{
	glGetTexEnv<4>(target,GLTexEnvEnums::COLOR,param.getRgba());
	}

#endif
