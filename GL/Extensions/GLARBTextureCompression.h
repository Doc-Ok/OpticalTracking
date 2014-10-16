/***********************************************************************
GLARBTextureCompression - OpenGL extension class for the
GL_ARB_texture_compression extension.
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

#ifndef GLEXTENSIONS_GLARBTEXTURECOMPRESSION_INCLUDED
#define GLEXTENSIONS_GLARBTEXTURECOMPRESSION_INCLUDED

#include <GL/gl.h>
#include <GL/TLSHelper.h>
#include <GL/Extensions/GLExtension.h>

/********************************
Extension-specific parts of gl.h:
********************************/

#ifndef GL_ARB_texture_compression
#define GL_ARB_texture_compression 1

/* Extension-specific functions: */
typedef void (APIENTRY * PFNGLCOMPRESSEDTEXIMAGE3DARBPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data);
typedef void (APIENTRY * PFNGLCOMPRESSEDTEXIMAGE2DARBPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data);
typedef void (APIENTRY * PFNGLCOMPRESSEDTEXIMAGE1DARBPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data);
typedef void (APIENTRY * PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data);
typedef void (APIENTRY * PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data);
typedef void (APIENTRY * PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data);
typedef void (APIENTRY * PFNGLGETCOMPRESSEDTEXIMAGEARBPROC) (GLenum target, GLint level, GLvoid *img);

/* Extension-specific constants: */
#define GL_COMPRESSED_ALPHA_ARB           0x84E9
#define GL_COMPRESSED_LUMINANCE_ARB       0x84EA
#define GL_COMPRESSED_LUMINANCE_ALPHA_ARB 0x84EB
#define GL_COMPRESSED_INTENSITY_ARB       0x84EC
#define GL_COMPRESSED_RGB_ARB             0x84ED
#define GL_COMPRESSED_RGBA_ARB            0x84EE
#define GL_TEXTURE_COMPRESSION_HINT_ARB   0x84EF
#define GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB 0x86A0
#define GL_TEXTURE_COMPRESSED_ARB         0x86A1
#define GL_NUM_COMPRESSED_TEXTURE_FORMATS_ARB 0x86A2
#define GL_COMPRESSED_TEXTURE_FORMATS_ARB 0x86A3

#endif

/* Forward declarations of friend functions: */
void glCompressedTexImage3DARB(GLenum target,GLint level,GLenum internalformat,GLsizei width,GLsizei height,GLsizei depth,GLint border,GLsizei imageSize,const GLvoid* data);
void glCompressedTexImage2DARB(GLenum target,GLint level,GLenum internalformat,GLsizei width,GLsizei height,GLint border,GLsizei imageSize,const GLvoid* data);
void glCompressedTexImage1DARB(GLenum target,GLint level,GLenum internalformat,GLsizei width,GLint border,GLsizei imageSize,const GLvoid* data);
void glCompressedTexSubImage3DARB(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLint zoffset,GLsizei width,GLsizei height,GLsizei depth,GLenum format,GLsizei imageSize,const GLvoid* data);
void glCompressedTexSubImage2DARB(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLsizei width,GLsizei height,GLenum format,GLsizei imageSize,const GLvoid* data);
void glCompressedTexSubImage1DARB(GLenum target,GLint level,GLint xoffset,GLsizei width,GLenum format,GLsizei imageSize,const GLvoid* data);
void glGetCompressedTexImageARB(GLenum target,GLint level,GLvoid* img);

class GLARBTextureCompression:public GLExtension
	{
	/* Elements: */
	private:
	static GL_THREAD_LOCAL(GLARBTextureCompression*) current; // Pointer to extension object for current OpenGL context
	static const char* name; // Extension name
	PFNGLCOMPRESSEDTEXIMAGE3DARBPROC glCompressedTexImage3DARBProc;
	PFNGLCOMPRESSEDTEXIMAGE2DARBPROC glCompressedTexImage2DARBProc;
	PFNGLCOMPRESSEDTEXIMAGE1DARBPROC glCompressedTexImage1DARBProc;
	PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC glCompressedTexSubImage3DARBProc;
	PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC glCompressedTexSubImage2DARBProc;
	PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC glCompressedTexSubImage1DARBProc;
	PFNGLGETCOMPRESSEDTEXIMAGEARBPROC glGetCompressedTexImageARBProc;
	
	/* Constructors and destructors: */
	private:
	GLARBTextureCompression(void);
	public:
	virtual ~GLARBTextureCompression(void);
	
	/* Methods: */
	public:
	virtual const char* getExtensionName(void) const;
	virtual void activate(void);
	virtual void deactivate(void);
	static bool isSupported(void); // Returns true if the extension is supported in the current OpenGL context
	static void initExtension(void); // Initializes the extension in the current OpenGL context
	
	/* Extension entry points: */
	inline friend void glCompressedTexImage3DARB(GLenum target,GLint level,GLenum internalformat,GLsizei width,GLsizei height,GLsizei depth,GLint border,GLsizei imageSize,const GLvoid* data)
		{
		GLARBTextureCompression::current->glCompressedTexImage3DARBProc(target,level,internalformat,width,height,depth,border,imageSize,data);
		}
	inline friend void glCompressedTexImage2DARB(GLenum target,GLint level,GLenum internalformat,GLsizei width,GLsizei height,GLint border,GLsizei imageSize,const GLvoid* data)
		{
		GLARBTextureCompression::current->glCompressedTexImage2DARBProc(target,level,internalformat,width,height,border,imageSize,data);
		}
	inline friend void glCompressedTexImage1DARB(GLenum target,GLint level,GLenum internalformat,GLsizei width,GLint border,GLsizei imageSize,const GLvoid* data)
		{
		GLARBTextureCompression::current->glCompressedTexImage1DARBProc(target,level,internalformat,width,border,imageSize,data);
		}
	inline friend void glCompressedTexSubImage3DARB(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLint zoffset,GLsizei width,GLsizei height,GLsizei depth,GLenum format,GLsizei imageSize,const GLvoid* data)
		{
		GLARBTextureCompression::current->glCompressedTexSubImage3DARBProc(target,level,xoffset,yoffset,zoffset,width,height,depth,format,imageSize,data);
		}
	inline friend void glCompressedTexSubImage2DARB(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLsizei width,GLsizei height,GLenum format,GLsizei imageSize,const GLvoid* data)
		{
		GLARBTextureCompression::current->glCompressedTexSubImage2DARBProc(target,level,xoffset,yoffset,width,height,format,imageSize,data);
		}
	inline friend void glCompressedTexSubImage1DARB(GLenum target,GLint level,GLint xoffset,GLsizei width,GLenum format,GLsizei imageSize,const GLvoid* data)
		{
		GLARBTextureCompression::current->glCompressedTexSubImage1DARBProc(target,level,xoffset,width,format,imageSize,data);
		}
	inline friend void glGetCompressedTexImageARB(GLenum target,GLint level,GLvoid* img)
		{
		GLARBTextureCompression::current->glGetCompressedTexImageARBProc(target,level,img);
		}
	};

/*******************************
Extension-specific entry points:
*******************************/

#endif
