/***********************************************************************
GLShader - Simple class to encapsulate vertex and fragment programs
written in the OpenGL Shading Language; assumes that vertex and fragment
shader objects are not shared between shader programs.
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

#include <string.h>
#include <stdexcept>
#include <Misc/ThrowStdErr.h>
#include <GL/gl.h>
#include <GL/GLExtensionManager.h>
#include <GL/Extensions/GLARBShaderObjects.h>
#include <GL/Extensions/GLARBVertexShader.h>
#include <GL/Extensions/GLARBFragmentShader.h>

#include <GL/GLShader.h>

/*************************
Methods of class GLShader:
*************************/

GLShader::GLShader(void)
	:programObject(0)
	{
	/* Initialize the required extensions; extension manager will throw exceptions if any are not supported: */
	GLARBShaderObjects::initExtension();
	GLARBVertexShader::initExtension();
	GLARBFragmentShader::initExtension();
	}

GLShader::~GLShader(void)
	{
	/* Reset the shader: */
	reset();
	}

bool GLShader::isSupported(void)
	{
	return GLARBShaderObjects::isSupported()&&GLARBVertexShader::isSupported()&&GLARBFragmentShader::isSupported();
	}

void GLShader::initExtensions(void)
	{
	/* Initialize the required extensions; extension manager will throw exceptions if any are not supported: */
	GLARBShaderObjects::initExtension();
	GLARBVertexShader::initExtension();
	GLARBFragmentShader::initExtension();
	}

void GLShader::compileVertexShader(const char* shaderSourceFileName)
	{
	if(programObject!=0)
		Misc::throwStdErr("GLShader::compileVertexShader: Attempt to compile after linking");
	
	GLhandleARB vertexShaderObject=0;
	try
		{
		/* Create a new vertex shader: */
		vertexShaderObject=glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
		
		/* Load and compile the shader source code: */
		glCompileShaderFromFile(vertexShaderObject,shaderSourceFileName);
		
		/* Store the shader for linking: */
		vertexShaderObjects.push_back(vertexShaderObject);
		}
	catch(std::runtime_error)
		{
		/* Delete the vertex shader: */
		if(vertexShaderObject!=0)
			glDeleteObjectARB(vertexShaderObject);
		
		throw;
		}
	}

void GLShader::compileVertexShaderFromString(const char* shaderSource)
	{
	if(programObject!=0)
		Misc::throwStdErr("GLShader::compileVertexShaderFromString: Attempt to compile after linking");
	
	GLhandleARB vertexShaderObject=0;
	try
		{
		/* Create a new vertex shader: */
		vertexShaderObject=glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
		
		/* Compile the shader source code: */
		glCompileShaderFromString(vertexShaderObject,shaderSource);
		
		/* Store the shader for linking: */
		vertexShaderObjects.push_back(vertexShaderObject);
		}
	catch(std::runtime_error)
		{
		/* Delete the vertex shader: */
		if(vertexShaderObject!=0)
			glDeleteObjectARB(vertexShaderObject);
		
		throw;
		}
	}

void GLShader::compileFragmentShader(const char* shaderSourceFileName)
	{
	if(programObject!=0)
		Misc::throwStdErr("GLShader::compileFragmentShader: Attempt to compile after linking");
	
	GLhandleARB fragmentShaderObject=0;
	try
		{
		/* Create a new fragment shader: */
		fragmentShaderObject=glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
		
		/* Load and compile the shader source code: */
		glCompileShaderFromFile(fragmentShaderObject,shaderSourceFileName);
		
		/* Store the shader for linking: */
		fragmentShaderObjects.push_back(fragmentShaderObject);
		}
	catch(std::runtime_error)
		{
		/* Delete the fragment shader: */
		if(fragmentShaderObject!=0)
			glDeleteObjectARB(fragmentShaderObject);
		
		throw;
		}
	}

void GLShader::compileFragmentShaderFromString(const char* shaderSource)
	{
	if(programObject!=0)
		Misc::throwStdErr("GLShader::compileFragmentShaderFromString: Attempt to compile after linking");
	
	GLhandleARB fragmentShaderObject=0;
	try
		{
		/* Create a new fragment shader: */
		fragmentShaderObject=glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
		
		/* Compile the shader source code: */
		glCompileShaderFromString(fragmentShaderObject,shaderSource);
		
		/* Store the shader for linking: */
		fragmentShaderObjects.push_back(fragmentShaderObject);
		}
	catch(std::runtime_error)
		{
		/* Delete the fragment shader: */
		if(fragmentShaderObject!=0)
			glDeleteObjectARB(fragmentShaderObject);
		
		throw;
		}
	}

void GLShader::bindAttribLocation(GLuint index,const char* attributeName)
	{
	if(programObject!=0)
		Misc::throwStdErr("GLShader::bindAttribLocation: Attempt to bind attribute location after linking");
	
	/* Bind the attribute location: */
	glBindAttribLocationARB(programObject,index,attributeName);
	}

void GLShader::linkShader(void)
	{
	if(programObject!=0)
		Misc::throwStdErr("GLShader::linkShader: Attempt to link shader program multiple times");
	
	/* Create the program object: */
	programObject=glCreateProgramObjectARB();
	
	/* Attach all previously compiled shaders to the program object: */
	for(HandleList::iterator vsoIt=vertexShaderObjects.begin();vsoIt!=vertexShaderObjects.end();++vsoIt)
		glAttachObjectARB(programObject,*vsoIt);
	for(HandleList::iterator fsoIt=fragmentShaderObjects.begin();fsoIt!=fragmentShaderObjects.end();++fsoIt)
		glAttachObjectARB(programObject,*fsoIt);
	
	/* Link the program: */
	glLinkProgramARB(programObject);
	
	/* Check if the program linked successfully: */
	GLint linkStatus;
	glGetObjectParameterivARB(programObject,GL_OBJECT_LINK_STATUS_ARB,&linkStatus);
	if(!linkStatus)
		{
		/* Get some more detailed information: */
		GLcharARB linkLogBuffer[2048];
		GLsizei linkLogSize;
		glGetInfoLogARB(programObject,sizeof(linkLogBuffer),&linkLogSize,linkLogBuffer);
		
		/* Delete the program object: */
		glDeleteObjectARB(programObject);
		programObject=0;
		
		/* Signal an error: */
		Misc::throwStdErr("GLShader::linkShader: Error \"%s\" while linking shader program",linkLogBuffer);
		}
	}

void GLShader::reset(void)
	{
	/* Check if the program has already been linked: */
	if(programObject!=0)
		{
		/* Detach all shaders from the shader program: */
		for(HandleList::iterator vsoIt=vertexShaderObjects.begin();vsoIt!=vertexShaderObjects.end();++vsoIt)
			glDetachObjectARB(programObject,*vsoIt);
		for(HandleList::iterator fsoIt=fragmentShaderObjects.begin();fsoIt!=fragmentShaderObjects.end();++fsoIt)
			glDetachObjectARB(programObject,*fsoIt);
		
		/* Delete the shader program: */
		glDeleteObjectARB(programObject);
		programObject=0;
		}
	
	/* Delete all shaders: */
	for(HandleList::iterator vsoIt=vertexShaderObjects.begin();vsoIt!=vertexShaderObjects.end();++vsoIt)
		glDeleteObjectARB(*vsoIt);
	vertexShaderObjects.clear();
	for(HandleList::iterator fsoIt=fragmentShaderObjects.begin();fsoIt!=fragmentShaderObjects.end();++fsoIt)
		glDeleteObjectARB(*fsoIt);
	fragmentShaderObjects.clear();
	}

int GLShader::getAttribLocation(const char* attributeName) const
	{
	if(programObject==0)
		Misc::throwStdErr("GLShader::getAttribLocation: Attempt to use shader program before linking");
	
	/* Return the uniform variable index: */
	return glGetAttribLocationARB(programObject,attributeName);
	}

int GLShader::getUniformLocation(const char* uniformName) const
	{
	if(programObject==0)
		Misc::throwStdErr("GLShader::getUniformLocation: Attempt to use shader program before linking");
	
	/* Return the uniform variable index: */
	return glGetUniformLocationARB(programObject,uniformName);
	}

void GLShader::useProgram(void) const
	{
	if(programObject==0)
		Misc::throwStdErr("GLShader::useProgram: Attempt to use shader program before linking");
	
	/* Install the shader program: */
	glUseProgramObjectARB(programObject);
	}

void GLShader::disablePrograms(void)
	{
	glUseProgramObjectARB(0);
	}
