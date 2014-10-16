/***********************************************************************
GLNVOcclusionQuery - OpenGL extension class for the
GL_NV_occlusion_query extension.
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

#ifndef GLEXTENSIONS_GLNVOCCLUSIONQUERY_INCLUDED
#define GLEXTENSIONS_GLNVOCCLUSIONQUERY_INCLUDED

#include <GL/gl.h>
#include <GL/TLSHelper.h>
#include <GL/Extensions/GLExtension.h>

/********************************
Extension-specific parts of gl.h:
********************************/

#ifndef GL_NV_occlusion_query
#define GL_NV_occlusion_query 1

/* Extension-specific functions: */
typedef void (APIENTRY * PFNGLGENOCCLUSIONQUERIESNVPROC) (GLsizei n, GLuint *ids);
typedef void (APIENTRY * PFNGLDELETEOCCLUSIONQUERIESNVPROC) (GLsizei n, const GLuint *ids);
typedef GLboolean (APIENTRY * PFNGLISOCCLUSIONQUERYNVPROC) (GLuint id);
typedef void (APIENTRY * PFNGLBEGINOCCLUSIONQUERYNVPROC) (GLuint id);
typedef void (APIENTRY * PFNGLENDOCCLUSIONQUERYNVPROC) (GLvoid);
typedef void (APIENTRY * PFNGLGETOCCLUSIONQUERYIVNVPROC) (GLuint id, GLenum pname, GLint *params);
typedef void (APIENTRY * PFNGLGETOCCLUSIONQUERYUIVNVPROC) (GLuint id, GLenum pname, GLuint *params);

/* Extension-specific constants: */
#define GL_PIXEL_COUNTER_BITS_NV          0x8864
#define GL_CURRENT_OCCLUSION_QUERY_ID_NV  0x8865
#define GL_PIXEL_COUNT_NV                 0x8866
#define GL_PIXEL_COUNT_AVAILABLE_NV       0x8867

#endif

/* Forward declarations of friend functions: */
void glGenOcclusionQueriesNV(GLsizei n,GLuint* ids);
void glDeleteOcclusionQueriesNV(GLsizei n,const GLuint* ids);
GLboolean glIsOcclusionQueryNV(GLuint id);
void glBeginOcclusionQueryNV(GLuint id);
void glEndOcclusionQueryNV(void);
void glGetOcclusionQueryivNV(GLuint id,GLenum pname,GLint* params);
void glGetOcclusionQueryuivNV(GLuint id,GLenum pname,GLuint* params);

class GLNVOcclusionQuery:public GLExtension
	{
	/* Elements: */
	private:
	static GL_THREAD_LOCAL(GLNVOcclusionQuery*) current; // Pointer to extension object for current OpenGL context
	static const char* name; // Extension name
	PFNGLGENOCCLUSIONQUERIESNVPROC glGenOcclusionQueriesNVProc;
	PFNGLDELETEOCCLUSIONQUERIESNVPROC glDeleteOcclusionQueriesNVProc;
	PFNGLISOCCLUSIONQUERYNVPROC glIsOcclusionQueryNVProc;
	PFNGLBEGINOCCLUSIONQUERYNVPROC glBeginOcclusionQueryNVProc;
	PFNGLENDOCCLUSIONQUERYNVPROC glEndOcclusionQueryNVProc;
	PFNGLGETOCCLUSIONQUERYIVNVPROC glGetOcclusionQueryivNVProc;
	PFNGLGETOCCLUSIONQUERYUIVNVPROC glGetOcclusionQueryuivNVProc;
	
	/* Constructors and destructors: */
	private:
	GLNVOcclusionQuery(void);
	public:
	virtual ~GLNVOcclusionQuery(void);
	
	/* Methods: */
	public:
	virtual const char* getExtensionName(void) const;
	virtual void activate(void);
	virtual void deactivate(void);
	static bool isSupported(void); // Returns true if the extension is supported in the current OpenGL context
	static void initExtension(void); // Initializes the extension in the current OpenGL context
	
	/* Extension entry points: */
	inline friend void glGenOcclusionQueriesNV(GLsizei n,GLuint* ids)
		{
		GLNVOcclusionQuery::current->glGenOcclusionQueriesNVProc(n,ids);
		}
	inline friend void glDeleteOcclusionQueriesNV(GLsizei n,const GLuint* ids)
		{
		GLNVOcclusionQuery::current->glDeleteOcclusionQueriesNVProc(n,ids);
		}
	inline friend GLboolean glIsOcclusionQueryNV(GLuint id)
		{
		return GLNVOcclusionQuery::current->glIsOcclusionQueryNVProc(id);
		}
	inline friend void glBeginOcclusionQueryNV(GLuint id)
		{
		GLNVOcclusionQuery::current->glBeginOcclusionQueryNVProc(id);
		}
	inline friend void glEndOcclusionQueryNV(void)
		{
		GLNVOcclusionQuery::current->glEndOcclusionQueryNVProc();
		}
	inline friend void glGetOcclusionQueryivNV(GLuint id,GLenum pname,GLint* params)
		{
		GLNVOcclusionQuery::current->glGetOcclusionQueryivNVProc(id,pname,params);
		}
	inline friend void glGetOcclusionQueryuivNV(GLuint id,GLenum pname,GLuint* params)
		{
		GLNVOcclusionQuery::current->glGetOcclusionQueryuivNVProc(id,pname,params);
		}
	};

/*******************************
Extension-specific entry points:
*******************************/

#endif
