/***********************************************************************
GLARBDebugOutput - OpenGL extension class for the GL_ARB_debug_output
extension.
Copyright (c) 2014 Oliver Kreylos

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

#ifndef GLEXTENSIONS_GLARBDEBUGOUTPUT_INCLUDED
#define GLEXTENSIONS_GLARBDEBUGOUTPUT_INCLUDED

#include <GL/gl.h>
#include <GL/TLSHelper.h>
#include <GL/Extensions/GLExtension.h>

/********************************
Extension-specific parts of gl.h:
********************************/

#ifndef GL_ARB_debug_output
#define GL_ARB_debug_output 1

/* Extension-specific functions: */
typedef void (APIENTRY *GLDEBUGPROCARB)(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,const void *userParam);
typedef void (APIENTRYP PFNGLDEBUGMESSAGECONTROLARBPROC) (GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled);
typedef void (APIENTRYP PFNGLDEBUGMESSAGEINSERTARBPROC) (GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf);
typedef void (APIENTRYP PFNGLDEBUGMESSAGECALLBACKARBPROC) (GLDEBUGPROCARB callback, const void *userParam);
typedef GLuint (APIENTRYP PFNGLGETDEBUGMESSAGELOGARBPROC) (GLuint count, GLsizei bufsize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog);

/* Extension-specific constants: */
#define GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB   0x8242
#define GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH_ARB 0x8243
#define GL_DEBUG_CALLBACK_FUNCTION_ARB    0x8244
#define GL_DEBUG_CALLBACK_USER_PARAM_ARB  0x8245
#define GL_DEBUG_SOURCE_API_ARB           0x8246
#define GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB 0x8247
#define GL_DEBUG_SOURCE_SHADER_COMPILER_ARB 0x8248
#define GL_DEBUG_SOURCE_THIRD_PARTY_ARB   0x8249
#define GL_DEBUG_SOURCE_APPLICATION_ARB   0x824A
#define GL_DEBUG_SOURCE_OTHER_ARB         0x824B
#define GL_DEBUG_TYPE_ERROR_ARB           0x824C
#define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB 0x824D
#define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB 0x824E
#define GL_DEBUG_TYPE_PORTABILITY_ARB     0x824F
#define GL_DEBUG_TYPE_PERFORMANCE_ARB     0x8250
#define GL_DEBUG_TYPE_OTHER_ARB           0x8251
#define GL_MAX_DEBUG_MESSAGE_LENGTH_ARB   0x9143
#define GL_MAX_DEBUG_LOGGED_MESSAGES_ARB  0x9144
#define GL_DEBUG_LOGGED_MESSAGES_ARB      0x9145
#define GL_DEBUG_SEVERITY_HIGH_ARB        0x9146
#define GL_DEBUG_SEVERITY_MEDIUM_ARB      0x9147
#define GL_DEBUG_SEVERITY_LOW_ARB         0x9148

#endif

/* Forward declarations of friend functions: */
void glDebugMessageControlARB(GLenum source,GLenum type,GLenum severity,GLsizei count,const GLuint* ids,GLboolean enabled);
void glDebugMessageInsertARB(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar* buf);
void glDebugMessageCallbackARB(GLDEBUGPROCARB callback,const void* userParam);
GLuint glGetDebugMessageLogARB(GLuint count,GLsizei bufsize,GLenum* sources,GLenum* types,GLuint* ids,GLenum* severities,GLsizei* lengths,GLchar* messageLog);

class GLARBDebugOutput:public GLExtension
	{
	/* Elements: */
	private:
	static GL_THREAD_LOCAL(GLARBDebugOutput*) current; // Pointer to extension object for current OpenGL context
	static const char* name; // Extension name
	PFNGLDEBUGMESSAGECONTROLARBPROC glDebugMessageControlARBProc;
	PFNGLDEBUGMESSAGEINSERTARBPROC glDebugMessageInsertARBProc;
	PFNGLDEBUGMESSAGECALLBACKARBPROC glDebugMessageCallbackARBProc;
	PFNGLGETDEBUGMESSAGELOGARBPROC glGetDebugMessageLogARBProc;
	
	/* Constructors and destructors: */
	private:
	GLARBDebugOutput(void);
	public:
	virtual ~GLARBDebugOutput(void);
	
	/* Methods: */
	public:
	virtual const char* getExtensionName(void) const;
	virtual void activate(void);
	virtual void deactivate(void);
	static bool isSupported(void); // Returns true if the extension is supported in the current OpenGL context
	static void initExtension(void); // Initializes the extension in the current OpenGL context
	
	/* Extension entry points: */
	inline friend void glDebugMessageControlARB(GLenum source,GLenum type,GLenum severity,GLsizei count,const GLuint* ids,GLboolean enabled)
		{
		GLARBDebugOutput::current->glDebugMessageControlARBProc(source,type,severity,count,ids,enabled);
		}
	inline friend void glDebugMessageInsertARB(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar* buf)
		{
		GLARBDebugOutput::current->glDebugMessageInsertARBProc(source,type,id,severity,length,buf);
		}
	inline friend void glDebugMessageCallbackARB(GLDEBUGPROCARB callback,const void* userParam)
		{
		GLARBDebugOutput::current->glDebugMessageCallbackARBProc(callback,userParam);
		}
	inline friend GLuint glGetDebugMessageLogARB(GLuint count,GLsizei bufsize,GLenum* sources,GLenum* types,GLuint* ids,GLenum* severities,GLsizei* lengths,GLchar* messageLog)
		{
		return GLARBDebugOutput::current->glGetDebugMessageLogARBProc(count,bufsize,sources,types,ids,severities,lengths,messageLog);
		}
	};

/*******************************
Extension-specific entry points:
*******************************/

#endif
