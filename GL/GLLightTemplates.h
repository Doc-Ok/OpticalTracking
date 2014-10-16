/***********************************************************************
GLLightTemplates - Templatized versions of glLight function.
The 'GLenum light' parameter of all glLight functions has been
replaced by a 'GLsizei lightIndex' parameter in the range from 0 to the
number of light sources-1.
The 'GLenum pname' parameter of all glLight functions has been replaced
by a 'GLLightEnums::Pname pname' parameter.
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

#ifndef GLLIGHTTEMPLATES_INCLUDED
#define GLLIGHTTEMPLATES_INCLUDED

#include <GL/gl.h>
#include <GL/GLColor.h>
#include <GL/GLVector.h>
#include <GL/GLLightEnums.h>

/************************************************
Overloaded versions of single-value glLight call:
************************************************/

inline void glLight(GLsizei lightIndex,GLLightEnums::Pname pname,GLint param)
	{
	glLighti(GL_LIGHT0+lightIndex,pname,param);
	}

inline void glLight(GLsizei lightIndex,GLLightEnums::Pname pname,GLfloat param)
	{
	glLightf(GL_LIGHT0+lightIndex,pname,param);
	}

inline void glLight(GLsizei lightIndex,GLLightEnums::Pname pname,GLdouble param)
	{
	glLightf(GL_LIGHT0+lightIndex,pname,GLfloat(param));
	}

/*************************************************
Dummy generic version of array-based glLight call:
*************************************************/

template <GLsizei numComponentsParam,class ScalarParam>
void glLight(GLsizei lightIndex,GLLightEnums::Pname pname,const ScalarParam components[numComponentsParam]);

/*************************************************
Specialized versions of array-based glLight calls:
*************************************************/

template <>
inline void glLight<1,GLint>(GLsizei lightIndex,GLLightEnums::Pname pname,const GLint params[1])
	{
	glLightiv(GL_LIGHT0+lightIndex,pname,params);
	}

template <>
inline void glLight<1,GLfloat>(GLsizei lightIndex,GLLightEnums::Pname pname,const GLfloat params[1])
	{
	glLightfv(GL_LIGHT0+lightIndex,pname,params);
	}

template <>
inline void glLight<1,GLdouble>(GLsizei lightIndex,GLLightEnums::Pname pname,const GLdouble params[1])
	{
	GLfloat tempParams[1];
	for(GLsizei i=0;i<1;++i)
		tempParams[i]=GLfloat(params[i]);
	glLightfv(GL_LIGHT0+lightIndex,pname,tempParams);
	}

template <>
inline void glLight<3,GLint>(GLsizei lightIndex,GLLightEnums::Pname pname,const GLint params[3])
	{
	glLightiv(GL_LIGHT0+lightIndex,pname,params);
	}

template <>
inline void glLight<3,GLfloat>(GLsizei lightIndex,GLLightEnums::Pname pname,const GLfloat params[3])
	{
	glLightfv(GL_LIGHT0+lightIndex,pname,params);
	}

template <>
inline void glLight<3,GLdouble>(GLsizei lightIndex,GLLightEnums::Pname pname,const GLdouble params[3])
	{
	GLfloat tempParams[3];
	for(GLsizei i=0;i<3;++i)
		tempParams[i]=GLfloat(params[i]);
	glLightfv(GL_LIGHT0+lightIndex,pname,tempParams);
	}

template <>
inline void glLight<4,GLint>(GLsizei lightIndex,GLLightEnums::Pname pname,const GLint params[4])
	{
	glLightiv(GL_LIGHT0+lightIndex,pname,params);
	}

template <>
inline void glLight<4,GLfloat>(GLsizei lightIndex,GLLightEnums::Pname pname,const GLfloat params[4])
	{
	glLightfv(GL_LIGHT0+lightIndex,pname,params);
	}

template <>
inline void glLight<4,GLdouble>(GLsizei lightIndex,GLLightEnums::Pname pname,const GLdouble params[4])
	{
	GLfloat tempParams[4];
	for(GLsizei i=0;i<4;++i)
		tempParams[i]=GLfloat(params[i]);
	glLightfv(GL_LIGHT0+lightIndex,pname,tempParams);
	}

/************************************
Generic versions of glLight... calls:
************************************/

inline void glEnableLight(GLsizei lightIndex)
	{
	glEnable(GL_LIGHT0+lightIndex);
	}

inline void glDisableLight(GLsizei lightIndex)
	{
	glDisable(GL_LIGHT0+lightIndex);
	}

template <class ScalarParam>
inline void glLightAmbient(GLsizei lightIndex,const ScalarParam params[4])
	{
	glLight<4>(lightIndex,GLLightEnums::AMBIENT,params);
	}

template <class ScalarParam>
inline void glLightAmbient(GLsizei lightIndex,const GLColor<ScalarParam,4>& param)
	{
	glLight<4>(lightIndex,GLLightEnums::AMBIENT,param.getRgba());
	}

template <class ScalarParam>
inline void glLightDiffuse(GLsizei lightIndex,const ScalarParam params[4])
	{
	glLight<4>(lightIndex,GLLightEnums::DIFFUSE,params);
	}

template <class ScalarParam>
inline void glLightDiffuse(GLsizei lightIndex,const GLColor<ScalarParam,4>& param)
	{
	glLight<4>(lightIndex,GLLightEnums::DIFFUSE,param.getRgba());
	}

template <class ScalarParam>
inline void glLightSpecular(GLsizei lightIndex,const ScalarParam params[4])
	{
	glLight<4>(lightIndex,GLLightEnums::SPECULAR,params);
	}

template <class ScalarParam>
inline void glLightSpecular(GLsizei lightIndex,const GLColor<ScalarParam,4>& param)
	{
	glLight<4>(lightIndex,GLLightEnums::SPECULAR,param.getRgba());
	}

template <class ScalarParam>
inline void glLightPosition(GLsizei lightIndex,const ScalarParam params[4])
	{
	glLight<4>(lightIndex,GLLightEnums::POSITION,params);
	}

template <class ScalarParam>
inline void glLightPosition(GLsizei lightIndex,const GLVector<ScalarParam,4>& param)
	{
	glLight<4>(lightIndex,GLLightEnums::POSITION,param.getXyzw());
	}

template <class ScalarParam>
inline void glLightSpotDirection(GLsizei lightIndex,const ScalarParam params[3])
	{
	glLight<3>(lightIndex,GLLightEnums::SPOT_DIRECTION,params);
	}

template <class ScalarParam>
inline void glLightSpotDirection(GLsizei lightIndex,const GLVector<ScalarParam,3>& param)
	{
	glLight<3>(lightIndex,GLLightEnums::SPOT_DIRECTION,param.getXyzw());
	}

template <class ScalarParam>
inline void glLightSpotExponent(GLsizei lightIndex,ScalarParam param)
	{
	glLight(lightIndex,GLLightEnums::SPOT_EXPONENT,param);
	}

template <class ScalarParam>
inline void glLightSpotCutoff(GLsizei lightIndex,ScalarParam param)
	{
	glLight(lightIndex,GLLightEnums::SPOT_CUTOFF,param);
	}

template <class ScalarParam>
inline void glLightConstantAttenuation(GLsizei lightIndex,ScalarParam param)
	{
	glLight(lightIndex,GLLightEnums::CONSTANT_ATTENUATION,param);
	}

template <class ScalarParam>
inline void glLightLinearAttenuation(GLsizei lightIndex,ScalarParam param)
	{
	glLight(lightIndex,GLLightEnums::LINEAR_ATTENUATION,param);
	}

template <class ScalarParam>
inline void glLightQuadraticAttenuation(GLsizei lightIndex,ScalarParam param)
	{
	glLight(lightIndex,GLLightEnums::QUADRATIC_ATTENUATION,param);
	}

#endif
