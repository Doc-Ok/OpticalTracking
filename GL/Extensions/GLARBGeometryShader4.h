/***********************************************************************
GLARBGeometryShader4 - OpenGL extension class for the
GL_ARB_geometry_shader4 extension.
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

#ifndef GLEXTENSIONS_GLARBGEOMETRYSHADER4_INCLUDED
#define GLEXTENSIONS_GLARBGEOMETRYSHADER4_INCLUDED

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

#ifndef GL_ARB_geometry_shader4
#define GL_ARB_geometry_shader4 1

/* Extension-specific functions: */
typedef void (APIENTRY * PFNGLPROGRAMPARAMETERIARBPROC) (GLuint program, GLenum pname, GLint value);
typedef void (APIENTRY * PFNGLFRAMEBUFFERTEXTUREARBPROC) (GLenum target, GLenum attachment, GLuint texture, GLint level);
typedef void (APIENTRY * PFNGLFRAMEBUFFERTEXTURELAYERARBPROC) (GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
typedef void (APIENTRY * PFNGLFRAMEBUFFERTEXTUREFACEARBPROC) (GLenum target, GLenum attachment, GLuint texture, GLint level, GLenum face);

/* Extension-specific constants: */
#define GL_GEOMETRY_SHADER_ARB            0x8DD9
#define GL_MAX_GEOMETRY_VARYING_COMPONENTS_ARB 0x8DDD
#define GL_MAX_VERTEX_VARYING_COMPONENTS_ARB 0x8DDE
#define GL_MAX_VARYING_COMPONENTS_ARB     0x8B4B
#define GL_MAX_GEOMETRY_UNIFORM_COMPONENTS_ARB 0x8DDF
#define GL_MAX_GEOMETRY_OUTPUT_VERTICES_ARB 0x8DE0
#define GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_ARB 0x8DE1
#define GL_GEOMETRY_VERTICES_OUT_ARB      0x8DDA
#define GL_GEOMETRY_INPUT_TYPE_ARB        0x8DDB
#define GL_GEOMETRY_OUTPUT_TYPE_ARB       0x8DDC
#define GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_ARB 0x8C29
#define GL_LINES_ADJACENCY_ARB            0xA
#define GL_LINE_STRIP_ADJACENCY_ARB       0xB
#define GL_TRIANGLES_ADJACENCY_ARB        0xC
#define GL_TRIANGLE_STRIP_ADJACENCY_ARB   0xD
#define GL_FRAMEBUFFER_ATTACHMENT_LAYERED_ARB 0x8DA7
#define GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS_ARB 0x8DA8
#define GL_FRAMEBUFFER_INCOMPLETE_LAYER_COUNT_ARB 0x8DA9
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER_ARB 0x8CD4
#define GL_PROGRAM_POINT_SIZE_ARB         0x8642

#endif

/* Forward declarations of friend functions: */
void glProgramParameteriARB(GLuint program,GLenum pname,GLint value);
void glFramebufferTextureARB(GLenum target,GLenum attachment,GLuint texture,GLint level);
void glFramebufferTextureLayerARB(GLenum target,GLenum attachment,GLuint texture,GLint level,GLint layer);
void glFramebufferTextureFaceARB(GLenum target,GLenum attachment,GLuint texture,GLint level,GLenum face);

class GLARBGeometryShader4:public GLExtension
	{
	/* Elements: */
	private:
	static GL_THREAD_LOCAL(GLARBGeometryShader4*) current; // Pointer to extension object for current OpenGL context
	static const char* name; // Extension name
	PFNGLPROGRAMPARAMETERIARBPROC glProgramParameteriARBProc;
	PFNGLFRAMEBUFFERTEXTUREARBPROC glFramebufferTextureARBProc;
	PFNGLFRAMEBUFFERTEXTURELAYERARBPROC glFramebufferTextureLayerARBProc;
	PFNGLFRAMEBUFFERTEXTUREFACEARBPROC glFramebufferTextureFaceARBProc;
	
	/* Constructors and destructors: */
	private:
	GLARBGeometryShader4(void);
	public:
	virtual ~GLARBGeometryShader4(void);
	
	/* Methods: */
	public:
	virtual const char* getExtensionName(void) const;
	virtual void activate(void);
	virtual void deactivate(void);
	static bool isSupported(void); // Returns true if the extension is supported in the current OpenGL context
	static void initExtension(void); // Initializes the extension in the current OpenGL context
	
	/* Extension entry points: */
	inline friend void glProgramParameteriARB(GLuint program,GLenum pname,GLint value)
		{
		GLARBGeometryShader4::current->glProgramParameteriARBProc(program,pname,value);
		}
	inline friend void glFramebufferTextureARB(GLenum target,GLenum attachment,GLuint texture,GLint level)
		{
		GLARBGeometryShader4::current->glFramebufferTextureARBProc(target,attachment,texture,level);
		}
	inline friend void glFramebufferTextureLayerARB(GLenum target,GLenum attachment,GLuint texture,GLint level,GLint layer)
		{
		GLARBGeometryShader4::current->glFramebufferTextureLayerARBProc(target,attachment,texture,level,layer);
		}
	inline friend void glFramebufferTextureFaceARB(GLenum target,GLenum attachment,GLuint texture,GLint level,GLenum face)
		{
		GLARBGeometryShader4::current->glFramebufferTextureFaceARBProc(target, attachment, texture, level, face);
		}
	};

/*******************************
Extension-specific entry points:
*******************************/

/****************
Helper functions:
****************/

GLhandleARB glCompileARBGeometryShader4FromString(const char* shaderSource); // Compiles a new geometry shader object from a C-style string; throws exception on errors
GLhandleARB glCompileARBGeometryShader4FromStrings(size_t numShaderSources,...); // Compiles a new geometry shader object from a C-style string; throws exception on errors
GLhandleARB glCompileARBGeometryShader4FromFile(const char* shaderSourceFileName); // Compiles a new geometry shader object from a source file; throws exception on errors
GLhandleARB glCompileARBGeometryShader4FromFile(const char* shaderSourceFileName,IO::File& shaderSourceFile); // Ditto, with already-opened IO::File object

#endif
