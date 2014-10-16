/***********************************************************************
GLARBShaderObjects - OpenGL extension class for the
GL_ARB_shader_objects extension.
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

#ifndef GLEXTENSIONS_GLARBSHADEROBJECTS_INCLUDED
#define GLEXTENSIONS_GLARBSHADEROBJECTS_INCLUDED

#include <stddef.h>
#include <stdarg.h>
#include <vector>
#include <GL/gl.h>
#include <GL/TLSHelper.h>
#include <GL/Extensions/GLExtension.h>

/* Forward declarations: */
namespace IO {
class File;
}

/********************************
Extension-specific parts of gl.h:
********************************/

#ifndef GL_ARB_shader_objects
#define GL_ARB_shader_objects 1

/* Extension-specific types: */
typedef char GLcharARB;		/* native character */
typedef unsigned int GLhandleARB;	/* shader object handle */

/* Extension-specific functions: */
typedef void (APIENTRY * PFNGLDELETEOBJECTARBPROC)(GLhandleARB obj);
typedef GLhandleARB (APIENTRY * PFNGLGETHANDLEARBPROC)(GLenum pname);
typedef void (APIENTRY * PFNGLDETACHOBJECTARBPROC)(GLhandleARB containerObj, GLhandleARB attachedObj);
typedef GLhandleARB (APIENTRY * PFNGLCREATESHADEROBJECTARBPROC)(GLenum shaderType);
typedef void (APIENTRY * PFNGLSHADERSOURCEARBPROC)(GLhandleARB shaderObj, GLsizei count, const GLcharARB* *string, const GLint *length);
typedef void (APIENTRY * PFNGLCOMPILESHADERARBPROC)(GLhandleARB shaderObj);
typedef GLhandleARB (APIENTRY * PFNGLCREATEPROGRAMOBJECTARBPROC)(void);
typedef void (APIENTRY * PFNGLATTACHOBJECTARBPROC)(GLhandleARB containerObj, GLhandleARB obj);
typedef void (APIENTRY * PFNGLLINKPROGRAMARBPROC)(GLhandleARB programObj);
typedef void (APIENTRY * PFNGLUSEPROGRAMOBJECTARBPROC)(GLhandleARB programObj);
typedef void (APIENTRY * PFNGLVALIDATEPROGRAMARBPROC)(GLhandleARB programObj);
typedef void (APIENTRY * PFNGLUNIFORM1FARBPROC)(GLint location, GLfloat v0);
typedef void (APIENTRY * PFNGLUNIFORM2FARBPROC)(GLint location, GLfloat v0, GLfloat v1);
typedef void (APIENTRY * PFNGLUNIFORM3FARBPROC)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void (APIENTRY * PFNGLUNIFORM4FARBPROC)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void (APIENTRY * PFNGLUNIFORM1IARBPROC)(GLint location, GLint v0);
typedef void (APIENTRY * PFNGLUNIFORM2IARBPROC)(GLint location, GLint v0, GLint v1);
typedef void (APIENTRY * PFNGLUNIFORM3IARBPROC)(GLint location, GLint v0, GLint v1, GLint v2);
typedef void (APIENTRY * PFNGLUNIFORM4IARBPROC)(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
typedef void (APIENTRY * PFNGLUNIFORM1FVARBPROC)(GLint location, GLsizei count, const GLfloat *value);
typedef void (APIENTRY * PFNGLUNIFORM2FVARBPROC)(GLint location, GLsizei count, const GLfloat *value);
typedef void (APIENTRY * PFNGLUNIFORM3FVARBPROC)(GLint location, GLsizei count, const GLfloat *value);
typedef void (APIENTRY * PFNGLUNIFORM4FVARBPROC)(GLint location, GLsizei count, const GLfloat *value);
typedef void (APIENTRY * PFNGLUNIFORM1IVARBPROC)(GLint location, GLsizei count, const GLint *value);
typedef void (APIENTRY * PFNGLUNIFORM2IVARBPROC)(GLint location, GLsizei count, const GLint *value);
typedef void (APIENTRY * PFNGLUNIFORM3IVARBPROC)(GLint location, GLsizei count, const GLint *value);
typedef void (APIENTRY * PFNGLUNIFORM4IVARBPROC)(GLint location, GLsizei count, const GLint *value);
typedef void (APIENTRY * PFNGLUNIFORMMATRIX2FVARBPROC)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRY * PFNGLUNIFORMMATRIX3FVARBPROC)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRY * PFNGLUNIFORMMATRIX4FVARBPROC)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRY * PFNGLGETOBJECTPARAMETERFVARBPROC)(GLhandleARB obj, GLenum pname, GLfloat *params);
typedef void (APIENTRY * PFNGLGETOBJECTPARAMETERIVARBPROC)(GLhandleARB obj, GLenum pname, GLint *params);
typedef void (APIENTRY * PFNGLGETINFOLOGARBPROC)(GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *infoLog);
typedef void (APIENTRY * PFNGLGETATTACHEDOBJECTSARBPROC)(GLhandleARB containerObj, GLsizei maxCount, GLsizei *count, GLhandleARB *obj);
typedef GLint (APIENTRY * PFNGLGETUNIFORMLOCATIONARBPROC)(GLhandleARB programObj, const GLcharARB *name);
typedef void (APIENTRY * PFNGLGETACTIVEUNIFORMARBPROC)(GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei *length, GLint *size, GLenum *type, GLcharARB *name);
typedef void (APIENTRY * PFNGLGETUNIFORMFVARBPROC)(GLhandleARB programObj, GLint location, GLfloat *params);
typedef void (APIENTRY * PFNGLGETUNIFORMIVARBPROC)(GLhandleARB programObj, GLint location, GLint *params);
typedef void (APIENTRY * PFNGLGETSHADERSOURCEARBPROC)(GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *source);

/* Extension-specific constants: */
#define GL_PROGRAM_OBJECT_ARB             0x8B40
#define GL_SHADER_OBJECT_ARB              0x8B48
#define GL_OBJECT_TYPE_ARB                0x8B4E
#define GL_OBJECT_SUBTYPE_ARB             0x8B4F
#define GL_FLOAT_VEC2_ARB                 0x8B50
#define GL_FLOAT_VEC3_ARB                 0x8B51
#define GL_FLOAT_VEC4_ARB                 0x8B52
#define GL_INT_VEC2_ARB                   0x8B53
#define GL_INT_VEC3_ARB                   0x8B54
#define GL_INT_VEC4_ARB                   0x8B55
#define GL_BOOL_ARB                       0x8B56
#define GL_BOOL_VEC2_ARB                  0x8B57
#define GL_BOOL_VEC3_ARB                  0x8B58
#define GL_BOOL_VEC4_ARB                  0x8B59
#define GL_FLOAT_MAT2_ARB                 0x8B5A
#define GL_FLOAT_MAT3_ARB                 0x8B5B
#define GL_FLOAT_MAT4_ARB                 0x8B5C
#define GL_SAMPLER_1D_ARB                 0x8B5D
#define GL_SAMPLER_2D_ARB                 0x8B5E
#define GL_SAMPLER_3D_ARB                 0x8B5F
#define GL_SAMPLER_CUBE_ARB               0x8B60
#define GL_SAMPLER_1D_SHADOW_ARB          0x8B61
#define GL_SAMPLER_2D_SHADOW_ARB          0x8B62
#define GL_SAMPLER_2D_RECT_ARB            0x8B63
#define GL_SAMPLER_2D_RECT_SHADOW_ARB     0x8B64
#define GL_OBJECT_DELETE_STATUS_ARB       0x8B80
#define GL_OBJECT_COMPILE_STATUS_ARB      0x8B81
#define GL_OBJECT_LINK_STATUS_ARB         0x8B82
#define GL_OBJECT_VALIDATE_STATUS_ARB     0x8B83
#define GL_OBJECT_INFO_LOG_LENGTH_ARB     0x8B84
#define GL_OBJECT_ATTACHED_OBJECTS_ARB    0x8B85
#define GL_OBJECT_ACTIVE_UNIFORMS_ARB     0x8B86
#define GL_OBJECT_ACTIVE_UNIFORM_MAX_LENGTH_ARB 0x8B87
#define GL_OBJECT_SHADER_SOURCE_LENGTH_ARB 0x8B88

#endif

/* Forward declarations of friend functions: */
void glDeleteObjectARB(GLhandleARB obj);
GLhandleARB glGetHandleARB(GLenum pname);
void glDetachObjectARB(GLhandleARB containerObj,GLhandleARB attachedObj);
GLhandleARB glCreateShaderObjectARB(GLenum shaderType);
void glShaderSourceARB(GLhandleARB shaderObj,GLsizei count,const GLcharARB** string,const GLint* length);
void glCompileShaderARB(GLhandleARB shaderObj);
GLhandleARB glCreateProgramObjectARB(void);
void glAttachObjectARB(GLhandleARB containerObj,GLhandleARB obj);
void glLinkProgramARB(GLhandleARB programObj);
void glUseProgramObjectARB(GLhandleARB programObj);
void glValidateProgramARB(GLhandleARB programObj);
void glUniform1fARB(GLint location,GLfloat v0);
void glUniform2fARB(GLint location,GLfloat v0,GLfloat v1);
void glUniform3fARB(GLint location,GLfloat v0,GLfloat v1,GLfloat v2);
void glUniform4fARB(GLint location,GLfloat v0,GLfloat v1,GLfloat v2,GLfloat v3);
void glUniform1iARB(GLint location,GLint v0);
void glUniform2iARB(GLint location,GLint v0,GLint v1);
void glUniform3iARB(GLint location,GLint v0,GLint v1,GLint v2);
void glUniform4iARB(GLint location,GLint v0,GLint v1,GLint v2,GLint v3);
void glUniform1fvARB(GLint location,GLsizei count,const GLfloat* value);
void glUniform2fvARB(GLint location,GLsizei count,const GLfloat* value);
void glUniform3fvARB(GLint location,GLsizei count,const GLfloat* value);
void glUniform4fvARB(GLint location,GLsizei count,const GLfloat* value);
void glUniform1ivARB(GLint location,GLsizei count,const GLint* value);
void glUniform2ivARB(GLint location,GLsizei count,const GLint* value);
void glUniform3ivARB(GLint location,GLsizei count,const GLint* value);
void glUniform4ivARB(GLint location,GLsizei count,const GLint* value);
void glUniformMatrix2fvARB(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value);
void glUniformMatrix3fvARB(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value);
void glUniformMatrix4fvARB(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value);
void glGetObjectParameterfvARB(GLhandleARB obj,GLenum pname,GLfloat* params);
void glGetObjectParameterivARB(GLhandleARB obj,GLenum pname,GLint* params);
void glGetInfoLogARB(GLhandleARB obj,GLsizei maxLength,GLsizei* length,GLcharARB* infoLog);
void glGetAttachedObjectsARB(GLhandleARB containerObj,GLsizei maxCount,GLsizei* count,GLhandleARB* obj);
GLint glGetUniformLocationARB(GLhandleARB programObj,const GLcharARB* name);
void glGetActiveUniformARB(GLhandleARB programObj,GLuint index,GLsizei maxLength,GLsizei* length,GLint* size,GLenum* type,GLcharARB* name);
void glGetUniformfvARB(GLhandleARB programObj,GLint location,GLfloat* params);
void glGetUniformivARB(GLhandleARB programObj,GLint location,GLint* params);
void glGetShaderSourceARB(GLhandleARB obj,GLsizei maxLength,GLsizei* length,GLcharARB* source);

class GLARBShaderObjects:public GLExtension
	{
	/* Elements: */
	private:
	static GL_THREAD_LOCAL(GLARBShaderObjects*) current; // Pointer to extension object for current OpenGL context
	static const char* name; // Extension name
	PFNGLDELETEOBJECTARBPROC glDeleteObjectARBProc;
	PFNGLGETHANDLEARBPROC glGetHandleARBProc;
	PFNGLDETACHOBJECTARBPROC glDetachObjectARBProc;
	PFNGLCREATESHADEROBJECTARBPROC glCreateShaderObjectARBProc;
	PFNGLSHADERSOURCEARBPROC glShaderSourceARBProc;
	PFNGLCOMPILESHADERARBPROC glCompileShaderARBProc;
	PFNGLCREATEPROGRAMOBJECTARBPROC glCreateProgramObjectARBProc;
	PFNGLATTACHOBJECTARBPROC glAttachObjectARBProc;
	PFNGLLINKPROGRAMARBPROC glLinkProgramARBProc;
	PFNGLUSEPROGRAMOBJECTARBPROC glUseProgramObjectARBProc;
	PFNGLVALIDATEPROGRAMARBPROC glValidateProgramARBProc;
	PFNGLUNIFORM1FARBPROC glUniform1fARBProc;
	PFNGLUNIFORM2FARBPROC glUniform2fARBProc;
	PFNGLUNIFORM3FARBPROC glUniform3fARBProc;
	PFNGLUNIFORM4FARBPROC glUniform4fARBProc;
	PFNGLUNIFORM1IARBPROC glUniform1iARBProc;
	PFNGLUNIFORM2IARBPROC glUniform2iARBProc;
	PFNGLUNIFORM3IARBPROC glUniform3iARBProc;
	PFNGLUNIFORM4IARBPROC glUniform4iARBProc;
	PFNGLUNIFORM1FVARBPROC glUniform1fvARBProc;
	PFNGLUNIFORM2FVARBPROC glUniform2fvARBProc;
	PFNGLUNIFORM3FVARBPROC glUniform3fvARBProc;
	PFNGLUNIFORM4FVARBPROC glUniform4fvARBProc;
	PFNGLUNIFORM1IVARBPROC glUniform1ivARBProc;
	PFNGLUNIFORM2IVARBPROC glUniform2ivARBProc;
	PFNGLUNIFORM3IVARBPROC glUniform3ivARBProc;
	PFNGLUNIFORM4IVARBPROC glUniform4ivARBProc;
	PFNGLUNIFORMMATRIX2FVARBPROC glUniformMatrix2fvARBProc;
	PFNGLUNIFORMMATRIX3FVARBPROC glUniformMatrix3fvARBProc;
	PFNGLUNIFORMMATRIX4FVARBPROC glUniformMatrix4fvARBProc;
	PFNGLGETOBJECTPARAMETERFVARBPROC glGetObjectParameterfvARBProc;
	PFNGLGETOBJECTPARAMETERIVARBPROC glGetObjectParameterivARBProc;
	PFNGLGETINFOLOGARBPROC glGetInfoLogARBProc;
	PFNGLGETATTACHEDOBJECTSARBPROC glGetAttachedObjectsARBProc;
	PFNGLGETUNIFORMLOCATIONARBPROC glGetUniformLocationARBProc;
	PFNGLGETACTIVEUNIFORMARBPROC glGetActiveUniformARBProc;
	PFNGLGETUNIFORMFVARBPROC glGetUniformfvARBProc;
	PFNGLGETUNIFORMIVARBPROC glGetUniformivARBProc;
	PFNGLGETSHADERSOURCEARBPROC glGetShaderSourceARBProc;
	
	/* Constructors and destructors: */
	private:
	GLARBShaderObjects(void);
	public:
	virtual ~GLARBShaderObjects(void);
	
	/* Methods: */
	public:
	virtual const char* getExtensionName(void) const;
	virtual void activate(void);
	virtual void deactivate(void);
	static bool isSupported(void); // Returns true if the extension is supported in the current OpenGL context
	static void initExtension(void); // Initializes the extension in the current OpenGL context
	
	/* Extension entry points: */
	inline friend void glDeleteObjectARB(GLhandleARB obj)
		{
		GLARBShaderObjects::current->glDeleteObjectARBProc(obj);
		}
	inline friend GLhandleARB glGetHandleARB(GLenum pname)
		{
		return GLARBShaderObjects::current->glGetHandleARBProc(pname);
		}
	inline friend void glDetachObjectARB(GLhandleARB containerObj,GLhandleARB attachedObj)
		{
		GLARBShaderObjects::current->glDetachObjectARBProc(containerObj,attachedObj);
		}
	inline friend GLhandleARB glCreateShaderObjectARB(GLenum shaderType)
		{
		return GLARBShaderObjects::current->glCreateShaderObjectARBProc(shaderType);
		}
	inline friend void glShaderSourceARB(GLhandleARB shaderObj,GLsizei count,const GLcharARB** string,const GLint* length)
		{
		GLARBShaderObjects::current->glShaderSourceARBProc(shaderObj,count,string,length);
		}
	inline friend void glCompileShaderARB(GLhandleARB shaderObj)
		{
		GLARBShaderObjects::current->glCompileShaderARBProc(shaderObj);
		}
	inline friend GLhandleARB glCreateProgramObjectARB(void)
		{
		return GLARBShaderObjects::current->glCreateProgramObjectARBProc();
		}
	inline friend void glAttachObjectARB(GLhandleARB containerObj,GLhandleARB obj)
		{
		GLARBShaderObjects::current->glAttachObjectARBProc(containerObj,obj);
		}
	inline friend void glLinkProgramARB(GLhandleARB programObj)
		{
		GLARBShaderObjects::current->glLinkProgramARBProc(programObj);
		}
	inline friend void glUseProgramObjectARB(GLhandleARB programObj)
		{
		GLARBShaderObjects::current->glUseProgramObjectARBProc(programObj);
		}
	inline friend void glValidateProgramARB(GLhandleARB programObj)
		{
		GLARBShaderObjects::current->glValidateProgramARBProc(programObj);
		}
	inline friend void glUniform1fARB(GLint location,GLfloat v0)
		{
		GLARBShaderObjects::current->glUniform1fARBProc(location,v0);
		}
	inline friend void glUniform2fARB(GLint location,GLfloat v0,GLfloat v1)
		{
		GLARBShaderObjects::current->glUniform2fARBProc(location,v0,v1);
		}
	inline friend void glUniform3fARB(GLint location,GLfloat v0,GLfloat v1,GLfloat v2)
		{
		GLARBShaderObjects::current->glUniform3fARBProc(location,v0,v1,v2);
		}
	inline friend void glUniform4fARB(GLint location,GLfloat v0,GLfloat v1,GLfloat v2,GLfloat v3)
		{
		GLARBShaderObjects::current->glUniform4fARBProc(location,v0,v1,v2,v3);
		}
	inline friend void glUniform1iARB(GLint location,GLint v0)
		{
		GLARBShaderObjects::current->glUniform1iARBProc(location,v0);
		}
	inline friend void glUniform2iARB(GLint location,GLint v0,GLint v1)
		{
		GLARBShaderObjects::current->glUniform2iARBProc(location,v0,v1);
		}
	inline friend void glUniform3iARB(GLint location,GLint v0,GLint v1,GLint v2)
		{
		GLARBShaderObjects::current->glUniform3iARBProc(location,v0,v1,v2);
		}
	inline friend void glUniform4iARB(GLint location,GLint v0,GLint v1,GLint v2,GLint v3)
		{
		GLARBShaderObjects::current->glUniform4iARBProc(location,v0,v1,v2,v3);
		}
	inline friend void glUniform1fvARB(GLint location,GLsizei count,const GLfloat* value)
		{
		GLARBShaderObjects::current->glUniform1fvARBProc(location,count,value);
		}
	inline friend void glUniform2fvARB(GLint location,GLsizei count,const GLfloat* value)
		{
		GLARBShaderObjects::current->glUniform2fvARBProc(location,count,value);
		}
	inline friend void glUniform3fvARB(GLint location,GLsizei count,const GLfloat* value)
		{
		GLARBShaderObjects::current->glUniform3fvARBProc(location,count,value);
		}
	inline friend void glUniform4fvARB(GLint location,GLsizei count,const GLfloat* value)
		{
		GLARBShaderObjects::current->glUniform4fvARBProc(location,count,value);
		}
	inline friend void glUniform1ivARB(GLint location,GLsizei count,const GLint* value)
		{
		GLARBShaderObjects::current->glUniform1ivARBProc(location,count,value);
		}
	inline friend void glUniform2ivARB(GLint location,GLsizei count,const GLint* value)
		{
		GLARBShaderObjects::current->glUniform2ivARBProc(location,count,value);
		}
	inline friend void glUniform3ivARB(GLint location,GLsizei count,const GLint* value)
		{
		GLARBShaderObjects::current->glUniform3ivARBProc(location,count,value);
		}
	inline friend void glUniform4ivARB(GLint location,GLsizei count,const GLint* value)
		{
		GLARBShaderObjects::current->glUniform4ivARBProc(location,count,value);
		}
	inline friend void glUniformMatrix2fvARB(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value)
		{
		GLARBShaderObjects::current->glUniformMatrix2fvARBProc(location,count,transpose,value);
		}
	inline friend void glUniformMatrix3fvARB(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value)
		{
		GLARBShaderObjects::current->glUniformMatrix3fvARBProc(location,count,transpose,value);
		}
	inline friend void glUniformMatrix4fvARB(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value)
		{
		GLARBShaderObjects::current->glUniformMatrix4fvARBProc(location,count,transpose,value);
		}
	inline friend void glGetObjectParameterfvARB(GLhandleARB obj,GLenum pname,GLfloat* params)
		{
		GLARBShaderObjects::current->glGetObjectParameterfvARBProc(obj,pname,params);
		}
	inline friend void glGetObjectParameterivARB(GLhandleARB obj,GLenum pname,GLint* params)
		{
		GLARBShaderObjects::current->glGetObjectParameterivARBProc(obj,pname,params);
		}
	inline friend void glGetInfoLogARB(GLhandleARB obj,GLsizei maxLength,GLsizei* length,GLcharARB* infoLog)
		{
		GLARBShaderObjects::current->glGetInfoLogARBProc(obj,maxLength,length,infoLog);
		}
	inline friend void glGetAttachedObjectsARB(GLhandleARB containerObj,GLsizei maxCount,GLsizei* count,GLhandleARB* obj)
		{
		GLARBShaderObjects::current->glGetAttachedObjectsARBProc(containerObj,maxCount,count,obj);
		}
	inline friend GLint glGetUniformLocationARB(GLhandleARB programObj,const GLcharARB* name)
		{
		return GLARBShaderObjects::current->glGetUniformLocationARBProc(programObj,name);
		}
	inline friend void glGetActiveUniformARB(GLhandleARB programObj,GLuint index,GLsizei maxLength,GLsizei* length,GLint* size,GLenum* type,GLcharARB* name)
		{
		GLARBShaderObjects::current->glGetActiveUniformARBProc(programObj,index,maxLength,length,size,type,name);
		}
	inline friend void glGetUniformfvARB(GLhandleARB programObj,GLint location,GLfloat* params)
		{
		GLARBShaderObjects::current->glGetUniformfvARBProc(programObj,location,params);
		}
	inline friend void glGetUniformivARB(GLhandleARB programObj,GLint location,GLint* params)
		{
		GLARBShaderObjects::current->glGetUniformivARBProc(programObj,location,params);
		}
	inline friend void glGetShaderSourceARB(GLhandleARB obj,GLsizei maxLength,GLsizei* length,GLcharARB* source)
		{
		GLARBShaderObjects::current->glGetShaderSourceARBProc(obj,maxLength,length,source);
		}
	};

/*******************************
Extension-specific entry points:
*******************************/

/*********************************************************
Overloaded versions of component-based glUniformARB calls:
*********************************************************/

inline void glUniformARB(GLint location,GLint v0)
	{
	glUniform1iARB(location,v0);
	}

inline void glUniformARB(GLint location,GLfloat v0)
	{
	glUniform1fARB(location,v0);
	}

inline void glUniformARB(GLint location,GLint v0,GLint v1)
	{
	glUniform2iARB(location,v0,v1);
	}

inline void glUniformARB(GLint location,GLfloat v0,GLfloat v1)
	{
	glUniform2fARB(location,v0,v1);
	}

inline void glUniformARB(GLint location,GLint v0,GLint v1,GLint v2)
	{
	glUniform3iARB(location,v0,v1,v2);
	}

inline void glUniformARB(GLint location,GLfloat v0,GLfloat v1,GLfloat v2)
	{
	glUniform3fARB(location,v0,v1,v2);
	}

inline void glUniformARB(GLint location,GLint v0,GLint v1,GLint v2,GLint v3)
	{
	glUniform4iARB(location,v0,v1,v2,v3);
	}

inline void glUniformARB(GLint location,GLfloat v0,GLfloat v1,GLfloat v2,GLfloat v3)
	{
	glUniform4fARB(location,v0,v1,v2,v3);
	}

/******************************************************
Dummy generic version of array-based glUniformARB call:
******************************************************/

template <GLsizei numComponents,class ScalarParam>
void glUniformARB(GLint location,GLsizei count,const ScalarParam components[numComponents]);

/******************************************************
Specialized versions of array-based glUniformARB calls:
******************************************************/

template <>
inline void glUniformARB<1,GLint>(GLint location,GLsizei count,const GLint components[1])
	{
	glUniform1ivARB(location,count,components);
	}

template <>
inline void glUniformARB<1,GLfloat>(GLint location,GLsizei count,const GLfloat components[1])
	{
	glUniform1fvARB(location,count,components);
	}

template <>
inline void glUniformARB<2,GLint>(GLint location,GLsizei count,const GLint components[2])
	{
	glUniform2ivARB(location,count,components);
	}

template <>
inline void glUniformARB<2,GLfloat>(GLint location,GLsizei count,const GLfloat components[2])
	{
	glUniform2fvARB(location,count,components);
	}

template <>
inline void glUniformARB<3,GLint>(GLint location,GLsizei count,const GLint components[3])
	{
	glUniform3ivARB(location,count,components);
	}

template <>
inline void glUniformARB<3,GLfloat>(GLint location,GLsizei count,const GLfloat components[3])
	{
	glUniform3fvARB(location,count,components);
	}

template <>
inline void glUniformARB<4,GLint>(GLint location,GLsizei count,const GLint components[4])
	{
	glUniform4ivARB(location,count,components);
	}

template <>
inline void glUniformARB<4,GLfloat>(GLint location,GLsizei count,const GLfloat components[4])
	{
	glUniform4fvARB(location,count,components);
	}

/****************
Helper functions;
****************/

void glCompileShaderFromString(GLhandleARB shaderObject,const char* shaderSource); // Compiles a shader from a C-style string; throws exception on errors
void glCompileShaderFromStrings(GLhandleARB shaderObject,size_t numShaderSources,va_list ap); // Compiles a shader from a list of C-style strings; throws exception on errors
void glCompileShaderFromStrings(GLhandleARB shaderObject,size_t numShaderSources,...); // Ditto, using variable number of arguments
void glCompileShaderFromFile(GLhandleARB shaderObject,const char* shaderSourceFileName); // Loads and compiles shader from source file of given name; throws exception on errors
void glCompileShaderFromFile(GLhandleARB shaderObject,const char* shaderSourceFileName,IO::File& shaderSourceFile); // Ditto, with already-opened IO::File object
GLhandleARB glLinkShader(const std::vector<GLhandleARB>& shaderObjects); // Links the given list of vertex and/or fragment shaders into a new shader program and returns its handle; throws exception on errors
GLhandleARB glLinkShader(size_t numShaderObjects,va_list ap); // Links the given list of vertex and/or fragment shaders into a new shader program and returns its handle; throws exception on errors
GLhandleARB glLinkShader(size_t numShaderObjects,...); // Ditto, using variable number of arguments
GLhandleARB glLinkShader(GLhandleARB vertexShaderObject,GLhandleARB fragmentShaderObject); // Links one vertex shader and one fragment shader into a new shader program and returns its handle; throws exception on errors

#endif
