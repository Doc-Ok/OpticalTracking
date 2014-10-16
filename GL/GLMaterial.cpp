/***********************************************************************
GLMaterial - Class to encapsulate OpenGL material properties.
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

#include <GL/GLMaterialTemplates.h>
#include <GL/GLGetMaterialTemplates.h>

#include <GL/GLMaterial.h>

/***************************
Methods of class GLMaterial:
***************************/

GLMaterial::GLMaterial(void)
	:ambient(0.2f,0.2f,0.2f,1.0f),
	 diffuse(0.8f,0.8f,0.8f,1.0f),
	 specular(0.0f,0.0f,0.0f,1.0f),
	 shininess(0.0f),
	 emission(0.0f,0.0f,0.0f,1.0f)
	{
	}

GLMaterial::GLMaterial(const GLMaterial::Color& sAmbientDiffuse)
	:ambient(sAmbientDiffuse),
	 diffuse(sAmbientDiffuse),
	 specular(0.0f,0.0f,0.0f,1.0f),
	 shininess(0.0f),
	 emission(0.0f,0.0f,0.0f,1.0f)
	{
	}

GLMaterial::GLMaterial(const GLMaterial::Color& sAmbientDiffuse,const GLMaterial::Color& sSpecular,GLMaterial::Scalar sShininess)
	:ambient(sAmbientDiffuse),
	 diffuse(sAmbientDiffuse),
	 specular(sSpecular),
	 shininess(sShininess),
	 emission(0.0f,0.0f,0.0f,1.0f)
	{
	}

GLMaterial::GLMaterial(const GLMaterial::Color& sAmbient,const GLMaterial::Color& sDiffuse,const GLMaterial::Color& sSpecular,GLMaterial::Scalar sShininess)
	:ambient(sAmbient),
	 diffuse(sDiffuse),
	 specular(sSpecular),
	 shininess(sShininess),
	 emission(0.0f,0.0f,0.0f,1.0f)
	{
	}

GLMaterial::GLMaterial(const GLMaterial::Color& sAmbientDiffuse,const GLMaterial::Color& sSpecular,GLMaterial::Scalar sShininess,const GLMaterial::Color& sEmission)
	:ambient(sAmbientDiffuse),
	 diffuse(sAmbientDiffuse),
	 specular(sSpecular),
	 shininess(sShininess),
	 emission(sEmission)
	{
	}

GLMaterial::GLMaterial(const GLMaterial::Color& sAmbient,const GLMaterial::Color& sDiffuse,const GLMaterial::Color& sSpecular,GLMaterial::Scalar sShininess,const GLMaterial::Color& sEmission)
	:ambient(sAmbient),
	 diffuse(sDiffuse),
	 specular(sSpecular),
	 shininess(sShininess),
	 emission(sEmission)
	{
	}

void glMaterial(GLMaterial::Face face,const GLMaterial& material)
	{
	glMaterialAmbient(face,material.ambient);
	glMaterialDiffuse(face,material.diffuse);
	glMaterialSpecular(face,material.specular);
	glMaterialShininess(face,material.shininess);
	glMaterialEmission(face,material.emission);
	}

void glGetMaterial(GLMaterial::Face face,GLMaterial& material)
	{
	glGetMaterialAmbient(face,material.ambient);
	glGetMaterialDiffuse(face,material.diffuse);
	glGetMaterialSpecular(face,material.specular);
	glGetMaterialShininess(face,material.shininess);
	glGetMaterialEmission(face,material.emission);
	}

GLMaterial glGetMaterial(GLMaterial::Face face)
	{
	GLMaterial result;
	glGetMaterialAmbient(face,result.ambient);
	glGetMaterialDiffuse(face,result.diffuse);
	glGetMaterialSpecular(face,result.specular);
	glGetMaterialShininess(face,result.shininess);
	glGetMaterialEmission(face,result.emission);
	return result;
	}
