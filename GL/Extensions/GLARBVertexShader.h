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

#ifndef GLEXTENSIONS_GLARBVERTEXSHADER_INCLUDED
#define GLEXTENSIONS_GLARBVERTEXSHADER_INCLUDED

#include <stddef.h>
#include <GL/gl.h>
#include <GL/TLSHelper.h>
#include <GL/Extensions/GLExtension.h>
#include <GL/Extensions/GLARBVertexProgram.h>
#include <GL/Extensions/GLARBShaderObjects.h>

/* Forward declarations: */
namespace IO {
class File;
}

/********************************
Extension-specific parts of gl.h:
********************************/

#ifndef GL_ARB_vertex_shader
#define GL_ARB_vertex_shader 1

/* Extension-specific functions: */
typedef void (APIENTRY * PFNGLBINDATTRIBLOCATIONARBPROC)(GLhandleARB programObj, GLuint index, const GLcharARB *name);
typedef void (APIENTRY * PFNGLGETACTIVEATTRIBARBPROC)(GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei *length, GLint *size, GLenum *type, GLcharARB *name);
typedef GLint (APIENTRY * PFNGLGETATTRIBLOCATIONARBPROC)(GLhandleARB programObj, const GLcharARB *name);

/* Extension-specific constants: */
#define GL_VERTEX_SHADER_ARB              0x8B31
#define GL_MAX_VERTEX_UNIFORM_COMPONENTS_ARB 0x8B4A
#define GL_MAX_VARYING_FLOATS_ARB         0x8B4B
#define GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS_ARB 0x8B4C
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS_ARB 0x8B4D
#define GL_OBJECT_ACTIVE_ATTRIBUTES_ARB   0x8B89
#define GL_OBJECT_ACTIVE_ATTRIBUTE_MAX_LENGTH_ARB 0x8B8A

#endif

/* Forward declarations of friend functions: */
void glBindAttribLocationARB(GLhandleARB programObj,GLuint index,const GLcharARB* name);
void glGetActiveAttribARB(GLhandleARB programObj,GLuint index,GLsizei maxLength,GLsizei* length,GLint* size,GLenum* type,GLcharARB* name);
GLint glGetAttribLocationARB(GLhandleARB programObj,const GLcharARB* name);

class GLARBVertexShader:public GLExtension
	{
	/* Elements: */
	private:
	static GL_THREAD_LOCAL(GLARBVertexShader*) current; // Pointer to extension object for current OpenGL context
	static const char* name; // Extension name
	PFNGLBINDATTRIBLOCATIONARBPROC glBindAttribLocationARBProc;
	PFNGLGETACTIVEATTRIBARBPROC glGetActiveAttribARBProc;
	PFNGLGETATTRIBLOCATIONARBPROC glGetAttribLocationARBProc;
	
	/* Constructors and destructors: */
	private:
	GLARBVertexShader(void);
	public:
	virtual ~GLARBVertexShader(void);
	
	/* Methods: */
	public:
	virtual const char* getExtensionName(void) const;
	virtual void activate(void);
	virtual void deactivate(void);
	static bool isSupported(void); // Returns true if the extension is supported in the current OpenGL context
	static void initExtension(void); // Initializes the extension in the current OpenGL context
	
	/* Extension entry points: */
	inline friend void glBindAttribLocationARB(GLhandleARB programObj,GLuint index,const GLcharARB* name)
		{
		GLARBVertexShader::current->glBindAttribLocationARBProc(programObj,index,name);
		}
	inline friend void glGetActiveAttribARB(GLhandleARB programObj,GLuint index,GLsizei maxLength,GLsizei* length,GLint* size,GLenum* type,GLcharARB* name)
		{
		GLARBVertexShader::current->glGetActiveAttribARBProc(programObj,index,maxLength,length,size,type,name);
		}
	inline friend GLint glGetAttribLocationARB(GLhandleARB programObj,const GLcharARB* name)
		{
		return GLARBVertexShader::current->glGetAttribLocationARBProc(programObj,name);
		}
	};

/*******************************
Extension-specific entry points:
*******************************/

/****************
Helper functions:
****************/

GLhandleARB glCompileVertexShaderFromString(const char* shaderSource); // Compiles a new vertex shader object from a C-style string; throws exception on errors
GLhandleARB glCompileVertexShaderFromStrings(size_t numShaderSources,...); // Compiles a new vertex shader object from a list of C-style strings; throws exception on errors
GLhandleARB glCompileVertexShaderFromFile(const char* shaderSourceFileName); // Compiles a new vertex shader object from a source file; throws exception on errors
GLhandleARB glCompileVertexShaderFromFile(const char* shaderSourceFileName,IO::File& shaderSourceFile); // Ditto, with already-opened IO::File object

#endif
