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

#ifndef GLSHADER_INCLUDED
#define GLSHADER_INCLUDED

#include <vector>
#include <GL/gl.h>
#include <GL/Extensions/GLARBShaderObjects.h>

class GLShader
	{
	/* Embedded classes: */
	protected:
	typedef std::vector<GLhandleARB> HandleList; // Type for list of object handles
	
	/* Elements: */
	HandleList vertexShaderObjects; // List of handles of compiled vertex shader objects
	HandleList fragmentShaderObjects; // List of handles of compiled fragment shader objects
	GLhandleARB programObject; // Handle for the linked shader program
	
	/* Constructors and destructors: */
	public:
	GLShader(void); // Creates an "empty" shader
	private:
	GLShader(const GLShader& source); // Prohibit copy constructor
	GLShader& operator=(const GLShader& source); // Prohibit assignment operator
	public:
	~GLShader(void); // Destroys a shader
	
	/* Methods: */
	static bool isSupported(void); // Returns true if the current OpenGL context supports GLSL shaders
	static void initExtensions(void); // Initializes the OpenGL extensions required by GLSL shaders (optional; implicitly done by GLShader constructor)
	void compileVertexShader(const char* shaderSourceFileName); // Loads and compiles a vertex shader from a source file
	void compileVertexShaderFromString(const char* shaderSource); // Compiles a vertex shader from a source code string
	void compileFragmentShader(const char* shaderSourceFileName); // Loads and compiles a fragment shader from a source file
	void compileFragmentShaderFromString(const char* shaderSource); // Compiles a fragment shader from a source code string
	void bindAttribLocation(GLuint index,const char* attributeName); // Binds the named attribute variable to the given attribute index
	void linkShader(void); // Links all previously loaded vertex and fragment shaders into a shader program
	void reset(void); // Deletes all compiled vertex and fragment shaders and the linked program
	bool isValid(void) const // Returns true if the shader linked successfully and can be used
		{
		return programObject!=0;
		}
	int getAttribLocation(const char* attributeName) const; // Returns the index of an attribute variable defined in the shader program
	int getUniformLocation(const char* uniformName) const; // Returns the index of a uniform variable defined in the shader program
	void useProgram(void) const; // Installs the shader program in the current OpenGL context
	static void disablePrograms(void); // Removes any installed shader programs from the current OpenGL context
	};

#endif
