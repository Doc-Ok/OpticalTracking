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

#include <Video/YpCbCr420Texture.h>

#include <GL/gl.h>
#include <GL/Extensions/GLARBMultitexture.h>
#include <GL/Extensions/GLARBTextureNonPowerOfTwo.h>
#include <GL/GLContextData.h>
#include <Video/Colorspaces.h>

namespace Video {

/*******************************************
Methods of class YpCbCr420Texture::DataItem:
*******************************************/

YpCbCr420Texture::DataItem::DataItem(void)
	:npotdtSupported(GLARBTextureNonPowerOfTwo::isSupported()),
	 shaderSupported(GLARBMultitexture::isSupported()&&GLShader::isSupported()),
	 chromaKey(false),frameNumber(0)
	{
	/* Initialize optional OpenGL extensions: */
	if(npotdtSupported)
		GLARBTextureNonPowerOfTwo::initExtension();
	if(shaderSupported)
		GLARBMultitexture::initExtension();
	
	/* Create the texture objects: */
	if(shaderSupported)
		{
		/* Create three texture objects containing the Yp, Cb, and Cr image planes, respectively: */
		glGenTextures(3,planeTextureIds);
		}
	else
		{
		/* Create a single texture object containing an RGB video frame: */
		glGenTextures(1,planeTextureIds);
		}
	
	/* Initialize the texture sizes: */
	for(int i=0;i<3;++i)
		textureSizes[i][0]=textureSizes[i][1]=0;
	}

YpCbCr420Texture::DataItem::~DataItem(void)
	{
	/* Destroy the texture objects: */
	if(shaderSupported)
		glDeleteTextures(3,planeTextureIds);
	else
		glDeleteTextures(1,planeTextureIds);
	}

namespace {

/***************************************
Source code for the YbCbCr 4:2:0 shader:
***************************************/

const char* ypcbcr420VertexShaderSource="\
	void main() \n\
		{ \n\
		/* Copy the texture coordinate: */ \n\
		gl_TexCoord[0]=gl_MultiTexCoord0; \n\
		 \n\
		/* Transform the vertex: */ \n\
		gl_Position=ftransform(); \n\
		}\n\
	";

const char* ypcbcr420FragmentShaderSource="\
	uniform sampler2D ypTextureSampler; // Sampler for input Y' texture \n\
	uniform sampler2D cbTextureSampler; // Sampler for input Cb texture \n\
	uniform sampler2D crTextureSampler; // Sampler for input Cr texture \n\
	 \n\
	void main() \n\
		{ \n\
		/* Get the interpolated texture color in Y'CbCr space: */ \n\
		vec3 ypcbcr; \n\
		ypcbcr.r=texture2D(ypTextureSampler,gl_TexCoord[0].st); \n\
		ypcbcr.g=texture2D(cbTextureSampler,gl_TexCoord[0].st); \n\
		ypcbcr.b=texture2D(crTextureSampler,gl_TexCoord[0].st); \n\
		 \n\
		/* Convert the color to RGB directly: */ \n\
		float grey=(ypcbcr[0]-16.0/255.0)*1.16438; \n\
		vec4 rgb; \n\
		rgb[0]=grey+(ypcbcr[2]-128.0/255.0)*1.59603; \n\
		rgb[1]=grey-(ypcbcr[1]-128.0/255.0)*0.391761-(ypcbcr[2]-128.0/255.0)*0.81297; \n\
		rgb[2]=grey+(ypcbcr[1]-128.0/255.0)*2.01723; \n\
		rgb[3]=1.0; \n\
		\n\
		/* Store the final color: */ \n\
		gl_FragColor=rgb; \n\
		}\n\
	";

const char* ypcbcr420FragmentShaderSourceChromaKey="\
	uniform sampler2D ypTextureSampler; // Sampler for input Y' texture \n\
	uniform sampler2D cbTextureSampler; // Sampler for input Cb texture \n\
	uniform sampler2D crTextureSampler; // Sampler for input Cr texture \n\
	 \n\
	void main() \n\
		{ \n\
		/* Get the interpolated texture color in Y'CbCr space: */ \n\
		vec3 ypcbcr; \n\
		ypcbcr.r=texture2D(ypTextureSampler,gl_TexCoord[0].st); \n\
		ypcbcr.g=texture2D(cbTextureSampler,gl_TexCoord[0].st); \n\
		ypcbcr.b=texture2D(crTextureSampler,gl_TexCoord[0].st); \n\
		 \n\
		/* Convert the color to RGB directly: */ \n\
		float grey=(ypcbcr[0]-16.0/255.0)*1.16438; \n\
		vec4 rgb; \n\
		rgb[0]=grey+(ypcbcr[2]-128.0/255.0)*1.59603; \n\
		rgb[1]=grey-(ypcbcr[1]-128.0/255.0)*0.391761-(ypcbcr[2]-128.0/255.0)*0.81297; \n\
		rgb[2]=grey+(ypcbcr[1]-128.0/255.0)*2.01723; \n\
		rgb[3]=1.0; \n\
		 \n\
		/* Chroma-key the pixel: */ \n\
		if(rgb[0]>=0.95&&rgb[1]<=0.05&&rgb[2]>=0.95) \n\
			discard; \n\
		\n\
		/* Store the final color: */ \n\
		gl_FragColor=rgb; \n\
		}\n\
	";

}

void YpCbCr420Texture::DataItem::buildShader(bool newChromaKey)
	{
	/* Copy the chroma key setting: */
	chromaKey=newChromaKey;

	/* Build the Y'CbCr 4:2:0 -> RGB conversion shader: */
	ypcbcr420Shader.compileVertexShaderFromString(ypcbcr420VertexShaderSource);
	ypcbcr420Shader.compileFragmentShaderFromString(chromaKey?ypcbcr420FragmentShaderSourceChromaKey:ypcbcr420FragmentShaderSource);
	ypcbcr420Shader.linkShader();

	/* Get the shader's texture sampler uniform location: */
	textureSamplerLocs[0]=ypcbcr420Shader.getUniformLocation("ypTextureSampler");
	textureSamplerLocs[1]=ypcbcr420Shader.getUniformLocation("cbTextureSampler");
	textureSamplerLocs[2]=ypcbcr420Shader.getUniformLocation("crTextureSampler");
	}

/*********************************
Methods of class YpCbCr420Texture:
*********************************/

YpCbCr420Texture::YpCbCr420Texture(void)
	:GLObject(false),
	 chromaKey(false),frameNumber(0)
	{
	/* Initialize the image planes: */
	for(int i=0;i<3;++i)
		{
		planes[i].size[0]=planes[i].size[1]=0;
		planes[i].base=0;
		planes[i].stride=0;
		}
	
	GLObject::init();
	}

void YpCbCr420Texture::initContext(GLContextData& contextData) const
	{
	/* Create and store the data item: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	
	/* Initialize the texture objects: */
	int numTextures=dataItem->shaderSupported?3:1;
	for(int i=0;i<numTextures;++i)
		{
		/* Bind the texture: */
		glBindTexture(GL_TEXTURE_2D,dataItem->planeTextureIds[i]);
		
		/* Initialize basic texture settings: */
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_BASE_LEVEL,0);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAX_LEVEL,0);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		}
	glBindTexture(GL_TEXTURE_2D,0);
	
	if(dataItem->shaderSupported)
		{
		/* Build the rendering shader: */
		dataItem->buildShader(chromaKey);
		}
	}

void YpCbCr420Texture::setChromaKey(bool newChromaKey)
	{
	/* Set the flag: */
	chromaKey=newChromaKey;
	}

void YpCbCr420Texture::setFrameSize(unsigned int newFrameWidth,unsigned int newFrameHeight)
	{
	/* Initialize the image plane's sizes: */
	for(int i=0;i<3;++i)
		{
		planes[i].size[0]=newFrameWidth;
		planes[i].size[1]=newFrameHeight;
		if(i>0)
			{
			planes[i].size[0]>>=1;
			planes[i].size[1]>>=1;
			}
		}
	}

void YpCbCr420Texture::setFrame(const void* yp,unsigned int ypStride,const void* cb,unsigned int cbStride,const void* cr,unsigned int crStride)
	{
	/* Retain the frame's plane storage descriptors: */
	planes[0].base=static_cast<const unsigned char*>(yp);
	planes[0].stride=ptrdiff_t(ypStride);
	planes[1].base=static_cast<const unsigned char*>(cb);
	planes[1].stride=ptrdiff_t(cbStride);
	planes[2].base=static_cast<const unsigned char*>(cr);
	planes[2].stride=ptrdiff_t(crStride);
	
	/* Increment the frame number to invalidate the cached frame: */
	++frameNumber;
	}

void YpCbCr420Texture::install(GLContextData& contextData,GLfloat texCoord[2]) const
	{
	/* Get the context data item: */
	DataItem* dataItem=contextData.retrieveDataItem<DataItem>(this);
	
	glPushAttrib(GL_TEXTURE_BIT);
	
	if(dataItem->shaderSupported)
		{
		/* Bind and, if necessary, update the image plane textures: */
		for(int i=0;i<3;++i)
			{
			/* Bind the plane texture: */
			glActiveTextureARB(GL_TEXTURE0_ARB+i);
			glBindTexture(GL_TEXTURE_2D,dataItem->planeTextureIds[i]);
			
			/* Check if it's outdated: */
			if(dataItem->frameNumber!=frameNumber)
				{
				/* Calculate the plane's texture size: */
				if(dataItem->npotdtSupported)
					{
					dataItem->textureSizes[i][0]=planes[i].size[0];
					dataItem->textureSizes[i][1]=planes[i].size[1];
					}
				else
					{
					/* Calculate the next power-of-two texture size: */
					bool mustResize=false;
					for(int j=0;j<2;++j)
						{
						unsigned int newTextureSize;
						for(newTextureSize=1;newTextureSize<planes[i].size[j];newTextureSize<<=1)
							;
						if(dataItem->textureSizes[i][j]!=newTextureSize)
							{
							dataItem->textureSizes[i][j]=newTextureSize;
							mustResize=true;
							}
						}
					
					if(mustResize)
						{
						/* Re-upload the texture: */
						glTexImage2D(GL_TEXTURE_2D,0,GL_LUMINANCE8,dataItem->textureSizes[i][0],dataItem->textureSizes[i][1],0,GL_LUMINANCE,GL_UNSIGNED_BYTE,0);
						}
					}
				
				/* Set up the pixel transfer pipeline: */
				glPixelStorei(GL_UNPACK_ROW_LENGTH,planes[i].stride);
				glPixelStorei(GL_UNPACK_SKIP_ROWS,0);
				glPixelStorei(GL_UNPACK_SKIP_PIXELS,0);
				glPixelStorei(GL_UNPACK_ALIGNMENT,1);
				
				/* Upload the plane texture: */
				if(dataItem->npotdtSupported)
					glTexImage2D(GL_TEXTURE_2D,0,GL_LUMINANCE8,planes[i].size[0],planes[i].size[1],0,GL_LUMINANCE,GL_UNSIGNED_BYTE,planes[i].base);
				else
					glTexSubImage2D(GL_TEXTURE_2D,0,0,0,planes[i].size[0],planes[i].size[1],GL_LUMINANCE,GL_UNSIGNED_BYTE,planes[i].base);
				}
			}
		glActiveTextureARB(GL_TEXTURE0_ARB);
		glPixelStorei(GL_UNPACK_ROW_LENGTH,0);
		
		/* Mark the image plane textures as valid: */
		dataItem->frameNumber=frameNumber;
		
		/* Check if the shader is valid: */
		if(dataItem->chromaKey!=chromaKey)
			{
			/* Rebuild the rendering shader: */
			dataItem->buildShader(chromaKey);
			}
		
		/* Install the Y'CbCr 4:2:0 -> RGB shader: */
		dataItem->ypcbcr420Shader.useProgram();
		glUniform1iARB(dataItem->textureSamplerLocs[0],0);
		glUniform1iARB(dataItem->textureSamplerLocs[1],1);
		glUniform1iARB(dataItem->textureSamplerLocs[2],2);
		}
	else
		{
		/* Bind the luminance texture: */
		glBindTexture(GL_TEXTURE_2D,dataItem->planeTextureIds[0]);
		
		/* Check if it's outdated: */
		if(dataItem->frameNumber!=frameNumber)
			{
			/* Calculate the texture's size: */
			if(dataItem->npotdtSupported)
				{
				dataItem->textureSizes[0][0]=planes[0].size[0];
				dataItem->textureSizes[0][1]=planes[0].size[1];
				}
			else
				{
				/* Calculate the next power-of-two texture size: */
				bool mustResize=false;
				for(int j=0;j<2;++j)
					{
					unsigned int newTextureSize;
					for(newTextureSize=1;newTextureSize<planes[0].size[j];newTextureSize<<=1)
						;
					if(dataItem->textureSizes[0][j]!=newTextureSize)
						{
						dataItem->textureSizes[0][j]=newTextureSize;
						mustResize=true;
						}
					}
				
				if(mustResize)
					{
					/* Re-upload the texture: */
					glTexImage2D(GL_TEXTURE_2D,0,GL_RGB8,dataItem->textureSizes[0][0],dataItem->textureSizes[0][1],0,GL_RGB,GL_UNSIGNED_BYTE,0);
					}
				}
			
			/* Convert the Y'CbCr 4:2:0 image to a temporary RGB image: */
			unsigned char* rgb=new unsigned char[planes[0].size[0]*planes[0].size[1]*3];
			const unsigned char* ypRowPtr=planes[0].base;
			const unsigned char* cbRowPtr=planes[1].base;
			const unsigned char* crRowPtr=planes[2].base;
			unsigned char* rgbRowPtr=rgb;
			for(unsigned int y=0;y<planes[0].size[1];y+=2)
				{
				/* Convert two rows: */
				const unsigned char* ypPtr=ypRowPtr;
				const unsigned char* cbPtr=cbRowPtr;
				const unsigned char* crPtr=crRowPtr;
				unsigned char* rgbPtr=rgbRowPtr;
				for(unsigned int x=0;x<planes[0].size[0];x+=2)
					{
					/* Convert a 2x2 pixel block: */
					unsigned char ypcbcr[3];
					ypcbcr[0]=ypPtr[0];
					ypcbcr[1]=*cbPtr;
					ypcbcr[2]=*crPtr;
					Video::ypcbcrToRgb(ypcbcr,rgbPtr);
					
					ypcbcr[0]=ypPtr[1];
					Video::ypcbcrToRgb(ypcbcr,rgbPtr+3);
					
					ypcbcr[0]=ypPtr[planes[0].stride];
					Video::ypcbcrToRgb(ypcbcr,rgbPtr+planes[0].size[0]*3);
					
					ypcbcr[0]=ypPtr[planes[0].stride+1];
					Video::ypcbcrToRgb(ypcbcr,rgbPtr+planes[0].size[0]*3+3);
					
					/* Go to the next pixel block: */
					ypPtr+=2;
					++cbPtr;
					++crPtr;
					rgbPtr+=2*3;
					}
				
				/* Go to the next two rows: */
				ypRowPtr+=planes[0].stride*2;
				cbRowPtr+=planes[1].stride;
				crRowPtr+=planes[2].stride;
				rgbRowPtr+=planes[0].size[0]*2*3;
				}
			
			/* Set up the pixel transfer pipeline: */
			glPixelStorei(GL_UNPACK_ROW_LENGTH,0);
			glPixelStorei(GL_UNPACK_SKIP_ROWS,0);
			glPixelStorei(GL_UNPACK_SKIP_PIXELS,0);
			glPixelStorei(GL_UNPACK_ALIGNMENT,1);
			
			/* Upload the RGB texture: */
			if(dataItem->npotdtSupported)
				glTexImage2D(GL_TEXTURE_2D,0,GL_RGB8,planes[0].size[0],planes[0].size[1],0,GL_RGB,GL_UNSIGNED_BYTE,rgb);
			else
				glTexSubImage2D(GL_TEXTURE_2D,0,0,0,planes[0].size[0],planes[0].size[1],GL_RGB,GL_UNSIGNED_BYTE,rgb);
			
			/* Destroy the temporary RGB image: */
			delete[] rgb;
			}
		
		/* Enable texturing: */
		glEnable(GL_TEXTURE_2D);
		glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
		}
	
	/* Calculate the upper-right corner texture coordinate: */
	for(int i=0;i<2;++i)
		texCoord[i]=GLfloat(planes[0].size[i])/GLfloat(dataItem->textureSizes[0][i]);
	}

void YpCbCr420Texture::uninstall(GLContextData& contextData) const
	{
	/* Get the context data item: */
	DataItem* dataItem=contextData.retrieveDataItem<DataItem>(this);
	
	if(dataItem->shaderSupported)
		{
		/* Disable the shader: */
		GLShader::disablePrograms();
		
		/* Unbind the component textures: */
		for(int i=2;i>=0;--i)
			{
			glActiveTextureARB(GL_TEXTURE0_ARB+i);
			glBindTexture(GL_TEXTURE_2D,0);
			}
		}
	else
		{
		/* Disable texturing: */
		glDisable(GL_TEXTURE_2D);
		
		/* Unbind the luminance texture: */
		glBindTexture(GL_TEXTURE_2D,0);
		}
	
	glPopAttrib();
	}

}
