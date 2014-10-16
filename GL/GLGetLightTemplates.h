/***********************************************************************
GLGetLightTemplates - Templatized versions of glGetLight function.
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

#ifndef GLGETLIGHTTEMPLATES_INCLUDED
#define GLGETLIGHTTEMPLATES_INCLUDED

#include <GL/gl.h>
#include <GL/GLColor.h>
#include <GL/GLVector.h>
#include <GL/GLLightEnums.h>

/****************************************************
Dummy generic version of array-based glGetLight call:
****************************************************/

template <GLsizei numComponentsParam,class ScalarParam>
void glGetLight(GLsizei lightIndex,GLLightEnums::Pname pname,ScalarParam components[numComponentsParam]);

/****************************************************
Specialized versions of array-based glGetLight calls:
****************************************************/

template <>
inline void glGetLight<1,GLint>(GLsizei lightIndex,GLLightEnums::Pname pname,GLint components[1])
	{
	glGetLightiv(GL_LIGHT0+lightIndex,pname,components);
	}

template <>
inline void glGetLight<1,GLfloat>(GLsizei lightIndex,GLLightEnums::Pname pname,GLfloat components[1])
	{
	glGetLightfv(GL_LIGHT0+lightIndex,pname,components);
	}

template <>
inline void glGetLight<1,GLdouble>(GLsizei lightIndex,GLLightEnums::Pname pname,GLdouble components[1])
	{
	GLfloat tempComponents[1];
	glGetLightfv(GL_LIGHT0+lightIndex,pname,tempComponents);
	for(GLsizei i=0;i<1;++i)
		components[i]=GLdouble(tempComponents[i]);
	}

template <>
inline void glGetLight<3,GLint>(GLsizei lightIndex,GLLightEnums::Pname pname,GLint components[3])
	{
	glGetLightiv(GL_LIGHT0+lightIndex,pname,components);
	}

template <>
inline void glGetLight<3,GLfloat>(GLsizei lightIndex,GLLightEnums::Pname pname,GLfloat components[3])
	{
	glGetLightfv(GL_LIGHT0+lightIndex,pname,components);
	}

template <>
inline void glGetLight<3,GLdouble>(GLsizei lightIndex,GLLightEnums::Pname pname,GLdouble components[3])
	{
	GLfloat tempComponents[3];
	glGetLightfv(GL_LIGHT0+lightIndex,pname,tempComponents);
	for(GLsizei i=0;i<3;++i)
		components[i]=GLdouble(tempComponents[i]);
	}

template <>
inline void glGetLight<4,GLint>(GLsizei lightIndex,GLLightEnums::Pname pname,GLint components[4])
	{
	glGetLightiv(GL_LIGHT0+lightIndex,pname,components);
	}

template <>
inline void glGetLight<4,GLfloat>(GLsizei lightIndex,GLLightEnums::Pname pname,GLfloat components[4])
	{
	glGetLightfv(GL_LIGHT0+lightIndex,pname,components);
	}

template <>
inline void glGetLight<4,GLdouble>(GLsizei lightIndex,GLLightEnums::Pname pname,GLdouble components[4])
	{
	GLfloat tempComponents[4];
	glGetLightfv(GL_LIGHT0+lightIndex,pname,tempComponents);
	for(GLsizei i=0;i<4;++i)
		components[i]=GLdouble(tempComponents[i]);
	}

/***********************************************
Generic version of single-value glGetLight call:
***********************************************/

template <class ScalarParam>
inline ScalarParam glGetLight(GLsizei lightIndex,GLLightEnums::Pname pname)
	{
	ScalarParam result[1];
	glGetLight<1>(lightIndex,pname,result);
	return result[0];
	}

/***************************************
Generic versions of glGetLight... calls:
***************************************/

template <class ScalarParam>
inline void glGetLightAmbient(GLsizei lightIndex,ScalarParam params[4])
	{
	glGetLight<4>(lightIndex,GLLightEnums::AMBIENT,params);
	}

template <class ScalarParam>
inline GLColor<ScalarParam,4> glGetLightAmbient(GLsizei lightIndex)
	{
	GLColor<ScalarParam,4> result;
	glGetLight<4>(lightIndex,GLLightEnums::AMBIENT,result.getRgba());
	return result;
	}

template <class ScalarParam>
inline void glGetLightAmbient(GLsizei lightIndex,GLColor<ScalarParam,4>& param)
	{
	glGetLight<4>(lightIndex,GLLightEnums::AMBIENT,param.getRgba());
	}

template <class ScalarParam>
inline void glGetLightDiffuse(GLsizei lightIndex,ScalarParam params[4])
	{
	glGetLight<4>(lightIndex,GLLightEnums::DIFFUSE,params);
	}

template <class ScalarParam>
inline GLColor<ScalarParam,4> glGetLightDiffuse(GLsizei lightIndex)
	{
	GLColor<ScalarParam,4> result;
	glGetLight<4>(lightIndex,GLLightEnums::DIFFUSE,result.getRgba());
	return result;
	}

template <class ScalarParam>
inline void glGetLightDiffuse(GLsizei lightIndex,GLColor<ScalarParam,4>& param)
	{
	glGetLight<4>(lightIndex,GLLightEnums::DIFFUSE,param.getRgba());
	}

template <class ScalarParam>
inline void glGetLightSpecular(GLsizei lightIndex,ScalarParam params[4])
	{
	glGetLight<4>(lightIndex,GLLightEnums::SPECULAR,params);
	}

template <class ScalarParam>
inline GLColor<ScalarParam,4> glGetLightSpecular(GLsizei lightIndex)
	{
	GLColor<ScalarParam,4> result;
	glGetLight<4>(lightIndex,GLLightEnums::SPECULAR,result.getRgba());
	return result;
	}

template <class ScalarParam>
inline void glGetLightSpecular(GLsizei lightIndex,GLColor<ScalarParam,4>& param)
	{
	glGetLight<4>(lightIndex,GLLightEnums::SPECULAR,param.getRgba());
	}

template <class ScalarParam>
inline void glGetLightPosition(GLsizei lightIndex,ScalarParam params[4])
	{
	glGetLight<4>(lightIndex,GLLightEnums::POSITION,params);
	}

template <class ScalarParam>
inline GLVector<ScalarParam,4> glGetLightPosition(GLsizei lightIndex)
	{
	GLVector<ScalarParam,4> result;
	glGetLight<4>(lightIndex,GLLightEnums::POSITION,result.getXyzw());
	return result;
	}

template <class ScalarParam>
inline void glGetLightPosition(GLsizei lightIndex,GLVector<ScalarParam,4>& param)
	{
	glGetLight<4>(lightIndex,GLLightEnums::POSITION,param.getXyzw());
	}

template <class ScalarParam>
inline void glGetLightSpotDirection(GLsizei lightIndex,ScalarParam params[3])
	{
	glGetLight<3>(lightIndex,GLLightEnums::SPOT_DIRECTION,params);
	}

template <class ScalarParam>
inline GLVector<ScalarParam,3> glGetLightSpotDirection(GLsizei lightIndex)
	{
	GLVector<ScalarParam,3> result;
	glGetLight<3>(lightIndex,GLLightEnums::SPOT_DIRECTION,result.getXyzw());
	return result;
	}

template <class ScalarParam>
inline void glGetLightSpotDirection(GLsizei lightIndex,GLVector<ScalarParam,3>& param)
	{
	glGetLight<3>(lightIndex,GLLightEnums::SPOT_DIRECTION,param.getXyzw());
	}

template <class ScalarParam>
inline ScalarParam glGetLightSpotExponent(GLsizei lightIndex)
	{
	return glGetLight<ScalarParam>(lightIndex,GLLightEnums::SPOT_EXPONENT);
	}

template <class ScalarParam>
inline void glGetLightSpotExponent(GLsizei lightIndex,ScalarParam& param)
	{
	glGetLight<1>(lightIndex,GLLightEnums::SPOT_EXPONENT,&param);
	}

template <class ScalarParam>
inline ScalarParam glGetLightSpotCutoff(GLsizei lightIndex)
	{
	return glGetLight<ScalarParam>(lightIndex,GLLightEnums::SPOT_CUTOFF);
	}

template <class ScalarParam>
inline void glGetLightSpotCutoff(GLsizei lightIndex,ScalarParam& param)
	{
	glGetLight<1>(lightIndex,GLLightEnums::SPOT_CUTOFF,&param);
	}

template <class ScalarParam>
inline ScalarParam glGetLightConstantAttenuation(GLsizei lightIndex)
	{
	return glGetLight<ScalarParam>(lightIndex,GLLightEnums::CONSTANT_ATTENUATION);
	}

template <class ScalarParam>
inline void glGetLightConstantAttenuation(GLsizei lightIndex,ScalarParam& param)
	{
	glGetLight<1>(lightIndex,GLLightEnums::CONSTANT_ATTENUATION,&param);
	}

template <class ScalarParam>
inline ScalarParam glGetLightLinearAttenuation(GLsizei lightIndex)
	{
	return glGetLight<ScalarParam>(lightIndex,GLLightEnums::LINEAR_ATTENUATION);
	}

template <class ScalarParam>
inline void glGetLightLinearAttenuation(GLsizei lightIndex,ScalarParam& param)
	{
	glGetLight<1>(lightIndex,GLLightEnums::LINEAR_ATTENUATION,&param);
	}

template <class ScalarParam>
inline ScalarParam glGetLightQuadraticAttenuation(GLsizei lightIndex)
	{
	return glGetLight<ScalarParam>(lightIndex,GLLightEnums::QUADRATIC_ATTENUATION);
	}

template <class ScalarParam>
inline void glGetLightQuadraticAttenuation(GLsizei lightIndex,ScalarParam& param)
	{
	glGetLight<1>(lightIndex,GLLightEnums::QUADRATIC_ATTENUATION,&param);
	}

#endif
