/***********************************************************************
GLLight - Class to encapsulate OpenGL light source properties.
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

#include <GL/GLLightTemplates.h>
#include <GL/GLGetLightTemplates.h>

#include <GL/GLLight.h>

/************************
Methods of class GLLight:
************************/

GLLight::GLLight(void)
	:ambient(0.0f,0.0f,0.0f,1.0f),
	 diffuse(1.0f,1.0f,1.0f,1.0f),
	 specular(1.0f,1.0f,1.0f,1.0f),
	 position(0.0f,0.0f,1.0f,0.0f),
	 spotDirection(0.0f,0.0f,-1.0f),
	 spotCutoff(180.0f),
	 spotExponent(0.0f),
	 constantAttenuation(1.0f),
	 linearAttenuation(0.0f),
	 quadraticAttenuation(0.0f)
	{
	}

GLLight::GLLight(const GLLight::Color& sColor,
                 const GLLight::Position& sPosition,
                 GLLight::Scalar sConstantAttenuation,
                 GLLight::Scalar sLinearAttenuation,
                 GLLight::Scalar sQuadraticAttenuation)
	:ambient(0.0f,0.0f,0.0f,1.0f),
	 diffuse(sColor),
	 specular(sColor),
	 position(sPosition),
	 spotDirection(0.0f,0.0f,-1.0f),
	 spotCutoff(180.0f),
	 spotExponent(0.0f),
	 constantAttenuation(sConstantAttenuation),
	 linearAttenuation(sLinearAttenuation),
	 quadraticAttenuation(sQuadraticAttenuation)
	{
	}

GLLight::GLLight(const GLLight::Color& sColor,
                 const GLLight::Position& sPosition,
                 const GLLight::SpotDirection& sSpotDirection,
                 GLLight::Scalar sSpotCutoff,
                 GLLight::Scalar sSpotExponent,
                 GLLight::Scalar sConstantAttenuation,
                 GLLight::Scalar sLinearAttenuation,
                 GLLight::Scalar sQuadraticAttenuation)
	:ambient(0.0f,0.0f,0.0f,1.0f),
	 diffuse(sColor),
	 specular(sColor),
	 position(sPosition),
	 spotDirection(sSpotDirection),
	 spotCutoff(sSpotCutoff),
	 spotExponent(sSpotExponent),
	 constantAttenuation(sConstantAttenuation),
	 linearAttenuation(sLinearAttenuation),
	 quadraticAttenuation(sQuadraticAttenuation)
	{
	}

GLLight::GLLight(const GLLight::Color& sAmbient,
                 const GLLight::Color& sDiffuse,
                 const GLLight::Color& sSpecular,
                 const GLLight::Position& sPosition,
                 const GLLight::SpotDirection& sSpotDirection,
                 GLLight::Scalar sSpotCutoff,
                 GLLight::Scalar sSpotExponent,
                 GLLight::Scalar sConstantAttenuation,
                 GLLight::Scalar sLinearAttenuation,
                 GLLight::Scalar sQuadraticAttenuation)
	:ambient(sAmbient),
	 diffuse(sDiffuse),
	 specular(sSpecular),
	 position(sPosition),
	 spotDirection(sSpotDirection),
	 spotCutoff(sSpotCutoff),
	 spotExponent(sSpotExponent),
	 constantAttenuation(sConstantAttenuation),
	 linearAttenuation(sLinearAttenuation),
	 quadraticAttenuation(sQuadraticAttenuation)
	{
	}

void glLight(GLsizei lightIndex,const GLLight& light)
	{
	glLightAmbient(lightIndex,light.ambient);
	glLightDiffuse(lightIndex,light.diffuse);
	glLightSpecular(lightIndex,light.specular);
	glLightPosition(lightIndex,light.position);
	glLightSpotDirection(lightIndex,light.spotDirection);
	glLightSpotCutoff(lightIndex,light.spotCutoff);
	glLightSpotExponent(lightIndex,light.spotExponent);
	glLightConstantAttenuation(lightIndex,light.constantAttenuation);
	glLightLinearAttenuation(lightIndex,light.linearAttenuation);
	glLightQuadraticAttenuation(lightIndex,light.quadraticAttenuation);
	}

void glGetLight(GLsizei lightIndex,GLLight& light)
	{
	glGetLightAmbient(lightIndex,light.ambient);
	glGetLightDiffuse(lightIndex,light.diffuse);
	glGetLightSpecular(lightIndex,light.specular);
	glGetLightPosition(lightIndex,light.position);
	glGetLightSpotDirection(lightIndex,light.spotDirection);
	glGetLightSpotCutoff(lightIndex,light.spotCutoff);
	glGetLightSpotExponent(lightIndex,light.spotExponent);
	glGetLightConstantAttenuation(lightIndex,light.constantAttenuation);
	glGetLightLinearAttenuation(lightIndex,light.linearAttenuation);
	glGetLightQuadraticAttenuation(lightIndex,light.quadraticAttenuation);
	}

GLLight glGetLight(GLsizei lightIndex)
	{
	GLLight result;
	glGetLightAmbient(lightIndex,result.ambient);
	glGetLightDiffuse(lightIndex,result.diffuse);
	glGetLightSpecular(lightIndex,result.specular);
	glGetLightPosition(lightIndex,result.position);
	glGetLightSpotDirection(lightIndex,result.spotDirection);
	glGetLightSpotCutoff(lightIndex,result.spotCutoff);
	glGetLightSpotExponent(lightIndex,result.spotExponent);
	glGetLightConstantAttenuation(lightIndex,result.constantAttenuation);
	glGetLightLinearAttenuation(lightIndex,result.linearAttenuation);
	glGetLightQuadraticAttenuation(lightIndex,result.quadraticAttenuation);
	return result;
	}
