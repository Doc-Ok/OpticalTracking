/***********************************************************************
GLGeometryShader - Class to represent GLSL shaders that contain at least
one geometry shader according to the GL_EXT_geometry_shader4 extension.
Copyright (c) 2009-2013 Oliver Kreylos
Based on code provided by Tony Bernardin

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

#include <stdexcept>
#include <Misc/ThrowStdErr.h>
#include <GL/gl.h>
#include <GL/GLExtensionManager.h>
#include <GL/Extensions/GLARBShaderObjects.h>
#include <GL/Extensions/GLEXTGeometryShader4.h>

#include <GL/GLGeometryShader.h>

/*********************************
Methods of class GLGeometryShader:
*********************************/

GLGeometryShader::GLGeometryShader(void)
	{
	/* Initialize the required extensions, extension manager will throw exception if any are not supported: */
	GLEXTGeometryShader4::initExtension();
	}

GLGeometryShader::~GLGeometryShader(void)
	{
	if(programObject!=0)
		{
		/* Detach all geometry shaders from the shader program: */
		for(HandleList::iterator gsoIt=geometryShaderObjects.begin();gsoIt!=geometryShaderObjects.end();++gsoIt)
			glDetachObjectARB(programObject,*gsoIt);
		}
	
	/* Delete all geometry shaders: */
	for(HandleList::iterator gsoIt=geometryShaderObjects.begin();gsoIt!=geometryShaderObjects.end();++gsoIt)
		glDeleteObjectARB(*gsoIt);
	}

bool GLGeometryShader::isSupported(void)
	{
	return GLShader::isSupported()&&GLEXTGeometryShader4::isSupported();
	}

void GLGeometryShader::compileGeometryShader(const char* shaderSourceFileName)
	{
	if(programObject!=0)
		Misc::throwStdErr("GLGeometryShader::compileGeometryShader: Attempt to compile after linking");
	
	GLhandleARB geometryShaderObject=0;
	try
		{
		/* Create a new geometry shader: */
		geometryShaderObject=glCreateShaderObjectARB(GL_GEOMETRY_SHADER_EXT);
		
		/* Load and compile the shader source code: */
		glCompileShaderFromFile(geometryShaderObject,shaderSourceFileName);
		
		/* Store the shader for linking: */
		geometryShaderObjects.push_back(geometryShaderObject);
		}
	catch(std::runtime_error)
		{
		/* Delete the geometry shader: */
		if(geometryShaderObject!=0)
			glDeleteObjectARB(geometryShaderObject);
		
		throw;
		}
	}

void GLGeometryShader::compileGeometryShaderFromString(const char* shaderSource)
	{
	if(programObject!=0)
		Misc::throwStdErr("GLGeometryShader::compileGeometryShaderFromString: Attempt to compile after linking");
	
	GLhandleARB geometryShaderObject=0;
	try
		{
		/* Create a new geometry shader: */
		geometryShaderObject=glCreateShaderObjectARB(GL_GEOMETRY_SHADER_EXT);
		
		/* Compile the shader source code: */
		glCompileShaderFromString(geometryShaderObject,shaderSource);
		
		/* Store the shader for linking: */
		geometryShaderObjects.push_back(geometryShaderObject);
		}
	catch(std::runtime_error)
		{
		/* Delete the geometry shader: */
		if(geometryShaderObject!=0)
			glDeleteObjectARB(geometryShaderObject);
		
		throw;
		}
	}

void GLGeometryShader::linkShader(GLint geometryInputType,GLint geometryOutputType,GLint maxNumOutputVertices)
	{
	if(programObject!=0)
		Misc::throwStdErr("GLGeometryShader::linkShader: Attempt to link shader program multiple times");
	
	/* Create the program object: */
	programObject=glCreateProgramObjectARB();
	
	/* Attach all previously compiled shaders to the program object: */
	for(HandleList::iterator vsoIt=vertexShaderObjects.begin();vsoIt!=vertexShaderObjects.end();++vsoIt)
		glAttachObjectARB(programObject,*vsoIt);
	for(HandleList::iterator fsoIt=fragmentShaderObjects.begin();fsoIt!=fragmentShaderObjects.end();++fsoIt)
		glAttachObjectARB(programObject,*fsoIt);
	for(HandleList::iterator gsoIt=geometryShaderObjects.begin();gsoIt!=geometryShaderObjects.end();++gsoIt)
		glAttachObjectARB(programObject,*gsoIt);
	
	/* Configure the geometry shader parameters of the program object: */
	if(!geometryShaderObjects.empty())
		{
		/* Set the input/output parameters: */
		glProgramParameteriEXT(GLuint(programObject),GL_GEOMETRY_INPUT_TYPE_EXT,geometryInputType);
		glProgramParameteriEXT(GLuint(programObject),GL_GEOMETRY_OUTPUT_TYPE_EXT,geometryOutputType);
		glProgramParameteriEXT(GLuint(programObject),GL_GEOMETRY_VERTICES_OUT_EXT,maxNumOutputVertices);
		
		/* Check for errors: */
		GLenum err=glGetError();
		if(err!=GL_NO_ERROR)
			Misc::throwStdErr("GLGeometryShader::linkShader: Error #%u while setting geometry shader parameters",err);
		}
	
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
		
		/* Signal an error: */
		Misc::throwStdErr("GLGeometryShader::linkShader: Error \"%s\" while linking shader program",linkLogBuffer);
		}
	}
