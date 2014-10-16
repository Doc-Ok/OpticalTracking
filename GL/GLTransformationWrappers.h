/***********************************************************************
GLTransformationWrappers - Wrapper functions to use templatized geometry
library transformation objects as parameters to OpenGL matrix functions.
Copyright (c) 2002-2010 Oliver Kreylos

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

#ifndef GLTRANSFORMATIONWRAPPERS_INCLUDED
#define GLTRANSFORMATIONWRAPPERS_INCLUDED

#include <GL/gl.h>
#include <GL/GLMatrixEnums.h>

/* Forward declarations: */
namespace Geometry {
template <class ScalarParam,int dimensionParam>
class TranslationTransformation;
template <class ScalarParam,int dimensionParam>
class RotationTransformation;
template <class ScalarParam,int dimensionParam>
class OrthonormalTransformation;
template <class ScalarParam,int dimensionParam>
class UniformScalingTransformation;
template <class ScalarParam,int dimensionParam>
class OrthogonalTransformation;
template <class ScalarParam,int dimensionParam>
class ScalingTransformation;
template <class ScalarParam,int dimensionParam>
class AffineTransformation;
template <class ScalarParam,int dimensionParam>
class ProjectiveTransformation;
}

/********************************
Transformation wrapper functions:
********************************/

template <class ScalarParam>
void glLoadMatrix(const Geometry::TranslationTransformation<ScalarParam,3>& t);
template <class ScalarParam>
void glMultMatrix(const Geometry::TranslationTransformation<ScalarParam,3>& t);
template <class ScalarParam>
void glLoadMatrix(const Geometry::RotationTransformation<ScalarParam,3>& t);
template <class ScalarParam>
void glMultMatrix(const Geometry::RotationTransformation<ScalarParam,3>& t);
template <class ScalarParam>
void glLoadMatrix(const Geometry::OrthonormalTransformation<ScalarParam,3>& t);
template <class ScalarParam>
void glMultMatrix(const Geometry::OrthonormalTransformation<ScalarParam,3>& t);
template <class ScalarParam>
void glLoadMatrix(const Geometry::UniformScalingTransformation<ScalarParam,3>& t);
template <class ScalarParam>
void glMultMatrix(const Geometry::UniformScalingTransformation<ScalarParam,3>& t);
template <class ScalarParam>
void glLoadMatrix(const Geometry::OrthogonalTransformation<ScalarParam,3>& t);
template <class ScalarParam>
void glMultMatrix(const Geometry::OrthogonalTransformation<ScalarParam,3>& t);
template <class ScalarParam>
void glLoadMatrix(const Geometry::ScalingTransformation<ScalarParam,3>& t);
template <class ScalarParam>
void glMultMatrix(const Geometry::ScalingTransformation<ScalarParam,3>& t);
template <class ScalarParam>
void glLoadMatrix(const Geometry::AffineTransformation<ScalarParam,3>& t);
template <class ScalarParam>
void glMultMatrix(const Geometry::AffineTransformation<ScalarParam,3>& t);
template <class ScalarParam>
void glLoadMatrix(const Geometry::ProjectiveTransformation<ScalarParam,3>& t);
template <class ScalarParam>
void glMultMatrix(const Geometry::ProjectiveTransformation<ScalarParam,3>& t);

template <class ScalarParam>
Geometry::ProjectiveTransformation<ScalarParam,3> glGetMatrix(GLMatrixEnums::Pname pname);

template <class ScalarParam>
inline Geometry::ProjectiveTransformation<ScalarParam,3> glGetColorMatrix(void)
	{
	return glGetMatrix<ScalarParam>(GLMatrixEnums::COLOR);
	}

template <class ScalarParam>
inline Geometry::ProjectiveTransformation<ScalarParam,3> glGetModelviewMatrix(void)
	{
	return glGetMatrix<ScalarParam>(GLMatrixEnums::MODELVIEW);
	}

template <class ScalarParam>
inline Geometry::ProjectiveTransformation<ScalarParam,3> glGetProjectionMatrix(void)
	{
	return glGetMatrix<ScalarParam>(GLMatrixEnums::PROJECTION);
	}

template <class ScalarParam>
inline Geometry::ProjectiveTransformation<ScalarParam,3> glGetTextureMatrix(void)
	{
	return glGetMatrix<ScalarParam>(GLMatrixEnums::TEXTURE);
	}

/***************************************
GLSL uniform variable wrapper functions:
***************************************/

template <class ScalarParam>
void glUniformARB(GLint location,const Geometry::OrthonormalTransformation<ScalarParam,3>& t);
template <class ScalarParam>
void glUniformARB(GLint location,const Geometry::OrthogonalTransformation<ScalarParam,3>& t);
template <class ScalarParam>
void glUniformARB(GLint location,const Geometry::AffineTransformation<ScalarParam,3>& t);
template <class ScalarParam>
void glUniformARB(GLint location,const Geometry::ProjectiveTransformation<ScalarParam,3>& t);

#if defined(GLGEOMETRY_NONSTANDARD_TEMPLATES) && !defined(GLTRANSFORMATIONWRAPPERS_IMPLEMENTATION)
#include <GL/GLTransformationWrappers.icpp>
#endif

#endif
