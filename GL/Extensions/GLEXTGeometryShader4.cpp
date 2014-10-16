/***********************************************************************
GLEXTGeometryShader4 - OpenGL extension class for the
GL_EXT_geometry_shader4 extension.
Copyright (c) 2007-2014 Tony Bernardin, Oliver Kreylos

This file is part of the OpenGL Support Library (GLSupport).

The OpenGL Support Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The OpenGL Support Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the OpenGL Support Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <GL/Extensions/GLEXTGeometryShader4.h>

#include <GL/gl.h>
#include <GL/GLContextData.h>
#include <GL/GLExtensionManager.h>

/*********************************************
Static elements of class GLEXTGeometryShader4:
*********************************************/

GL_THREAD_LOCAL(GLEXTGeometryShader4*) GLEXTGeometryShader4::current=0;
const char* GLEXTGeometryShader4::name="GL_EXT_geometry_shader4";

/*************************************
Methods of class GLEXTGeometryShader4:
*************************************/

GLEXTGeometryShader4::GLEXTGeometryShader4(void)
	:glProgramParameteriEXTProc(GLExtensionManager::getFunction<PFNGLPROGRAMPARAMETERIEXTPROC>("glProgramParameteriEXT")),
	 glFramebufferTextureEXTProc(GLExtensionManager::getFunction<PFNGLFRAMEBUFFERTEXTUREEXTPROC>("glFramebufferTextureEXT")),
	 glFramebufferTextureLayerEXTProc(GLExtensionManager::getFunction<PFNGLFRAMEBUFFERTEXTURELAYEREXTPROC>("glFramebufferTextureLayerEXT")),
	 glFramebufferTextureFaceEXTProc(GLExtensionManager::getFunction<PFNGLFRAMEBUFFERTEXTUREFACEEXTPROC>("glFramebufferTextureFaceEXT"))
	{
	}

GLEXTGeometryShader4::~GLEXTGeometryShader4(void)
	{
	}

const char* GLEXTGeometryShader4::getExtensionName(void) const
	{
	return name;
	}

void GLEXTGeometryShader4::activate(void)
	{
	current=this;
	}

void GLEXTGeometryShader4::deactivate(void)
	{
	current=0;
	}

bool GLEXTGeometryShader4::isSupported(void)
	{
	/* Ask the current extension manager whether the extension is supported in the current OpenGL context: */
	return GLExtensionManager::isExtensionSupported(name);
	}

void GLEXTGeometryShader4::initExtension(void)
	{
	/* Check if the extension is already initialized: */
	if(!GLExtensionManager::isExtensionRegistered(name))
		{
		/* Create a new extension object: */
		GLEXTGeometryShader4* newExtension=new GLEXTGeometryShader4;
		
		/* Register the extension with the current extension manager: */
		GLExtensionManager::registerExtension(newExtension);
		}
	}

GLhandleARB glCompileEXTGeometryShader4FromString(const char* shaderSource)
	{
	/* Create a new geometry shader: */
	GLhandleARB geometryShaderObject=glCreateShaderObjectARB(GL_GEOMETRY_SHADER_EXT);
	
	try
		{
		/* Load and compile the shader source: */
		glCompileShaderFromString(geometryShaderObject,shaderSource);
		}
	catch(std::runtime_error err)
		{
		/* Clean up and re-throw the exception: */
		glDeleteObjectARB(geometryShaderObject);
		
		throw;
		}
	
	return geometryShaderObject;
	}

GLhandleARB glCompileEXTGeometryShader4FromFile(const char* shaderSourceFileName)
	{
	/* Create a new vertex shader: */
	GLhandleARB geometryShaderObject=glCreateShaderObjectARB(GL_GEOMETRY_SHADER_EXT);
	
	try
		{
		/* Load and compile the shader source: */
		glCompileShaderFromFile(geometryShaderObject,shaderSourceFileName);
		}
	catch(std::runtime_error err)
		{
		/* Clean up and re-throw the exception: */
		glDeleteObjectARB(geometryShaderObject);
		
		throw;
		}
	
	return geometryShaderObject;
	}
