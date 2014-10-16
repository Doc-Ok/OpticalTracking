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

#include <GL/GLTransformationWrappers.icpp>

/**************************************
Overloaded versions of glUniform calls:
**************************************/

template <>
void
glUniformARB<GLfloat>(
	GLint location,
	const Geometry::ProjectiveTransformation<GLfloat,3>& t)
	{
	/* Directly upload the transformation's matrix entries: */
	glUniformMatrix4fvARB(location,1,GL_TRUE,t.getMatrix().getEntries());
	}

template <>
void
glUniformARB<GLdouble>(
	GLint location,
	const Geometry::ProjectiveTransformation<GLdouble,3>& t)
	{
	/* Write the transformation to a 4x4 matrix: */
	const GLdouble* tmPtr=t.getMatrix().getEntries();
	GLfloat matrix[16];
	GLfloat* mPtr=matrix;
	for(int i=0;i<16;++i,++tmPtr,++mPtr)
			*mPtr=GLfloat(*tmPtr);
	glUniformMatrix4fvARB(location,1,GL_TRUE,matrix);
	}

/********************************************************
Force instantiation of all standard GLGeometry functions:
********************************************************/

template void glLoadMatrix(const Geometry::TranslationTransformation<float,3>&);
template void glLoadMatrix(const Geometry::TranslationTransformation<double,3>&);
template void glMultMatrix(const Geometry::TranslationTransformation<float,3>&);
template void glMultMatrix(const Geometry::TranslationTransformation<double,3>&);

template void glLoadMatrix(const Geometry::RotationTransformation<float,3>&);
template void glLoadMatrix(const Geometry::RotationTransformation<double,3>&);
template void glMultMatrix(const Geometry::RotationTransformation<float,3>&);
template void glMultMatrix(const Geometry::RotationTransformation<double,3>&);

template void glLoadMatrix(const Geometry::OrthonormalTransformation<float,3>&);
template void glLoadMatrix(const Geometry::OrthonormalTransformation<double,3>&);
template void glMultMatrix(const Geometry::OrthonormalTransformation<float,3>&);
template void glMultMatrix(const Geometry::OrthonormalTransformation<double,3>&);

template void glLoadMatrix(const Geometry::UniformScalingTransformation<float,3>&);
template void glLoadMatrix(const Geometry::UniformScalingTransformation<double,3>&);
template void glMultMatrix(const Geometry::UniformScalingTransformation<float,3>&);
template void glMultMatrix(const Geometry::UniformScalingTransformation<double,3>&);

template void glLoadMatrix(const Geometry::OrthogonalTransformation<float,3>&);
template void glLoadMatrix(const Geometry::OrthogonalTransformation<double,3>&);
template void glMultMatrix(const Geometry::OrthogonalTransformation<float,3>&);
template void glMultMatrix(const Geometry::OrthogonalTransformation<double,3>&);

template void glLoadMatrix(const Geometry::ScalingTransformation<float,3>&);
template void glLoadMatrix(const Geometry::ScalingTransformation<double,3>&);
template void glMultMatrix(const Geometry::ScalingTransformation<float,3>&);
template void glMultMatrix(const Geometry::ScalingTransformation<double,3>&);

template void glLoadMatrix(const Geometry::AffineTransformation<float,3>&);
template void glLoadMatrix(const Geometry::AffineTransformation<double,3>&);
template void glMultMatrix(const Geometry::AffineTransformation<float,3>&);
template void glMultMatrix(const Geometry::AffineTransformation<double,3>&);

template void glLoadMatrix(const Geometry::ProjectiveTransformation<float,3>&);
template void glLoadMatrix(const Geometry::ProjectiveTransformation<double,3>&);
template void glMultMatrix(const Geometry::ProjectiveTransformation<float,3>&);
template void glMultMatrix(const Geometry::ProjectiveTransformation<double,3>&);

template Geometry::ProjectiveTransformation<float,3> glGetMatrix<float>(GLMatrixEnums::Pname pname);
template Geometry::ProjectiveTransformation<double,3> glGetMatrix<double>(GLMatrixEnums::Pname pname);

template void glUniformARB(GLint,const Geometry::OrthonormalTransformation<float,3>&);
template void glUniformARB(GLint,const Geometry::OrthonormalTransformation<double,3>&);
template void glUniformARB(GLint,const Geometry::OrthogonalTransformation<float,3>&);
template void glUniformARB(GLint,const Geometry::OrthogonalTransformation<double,3>&);
template void glUniformARB(GLint,const Geometry::AffineTransformation<float,3>&);
template void glUniformARB(GLint,const Geometry::AffineTransformation<double,3>&);
