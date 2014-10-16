/***********************************************************************
GLEXTFramebufferObject - OpenGL extension class for the
GL_EXT_framebuffer_object extension.
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

#ifndef GLEXTENSIONS_GLEXTFRAMEBUFFEROBJECT_INCLUDED
#define GLEXTENSIONS_GLEXTFRAMEBUFFEROBJECT_INCLUDED

#include <ostream>
#include <GL/gl.h>
#include <GL/TLSHelper.h>
#include <GL/Extensions/GLExtension.h>

/********************************
Extension-specific parts of gl.h:
********************************/

#ifndef GL_EXT_framebuffer_object
#define GL_EXT_framebuffer_object 1

/* Extension-specific functions: */
typedef GLboolean (APIENTRY * PFNGLISRENDERBUFFEREXTPROC) (GLuint renderbuffer);
typedef void (APIENTRY * PFNGLBINDRENDERBUFFEREXTPROC) (GLenum target, GLuint renderbuffer);
typedef void (APIENTRY * PFNGLDELETERENDERBUFFERSEXTPROC) (GLsizei n, const GLuint *renderbuffers);
typedef void (APIENTRY * PFNGLGENRENDERBUFFERSEXTPROC) (GLsizei n, GLuint *renderbuffers);
typedef void (APIENTRY * PFNGLRENDERBUFFERSTORAGEEXTPROC) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (APIENTRY * PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC) (GLenum target, GLenum pname, GLint *params);
typedef GLboolean (APIENTRY * PFNGLISFRAMEBUFFEREXTPROC) (GLuint framebuffer);
typedef void (APIENTRY * PFNGLBINDFRAMEBUFFEREXTPROC) (GLenum target, GLuint framebuffer);
typedef void (APIENTRY * PFNGLDELETEFRAMEBUFFERSEXTPROC) (GLsizei n, const GLuint *framebuffers);
typedef void (APIENTRY * PFNGLGENFRAMEBUFFERSEXTPROC) (GLsizei n, GLuint *framebuffers);
typedef GLenum (APIENTRY * PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC) (GLenum target);
typedef void (APIENTRY * PFNGLFRAMEBUFFERTEXTURE1DEXTPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void (APIENTRY * PFNGLFRAMEBUFFERTEXTURE2DEXTPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void (APIENTRY * PFNGLFRAMEBUFFERTEXTURE3DEXTPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
typedef void (APIENTRY * PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC) (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
typedef void (APIENTRY * PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC) (GLenum target, GLenum attachment, GLenum pname, GLint *params);
typedef void (APIENTRY * PFNGLGENERATEMIPMAPEXTPROC) (GLenum target);

/* Extension-specific constants: */
#define GL_INVALID_FRAMEBUFFER_OPERATION_EXT 0x0506
#define GL_MAX_RENDERBUFFER_SIZE_EXT      0x84E8
#define GL_FRAMEBUFFER_BINDING_EXT        0x8CA6
#define GL_RENDERBUFFER_BINDING_EXT       0x8CA7
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT 0x8CD0
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT 0x8CD1
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL_EXT 0x8CD2
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE_EXT 0x8CD3
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET_EXT 0x8CD4
#define GL_FRAMEBUFFER_COMPLETE_EXT       0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT 0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT 0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT 0x8CD9
#define GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT 0x8CDA
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT 0x8CDB
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT 0x8CDC
#define GL_FRAMEBUFFER_UNSUPPORTED_EXT    0x8CDD
#define GL_MAX_COLOR_ATTACHMENTS_EXT      0x8CDF
#define GL_COLOR_ATTACHMENT0_EXT          0x8CE0
#define GL_COLOR_ATTACHMENT1_EXT          0x8CE1
#define GL_COLOR_ATTACHMENT2_EXT          0x8CE2
#define GL_COLOR_ATTACHMENT3_EXT          0x8CE3
#define GL_COLOR_ATTACHMENT4_EXT          0x8CE4
#define GL_COLOR_ATTACHMENT5_EXT          0x8CE5
#define GL_COLOR_ATTACHMENT6_EXT          0x8CE6
#define GL_COLOR_ATTACHMENT7_EXT          0x8CE7
#define GL_COLOR_ATTACHMENT8_EXT          0x8CE8
#define GL_COLOR_ATTACHMENT9_EXT          0x8CE9
#define GL_COLOR_ATTACHMENT10_EXT         0x8CEA
#define GL_COLOR_ATTACHMENT11_EXT         0x8CEB
#define GL_COLOR_ATTACHMENT12_EXT         0x8CEC
#define GL_COLOR_ATTACHMENT13_EXT         0x8CED
#define GL_COLOR_ATTACHMENT14_EXT         0x8CEE
#define GL_COLOR_ATTACHMENT15_EXT         0x8CEF
#define GL_DEPTH_ATTACHMENT_EXT           0x8D00
#define GL_STENCIL_ATTACHMENT_EXT         0x8D20
#define GL_FRAMEBUFFER_EXT                0x8D40
#define GL_RENDERBUFFER_EXT               0x8D41
#define GL_RENDERBUFFER_WIDTH_EXT         0x8D42
#define GL_RENDERBUFFER_HEIGHT_EXT        0x8D43
#define GL_RENDERBUFFER_INTERNAL_FORMAT_EXT 0x8D44
#define GL_STENCIL_INDEX_EXT              0x8D45
#define GL_STENCIL_INDEX1_EXT             0x8D46
#define GL_STENCIL_INDEX4_EXT             0x8D47
#define GL_STENCIL_INDEX8_EXT             0x8D48
#define GL_STENCIL_INDEX16_EXT            0x8D49
#define GL_RENDERBUFFER_RED_SIZE_EXT      0x8D50
#define GL_RENDERBUFFER_GREEN_SIZE_EXT    0x8D51
#define GL_RENDERBUFFER_BLUE_SIZE_EXT     0x8D52
#define GL_RENDERBUFFER_ALPHA_SIZE_EXT    0x8D53
#define GL_RENDERBUFFER_DEPTH_SIZE_EXT    0x8D54
#define GL_RENDERBUFFER_STENCIL_SIZE_EXT  0x8D55

#endif

/* Forward declarations of friend functions: */
GLboolean glIsRenderbufferEXT(GLuint renderbuffer);
void glBindRenderbufferEXT(GLenum target,GLuint renderbuffer);
void glDeleteRenderbuffersEXT(GLsizei n,const GLuint* renderbuffers);
void glGenRenderbuffersEXT(GLsizei n,GLuint* renderbuffers);
void glRenderbufferStorageEXT(GLenum target,GLenum internalformat,GLsizei width,GLsizei height);
void glGetRenderbufferParameterivEXT(GLenum target,GLenum pname,GLint* params);
GLboolean glIsFramebufferEXT(GLuint framebuffer);
void glBindFramebufferEXT(GLenum target,GLuint framebuffer);
void glDeleteFramebuffersEXT(GLsizei n,const GLuint* framebuffers);
void glGenFramebuffersEXT(GLsizei n,GLuint* framebuffers);
GLenum glCheckFramebufferStatusEXT(GLenum target);
void glFramebufferTexture1DEXT(GLenum target,GLenum attachment,GLenum textarget,GLuint texture,GLint level);
void glFramebufferTexture2DEXT(GLenum target,GLenum attachment,GLenum textarget,GLuint texture,GLint level);
void glFramebufferTexture3DEXT(GLenum target,GLenum attachment,GLenum textarget,GLuint texture,GLint level,GLint zoffset);
void glFramebufferRenderbufferEXT(GLenum target,GLenum attachment,GLenum renderbuffertarget,GLuint renderbuffer);
void glGetFramebufferAttachmentParameterivEXT(GLenum target,GLenum attachment,GLenum pname,GLint* params);
void glGenerateMipmapEXT(GLenum target);

class GLEXTFramebufferObject:public GLExtension
	{
	/* Elements: */
	private:
	static GL_THREAD_LOCAL(GLEXTFramebufferObject*) current; // Pointer to extension object for current OpenGL context
	static const char* name; // Extension name
	PFNGLISRENDERBUFFEREXTPROC glIsRenderbufferEXTProc;
	PFNGLBINDRENDERBUFFEREXTPROC glBindRenderbufferEXTProc;
	PFNGLDELETERENDERBUFFERSEXTPROC glDeleteRenderbuffersEXTProc;
	PFNGLGENRENDERBUFFERSEXTPROC glGenRenderbuffersEXTProc;
	PFNGLRENDERBUFFERSTORAGEEXTPROC glRenderbufferStorageEXTProc;
	PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC glGetRenderbufferParameterivEXTProc;
	PFNGLISFRAMEBUFFEREXTPROC glIsFramebufferEXTProc;
	PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebufferEXTProc;
	PFNGLDELETEFRAMEBUFFERSEXTPROC glDeleteFramebuffersEXTProc;
	PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffersEXTProc;
	PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXTProc;
	PFNGLFRAMEBUFFERTEXTURE1DEXTPROC glFramebufferTexture1DEXTProc;
	PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2DEXTProc;
	PFNGLFRAMEBUFFERTEXTURE3DEXTPROC glFramebufferTexture3DEXTProc;
	PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbufferEXTProc;
	PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC glGetFramebufferAttachmentParameterivEXTProc;
	PFNGLGENERATEMIPMAPEXTPROC glGenerateMipmapEXTProc;
	
	/* Constructors and destructors: */
	private:
	GLEXTFramebufferObject(void);
	public:
	virtual ~GLEXTFramebufferObject(void);
	
	/* Methods: */
	public:
	virtual const char* getExtensionName(void) const;
	virtual void activate(void);
	virtual void deactivate(void);
	static bool isSupported(void); // Returns true if the extension is supported in the current OpenGL context
	static void initExtension(void); // Initializes the extension in the current OpenGL context
	
	/* Extension entry points: */
	inline friend GLboolean glIsRenderbufferEXT(GLuint renderbuffer)
		{
		return GLEXTFramebufferObject::current->glIsRenderbufferEXTProc(renderbuffer);
		}
	inline friend void glBindRenderbufferEXT(GLenum target,GLuint renderbuffer)
		{
		GLEXTFramebufferObject::current->glBindRenderbufferEXTProc(target,renderbuffer);
		}
	inline friend void glDeleteRenderbuffersEXT(GLsizei n,const GLuint* renderbuffers)
		{
		GLEXTFramebufferObject::current->glDeleteRenderbuffersEXTProc(n,renderbuffers);
		}
	inline friend void glGenRenderbuffersEXT(GLsizei n,GLuint* renderbuffers)
		{
		GLEXTFramebufferObject::current->glGenRenderbuffersEXTProc(n,renderbuffers);
		}
	inline friend void glRenderbufferStorageEXT(GLenum target,GLenum internalformat,GLsizei width,GLsizei height)
		{
		GLEXTFramebufferObject::current->glRenderbufferStorageEXTProc(target,internalformat,width,height);
		}
	inline friend void glGetRenderbufferParameterivEXT(GLenum target,GLenum pname,GLint* params)
		{
		GLEXTFramebufferObject::current->glGetRenderbufferParameterivEXTProc(target,pname,params);
		}
	inline friend GLboolean glIsFramebufferEXT(GLuint framebuffer)
		{
		return GLEXTFramebufferObject::current->glIsFramebufferEXTProc(framebuffer);
		}
	inline friend void glBindFramebufferEXT(GLenum target,GLuint framebuffer)
		{
		GLEXTFramebufferObject::current->glBindFramebufferEXTProc(target,framebuffer);
		}
	inline friend void glDeleteFramebuffersEXT(GLsizei n,const GLuint* framebuffers)
		{
		GLEXTFramebufferObject::current->glDeleteFramebuffersEXTProc(n,framebuffers);
		}
	inline friend void glGenFramebuffersEXT(GLsizei n,GLuint* framebuffers)
		{
		GLEXTFramebufferObject::current->glGenFramebuffersEXTProc(n,framebuffers);
		}
	inline friend GLenum glCheckFramebufferStatusEXT(GLenum target)
		{
		return GLEXTFramebufferObject::current->glCheckFramebufferStatusEXTProc(target);
		}
	inline friend void glFramebufferTexture1DEXT(GLenum target,GLenum attachment,GLenum textarget,GLuint texture,GLint level)
		{
		GLEXTFramebufferObject::current->glFramebufferTexture1DEXTProc(target,attachment,textarget,texture,level);
		}
	inline friend void glFramebufferTexture2DEXT(GLenum target,GLenum attachment,GLenum textarget,GLuint texture,GLint level)
		{
		GLEXTFramebufferObject::current->glFramebufferTexture2DEXTProc(target,attachment,textarget,texture,level);
		}
	inline friend void glFramebufferTexture3DEXT(GLenum target,GLenum attachment,GLenum textarget,GLuint texture,GLint level,GLint zoffset)
		{
		GLEXTFramebufferObject::current->glFramebufferTexture3DEXTProc(target,attachment,textarget,texture,level,zoffset);
		}
	inline friend void glFramebufferRenderbufferEXT(GLenum target,GLenum attachment,GLenum renderbuffertarget,GLuint renderbuffer)
		{
		GLEXTFramebufferObject::current->glFramebufferRenderbufferEXTProc(target,attachment,renderbuffertarget,renderbuffer);
		}
	inline friend void glGetFramebufferAttachmentParameterivEXT(GLenum target,GLenum attachment,GLenum pname,GLint* params)
		{
		GLEXTFramebufferObject::current->glGetFramebufferAttachmentParameterivEXTProc(target,attachment,pname,params);
		}
	inline friend void glGenerateMipmapEXT(GLenum target)
		{
		GLEXTFramebufferObject::current->glGenerateMipmapEXTProc(target);
		}
	};

/*******************************
Extension-specific entry points:
*******************************/

void glPrintFramebufferStatusEXT(std::ostream& stream,const char* tag); // Convenience function to check framebuffer status and print detailed message to given stream if incomplete
void glThrowFramebufferStatusExceptionEXT(const char* tag); // Convenience function to check framebuffer status and throw a std::runtime_error if incomplete

#endif
