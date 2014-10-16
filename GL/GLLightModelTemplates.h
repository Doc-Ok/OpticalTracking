/***********************************************************************
GLLightModelTemplates - Templatized versions of glLightModel function.
The 'GLenum pname' parameter of all glLightModel functions has been
replaced by a 'GLLightModelEnums::Pname pname' parameter.
The 'GLenum pname' parameter of the original functions has been replaced
by suffixing the function name with the pname value.
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

#ifndef GLLIGHTMODELTEMPLATES_INCLUDED
#define GLLIGHTMODELTEMPLATES_INCLUDED

#include <GL/gl.h>
#include <GL/GLColor.h>
#include <GL/GLLightModelEnums.h>

/************************************************
Overloaded versions of single-value glLight call:
************************************************/

inline void glLightModel(GLLightModelEnums::Pname pname,GLint param)
	{
	glLightModeli(pname,param);
	}

inline void glLightModel(GLLightModelEnums::Pname pname,GLfloat param)
	{
	glLightModelf(pname,param);
	}

inline void glLightModel(GLLightModelEnums::Pname pname,GLdouble param)
	{
	glLightModelf(pname,GLfloat(param));
	}

/******************************************************
Dummy generic version of array-based glLightModel call:
******************************************************/

template <GLsizei numComponentsParam,class ScalarParam>
void glLightModel(GLLightModelEnums::Pname pname,const ScalarParam components[numComponentsParam]);

/******************************************************
Specialized versions of array-based glLightModel calls:
******************************************************/

template <>
inline void glLightModel<1,GLint>(GLLightModelEnums::Pname pname,const GLint params[1])
	{
	glLightModeliv(pname,params);
	}

template <>
inline void glLightModel<1,GLfloat>(GLLightModelEnums::Pname pname,const GLfloat params[1])
	{
	glLightModelfv(pname,params);
	}

template <>
inline void glLightModel<1,GLdouble>(GLLightModelEnums::Pname pname,const GLdouble params[1])
	{
	GLfloat tempParams[1];
	for(GLsizei i=0;i<1;++i)
		tempParams[i]=GLfloat(params[i]);
	glLightModelfv(pname,tempParams);
	}

template <>
inline void glLightModel<4,GLint>(GLLightModelEnums::Pname pname,const GLint params[4])
	{
	glLightModeliv(pname,params);
	}

template <>
inline void glLightModel<4,GLfloat>(GLLightModelEnums::Pname pname,const GLfloat params[4])
	{
	glLightModelfv(pname,params);
	}

template <>
inline void glLightModel<4,GLdouble>(GLLightModelEnums::Pname pname,const GLdouble params[4])
	{
	GLfloat tempParams[4];
	for(GLsizei i=0;i<4;++i)
		tempParams[i]=GLfloat(params[i]);
	glLightModelfv(pname,tempParams);
	}

/*****************************************
Generic versions of glLightModel... calls:
*****************************************/

template <class ScalarParam>
inline void glLightModelAmbient(const ScalarParam params[4])
	{
	glLightModel<4>(GLLightModelEnums::AMBIENT,params);
	}

template <class ScalarParam>
inline void glLightModelAmbient(const GLColor<ScalarParam,4>& param)
	{
	glLightModel<4>(GLLightModelEnums::AMBIENT,param.getRgba());
	}

inline void glLightModelColorControl(GLLightModelEnums::ColorControl param)
	{
	glLightModel(GLLightModelEnums::COLOR_CONTROL,param);
	}

inline void glLightModelLocalViewer(GLboolean param)
	{
	glLightModel(GLLightModelEnums::LOCAL_VIEWER,param);
	}

inline void glLightModelTwoSide(GLboolean param)
	{
	glLightModel(GLLightModelEnums::TWO_SIDE,param);
	}

#endif
