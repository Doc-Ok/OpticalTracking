/***********************************************************************
GLFog - Class to encapsulate OpenGL fog properties.
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

#include <GL/GLFogTemplates.h>
#include <GL/GLGetFogTemplates.h>

#include <GL/GLFog.h>

/**********************
Methods of class GLFog:
**********************/

GLFog::GLFog(void)
	:mode(GLFogEnums::EXP),
	 density(1.0f),
	 start(0.0f),end(1.0f),
	 color(0.0f,0.0f,0.0f,0.0f)
	{
	}

GLFog::GLFog(const GLFog::Color& sColor)
	:mode(GLFogEnums::EXP),
	 density(1.0f),
	 start(0.0f),end(1.0f),
	 color(sColor)
	{
	}

GLFog::GLFog(GLFog::Mode sMode,
             const GLFog::Color& sColor)
	:mode(sMode),
	 density(1.0f),
	 start(0.0f),end(1.0f),
	 color(sColor)
	{
	}

GLFog::GLFog(GLFog::Mode sMode,
             GLFog::Scalar sDensity,
             const GLFog::Color& sColor)
	:mode(sMode),
	 density(sDensity),
	 start(0.0f),end(1.0f),
	 color(sColor)
	{
	}

GLFog::GLFog(GLFog::Mode sMode,
             GLFog::Scalar sStart,
             GLFog::Scalar sEnd,
             const GLFog::Color& sColor)
	:mode(sMode),
	 density(1.0f),
	 start(sStart),end(sEnd),
	 color(sColor)
	{
	}

GLFog::GLFog(GLFog::Mode sMode,
             GLFog::Scalar sDensity,
             GLFog::Scalar sStart,
             GLFog::Scalar sEnd,
             const GLFog::Color& sColor)
	:mode(sMode),
	 density(sDensity),
	 start(sStart),end(sEnd),
	 color(sColor)
	{
	}

void glFog(const GLFog& fog)
	{
	glFogMode(fog.mode);
	switch(fog.mode)
		{
		case GLFogEnums::EXP:
		case GLFogEnums::EXP2:
			glFogDensity(fog.density);
			break;
		
		case GLFogEnums::LINEAR:
			glFogStart(fog.start);
			glFogEnd(fog.end);
			break;
		}
	glFogColor(fog.color);
	}

void glGetFog(GLFog& fog)
	{
	glGetFogMode(fog.mode);
	glGetFogDensity(fog.density);
	glGetFogStart(fog.start);
	glGetFogEnd(fog.end);
	glGetFogColor(fog.color);
	}

GLFog glGetFog(void)
	{
	GLFog result;
	glGetFogMode(result.mode);
	glGetFogDensity(result.density);
	glGetFogStart(result.start);
	glGetFogEnd(result.end);
	glGetFogColor(result.color);
	return result;
	}
