/***********************************************************************
GLARBVertexProgram - OpenGL extension class for the
GL_ARB_vertex_program extension.
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

#ifndef GLEXTENSIONS_GLARBVERTEXPROGRAM_INCLUDED
#define GLEXTENSIONS_GLARBVERTEXPROGRAM_INCLUDED

#include <GL/gl.h>
#include <GL/TLSHelper.h>
#include <GL/Extensions/GLExtension.h>

/********************************
Extension-specific parts of gl.h:
********************************/

#ifndef GL_ARB_vertex_program
#define GL_ARB_vertex_program 1

/* Extension-specific functions: */
typedef void (APIENTRY * PFNGLVERTEXATTRIB1SARBPROC) (GLuint index, GLshort x);
typedef void (APIENTRY * PFNGLVERTEXATTRIB1FARBPROC) (GLuint index, GLfloat x);
typedef void (APIENTRY * PFNGLVERTEXATTRIB1DARBPROC) (GLuint index, GLdouble x);
typedef void (APIENTRY * PFNGLVERTEXATTRIB2SARBPROC) (GLuint index, GLshort x, GLshort y);
typedef void (APIENTRY * PFNGLVERTEXATTRIB2FARBPROC) (GLuint index, GLfloat x, GLfloat y);
typedef void (APIENTRY * PFNGLVERTEXATTRIB2DARBPROC) (GLuint index, GLdouble x, GLdouble y);
typedef void (APIENTRY * PFNGLVERTEXATTRIB3SARBPROC) (GLuint index, GLshort x, GLshort y, GLshort z);
typedef void (APIENTRY * PFNGLVERTEXATTRIB3FARBPROC) (GLuint index, GLfloat x, GLfloat y, GLfloat z);
typedef void (APIENTRY * PFNGLVERTEXATTRIB3DARBPROC) (GLuint index, GLdouble x, GLdouble y, GLdouble z);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4SARBPROC) (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4FARBPROC) (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4DARBPROC) (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4NUBARBPROC) (GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
typedef void (APIENTRY * PFNGLVERTEXATTRIB1SVARBPROC) (GLuint index, const GLshort *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB1FVARBPROC) (GLuint index, const GLfloat *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB1DVARBPROC) (GLuint index, const GLdouble *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB2SVARBPROC) (GLuint index, const GLshort *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB2FVARBPROC) (GLuint index, const GLfloat *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB2DVARBPROC) (GLuint index, const GLdouble *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB3SVARBPROC) (GLuint index, const GLshort *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB3FVARBPROC) (GLuint index, const GLfloat *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB3DVARBPROC) (GLuint index, const GLdouble *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4BVARBPROC) (GLuint index, const GLbyte *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4SVARBPROC) (GLuint index, const GLshort *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4IVARBPROC) (GLuint index, const GLint *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4UBVARBPROC) (GLuint index, const GLubyte *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4USVARBPROC) (GLuint index, const GLushort *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4UIVARBPROC) (GLuint index, const GLuint *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4FVARBPROC) (GLuint index, const GLfloat *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4DVARBPROC) (GLuint index, const GLdouble *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4NBVARBPROC) (GLuint index, const GLbyte *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4NSVARBPROC) (GLuint index, const GLshort *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4NIVARBPROC) (GLuint index, const GLint *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4NUBVARBPROC) (GLuint index, const GLubyte *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4NUSVARBPROC) (GLuint index, const GLushort *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4NUIVARBPROC) (GLuint index, const GLuint *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIBPOINTERARBPROC) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
typedef void (APIENTRY * PFNGLENABLEVERTEXATTRIBARRAYARBPROC) (GLuint index);
typedef void (APIENTRY * PFNGLDISABLEVERTEXATTRIBARRAYARBPROC) (GLuint index);
typedef void (APIENTRY * PFNGLPROGRAMSTRINGARBPROC) (GLenum target, GLenum format, GLsizei len, const void *string); 
typedef void (APIENTRY * PFNGLBINDPROGRAMARBPROC) (GLenum target, GLuint program);
typedef void (APIENTRY * PFNGLDELETEPROGRAMSARBPROC) (GLsizei n, const GLuint *programs);
typedef void (APIENTRY * PFNGLGENPROGRAMSARBPROC) (GLsizei n, GLuint *programs);
typedef void (APIENTRY * PFNGLPROGRAMENVPARAMETER4DARBPROC) (GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (APIENTRY * PFNGLPROGRAMENVPARAMETER4DVARBPROC) (GLenum target, GLuint index, const GLdouble *params);
typedef void (APIENTRY * PFNGLPROGRAMENVPARAMETER4FARBPROC) (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (APIENTRY * PFNGLPROGRAMENVPARAMETER4FVARBPROC) (GLenum target, GLuint index, const GLfloat *params);
typedef void (APIENTRY * PFNGLPROGRAMLOCALPARAMETER4DARBPROC) (GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (APIENTRY * PFNGLPROGRAMLOCALPARAMETER4DVARBPROC) (GLenum target, GLuint index, const GLdouble *params);
typedef void (APIENTRY * PFNGLPROGRAMLOCALPARAMETER4FARBPROC) (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (APIENTRY * PFNGLPROGRAMLOCALPARAMETER4FVARBPROC) (GLenum target, GLuint index, const GLfloat *params);
typedef void (APIENTRY * PFNGLGETPROGRAMENVPARAMETERDVARBPROC) (GLenum target, GLuint index, GLdouble *params);
typedef void (APIENTRY * PFNGLGETPROGRAMENVPARAMETERFVARBPROC) (GLenum target, GLuint index, GLfloat *params);
typedef void (APIENTRY * PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC) (GLenum target, GLuint index, GLdouble *params);
typedef void (APIENTRY * PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC) (GLenum target, GLuint index, GLfloat *params);
typedef void (APIENTRY * PFNGLGETPROGRAMIVARBPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (APIENTRY * PFNGLGETPROGRAMSTRINGARBPROC) (GLenum target, GLenum pname, void *string);
typedef void (APIENTRY * PFNGLGETVERTEXATTRIBDVARBPROC) (GLuint index, GLenum pname, GLdouble *params);
typedef void (APIENTRY * PFNGLGETVERTEXATTRIBFVARBPROC) (GLuint index, GLenum pname, GLfloat *params);
typedef void (APIENTRY * PFNGLGETVERTEXATTRIBIVARBPROC) (GLuint index, GLenum pname, GLint *params);
typedef void (APIENTRY * PFNGLGETVERTEXATTRIBPOINTERVARBPROC) (GLuint index, GLenum pname, void **pointer);
typedef GLboolean (APIENTRY * PFNGLISPROGRAMARBPROC) (GLuint program);

/* Extension-specific constants: */
#define GL_VERTEX_PROGRAM_ARB                       0x8620
#define GL_VERTEX_PROGRAM_POINT_SIZE_ARB            0x8642
#define GL_VERTEX_PROGRAM_TWO_SIDE_ARB              0x8643
#define GL_COLOR_SUM_ARB                            0x8458
#define GL_PROGRAM_FORMAT_ASCII_ARB                 0x8875
#define GL_VERTEX_ATTRIB_ARRAY_ENABLED_ARB          0x8622
#define GL_VERTEX_ATTRIB_ARRAY_SIZE_ARB             0x8623
#define GL_VERTEX_ATTRIB_ARRAY_STRIDE_ARB           0x8624
#define GL_VERTEX_ATTRIB_ARRAY_TYPE_ARB             0x8625
#define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED_ARB       0x886A
#define GL_CURRENT_VERTEX_ATTRIB_ARB                0x8626
#define GL_VERTEX_ATTRIB_ARRAY_POINTER_ARB          0x8645
#define GL_PROGRAM_LENGTH_ARB                       0x8627
#define GL_PROGRAM_FORMAT_ARB                       0x8876
#define GL_PROGRAM_BINDING_ARB                      0x8677
#define GL_PROGRAM_INSTRUCTIONS_ARB                 0x88A0
#define GL_MAX_PROGRAM_INSTRUCTIONS_ARB             0x88A1
#define GL_PROGRAM_NATIVE_INSTRUCTIONS_ARB          0x88A2
#define GL_MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB      0x88A3
#define GL_PROGRAM_TEMPORARIES_ARB                  0x88A4
#define GL_MAX_PROGRAM_TEMPORARIES_ARB              0x88A5
#define GL_PROGRAM_NATIVE_TEMPORARIES_ARB           0x88A6
#define GL_MAX_PROGRAM_NATIVE_TEMPORARIES_ARB       0x88A7
#define GL_PROGRAM_PARAMETERS_ARB                   0x88A8
#define GL_MAX_PROGRAM_PARAMETERS_ARB               0x88A9
#define GL_PROGRAM_NATIVE_PARAMETERS_ARB            0x88AA
#define GL_MAX_PROGRAM_NATIVE_PARAMETERS_ARB        0x88AB
#define GL_PROGRAM_ATTRIBS_ARB                      0x88AC
#define GL_MAX_PROGRAM_ATTRIBS_ARB                  0x88AD
#define GL_PROGRAM_NATIVE_ATTRIBS_ARB               0x88AE
#define GL_MAX_PROGRAM_NATIVE_ATTRIBS_ARB           0x88AF
#define GL_PROGRAM_ADDRESS_REGISTERS_ARB            0x88B0
#define GL_MAX_PROGRAM_ADDRESS_REGISTERS_ARB        0x88B1
#define GL_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB     0x88B2
#define GL_MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB 0x88B3
#define GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB         0x88B4
#define GL_MAX_PROGRAM_ENV_PARAMETERS_ARB           0x88B5
#define GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB          0x88B6
#define GL_PROGRAM_STRING_ARB                       0x8628
#define GL_PROGRAM_ERROR_POSITION_ARB               0x864B
#define GL_CURRENT_MATRIX_ARB                       0x8641
#define GL_TRANSPOSE_CURRENT_MATRIX_ARB             0x88B7
#define GL_CURRENT_MATRIX_STACK_DEPTH_ARB           0x8640
#define GL_MAX_VERTEX_ATTRIBS_ARB                   0x8869
#define GL_MAX_PROGRAM_MATRICES_ARB                 0x862F
#define GL_MAX_PROGRAM_MATRIX_STACK_DEPTH_ARB       0x862E
#define GL_PROGRAM_ERROR_STRING_ARB                 0x8874
#define GL_MATRIX0_ARB                              0x88C0
#define GL_MATRIX1_ARB                              0x88C1
#define GL_MATRIX2_ARB                              0x88C2
#define GL_MATRIX3_ARB                              0x88C3
#define GL_MATRIX4_ARB                              0x88C4
#define GL_MATRIX5_ARB                              0x88C5
#define GL_MATRIX6_ARB                              0x88C6
#define GL_MATRIX7_ARB                              0x88C7
#define GL_MATRIX8_ARB                              0x88C8
#define GL_MATRIX9_ARB                              0x88C9
#define GL_MATRIX10_ARB                             0x88CA
#define GL_MATRIX11_ARB                             0x88CB
#define GL_MATRIX12_ARB                             0x88CC
#define GL_MATRIX13_ARB                             0x88CD
#define GL_MATRIX14_ARB                             0x88CE
#define GL_MATRIX15_ARB                             0x88CF
#define GL_MATRIX16_ARB                             0x88D0
#define GL_MATRIX17_ARB                             0x88D1
#define GL_MATRIX18_ARB                             0x88D2
#define GL_MATRIX19_ARB                             0x88D3
#define GL_MATRIX20_ARB                             0x88D4
#define GL_MATRIX21_ARB                             0x88D5
#define GL_MATRIX22_ARB                             0x88D6
#define GL_MATRIX23_ARB                             0x88D7
#define GL_MATRIX24_ARB                             0x88D8
#define GL_MATRIX25_ARB                             0x88D9
#define GL_MATRIX26_ARB                             0x88DA
#define GL_MATRIX27_ARB                             0x88DB
#define GL_MATRIX28_ARB                             0x88DC
#define GL_MATRIX29_ARB                             0x88DD
#define GL_MATRIX30_ARB                             0x88DE
#define GL_MATRIX31_ARB                             0x88DF

#endif

/* Forward declarations of friend functions: */
void glVertexAttrib1sARB(GLuint index,GLshort x);
void glVertexAttrib1fARB(GLuint index,GLfloat x);
void glVertexAttrib1dARB(GLuint index,GLdouble x);
void glVertexAttrib2sARB(GLuint index,GLshort x,GLshort y);
void glVertexAttrib2fARB(GLuint index,GLfloat x,GLfloat y);
void glVertexAttrib2dARB(GLuint index,GLdouble x,GLdouble y);
void glVertexAttrib3sARB(GLuint index,GLshort x,GLshort y,GLshort z);
void glVertexAttrib3fARB(GLuint index,GLfloat x,GLfloat y,GLfloat z);
void glVertexAttrib3dARB(GLuint index,GLdouble x,GLdouble y,GLdouble z);
void glVertexAttrib4sARB(GLuint index,GLshort x,GLshort y,GLshort z,GLshort w);
void glVertexAttrib4fARB(GLuint index,GLfloat x,GLfloat y,GLfloat z,GLfloat w);
void glVertexAttrib4dARB(GLuint index,GLdouble x,GLdouble y,GLdouble z,GLdouble w);
void glVertexAttrib4NubARB(GLuint index,GLubyte x,GLubyte y,GLubyte z,GLubyte w);
void glVertexAttrib1svARB(GLuint index,const GLshort* v);
void glVertexAttrib1fvARB(GLuint index,const GLfloat* v);
void glVertexAttrib1dvARB(GLuint index,const GLdouble* v);
void glVertexAttrib2svARB(GLuint index,const GLshort* v);
void glVertexAttrib2fvARB(GLuint index,const GLfloat* v);
void glVertexAttrib2dvARB(GLuint index,const GLdouble* v);
void glVertexAttrib3svARB(GLuint index,const GLshort* v);
void glVertexAttrib3fvARB(GLuint index,const GLfloat* v);
void glVertexAttrib3dvARB(GLuint index,const GLdouble* v);
void glVertexAttrib4bvARB(GLuint index,const GLbyte* v);
void glVertexAttrib4svARB(GLuint index,const GLshort* v);
void glVertexAttrib4ivARB(GLuint index,const GLint* v);
void glVertexAttrib4ubvARB(GLuint index,const GLubyte* v);
void glVertexAttrib4usvARB(GLuint index,const GLushort* v);
void glVertexAttrib4uivARB(GLuint index,const GLuint* v);
void glVertexAttrib4fvARB(GLuint index,const GLfloat* v);
void glVertexAttrib4dvARB(GLuint index,const GLdouble* v);
void glVertexAttrib4NbvARB(GLuint index,const GLbyte* v);
void glVertexAttrib4NsvARB(GLuint index,const GLshort* v);
void glVertexAttrib4NivARB(GLuint index,const GLint* v);
void glVertexAttrib4NubvARB(GLuint index,const GLubyte* v);
void glVertexAttrib4NusvARB(GLuint index,const GLushort* v);
void glVertexAttrib4NuivARB(GLuint index,const GLuint* v);
void glVertexAttribPointerARB(GLuint index,GLint size,GLenum type,GLboolean normalized,GLsizei stride,const GLvoid* pointer);
void glEnableVertexAttribArrayARB(GLuint index);
void glDisableVertexAttribArrayARB(GLuint index);
void glProgramStringARB(GLenum target,GLenum format,GLsizei len,const GLvoid* string);
void glBindProgramARB(GLenum target,GLuint program);
void glDeleteProgramsARB(GLsizei n,const GLuint* programs);
void glGenProgramsARB(GLsizei n,GLuint* programs);
void glProgramEnvParameter4dARB(GLenum target,GLuint index,GLdouble x,GLdouble y,GLdouble z,GLdouble w);
void glProgramEnvParameter4dvARB(GLenum target,GLuint index,const GLdouble* params);
void glProgramEnvParameter4fARB(GLenum target,GLuint index,GLfloat x,GLfloat y,GLfloat z,GLfloat w);
void glProgramEnvParameter4fvARB(GLenum target,GLuint index,const GLfloat* params);
void glProgramLocalParameter4dARB(GLenum target,GLuint index,GLdouble x,GLdouble y,GLdouble z,GLdouble w);
void glProgramLocalParameter4dvARB(GLenum target,GLuint index,const GLdouble* params);
void glProgramLocalParameter4fARB(GLenum target,GLuint index,GLfloat x,GLfloat y,GLfloat z,GLfloat w);
void glProgramLocalParameter4fvARB(GLenum target,GLuint index,const GLfloat* params);
void glGetProgramEnvParameterdvARB(GLenum target,GLuint index,GLdouble* params);
void glGetProgramEnvParameterfvARB(GLenum target,GLuint index,GLfloat* params);
void glGetProgramLocalParameterdvARB(GLenum target,GLuint index,GLdouble* params);
void glGetProgramLocalParameterfvARB(GLenum target,GLuint index,GLfloat* params);
void glGetProgramivARB(GLenum target,GLenum pname,GLint* params);
void glGetProgramStringARB(GLenum target,GLenum pname,GLvoid* string);
void glGetVertexAttribdvARB(GLuint index,GLenum pname,GLdouble* params);
void glGetVertexAttribfvARB(GLuint index,GLenum pname,GLfloat* params);
void glGetVertexAttribivARB(GLuint index,GLenum pname,GLint* params);
void glGetVertexAttribPointervARB(GLuint index,GLenum pname,GLvoid** pointer);
GLboolean glIsProgramARB(GLuint program);

class GLARBVertexProgram:public GLExtension
	{
	/* Elements: */
	private:
	static GL_THREAD_LOCAL(GLARBVertexProgram*) current; // Pointer to extension object for current OpenGL context
	static const char* name; // Extension name
	PFNGLVERTEXATTRIB1SARBPROC glVertexAttrib1sARBProc;
	PFNGLVERTEXATTRIB1FARBPROC glVertexAttrib1fARBProc;
	PFNGLVERTEXATTRIB1DARBPROC glVertexAttrib1dARBProc;
	PFNGLVERTEXATTRIB2SARBPROC glVertexAttrib2sARBProc;
	PFNGLVERTEXATTRIB2FARBPROC glVertexAttrib2fARBProc;
	PFNGLVERTEXATTRIB2DARBPROC glVertexAttrib2dARBProc;
	PFNGLVERTEXATTRIB3SARBPROC glVertexAttrib3sARBProc;
	PFNGLVERTEXATTRIB3FARBPROC glVertexAttrib3fARBProc;
	PFNGLVERTEXATTRIB3DARBPROC glVertexAttrib3dARBProc;
	PFNGLVERTEXATTRIB4SARBPROC glVertexAttrib4sARBProc;
	PFNGLVERTEXATTRIB4FARBPROC glVertexAttrib4fARBProc;
	PFNGLVERTEXATTRIB4DARBPROC glVertexAttrib4dARBProc;
	PFNGLVERTEXATTRIB4NUBARBPROC glVertexAttrib4NubARBProc;
	PFNGLVERTEXATTRIB1SVARBPROC glVertexAttrib1svARBProc;
	PFNGLVERTEXATTRIB1FVARBPROC glVertexAttrib1fvARBProc;
	PFNGLVERTEXATTRIB1DVARBPROC glVertexAttrib1dvARBProc;
	PFNGLVERTEXATTRIB2SVARBPROC glVertexAttrib2svARBProc;
	PFNGLVERTEXATTRIB2FVARBPROC glVertexAttrib2fvARBProc;
	PFNGLVERTEXATTRIB2DVARBPROC glVertexAttrib2dvARBProc;
	PFNGLVERTEXATTRIB3SVARBPROC glVertexAttrib3svARBProc;
	PFNGLVERTEXATTRIB3FVARBPROC glVertexAttrib3fvARBProc;
	PFNGLVERTEXATTRIB3DVARBPROC glVertexAttrib3dvARBProc;
	PFNGLVERTEXATTRIB4BVARBPROC glVertexAttrib4bvARBProc;
	PFNGLVERTEXATTRIB4SVARBPROC glVertexAttrib4svARBProc;
	PFNGLVERTEXATTRIB4IVARBPROC glVertexAttrib4ivARBProc;
	PFNGLVERTEXATTRIB4UBVARBPROC glVertexAttrib4ubvARBProc;
	PFNGLVERTEXATTRIB4USVARBPROC glVertexAttrib4usvARBProc;
	PFNGLVERTEXATTRIB4UIVARBPROC glVertexAttrib4uivARBProc;
	PFNGLVERTEXATTRIB4FVARBPROC glVertexAttrib4fvARBProc;
	PFNGLVERTEXATTRIB4DVARBPROC glVertexAttrib4dvARBProc;
	PFNGLVERTEXATTRIB4NBVARBPROC glVertexAttrib4NbvARBProc;
	PFNGLVERTEXATTRIB4NSVARBPROC glVertexAttrib4NsvARBProc;
	PFNGLVERTEXATTRIB4NIVARBPROC glVertexAttrib4NivARBProc;
	PFNGLVERTEXATTRIB4NUBVARBPROC glVertexAttrib4NubvARBProc;
	PFNGLVERTEXATTRIB4NUSVARBPROC glVertexAttrib4NusvARBProc;
	PFNGLVERTEXATTRIB4NUIVARBPROC glVertexAttrib4NuivARBProc;
	PFNGLVERTEXATTRIBPOINTERARBPROC glVertexAttribPointerARBProc;
	PFNGLENABLEVERTEXATTRIBARRAYARBPROC glEnableVertexAttribArrayARBProc;
	PFNGLDISABLEVERTEXATTRIBARRAYARBPROC glDisableVertexAttribArrayARBProc;
	PFNGLPROGRAMSTRINGARBPROC glProgramStringARBProc;
	PFNGLBINDPROGRAMARBPROC glBindProgramARBProc;
	PFNGLDELETEPROGRAMSARBPROC glDeleteProgramsARBProc;
	PFNGLGENPROGRAMSARBPROC glGenProgramsARBProc;
	PFNGLPROGRAMENVPARAMETER4DARBPROC glProgramEnvParameter4dARBProc;
	PFNGLPROGRAMENVPARAMETER4DVARBPROC glProgramEnvParameter4dvARBProc;
	PFNGLPROGRAMENVPARAMETER4FARBPROC glProgramEnvParameter4fARBProc;
	PFNGLPROGRAMENVPARAMETER4FVARBPROC glProgramEnvParameter4fvARBProc;
	PFNGLPROGRAMLOCALPARAMETER4DARBPROC glProgramLocalParameter4dARBProc;
	PFNGLPROGRAMLOCALPARAMETER4DVARBPROC glProgramLocalParameter4dvARBProc;
	PFNGLPROGRAMLOCALPARAMETER4FARBPROC glProgramLocalParameter4fARBProc;
	PFNGLPROGRAMLOCALPARAMETER4FVARBPROC glProgramLocalParameter4fvARBProc;
	PFNGLGETPROGRAMENVPARAMETERDVARBPROC glGetProgramEnvParameterdvARBProc;
	PFNGLGETPROGRAMENVPARAMETERFVARBPROC glGetProgramEnvParameterfvARBProc;
	PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC glGetProgramLocalParameterdvARBProc;
	PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC glGetProgramLocalParameterfvARBProc;
	PFNGLGETPROGRAMIVARBPROC glGetProgramivARBProc;
	PFNGLGETPROGRAMSTRINGARBPROC glGetProgramStringARBProc;
	PFNGLGETVERTEXATTRIBDVARBPROC glGetVertexAttribdvARBProc;
	PFNGLGETVERTEXATTRIBFVARBPROC glGetVertexAttribfvARBProc;
	PFNGLGETVERTEXATTRIBIVARBPROC glGetVertexAttribivARBProc;
	PFNGLGETVERTEXATTRIBPOINTERVARBPROC glGetVertexAttribPointervARBProc;
	PFNGLISPROGRAMARBPROC glIsProgramARBProc;
	
	/* Constructors and destructors: */
	private:
	GLARBVertexProgram(void);
	public:
	virtual ~GLARBVertexProgram(void);
	
	/* Methods: */
	public:
	virtual const char* getExtensionName(void) const;
	virtual void activate(void);
	virtual void deactivate(void);
	static bool isSupported(void); // Returns true if the extension is supported in the current OpenGL context
	static void initExtension(void); // Initializes the extension in the current OpenGL context
	
	/* Extension entry points: */
	inline friend void glVertexAttrib1sARB(GLuint index,GLshort x)
		{
		GLARBVertexProgram::current->glVertexAttrib1sARBProc(index,x);
		}
	inline friend void glVertexAttrib1fARB(GLuint index,GLfloat x)
		{
		GLARBVertexProgram::current->glVertexAttrib1fARBProc(index,x);
		}
	inline friend void glVertexAttrib1dARB(GLuint index,GLdouble x)
		{
		GLARBVertexProgram::current->glVertexAttrib1dARBProc(index,x);
		}
	inline friend void glVertexAttrib2sARB(GLuint index,GLshort x,GLshort y)
		{
		GLARBVertexProgram::current->glVertexAttrib2sARBProc(index,x,y);
		}
	inline friend void glVertexAttrib2fARB(GLuint index,GLfloat x,GLfloat y)
		{
		GLARBVertexProgram::current->glVertexAttrib2fARBProc(index,x,y);
		}
	inline friend void glVertexAttrib2dARB(GLuint index,GLdouble x,GLdouble y)
		{
		GLARBVertexProgram::current->glVertexAttrib2dARBProc(index,x,y);
		}
	inline friend void glVertexAttrib3sARB(GLuint index,GLshort x,GLshort y,GLshort z)
		{
		GLARBVertexProgram::current->glVertexAttrib3sARBProc(index,x,y,z);
		}
	inline friend void glVertexAttrib3fARB(GLuint index,GLfloat x,GLfloat y,GLfloat z)
		{
		GLARBVertexProgram::current->glVertexAttrib3fARBProc(index,x,y,z);
		}
	inline friend void glVertexAttrib3dARB(GLuint index,GLdouble x,GLdouble y,GLdouble z)
		{
		GLARBVertexProgram::current->glVertexAttrib3dARBProc(index,x,y,z);
		}
	inline friend void glVertexAttrib4sARB(GLuint index,GLshort x,GLshort y,GLshort z,GLshort w)
		{
		GLARBVertexProgram::current->glVertexAttrib4sARBProc(index,x,y,z,w);
		}
	inline friend void glVertexAttrib4fARB(GLuint index,GLfloat x,GLfloat y,GLfloat z,GLfloat w)
		{
		GLARBVertexProgram::current->glVertexAttrib4fARBProc(index,x,y,z,w);
		}
	inline friend void glVertexAttrib4dARB(GLuint index,GLdouble x,GLdouble y,GLdouble z,GLdouble w)
		{
		GLARBVertexProgram::current->glVertexAttrib4dARBProc(index,x,y,z,w);
		}
	inline friend void glVertexAttrib4NubARB(GLuint index,GLubyte x,GLubyte y,GLubyte z,GLubyte w)
		{
		GLARBVertexProgram::current->glVertexAttrib4NubARBProc(index,x,y,z,w);
		}
	inline friend void glVertexAttrib1svARB(GLuint index,const GLshort* v)
		{
		GLARBVertexProgram::current->glVertexAttrib1svARBProc(index,v);
		}
	inline friend void glVertexAttrib1fvARB(GLuint index,const GLfloat* v)
		{
		GLARBVertexProgram::current->glVertexAttrib1fvARBProc(index,v);
		}
	inline friend void glVertexAttrib1dvARB(GLuint index,const GLdouble* v)
		{
		GLARBVertexProgram::current->glVertexAttrib1dvARBProc(index,v);
		}
	inline friend void glVertexAttrib2svARB(GLuint index,const GLshort* v)
		{
		GLARBVertexProgram::current->glVertexAttrib2svARBProc(index,v);
		}
	inline friend void glVertexAttrib2fvARB(GLuint index,const GLfloat* v)
		{
		GLARBVertexProgram::current->glVertexAttrib2fvARBProc(index,v);
		}
	inline friend void glVertexAttrib2dvARB(GLuint index,const GLdouble* v)
		{
		GLARBVertexProgram::current->glVertexAttrib2dvARBProc(index,v);
		}
	inline friend void glVertexAttrib3svARB(GLuint index,const GLshort* v)
		{
		GLARBVertexProgram::current->glVertexAttrib3svARBProc(index,v);
		}
	inline friend void glVertexAttrib3fvARB(GLuint index,const GLfloat* v)
		{
		GLARBVertexProgram::current->glVertexAttrib3fvARBProc(index,v);
		}
	inline friend void glVertexAttrib3dvARB(GLuint index,const GLdouble* v)
		{
		GLARBVertexProgram::current->glVertexAttrib3dvARBProc(index,v);
		}
	inline friend void glVertexAttrib4bvARB(GLuint index,const GLbyte* v)
		{
		GLARBVertexProgram::current->glVertexAttrib4bvARBProc(index,v);
		}
	inline friend void glVertexAttrib4svARB(GLuint index,const GLshort* v)
		{
		GLARBVertexProgram::current->glVertexAttrib4svARBProc(index,v);
		}
	inline friend void glVertexAttrib4ivARB(GLuint index,const GLint* v)
		{
		GLARBVertexProgram::current->glVertexAttrib4ivARBProc(index,v);
		}
	inline friend void glVertexAttrib4ubvARB(GLuint index,const GLubyte* v)
		{
		GLARBVertexProgram::current->glVertexAttrib4ubvARBProc(index,v);
		}
	inline friend void glVertexAttrib4usvARB(GLuint index,const GLushort* v)
		{
		GLARBVertexProgram::current->glVertexAttrib4usvARBProc(index,v);
		}
	inline friend void glVertexAttrib4uivARB(GLuint index,const GLuint* v)
		{
		GLARBVertexProgram::current->glVertexAttrib4uivARBProc(index,v);
		}
	inline friend void glVertexAttrib4fvARB(GLuint index,const GLfloat* v)
		{
		GLARBVertexProgram::current->glVertexAttrib4fvARBProc(index,v);
		}
	inline friend void glVertexAttrib4dvARB(GLuint index,const GLdouble* v)
		{
		GLARBVertexProgram::current->glVertexAttrib4dvARBProc(index,v);
		}
	inline friend void glVertexAttrib4NbvARB(GLuint index,const GLbyte* v)
		{
		GLARBVertexProgram::current->glVertexAttrib4NbvARBProc(index,v);
		}
	inline friend void glVertexAttrib4NsvARB(GLuint index,const GLshort* v)
		{
		GLARBVertexProgram::current->glVertexAttrib4NsvARBProc(index,v);
		}
	inline friend void glVertexAttrib4NivARB(GLuint index,const GLint* v)
		{
		GLARBVertexProgram::current->glVertexAttrib4NivARBProc(index,v);
		}
	inline friend void glVertexAttrib4NubvARB(GLuint index,const GLubyte* v)
		{
		GLARBVertexProgram::current->glVertexAttrib4NubvARBProc(index,v);
		}
	inline friend void glVertexAttrib4NusvARB(GLuint index,const GLushort* v)
		{
		GLARBVertexProgram::current->glVertexAttrib4NusvARBProc(index,v);
		}
	inline friend void glVertexAttrib4NuivARB(GLuint index,const GLuint* v)
		{
		GLARBVertexProgram::current->glVertexAttrib4NuivARBProc(index,v);
		}
	inline friend void glVertexAttribPointerARB(GLuint index,GLint size,GLenum type,GLboolean normalized,GLsizei stride,const GLvoid* pointer)
		{
		GLARBVertexProgram::current->glVertexAttribPointerARBProc(index,size,type,normalized,stride,pointer);
		}
	inline friend void glEnableVertexAttribArrayARB(GLuint index)
		{
		GLARBVertexProgram::current->glEnableVertexAttribArrayARBProc(index);
		}
	inline friend void glDisableVertexAttribArrayARB(GLuint index)
		{
		GLARBVertexProgram::current->glDisableVertexAttribArrayARBProc(index);
		}
	inline friend void glProgramStringARB(GLenum target,GLenum format,GLsizei len,const GLvoid* string)
		{
		GLARBVertexProgram::current->glProgramStringARBProc(target,format,len,string);
		}
	inline friend void glBindProgramARB(GLenum target,GLuint program)
		{
		GLARBVertexProgram::current->glBindProgramARBProc(target,program);
		}
	inline friend void glDeleteProgramsARB(GLsizei n,const GLuint* programs)
		{
		GLARBVertexProgram::current->glDeleteProgramsARBProc(n,programs);
		}
	inline friend void glGenProgramsARB(GLsizei n,GLuint* programs)
		{
		GLARBVertexProgram::current->glGenProgramsARBProc(n,programs);
		}
	inline friend void glProgramEnvParameter4dARB(GLenum target,GLuint index,GLdouble x,GLdouble y,GLdouble z,GLdouble w)
		{
		GLARBVertexProgram::current->glProgramEnvParameter4dARBProc(target,index,x,y,z,w);
		}
	inline friend void glProgramEnvParameter4dvARB(GLenum target,GLuint index,const GLdouble* params)
		{
		GLARBVertexProgram::current->glProgramEnvParameter4dvARBProc(target,index,params);
		}
	inline friend void glProgramEnvParameter4fARB(GLenum target,GLuint index,GLfloat x,GLfloat y,GLfloat z,GLfloat w)
		{
		GLARBVertexProgram::current->glProgramEnvParameter4fARBProc(target,index,x,y,z,w);
		}
	inline friend void glProgramEnvParameter4fvARB(GLenum target,GLuint index,const GLfloat* params)
		{
		GLARBVertexProgram::current->glProgramEnvParameter4fvARBProc(target,index,params);
		}
	inline friend void glProgramLocalParameter4dARB(GLenum target,GLuint index,GLdouble x,GLdouble y,GLdouble z,GLdouble w)
		{
		GLARBVertexProgram::current->glProgramLocalParameter4dARBProc(target,index,x,y,z,w);
		}
	inline friend void glProgramLocalParameter4dvARB(GLenum target,GLuint index,const GLdouble* params)
		{
		GLARBVertexProgram::current->glProgramLocalParameter4dvARBProc(target,index,params);
		}
	inline friend void glProgramLocalParameter4fARB(GLenum target,GLuint index,GLfloat x,GLfloat y,GLfloat z,GLfloat w)
		{
		GLARBVertexProgram::current->glProgramLocalParameter4fARBProc(target,index,x,y,z,w);
		}
	inline friend void glProgramLocalParameter4fvARB(GLenum target,GLuint index,const GLfloat* params)
		{
		GLARBVertexProgram::current->glProgramLocalParameter4fvARBProc(target,index,params);
		}
	inline friend void glGetProgramEnvParameterdvARB(GLenum target,GLuint index,GLdouble* params)
		{
		GLARBVertexProgram::current->glGetProgramEnvParameterdvARBProc(target,index,params);
		}
	inline friend void glGetProgramEnvParameterfvARB(GLenum target,GLuint index,GLfloat* params)
		{
		GLARBVertexProgram::current->glGetProgramEnvParameterfvARBProc(target,index,params);
		}
	inline friend void glGetProgramLocalParameterdvARB(GLenum target,GLuint index,GLdouble* params)
		{
		GLARBVertexProgram::current->glGetProgramLocalParameterdvARBProc(target,index,params);
		}
	inline friend void glGetProgramLocalParameterfvARB(GLenum target,GLuint index,GLfloat* params)
		{
		GLARBVertexProgram::current->glGetProgramLocalParameterfvARBProc(target,index,params);
		}
	inline friend void glGetProgramivARB(GLenum target,GLenum pname,GLint* params)
		{
		GLARBVertexProgram::current->glGetProgramivARBProc(target,pname,params);
		}
	inline friend void glGetProgramStringARB(GLenum target,GLenum pname,GLvoid* string)
		{
		GLARBVertexProgram::current->glGetProgramStringARBProc(target,pname,string);
		}
	inline friend void glGetVertexAttribdvARB(GLuint index,GLenum pname,GLdouble* params)
		{
		GLARBVertexProgram::current->glGetVertexAttribdvARBProc(index,pname,params);
		}
	inline friend void glGetVertexAttribfvARB(GLuint index,GLenum pname,GLfloat* params)
		{
		GLARBVertexProgram::current->glGetVertexAttribfvARBProc(index,pname,params);
		}
	inline friend void glGetVertexAttribivARB(GLuint index,GLenum pname,GLint* params)
		{
		GLARBVertexProgram::current->glGetVertexAttribivARBProc(index,pname,params);
		}
	inline friend void glGetVertexAttribPointervARB(GLuint index,GLenum pname,GLvoid** pointer)
		{
		GLARBVertexProgram::current->glGetVertexAttribPointervARBProc(index,pname,pointer);
		}
	inline friend GLboolean glIsProgramARB(GLuint program)
		{
		return GLARBVertexProgram::current->glIsProgramARBProc(program);
		}
	};

/*******************************
Extension-specific entry points:
*******************************/

/**************************************************************
Overloaded versions of component-based glVertexAttribARB calls:
**************************************************************/

inline void glVertexAttribARB(GLuint index,GLshort x)
	{
	glVertexAttrib1sARB(index,x);
	}

inline void glVertexAttribARB(GLuint index,GLfloat x)
	{
	glVertexAttrib1fARB(index,x);
	}

inline void glVertexAttribARB(GLuint index,GLdouble x)
	{
	glVertexAttrib1dARB(index,x);
	}

inline void glVertexAttribARB(GLuint index,GLshort x,GLshort y)
	{
	glVertexAttrib2sARB(index,x,y);
	}

inline void glVertexAttribARB(GLuint index,GLfloat x,GLfloat y)
	{
	glVertexAttrib2fARB(index,x,y);
	}

inline void glVertexAttribARB(GLuint index,GLdouble x,GLdouble y)
	{
	glVertexAttrib2dARB(index,x,y);
	}

inline void glVertexAttribARB(GLuint index,GLshort x,GLshort y,GLshort z)
	{
	glVertexAttrib3sARB(index,x,y,z);
	}

inline void glVertexAttribARB(GLuint index,GLfloat x,GLfloat y,GLfloat z)
	{
	glVertexAttrib3fARB(index,x,y,z);
	}

inline void glVertexAttribARB(GLuint index,GLdouble x,GLdouble y,GLdouble z)
	{
	glVertexAttrib3dARB(index,x,y,z);
	}

inline void glVertexAttribARB(GLuint index,GLshort x,GLshort y,GLshort z,GLshort w)
	{
	glVertexAttrib4sARB(index,x,y,z,w);
	}

inline void glVertexAttribARB(GLuint index,GLfloat x,GLfloat y,GLfloat z,GLfloat w)
	{
	glVertexAttrib4fARB(index,x,y,z,w);
	}

inline void glVertexAttribARB(GLuint index,GLdouble x,GLdouble y,GLdouble z,GLdouble w)
	{
	glVertexAttrib4dARB(index,x,y,z,w);
	}

inline void glVertexAttribNARB(GLuint index,GLubyte x,GLubyte y,GLubyte z,GLubyte w)
	{
	glVertexAttrib4NubARB(index,x,y,z,w);
	}

/***********************************************************
Dummy generic version of array-based glVertexAttribARB call:
***********************************************************/

template <GLsizei numComponents,class ScalarParam>
void glVertexAttribARB(GLuint index,const ScalarParam components[numComponents]);

template <GLsizei numComponents,class ScalarParam>
void glVertexAttribNARB(GLuint index,const ScalarParam components[numComponents]);

/***********************************************************
Specialized versions of array-based glVertexAttribARB calls:
***********************************************************/

template <>
inline void glVertexAttribARB<1,GLshort>(GLuint index,const GLshort components[1])
	{
	glVertexAttrib1svARB(index,components);
	}

template <>
inline void glVertexAttribARB<1,GLfloat>(GLuint index,const GLfloat components[1])
	{
	glVertexAttrib1fvARB(index,components);
	}

template <>
inline void glVertexAttribARB<1,GLdouble>(GLuint index,const GLdouble components[1])
	{
	glVertexAttrib1dvARB(index,components);
	}

template <>
inline void glVertexAttribARB<2,GLshort>(GLuint index,const GLshort components[2])
	{
	glVertexAttrib2svARB(index,components);
	}

template <>
inline void glVertexAttribARB<2,GLfloat>(GLuint index,const GLfloat components[2])
	{
	glVertexAttrib2fvARB(index,components);
	}

template <>
inline void glVertexAttribARB<2,GLdouble>(GLuint index,const GLdouble components[2])
	{
	glVertexAttrib2dvARB(index,components);
	}

template <>
inline void glVertexAttribARB<3,GLshort>(GLuint index,const GLshort components[3])
	{
	glVertexAttrib3svARB(index,components);
	}

template <>
inline void glVertexAttribARB<3,GLfloat>(GLuint index,const GLfloat components[3])
	{
	glVertexAttrib3fvARB(index,components);
	}

template <>
inline void glVertexAttribARB<3,GLdouble>(GLuint index,const GLdouble components[3])
	{
	glVertexAttrib3dvARB(index,components);
	}

template <>
inline void glVertexAttribARB<4,GLbyte>(GLuint index,const GLbyte components[4])
	{
	glVertexAttrib4bvARB(index,components);
	}

template <>
inline void glVertexAttribARB<4,GLubyte>(GLuint index,const GLubyte components[4])
	{
	glVertexAttrib4ubvARB(index,components);
	}

template <>
inline void glVertexAttribARB<4,GLshort>(GLuint index,const GLshort components[4])
	{
	glVertexAttrib4svARB(index,components);
	}

template <>
inline void glVertexAttribARB<4,GLushort>(GLuint index,const GLushort components[4])
	{
	glVertexAttrib4usvARB(index,components);
	}

template <>
inline void glVertexAttribARB<4,GLint>(GLuint index,const GLint components[4])
	{
	glVertexAttrib4ivARB(index,components);
	}

template <>
inline void glVertexAttribARB<4,GLuint>(GLuint index,const GLuint components[4])
	{
	glVertexAttrib4uivARB(index,components);
	}

template <>
inline void glVertexAttribARB<4,GLfloat>(GLuint index,const GLfloat components[4])
	{
	glVertexAttrib4fvARB(index,components);
	}

template <>
inline void glVertexAttribARB<4,GLdouble>(GLuint index,const GLdouble components[4])
	{
	glVertexAttrib4dvARB(index,components);
	}

template <>
inline void glVertexAttribNARB<4,GLbyte>(GLuint index,const GLbyte components[4])
	{
	glVertexAttrib4NbvARB(index,components);
	}

template <>
inline void glVertexAttribNARB<4,GLubyte>(GLuint index,const GLubyte components[4])
	{
	glVertexAttrib4NubvARB(index,components);
	}

template <>
inline void glVertexAttribNARB<4,GLshort>(GLuint index,const GLshort components[4])
	{
	glVertexAttrib4NsvARB(index,components);
	}

template <>
inline void glVertexAttribNARB<4,GLushort>(GLuint index,const GLushort components[4])
	{
	glVertexAttrib4NusvARB(index,components);
	}

template <>
inline void glVertexAttribNARB<4,GLint>(GLuint index,const GLint components[4])
	{
	glVertexAttrib4NivARB(index,components);
	}

template <>
inline void glVertexAttribNARB<4,GLuint>(GLuint index,const GLuint components[4])
	{
	glVertexAttrib4NuivARB(index,components);
	}

/*****************************************************
Overloaded versions of glVertexAttribPointerARB calls:
*****************************************************/

inline void glVertexAttribPointerARB(GLuint index,GLint size,GLboolean normalized,GLsizei stride,const GLbyte* pointer)
	{
	glVertexAttribPointerARB(index,size,GL_BYTE,normalized,stride,pointer);
	}

inline void glVertexAttribPointerARB(GLuint index,GLint size,GLboolean normalized,GLsizei stride,const GLubyte* pointer)
	{
	glVertexAttribPointerARB(index,size,GL_UNSIGNED_BYTE,normalized,stride,pointer);
	}

inline void glVertexAttribPointerARB(GLuint index,GLint size,GLboolean normalized,GLsizei stride,const GLshort* pointer)
	{
	glVertexAttribPointerARB(index,size,GL_SHORT,normalized,stride,pointer);
	}

inline void glVertexAttribPointerARB(GLuint index,GLint size,GLboolean normalized,GLsizei stride,const GLushort* pointer)
	{
	glVertexAttribPointerARB(index,size,GL_UNSIGNED_SHORT,normalized,stride,pointer);
	}

inline void glVertexAttribPointerARB(GLuint index,GLint size,GLboolean normalized,GLsizei stride,const GLint* pointer)
	{
	glVertexAttribPointerARB(index,size,GL_INT,normalized,stride,pointer);
	}

inline void glVertexAttribPointerARB(GLuint index,GLint size,GLboolean normalized,GLsizei stride,const GLuint* pointer)
	{
	glVertexAttribPointerARB(index,size,GL_UNSIGNED_INT,normalized,stride,pointer);
	}

inline void glVertexAttribPointerARB(GLuint index,GLint size,GLboolean normalized,GLsizei stride,const GLfloat* pointer)
	{
	glVertexAttribPointerARB(index,size,GL_FLOAT,normalized,stride,pointer);
	}

inline void glVertexAttribPointerARB(GLuint index,GLint size,GLboolean normalized,GLsizei stride,const GLdouble* pointer)
	{
	glVertexAttribPointerARB(index,size,GL_DOUBLE,normalized,stride,pointer);
	}

/*********************************************************************
Overloaded versions of component-based glProgramEnvParameterARB calls:
*********************************************************************/

inline void glProgramEnvParameterARB(GLenum target,GLuint index,GLfloat x,GLfloat y,GLfloat z,GLfloat w)
	{
	glProgramEnvParameter4fARB(target,index,x,y,z,w);
	}

inline void glProgramEnvParameterARB(GLenum target,GLuint index,GLdouble x,GLdouble y,GLdouble z,GLdouble w)
	{
	glProgramEnvParameter4dARB(target,index,x,y,z,w);
	}

/***********************************************************************
Overloaded versions of component-based glProgramLocalParameterARB calls:
***********************************************************************/

inline void glProgramLocalParameterARB(GLenum target,GLuint index,GLfloat x,GLfloat y,GLfloat z,GLfloat w)
	{
	glProgramLocalParameter4fARB(target,index,x,y,z,w);
	}

inline void glProgramLocalParameterARB(GLenum target,GLuint index,GLdouble x,GLdouble y,GLdouble z,GLdouble w)
	{
	glProgramLocalParameter4dARB(target,index,x,y,z,w);
	}

#endif
