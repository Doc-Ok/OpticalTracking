/***********************************************************************
GLARBVertexBufferObject - OpenGL extension class for the
GL_ARB_vertex_buffer_object extension.
Copyright (c) 2005-2014 Oliver Kreylos

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

#ifndef GLEXTENSIONS_GLARBVERTEXBUFFEROBJECT_INCLUDED
#define GLEXTENSIONS_GLARBVERTEXBUFFEROBJECT_INCLUDED

#include <stddef.h>
#include <GL/gl.h>
#include <GL/TLSHelper.h>
#include <GL/Extensions/GLExtension.h>

/********************************
Extension-specific parts of gl.h:
********************************/

#ifndef GL_ARB_vertex_buffer_object
#define GL_ARB_vertex_buffer_object 1

/* Extension-specific types: */
typedef ptrdiff_t GLintptrARB;
typedef ptrdiff_t GLsizeiptrARB;

/* Extension-specific functions: */
typedef void (APIENTRY * PFNGLBINDBUFFERARBPROC)(GLenum target, GLuint buffer);
typedef void (APIENTRY * PFNGLDELETEBUFFERSARBPROC)(GLsizei n, const GLuint *buffers);
typedef void (APIENTRY * PFNGLGENBUFFERSARBPROC)(GLsizei n, GLuint *buffers);
typedef GLboolean (APIENTRY * PFNGLISBUFFERARBPROC)(GLuint buffer);
typedef void (APIENTRY * PFNGLBUFFERDATAARBPROC)(GLenum target, GLsizeiptrARB size, const GLvoid *data, GLenum usage);
typedef void (APIENTRY * PFNGLBUFFERSUBDATAARBPROC)(GLenum target, GLintptrARB offset, GLsizeiptrARB size, const GLvoid *data);
typedef void (APIENTRY * PFNGLGETBUFFERSUBDATAARBPROC)(GLenum target, GLintptrARB offset, GLsizeiptrARB size, GLvoid *data);
typedef void* (APIENTRY * PFNGLMAPBUFFERARBPROC)(GLenum target, GLenum access);
typedef GLboolean (APIENTRY * PFNGLUNMAPBUFFERARBPROC)(GLenum target);
typedef void (APIENTRY * PFNGLGETBUFFERPARAMETERIVARBPROC)(GLenum target, GLenum pname, GLint *params);
typedef void (APIENTRY * PFNGLGETBUFFERPOINTERVARBPROC)(GLenum target, GLenum pname, GLvoid **params);

/* Extension-specific constants: */
#define GL_ARRAY_BUFFER_ARB                             0x8892
#define GL_ELEMENT_ARRAY_BUFFER_ARB                     0x8893
#define GL_ARRAY_BUFFER_BINDING_ARB                     0x8894
#define GL_ELEMENT_ARRAY_BUFFER_BINDING_ARB             0x8895
#define GL_VERTEX_ARRAY_BUFFER_BINDING_ARB              0x8896
#define GL_NORMAL_ARRAY_BUFFER_BINDING_ARB              0x8897
#define GL_COLOR_ARRAY_BUFFER_BINDING_ARB               0x8898
#define GL_INDEX_ARRAY_BUFFER_BINDING_ARB               0x8899
#define GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING_ARB       0x889A
#define GL_EDGE_FLAG_ARRAY_BUFFER_BINDING_ARB           0x889B
#define GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING_ARB     0x889C
#define GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING_ARB      0x889D
#define GL_WEIGHT_ARRAY_BUFFER_BINDING_ARB              0x889E
#define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING_ARB       0x889F
#define GL_STREAM_DRAW_ARB                              0x88E0
#define GL_STREAM_READ_ARB                              0x88E1
#define GL_STREAM_COPY_ARB                              0x88E2
#define GL_STATIC_DRAW_ARB                              0x88E4
#define GL_STATIC_READ_ARB                              0x88E5
#define GL_STATIC_COPY_ARB                              0x88E6
#define GL_DYNAMIC_DRAW_ARB                             0x88E8
#define GL_DYNAMIC_READ_ARB                             0x88E9
#define GL_DYNAMIC_COPY_ARB                             0x88EA
#define GL_READ_ONLY_ARB                                0x88B8
#define GL_WRITE_ONLY_ARB                               0x88B9
#define GL_READ_WRITE_ARB                               0x88BA
#define GL_BUFFER_SIZE_ARB                              0x8764
#define GL_BUFFER_USAGE_ARB                             0x8765
#define GL_BUFFER_ACCESS_ARB                            0x88BB
#define GL_BUFFER_MAPPED_ARB                            0x88BC
#define GL_BUFFER_MAP_POINTER_ARB                       0x88BD

#endif

/* Forward declarations of friend functions: */
void glBindBufferARB(GLenum target,GLuint buffer);
void glDeleteBuffersARB(GLsizei n,const GLuint* buffers);
void glGenBuffersARB(GLsizei n,GLuint* buffers);
GLboolean glIsBufferARB(GLuint buffer);
void glBufferDataARB(GLenum target,GLsizeiptrARB size,const GLvoid* data,GLenum usage);
void glBufferSubDataARB(GLenum target,GLintptrARB offset,GLsizeiptrARB size,const GLvoid* data);
void glGetBufferSubDataARB(GLenum target,GLintptrARB offset,GLsizeiptrARB size,GLvoid* data);
GLvoid* glMapBufferARB(GLenum target,GLenum access);
GLboolean glUnmapBufferARB(GLenum target);
void glGetBufferParameterivARB(GLenum target,GLenum pname,GLint* params);
void glGetBufferPointervARB(GLenum target,GLenum pname,GLvoid** params);

class GLARBVertexBufferObject:public GLExtension
	{
	/* Elements: */
	private:
	static GL_THREAD_LOCAL(GLARBVertexBufferObject*) current; // Pointer to extension object for current OpenGL context
	static const char* name; // Extension name
	PFNGLBINDBUFFERARBPROC glBindBufferARBProc;
	PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARBProc;
	PFNGLGENBUFFERSARBPROC glGenBuffersARBProc;
	PFNGLISBUFFERARBPROC glIsBufferARBProc;
	PFNGLBUFFERDATAARBPROC glBufferDataARBProc;
	PFNGLBUFFERSUBDATAARBPROC glBufferSubDataARBProc;
	PFNGLGETBUFFERSUBDATAARBPROC glGetBufferSubDataARBProc;
	PFNGLMAPBUFFERARBPROC glMapBufferARBProc;
	PFNGLUNMAPBUFFERARBPROC glUnmapBufferARBProc;
	PFNGLGETBUFFERPARAMETERIVARBPROC glGetBufferParameterivARBProc;
	PFNGLGETBUFFERPOINTERVARBPROC glGetBufferPointervARBProc;
	
	/* Constructors and destructors: */
	private:
	GLARBVertexBufferObject(void);
	public:
	virtual ~GLARBVertexBufferObject(void);
	
	/* Methods: */
	public:
	virtual const char* getExtensionName(void) const;
	virtual void activate(void);
	virtual void deactivate(void);
	static bool isSupported(void); // Returns true if the extension is supported in the current OpenGL context
	static void initExtension(void); // Initializes the extension in the current OpenGL context
	
	/* Extension entry points: */
	inline friend void glBindBufferARB(GLenum target,GLuint buffer)
		{
		GLARBVertexBufferObject::current->glBindBufferARBProc(target,buffer);
		}
	inline friend void glDeleteBuffersARB(GLsizei n,const GLuint* buffers)
		{
		GLARBVertexBufferObject::current->glDeleteBuffersARBProc(n,buffers);
		}
	inline friend void glGenBuffersARB(GLsizei n,GLuint* buffers)
		{
		GLARBVertexBufferObject::current->glGenBuffersARBProc(n,buffers);
		}
	inline friend GLboolean glIsBufferARB(GLuint buffer)
		{
		return GLARBVertexBufferObject::current->glIsBufferARBProc(buffer);
		}
	inline friend void glBufferDataARB(GLenum target,GLsizeiptrARB size,const GLvoid* data,GLenum usage)
		{
		GLARBVertexBufferObject::current->glBufferDataARBProc(target,size,data,usage);
		}
	inline friend void glBufferSubDataARB(GLenum target,GLintptrARB offset,GLsizeiptrARB size,const GLvoid* data)
		{
		GLARBVertexBufferObject::current->glBufferSubDataARBProc(target,offset,size,data);
		}
	inline friend void glGetBufferSubDataARB(GLenum target,GLintptrARB offset,GLsizeiptrARB size,GLvoid* data)
		{
		GLARBVertexBufferObject::current->glGetBufferSubDataARBProc(target,offset,size,data);
		}
	inline friend GLvoid* glMapBufferARB(GLenum target,GLenum access)
		{
		return GLARBVertexBufferObject::current->glMapBufferARBProc(target,access);
		}
	inline friend GLboolean glUnmapBufferARB(GLenum target)
		{
		return GLARBVertexBufferObject::current->glUnmapBufferARBProc(target);
		}
	inline friend void glGetBufferParameterivARB(GLenum target,GLenum pname,GLint* params)
		{
		GLARBVertexBufferObject::current->glGetBufferParameterivARBProc(target,pname,params);
		}
	inline friend void glGetBufferPointervARB(GLenum target,GLenum pname,GLvoid** params)
		{
		GLARBVertexBufferObject::current->glGetBufferPointervARBProc(target,pname,params);
		}
	};

/*******************************
Extension-specific entry points:
*******************************/

#endif
