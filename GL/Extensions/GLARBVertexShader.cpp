/***********************************************************************
GLARBVertexShader - OpenGL extension class for the GL_ARB_vertex_shader
extension.
Copyright (c) 2007-2014 Oliver Kreylos

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

#include <GL/Extensions/GLARBVertexShader.h>

#include <stdarg.h>
#include <GL/gl.h>
#include <GL/GLContextData.h>
#include <GL/GLExtensionManager.h>

/******************************************
Static elements of class GLARBVertexShader:
******************************************/

GL_THREAD_LOCAL(GLARBVertexShader*) GLARBVertexShader::current=0;
const char* GLARBVertexShader::name="GL_ARB_vertex_shader";

/**********************************
Methods of class GLARBVertexShader:
**********************************/

GLARBVertexShader::GLARBVertexShader(void)
	:glBindAttribLocationARBProc(GLExtensionManager::getFunction<PFNGLBINDATTRIBLOCATIONARBPROC>("glBindAttribLocationARB")),
	 glGetActiveAttribARBProc(GLExtensionManager::getFunction<PFNGLGETACTIVEATTRIBARBPROC>("glGetActiveAttribARB")),
	 glGetAttribLocationARBProc(GLExtensionManager::getFunction<PFNGLGETATTRIBLOCATIONARBPROC>("glGetAttribLocationARB"))
	{
	}

GLARBVertexShader::~GLARBVertexShader(void)
	{
	}

const char* GLARBVertexShader::getExtensionName(void) const
	{
	return name;
	}

void GLARBVertexShader::activate(void)
	{
	current=this;
	}

void GLARBVertexShader::deactivate(void)
	{
	current=0;
	}

bool GLARBVertexShader::isSupported(void)
	{
	/* Ask the current extension manager whether the extension is supported in the current OpenGL context: */
	return GLExtensionManager::isExtensionSupported(name);
	}

void GLARBVertexShader::initExtension(void)
	{
	/* Check if the extension is already initialized: */
	if(!GLExtensionManager::isExtensionRegistered(name))
		{
		/* Initialize the GL_ARB_vertex_program extension first (shares entry points): */
		GLARBVertexProgram::initExtension();
		
		/* Create a new extension object: */
		GLARBVertexShader* newExtension=new GLARBVertexShader;
		
		/* Register the extension with the current extension manager: */
		GLExtensionManager::registerExtension(newExtension);
		}
	}

GLhandleARB glCompileVertexShaderFromString(const char* shaderSource)
	{
	/* Create a new vertex shader: */
	GLhandleARB vertexShaderObject=glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	
	try
		{
		/* Load and compile the shader source: */
		glCompileShaderFromString(vertexShaderObject,shaderSource);
		}
	catch(...)
		{
		/* Clean up and re-throw the exception: */
		glDeleteObjectARB(vertexShaderObject);
		
		throw;
		}
	
	return vertexShaderObject;
	}

GLhandleARB glCompileVertexShaderFromStrings(size_t numShaderSources,...)
	{
	/* Create a new vertex shader: */
	GLhandleARB vertexShaderObject=glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	
	va_list ap;
	try
		{
		va_start(ap,numShaderSources);
		glCompileShaderFromStrings(vertexShaderObject,numShaderSources,ap);
		va_end(ap);
		}
	catch(...)
		{
		/* Clean up and re-throw the exception: */
		va_end(ap);
		glDeleteObjectARB(vertexShaderObject);
		throw;
		}
	
	return vertexShaderObject;
	}

GLhandleARB glCompileVertexShaderFromFile(const char* shaderSourceFileName)
	{
	/* Create a new vertex shader: */
	GLhandleARB vertexShaderObject=glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	
	try
		{
		/* Load and compile the shader source: */
		glCompileShaderFromFile(vertexShaderObject,shaderSourceFileName);
		}
	catch(...)
		{
		/* Clean up and re-throw the exception: */
		glDeleteObjectARB(vertexShaderObject);
		throw;
		}
	
	return vertexShaderObject;
	}

GLhandleARB glCompileVertexShaderFromFile(const char* shaderSourceFileName,IO::File& shaderSourceFile)
	{
	/* Create a new vertex shader: */
	GLhandleARB vertexShaderObject=glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	
	try
		{
		/* Load and compile the shader source: */
		glCompileShaderFromFile(vertexShaderObject,shaderSourceFileName,shaderSourceFile);
		}
	catch(...)
		{
		/* Clean up and re-throw the exception: */
		glDeleteObjectARB(vertexShaderObject);
		throw;
		}
	
	return vertexShaderObject;
	}
