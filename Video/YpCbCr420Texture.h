/***********************************************************************
YpCbCr420Texture - Class to use video frames in Y'CbCr 4:2:0 pixel
format as OpenGL textures.
Copyright (c) 2010-2013 Oliver Kreylos

This file is part of the Basic Video Library (Video).

The Basic Video Library is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as published
by the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

The Basic Video Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Basic Video Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef VIDEO_YPCBCR420TEXTURE_INCLUDED
#define VIDEO_YPCBCR420TEXTURE_INCLUDED

#include <GL/gl.h>
#include <GL/GLShader.h>
#include <GL/GLObject.h>

namespace Video {

class YpCbCr420Texture:public GLObject
	{
	/* Embedded classes: */
	private:
	struct ImagePlane // Structure for image planes
		{
		/* Elements: */
		public:
		unsigned int size[2]; // Plane's width and height
		const unsigned char* base; // Pointer to first pixel in image plane; storage is not owned by the object
		ptrdiff_t stride; // Offset in bytes between adjacent pixel rows in the image plane
		};
	
	struct DataItem:public GLObject::DataItem // Structure to hold per-context state
		{
		/* Elements: */
		public:
		bool npotdtSupported; // Flag whether the OpenGL context supports non-power-of-two dimension textures
		bool shaderSupported; // Flag whether GLSL shaders are supported by OpenGL
		GLuint planeTextureIds[3]; // Texture object IDs for the Y', Cb, and Cr image planes, respectively
		bool chromaKey; // Flag whether the currently compiled shader has chroma keying enabled
		GLShader ypcbcr420Shader; // GLSL shader to convert textures in Y'CbCr 4:2:0 pixel format to RGB on-the-fly
		int textureSamplerLocs[3]; // Location of the texture sampler uniform variables for Y', Cb, and Cr
		unsigned int textureSizes[3][2]; // Current size of the three image plane textures
		unsigned int frameNumber; // Version number of the video frame currently cached in the texture objects
		
		/* Constructors and destructors: */
		DataItem(void);
		virtual ~DataItem(void);
		
		/* Methods: */
		void buildShader(bool newChromaKey); // Rebuilds the rendering shader (assuming that shaders are supported) with the given chroma key setting
		};
	
	/* Elements: */
	private:
	bool chromaKey; // Flag whether to enable chroma keying on purple
	ImagePlane planes[3]; // The Yp, Cb, and Cr image planes, respectively
	unsigned int frameNumber; // Version number of the current video frame
	
	/* Constructors and destructors: */
	public:
	YpCbCr420Texture(void);
	
	/* Methods from GLObject: */
	virtual void initContext(GLContextData& contextData) const;
	
	/* New methods: */
	bool getChromaKey(void) // Returns the chroma keying flag
		{
		return chromaKey;
		}
	const unsigned int* getFrameSize(void) const // Returns the size of the video frame
		{
		return planes[0].size;
		}
	unsigned int getFrameWidth(void) const // Returns the width of the video frame
		{
		return planes[0].size[0];
		}
	unsigned int getFrameHeight(void) const // Returns the width of the video frame
		{
		return planes[0].size[1];
		}
	void setChromaKey(bool newChromaKey); // Enables or disables chroma keying on purple
	void setFrameSize(unsigned int newFrameWidth,unsigned int newFrameHeight); // Sets the video frame size for the next and all following frames
	void setFrame(const void* yp,unsigned int ypStride,const void* cb,unsigned int cbStride,const void* cr,unsigned int crStride); // Sets new frame; frame is not copied and storage must stay valid until next glRenderAction() call
	bool haveFrame(void) const // Returns true if the video texture contains a valid frame
		{
		return frameNumber>0;
		}
	void install(GLContextData& contextData,GLfloat texCoord[2]) const; // Sets the OpenGL context to apply the current video frame as a texture to all following primitives; updates texture coordinate to upper-right corner of texture
	void uninstall(GLContextData& contextData) const; // Resets the OpenGL context to its previous state
	};

}

#endif
