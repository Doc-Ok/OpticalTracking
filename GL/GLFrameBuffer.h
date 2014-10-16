/***********************************************************************
GLFrameBuffer - Simple class to encapsulate the state of and operations
on OpenGL frame buffer objects.
Copyright (c) 2012-2013 Oliver Kreylos

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

#ifndef GLFRAMEBUFFER_INCLUDED
#define GLFRAMEBUFFER_INCLUDED

#include <GL/gl.h>
#include <GL/Extensions/GLARBDepthTexture.h>
#include <GL/Extensions/GLARBTextureRectangle.h>
#include <GL/Extensions/GLEXTFramebufferObject.h>

class GLFrameBuffer
	{
	/* Embedded classes: */
	public:
	class Binder // Helper class to bind framebuffers and restore previous bindings using RAII
		{
		/* Elements: */
		private:
		GLuint previousFrameBufferId; // ID of previously bound frame buffer object
		
		/* Constructors and destructors: */
		public:
		Binder(const GLFrameBuffer& frameBuffer) // Binds the given frame buffer object
			{
			/* Query the currently bound frame buffer object: */
			glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT,&previousFrameBufferId);
			
			/* Bind the given frame buffer object: */
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,frameBuffer.frameBufferId);
			}
		~Binder(void)
			{
			/* Bind the previously bound frame buffer object: */
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,previousFrameBufferId);
			}
		};
	
	friend class Binder;
	
	/* Elements: */
	private:
	GLsizei size[2]; // The width and height of the frame buffer in pixels
	bool haveTextureRectangles; // Flag whether the local OpenGL support texture rectangles
	GLenum textureTarget; // Texture target to use for texture attachments
	GLsizei paddedSize[2]; // Actual size of the frame buffer in pixels, after padding if requested or rectangle textures are not supported
	GLuint frameBufferId; // The ID of the frame buffer object
	bool haveDepthTextures; // Flag whether the local OpenGL supports depth textures
	bool depthIsTexture; // Flag if the depth buffer is a texture object
	GLuint depthBufferId; // The ID of the render buffer or texture object attached as depth buffer; 0 if no depth buffer is used
	GLsizei numColorAttachments; // Number of color buffers attachable to a single frame buffer
	bool* colorIsTextures; // Array of flags if each of the color buffers is a texture object
	GLuint* colorBufferIds; // Array of IDs of the render buffer of texture objects attached as color buffers; 0 if no color buffer is used in a slot
	GLuint stencilBufferId; // The ID of the render buffer attached as a stencil buffer; 0 if no stencil buffer is used
	
	/* Private methods: */
	void deleteDepthAttachment(void); // Deletes the currently attached depth render buffer or texture
	void deleteColorAttachment(GLint colorAttachmentIndex); // Deletes the currently attached color render buffer or texture in the given attachment slot
	void bindAttachments(void); // Binds all depth and color attachments to the frame buffer
	void checkConsistency(void); // Checks the frame buffer's consistency after all attachments have been bound and read and draw buffers have been selected
	
	/* Constructors and destructors: */
	public:
	GLFrameBuffer(GLsizei width,GLsizei height,bool pad =false); // Creates a frame buffer of the given size with no attachments; pads size to power of two if flag is true
	private:
	GLFrameBuffer(const GLFrameBuffer& source); // Prohibit copy constructor
	GLFrameBuffer& operator=(const GLFrameBuffer& source); // Prohibit assignment operator
	public:
	~GLFrameBuffer(void); // Destroys the frame buffer and releases all allocated resources
	
	/* Methods: */
	static bool isSupported(void); // Returns true if the current OpenGL context supports frame buffer objects
	const GLsizei* getSize(void) const // Returns the width and height of the frame buffer
		{
		return size;
		}
	GLsizei getSize(int dimension) const // Ditto
		{
		return size[dimension];
		}
	void attachDepthBuffer(void); // Attaches a render buffer as the frame buffer's depth buffer
	bool canAttachDepthTexture(void) const // Returns true if the frame buffer supports use of textures as depth buffers
		{
		return haveDepthTextures;
		}
	void attachDepthTexture(GLenum pixelFormat =GL_DEPTH_COMPONENT24_ARB,GLenum filterMode =GL_NEAREST); // Attaches a texture object as the frame buffer's depth buffer
	void bindDepthTexture(void) const // Binds the texture object attached as depth buffer to the given texture target; fails if depth buffer is not a texture
		{
		glBindTexture(textureTarget,depthBufferId);
		}
	GLsizei getNumColorBuffers(void) const // Returns the maximum number of color buffer attachments supported by this frame buffer
		{
		return numColorAttachments;
		}
	void attachColorBuffer(GLint colorAttachmentIndex,GLenum pixelFormat); // Attaches a render buffer as the frame buffer's color buffer of the given index
	void attachColorTexture(GLint colorAttachmentIndex,GLenum pixelFormat,GLenum filterMode =GL_NEAREST); // Attaches a texture object as the frame buffer's color buffer of the given index
	void bindColorTexture(GLint colorAttachmentIndex) const // Binds the texture object attached as color buffer of the given index to the given texture target; fails if selected color buffer is not a texture
		{
		glBindTexture(textureTarget,colorBufferIds[colorAttachmentIndex]);
		}
	void attachStencilBuffer(GLenum pixelFormat =GL_STENCIL_INDEX8_EXT); // Attaches a render buffer as the frame buffer's stencil buffer
	void finish(GLenum readAttachment,GLenum writeAttachment); // Finishes the frame buffer; throws exception if the frame buffer is inconsistent
	void setDrawBuffer(GLenum attachmentIndex); // Selects a single color attachment for drawing
	void setDrawBuffers(GLsizei numAttachments,...); // Selects multiple color attachments for drawing
	void setReadBuffer(GLenum attachmentIndex); // Selects a single color attachment for reading
	void bind(void) const; // Binds this frame buffer object in the current OpenGL context
	static void unbind(void); // Unbinds the currently bound frame buffer object in the current OpenGL context
	};

#endif
