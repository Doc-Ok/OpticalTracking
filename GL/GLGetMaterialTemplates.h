/***********************************************************************
GLGetMaterialTemplates - Templatized versions of glGetMaterial function.
The 'GLenum face' parameter of all glMaterial functions has been
replaced by a 'GLMaterialEnums::Face face' parameter.
The 'GLenum pname' parameter of all glMaterial functions has been
replaced by a 'GLMaterialEnums::Pname pname' parameter.
The 'GLenum pname' parameter of the original functions has been replaced
by suffixing the function name with the pname value.
Copyright (c) 2003-2005 Oliver Kreylos

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

#ifndef GLGETMATERIALTEMPLATES_INCLUDED
#define GLGETMATERIALTEMPLATES_INCLUDED

#include <GL/gl.h>
#include <GL/GLColor.h>
#include <GL/GLMaterialEnums.h>

/*******************************************************
Dummy generic version of array-based glGetMaterial call:
*******************************************************/

template <GLsizei numComponentsParam,class ScalarParam>
void glGetMaterial(GLMaterialEnums::Face face,GLMaterialEnums::Pname pname,ScalarParam components[numComponentsParam]);

/*******************************************************
Specialized versions of array-based glGetMaterial calls:
*******************************************************/

template <>
inline void glGetMaterial<1,GLint>(GLMaterialEnums::Face face,GLMaterialEnums::Pname pname,GLint components[1])
	{
	glGetMaterialiv(face,pname,components);
	}

template <>
inline void glGetMaterial<1,GLfloat>(GLMaterialEnums::Face face,GLMaterialEnums::Pname pname,GLfloat components[1])
	{
	glGetMaterialfv(face,pname,components);
	}

template <>
inline void glGetMaterial<1,GLdouble>(GLMaterialEnums::Face face,GLMaterialEnums::Pname pname,GLdouble components[1])
	{
	GLfloat tempComponents[1];
	glGetMaterialfv(face,pname,tempComponents);
	for(GLsizei i=0;i<1;++i)
		components[i]=GLdouble(tempComponents[i]);
	}

template <>
inline void glGetMaterial<4,GLint>(GLMaterialEnums::Face face,GLMaterialEnums::Pname pname,GLint components[4])
	{
	glGetMaterialiv(face,pname,components);
	}

template <>
inline void glGetMaterial<4,GLfloat>(GLMaterialEnums::Face face,GLMaterialEnums::Pname pname,GLfloat components[4])
	{
	glGetMaterialfv(face,pname,components);
	}

template <>
inline void glGetMaterial<4,GLdouble>(GLMaterialEnums::Face face,GLMaterialEnums::Pname pname,GLdouble components[4])
	{
	GLfloat tempComponents[4];
	glGetMaterialfv(face,pname,tempComponents);
	for(GLsizei i=0;i<4;++i)
		components[i]=GLdouble(tempComponents[i]);
	}

/**************************************************
Generic version of single-value glGetMaterial call:
**************************************************/

template <class ScalarParam>
inline ScalarParam glGetMaterial(GLMaterialEnums::Face face,GLMaterialEnums::Pname pname)
	{
	ScalarParam result[1];
	glGetMaterial<1>(face,pname,result);
	return result[0];
	}

/******************************************
Generic versions of glGetMaterial... calls:
******************************************/

template <class ScalarParam>
inline void glGetMaterialAmbient(GLMaterialEnums::Face face,ScalarParam params[4])
	{
	glGetMaterial<4>(face,GLMaterialEnums::AMBIENT,params);
	}

template <class ScalarParam>
inline void glGetMaterialAmbient(GLMaterialEnums::Face face,GLColor<ScalarParam,4>& param)
	{
	glGetMaterial<4>(face,GLMaterialEnums::AMBIENT,param.getRgba());
	}

template <class ScalarParam>
inline GLColor<ScalarParam,4> glGetMaterialAmbient(GLMaterialEnums::Face face)
	{
	GLColor<ScalarParam,4> result;
	glGetMaterial<4>(face,GLMaterialEnums::AMBIENT,result.getRgba());
	return result;
	}

template <class ScalarParam>
inline void glGetMaterialDiffuse(GLMaterialEnums::Face face,ScalarParam params[4])
	{
	glGetMaterial<4>(face,GLMaterialEnums::DIFFUSE,params);
	}

template <class ScalarParam>
inline void glGetMaterialDiffuse(GLMaterialEnums::Face face,GLColor<ScalarParam,4>& param)
	{
	glGetMaterial<4>(face,GLMaterialEnums::DIFFUSE,param.getRgba());
	}

template <class ScalarParam>
inline GLColor<ScalarParam,4> glGetMaterialDiffuse(GLMaterialEnums::Face face)
	{
	GLColor<ScalarParam,4> result;
	glGetMaterial<4>(face,GLMaterialEnums::DIFFUSE,result.getRgba());
	return result;
	}

template <class ScalarParam>
inline void glGetMaterialSpecular(GLMaterialEnums::Face face,ScalarParam params[4])
	{
	glGetMaterial<4>(face,GLMaterialEnums::SPECULAR,params);
	}

template <class ScalarParam>
inline void glGetMaterialSpecular(GLMaterialEnums::Face face,GLColor<ScalarParam,4>& param)
	{
	glGetMaterial<4>(face,GLMaterialEnums::SPECULAR,param.getRgba());
	}

template <class ScalarParam>
inline GLColor<ScalarParam,4> glGetMaterialSpecular(GLMaterialEnums::Face face)
	{
	GLColor<ScalarParam,4> result;
	glGetMaterial<4>(face,GLMaterialEnums::SPECULAR,result.getRgba());
	return result;
	}

template <class ScalarParam>
inline ScalarParam glGetMaterialShininess(GLMaterialEnums::Face face)
	{
	return glGetMaterial<ScalarParam>(face,GLMaterialEnums::SHININESS);
	}

template <class ScalarParam>
inline void glGetMaterialShininess(GLMaterialEnums::Face face,ScalarParam& param)
	{
	glGetMaterial<1>(face,GLMaterialEnums::SHININESS,&param);
	}

template <class ScalarParam>
inline void glGetMaterialEmission(GLMaterialEnums::Face face,ScalarParam params[4])
	{
	glGetMaterial<4>(face,GLMaterialEnums::EMISSION,params);
	}

template <class ScalarParam>
inline void glGetMaterialEmission(GLMaterialEnums::Face face,GLColor<ScalarParam,4>& param)
	{
	glGetMaterial<4>(face,GLMaterialEnums::EMISSION,param.getRgba());
	}

template <class ScalarParam>
inline GLColor<ScalarParam,4> glGetMaterialEmission(GLMaterialEnums::Face face)
	{
	GLColor<ScalarParam,4> result;
	glGetMaterial<4>(face,GLMaterialEnums::EMISSION,result.getRgba());
	return result;
	}

template <class ScalarParam>
inline void glGetMaterialColorIndexes(GLMaterialEnums::Face face,ScalarParam params[3])
	{
	glGetMaterial<3>(face,GLMaterialEnums::COLOR_INDEXES,params);
	}

#endif
