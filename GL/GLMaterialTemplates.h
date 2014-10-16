/***********************************************************************
GLMaterialTemplates - Templatized versions of glMaterial function.
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

#ifndef GLMATERIALTEMPLATES_INCLUDED
#define GLMATERIALTEMPLATES_INCLUDED

#include <GL/gl.h>
#include <GL/GLColor.h>
#include <GL/GLMaterialEnums.h>

/***************************************************
Overloaded versions of single-value glMaterial call:
***************************************************/

inline void glMaterial(GLMaterialEnums::Face face,GLMaterialEnums::Pname pname,GLint param)
	{
	glMateriali(face,pname,param);
	}

inline void glMaterial(GLMaterialEnums::Face face,GLMaterialEnums::Pname pname,GLfloat param)
	{
	glMaterialf(face,pname,param);
	}

inline void glMaterial(GLMaterialEnums::Face face,GLMaterialEnums::Pname pname,GLdouble param)
	{
	glMaterialf(face,pname,GLfloat(param));
	}

/****************************************************
Dummy generic version of array-based glMaterial call:
****************************************************/

template <GLsizei numComponentsParam,class ScalarParam>
void glMaterial(GLMaterialEnums::Face face,GLMaterialEnums::Pname pname,const ScalarParam components[numComponentsParam]);

/****************************************************
Specialized versions of array-based glMaterial calls:
****************************************************/

template <>
inline void glMaterial<1,GLint>(GLMaterialEnums::Face face,GLMaterialEnums::Pname pname,const GLint params[1])
	{
	glMaterialiv(face,pname,params);
	}

template <>
inline void glMaterial<1,GLfloat>(GLMaterialEnums::Face face,GLMaterialEnums::Pname pname,const GLfloat params[1])
	{
	glMaterialfv(face,pname,params);
	}

template <>
inline void glMaterial<1,GLdouble>(GLMaterialEnums::Face face,GLMaterialEnums::Pname pname,const GLdouble params[1])
	{
	GLfloat tempParams[1];
	for(GLsizei i=0;i<1;++i)
		tempParams[i]=GLfloat(params[i]);
	glMaterialfv(face,pname,tempParams);
	}

template <>
inline void glMaterial<4,GLint>(GLMaterialEnums::Face face,GLMaterialEnums::Pname pname,const GLint params[4])
	{
	glMaterialiv(face,pname,params);
	}

template <>
inline void glMaterial<4,GLfloat>(GLMaterialEnums::Face face,GLMaterialEnums::Pname pname,const GLfloat params[4])
	{
	glMaterialfv(face,pname,params);
	}

template <>
inline void glMaterial<4,GLdouble>(GLMaterialEnums::Face face,GLMaterialEnums::Pname pname,const GLdouble params[4])
	{
	GLfloat tempParams[4];
	for(GLsizei i=0;i<4;++i)
		tempParams[i]=GLfloat(params[i]);
	glMaterialfv(face,pname,tempParams);
	}

/***************************************
Generic versions of glMaterial... calls:
***************************************/

template <class ScalarParam>
inline void glMaterialAmbient(GLMaterialEnums::Face face,const ScalarParam params[4])
	{
	glMaterial<4>(face,GLMaterialEnums::AMBIENT,params);
	}

template <class ScalarParam>
inline void glMaterialAmbient(GLMaterialEnums::Face face,const GLColor<ScalarParam,4>& param)
	{
	glMaterial<4>(face,GLMaterialEnums::AMBIENT,param.getRgba());
	}

template <class ScalarParam>
inline void glMaterialDiffuse(GLMaterialEnums::Face face,const ScalarParam params[4])
	{
	glMaterial<4>(face,GLMaterialEnums::DIFFUSE,params);
	}

template <class ScalarParam>
inline void glMaterialDiffuse(GLMaterialEnums::Face face,const GLColor<ScalarParam,4>& param)
	{
	glMaterial<4>(face,GLMaterialEnums::DIFFUSE,param.getRgba());
	}

template <class ScalarParam>
inline void glMaterialAmbientAndDiffuse(GLMaterialEnums::Face face,const ScalarParam params[4])
	{
	glMaterial<4>(face,GLMaterialEnums::AMBIENT_AND_DIFFUSE,params);
	}

template <class ScalarParam>
inline void glMaterialAmbientAndDiffuse(GLMaterialEnums::Face face,const GLColor<ScalarParam,4>& param)
	{
	glMaterial<4>(face,GLMaterialEnums::AMBIENT_AND_DIFFUSE,param.getRgba());
	}

template <class ScalarParam>
inline void glMaterialSpecular(GLMaterialEnums::Face face,const ScalarParam params[4])
	{
	glMaterial<4>(face,GLMaterialEnums::SPECULAR,params);
	}

template <class ScalarParam>
inline void glMaterialSpecular(GLMaterialEnums::Face face,const GLColor<ScalarParam,4>& param)
	{
	glMaterial<4>(face,GLMaterialEnums::SPECULAR,param.getRgba());
	}

template <class ScalarParam>
inline void glMaterialShininess(GLMaterialEnums::Face face,ScalarParam param)
	{
	glMaterial(face,GLMaterialEnums::SHININESS,param);
	}

template <class ScalarParam>
inline void glMaterialEmission(GLMaterialEnums::Face face,const ScalarParam params[4])
	{
	glMaterial<4>(face,GLMaterialEnums::EMISSION,params);
	}

template <class ScalarParam>
inline void glMaterialEmission(GLMaterialEnums::Face face,const GLColor<ScalarParam,4>& param)
	{
	glMaterial<4>(face,GLMaterialEnums::EMISSION,param.getRgba());
	}

template <class ScalarParam>
inline void glMaterialColorIndexes(GLMaterialEnums::Face face,const ScalarParam params[3])
	{
	glMaterial<3>(face,GLMaterialEnums::COLOR_INDEXES,params);
	}

#endif
