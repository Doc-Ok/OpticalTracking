/***********************************************************************
GLGeometryWrappers - Wrapper functions to use templatized geometry
library objects as parameters to OpenGL functions.
Copyright (c) 2002-2005 Oliver Kreylos

This file is part of the OpenGL Wrapper Library for the Templatized
Geometry Library (GLGeometry).

The OpenGL Wrapper Library for the Templatized Geometry Library is free
software; you can redistribute it and/or modify it under the terms of
the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any
later version.

The OpenGL Wrapper Library for the Templatized Geometry Library is
distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License along
with the OpenGL Wrapper Library for the Templatized Geometry Library; if
not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite
330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef GLGEOMETRYWRAPPERS_INCLUDED
#define GLGEOMETRYWRAPPERS_INCLUDED

#include <Math/Math.h>
#include <Geometry/ComponentArray.h>
#include <Geometry/Vector.h>
#include <Geometry/Point.h>
#include <Geometry/HVector.h>
#include <Geometry/Rotation.h>
#include <GL/gl.h>
#include <GL/GLVector.h>
#include <GL/GLTexCoordTemplates.h>
#include <GL/GLNormalTemplates.h>
#include <GL/GLVertexTemplates.h>
#include <GL/GLVertexArrayTemplates.h>
#include <GL/GLLightTemplates.h>
#include <GL/GLMatrixTemplates.h>

/*********************************
Overloaded versions of glTexCoord:
*********************************/

template <class ScalarType>
inline void glTexCoord(const Geometry::ComponentArray<ScalarType,1>& tc)
	{
	glTexCoord<1>(tc.getComponents());
	}

template <class ScalarType>
inline void glTexCoord(const Geometry::ComponentArray<ScalarType,2>& tc)
	{
	glTexCoord<2>(tc.getComponents());
	}

template <class ScalarType>
inline void glTexCoord(const Geometry::ComponentArray<ScalarType,3>& tc)
	{
	glTexCoord<3>(tc.getComponents());
	}

template <class ScalarType>
inline void glTexCoord(const Geometry::ComponentArray<ScalarType,4>& tc)
	{
	glTexCoord<4>(tc.getComponents());
	}

/****************************************
Overloaded versions of glTexCoordPointer:
****************************************/

template <class ScalarType,int numComponents>
inline void glTexCoordPointer(GLsizei stride,const Geometry::ComponentArray<ScalarType,numComponents>* pointer)
	{
	glTexCoordPointer(numComponents,stride,pointer[0].getComponents());
	}

/*******************************
Overloaded versions of glNormal:
*******************************/

template <class ScalarType>
inline void glNormal(const Geometry::Vector<ScalarType,3>& n)
	{
	glNormal<3>(n.getComponents());
	}

/**************************************
Overloaded versions of glNormalPointer:
**************************************/

template <class ScalarType>
inline void glNormalPointer(GLsizei stride,const Geometry::Vector<ScalarType,3>* pointer)
	{
	glNormalPointer(stride,pointer[0].getComponents());
	}

/*******************************
Overloaded versions of glVertex:
*******************************/

template <class ScalarType>
inline void glVertex(const Geometry::Point<ScalarType,2>& v)
	{
	glVertex<2>(v.getComponents());
	}

template <class ScalarType>
inline void glVertex(const Geometry::Point<ScalarType,3>& v)
	{
	glVertex<3>(v.getComponents());
	}

template <class ScalarType>
inline void glVertex(const Geometry::HVector<ScalarType,3>& v)
	{
	glVertex<4>(v.getComponents());
	}

/**************************************
Overloaded versions of glVertexPointer:
**************************************/

template <class ScalarType>
inline void glVertexPointer(GLsizei stride,const Geometry::Point<ScalarType,2>* pointer)
	{
	glVertexPointer(2,stride,pointer[0].getComponents());
	}

template <class ScalarType>
inline void glVertexPointer(GLsizei stride,const Geometry::Point<ScalarType,3>* pointer)
	{
	glVertexPointer(3,stride,pointer[0].getComponents());
	}

template <class ScalarType>
inline void glVertexPointer(GLsizei stride,const Geometry::HVector<ScalarType,3>* pointer)
	{
	glVertexPointer(4,stride,pointer[0].getComponents());
	}

/************************************
Overloaded versions of glLight calls:
************************************/

template <class ScalarParam>
inline void glLightPosition(GLsizei lightIndex,const Geometry::Point<ScalarParam,3>& p)
	{
	glLightPosition(lightIndex,Geometry::HVector<ScalarParam,3>(p).getComponents());
	}

template <class ScalarParam>
inline void glLightPosition(GLsizei lightIndex,const Geometry::Vector<ScalarParam,3>& v)
	{
	glLightPosition(lightIndex,Geometry::HVector<ScalarParam,3>(v).getComponents());
	}

template <class ScalarParam>
inline void glLightPosition(GLsizei lightIndex,const Geometry::HVector<ScalarParam,3>& v)
	{
	glLightPosition(lightIndex,v.getComponents());
	}

template <class ScalarParam>
inline void glLightSpotDirection(GLsizei lightIndex,const Geometry::Vector<ScalarParam,3>& v)
	{
	glLightSpotDirection(lightIndex,v.getComponents());
	}

/***********************************
Overloaded versions of matrix calls:
***********************************/

template <class ScalarParam>
inline void glTranslate(const Geometry::Vector<ScalarParam,2>& t)
	{
	glTranslate(t[0],t[1],ScalarParam(0));
	}

template <class ScalarParam>
inline void glTranslate(const Geometry::Vector<ScalarParam,3>& t)
	{
	glTranslate(t.getComponents());
	}

template <class ScalarParam>
inline void glRotate(ScalarParam angle)
	{
	glRotate(angle,ScalarParam(0),ScalarParam(0),ScalarParam(1));
	}

template <class ScalarParam>
inline void glRotate(ScalarParam angle,const Geometry::Vector<ScalarParam,3>& axis)
	{
	glRotate(angle,axis.getComponents());
	}

template <class ScalarParam>
inline void glRotate(const Geometry::Rotation<ScalarParam,2>& r)
	{
	glRotate(Math::deg(r.getAngle()),ScalarParam(0),ScalarParam(0),ScalarParam(1));
	}

template <class ScalarParam>
inline void glRotate(const Geometry::Rotation<ScalarParam,3>& r)
	{
	glRotate(Math::deg(r.getAngle()),r.getAxis().getComponents());
	}

template <class ScalarParam>
inline void glScale(const Geometry::ComponentArray<ScalarParam,2>& s)
	{
	glScale(s[0],s[1],ScalarParam(1));
	}

template <class ScalarParam>
inline void glScale(const Geometry::ComponentArray<ScalarParam,3>& s)
	{
	glScale(s.getComponents());
	}

#endif
