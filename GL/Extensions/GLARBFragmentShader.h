/***********************************************************************
GLARBFragmentShader - OpenGL extension class for the
GL_ARB_fragment_shader extension.
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

#ifndef GLEXTENSIONS_GLARBFRAGMENTSHADER_INCLUDED
#define GLEXTENSIONS_GLARBFRAGMENTSHADER_INCLUDED

#include <stddef.h>
#include <GL/gl.h>
#include <GL/TLSHelper.h>
#include <GL/Extensions/GLExtension.h>
#include <GL/Extensions/GLARBShaderObjects.h>

/* Forward declarations: */
namespace IO {
class File;
}

/********************************
Extension-specific parts of gl.h:
********************************/

#ifndef GL_ARB_fragment_shader
#define GL_ARB_fragment_shader 1

/* Extension-specific functions: */

/* Extension-specific constants: */
#define GL_FRAGMENT_SHADER_ARB            0x8B30
#define GL_MAX_FRAGMENT_UNIFORM_COMPONENTS_ARB 0x8B49
#define GL_FRAGMENT_SHADER_DERIVATIVE_HINT_ARB 0x8B8B

#endif

class GLARBFragmentShader:public GLExtension
	{
	/* Elements: */
	private:
	static GL_THREAD_LOCAL(GLARBFragmentShader*) current; // Pointer to extension object for current OpenGL context
	static const char* name; // Extension name
	
	/* Constructors and destructors: */
	private:
	GLARBFragmentShader(void);
	public:
	virtual ~GLARBFragmentShader(void);
	
	/* Methods: */
	public:
	virtual const char* getExtensionName(void) const;
	virtual void activate(void);
	virtual void deactivate(void);
	static bool isSupported(void); // Returns true if the extension is supported in the current OpenGL context
	static void initExtension(void); // Initializes the extension in the current OpenGL context
	
	/* Extension entry points: */
	};

/*******************************
Extension-specific entry points:
*******************************/

/****************
Helper functions:
****************/

GLhandleARB glCompileFragmentShaderFromString(const char* shaderSource); // Compiles a new fragment shader object from a C-style string; throws exception on errors
GLhandleARB glCompileFragmentShaderFromStrings(size_t numShaderSources,...); // Compiles a new fragment shader object from a list of C-style strings; throws exception on errors
GLhandleARB glCompileFragmentShaderFromFile(const char* shaderSourceFileName); // Compiles a new fragment shader object from a source file; throws exception on errors
GLhandleARB glCompileFragmentShaderFromFile(const char* shaderSourceFileName,IO::File& shaderSourceFile); // Ditto, with already-opened IO::File object

#endif
