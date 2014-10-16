/***********************************************************************
GLFont - Class to represent texture-based fonts and to render 3D text.
Copyright (c) 1999-2014 Oliver Kreylos

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

#include <GL/GLFont.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Misc/ThrowStdErr.h>
#include <IO/File.h>
#include <IO/OpenFile.h>
#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLTexEnvTemplates.h>
#include <GL/GLTexCoordTemplates.h>
#include <GL/GLVertexTemplates.h>
#include <GL/Config.h>

/*********************************
Methods of class GLFont::CharInfo:
*********************************/

void GLFont::CharInfo::read(IO::File& file)
	{
	file.read(width);
	file.read(ascent);
	file.read(descent);
	file.read(glyphOffset);
	file.read(rasterLineOffset);
	file.read(spanOffset);
	}

/***********************
Methods of class GLFont:
***********************/

GLsizei GLFont::calcStringWidth(const char* string) const
	{
	/* Figure out the string's total width in texels: */
	GLsizei result=maxLeftLap+maxRightLap+2;
	if(string!=0)
		{
		for(const char* cPtr=string;*cPtr!=0;++cPtr)
			{
			int charIndex=int(*cPtr)-firstCharacter;
			if(charIndex>=0&&charIndex<numCharacters)
				result+=characters[charIndex].width;
			}
		}
	
	return result;
	}

void GLFont::uploadStringTexture(const char* string,GLsizei stringWidth,GLsizei textureWidth) const
	{
	/* Calculate the proper texture image dimensions: */
	GLsizei imageWidth,imageHeight;
	int x,baseLineRow;
	if(antialiasing)
		{
		imageWidth=stringWidth;
		imageHeight=fontHeight;
		baseLineRow=baseLine;
		x=maxLeftLap+1;
		}
	else
		{
		imageWidth=stringWidth;
		imageHeight=fontHeight;
		baseLineRow=baseLine;
		x=maxLeftLap+1;
		}
	
	/* Create a luminance-only texture image of appropriate size: */
	GLubyte* image=new GLubyte[imageWidth*imageHeight];
	memset(image,255,imageWidth*imageHeight);
	
	if(string!=0)
		{
		/* Copy all characters into the texture image: */
		for(const char* cPtr=string;*cPtr!=0;++cPtr)
			{
			int charIndex=int(*cPtr)-firstCharacter;
			if(charIndex>=0&&charIndex<numCharacters)
				{
				const CharInfo* ciPtr=&characters[charIndex];
				const unsigned char* rasterLine=&rasterLines[ciPtr->rasterLineOffset];
				const unsigned char* span=&spans[ciPtr->spanOffset];
				
				/* Copy all raster lines: */
				for(int y=baseLineRow-ciPtr->descent;y<baseLineRow+ciPtr->ascent;++y,++rasterLine)
					{
					/* Copy all spans in this line: */
					GLubyte* texPtr=&image[imageWidth*y+x+ciPtr->glyphOffset];
					int numSpans=int(*rasterLine);
					for(int i=0;i<numSpans;++i,++span)
						{
						texPtr+=int((*span)>>3);
						int numPixels=int((*span)&0x07);
						for(int j=0;j<numPixels;++j,++texPtr)
							*texPtr=GLubyte(0);
						}
					}
				
				x+=ciPtr->width;
				}
			}
		}
	
	/* Upload the created texture image: */
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS,0);
	glPixelStorei(GL_UNPACK_ROW_LENGTH,0);
	glPixelStorei(GL_UNPACK_SKIP_ROWS,0);
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	glTexImage2D(GL_TEXTURE_2D,0,GL_LUMINANCE,textureWidth,textureHeight,0,GL_LUMINANCE,GL_UNSIGNED_BYTE,0);
	if(antialiasing)
		{
		static GLfloat kernel[3]={0.25,0.5,0.25};
		// static GLfloat kernel[5]={0.125,0.2,0.35,0.2,0.125};
		glConvolutionParameteri(GL_SEPARABLE_2D,GL_CONVOLUTION_BORDER_MODE,GL_REPLICATE_BORDER);
		glSeparableFilter2D(GL_SEPARABLE_2D,GL_LUMINANCE,3,3,GL_LUMINANCE,GL_FLOAT,kernel,kernel);
		glEnable(GL_SEPARABLE_2D);
		glTexSubImage2D(GL_TEXTURE_2D,0,0,0,imageWidth,imageHeight,GL_LUMINANCE,GL_UNSIGNED_BYTE,image);
		glDisable(GL_SEPARABLE_2D);
		}
	else
		glTexSubImage2D(GL_TEXTURE_2D,0,0,0,imageWidth,imageHeight,GL_LUMINANCE,GL_UNSIGNED_BYTE,image);
	
	/* Clean up and return: */
	delete[] image;
	}

void GLFont::uploadStringTexture(const char* string,const GLFont::Color& stringBackgroundColor,const GLFont::Color& stringForegroundColor,GLsizei stringWidth,GLsizei textureWidth) const
	{
	/* Calculate the proper texture image dimensions: */
	GLsizei imageWidth,imageHeight;
	int x,baseLineRow;
	if(antialiasing)
		{
		imageWidth=stringWidth;
		imageHeight=fontHeight;
		baseLineRow=baseLine;
		x=maxLeftLap+1;
		}
	else
		{
		imageWidth=stringWidth;
		imageHeight=fontHeight;
		baseLineRow=baseLine;
		x=maxLeftLap+1;
		}
	
	/* Convert the string colors to 8-bit RGBA: */
	GLColor<GLubyte,4> bg=stringBackgroundColor;
	GLColor<GLubyte,4> fg=stringForegroundColor;
	
	/* Create an RGBA texture image of appropriate size: */
	GLColor<GLubyte,4>* image=new GLColor<GLubyte,4>[imageWidth*imageHeight];
	GLColor<GLubyte,4>* iPtr=image;
	for(int i=imageWidth*imageHeight;i>0;--i,++iPtr)
		*iPtr=bg;
	
	if(string!=0)
		{
		/* Copy all characters into the texture image: */
		for(const char* cPtr=string;*cPtr!=0;++cPtr)
			{
			int charIndex=int(*cPtr)-firstCharacter;
			if(charIndex>=0&&charIndex<numCharacters)
				{
				const CharInfo* ciPtr=&characters[charIndex];
				const unsigned char* rasterLine=&rasterLines[ciPtr->rasterLineOffset];
				const unsigned char* span=&spans[ciPtr->spanOffset];
				
				/* Copy all raster lines: */
				for(int y=baseLineRow-ciPtr->descent;y<baseLineRow+ciPtr->ascent;++y,++rasterLine)
					{
					/* Copy all spans in this line: */
					iPtr=&image[imageWidth*y+x+ciPtr->glyphOffset];
					int numSpans=int(*rasterLine);
					for(int i=0;i<numSpans;++i,++span)
						{
						iPtr+=int((*span)>>3);
						int numPixels=int((*span)&0x07);
						for(int j=0;j<numPixels;++j,++iPtr)
							*iPtr=fg;
						}
					}
				
				x+=ciPtr->width;
				}
			}
		}
	
	/* Upload the created texture image: */
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS,0);
	glPixelStorei(GL_UNPACK_ROW_LENGTH,0);
	glPixelStorei(GL_UNPACK_SKIP_ROWS,0);
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,textureWidth,textureHeight,0,GL_RGBA,GL_UNSIGNED_BYTE,0);
	if(antialiasing)
		{
		static GLfloat kernel[3]={0.25,0.5,0.25};
		// static GLfloat kernel[5]={0.125,0.2,0.35,0.2,0.125};
		glConvolutionParameteri(GL_SEPARABLE_2D,GL_CONVOLUTION_BORDER_MODE,GL_REPLICATE_BORDER);
		glSeparableFilter2D(GL_SEPARABLE_2D,GL_RGBA8,3,3,GL_LUMINANCE,GL_FLOAT,kernel,kernel);
		glEnable(GL_SEPARABLE_2D);
		glTexSubImage2D(GL_TEXTURE_2D,0,0,0,imageWidth,imageHeight,GL_RGBA,GL_UNSIGNED_BYTE,image);
		glDisable(GL_SEPARABLE_2D);
		}
	else
		glTexSubImage2D(GL_TEXTURE_2D,0,0,0,imageWidth,imageHeight,GL_RGBA,GL_UNSIGNED_BYTE,image);
	
	/* Clean up and return: */
	delete[] image;
	}

void GLFont::uploadStringTexture(const char* string,const GLFont::Color& stringBackgroundColor,const GLFont::Color& stringForegroundColor,GLsizei selectionStart,GLsizei selectionEnd,const GLFont::Color& selectionBackgroundColor,const GLFont::Color& selectionForegroundColor,GLsizei stringWidth,GLsizei textureWidth) const
	{
	/* Calculate the proper texture image dimensions: */
	GLsizei imageWidth,imageHeight;
	int x,baseLineRow;
	if(antialiasing)
		{
		imageWidth=stringWidth;
		imageHeight=fontHeight;
		baseLineRow=baseLine;
		x=maxLeftLap+1;
		}
	else
		{
		imageWidth=stringWidth;
		imageHeight=fontHeight;
		baseLineRow=baseLine;
		x=maxLeftLap+1;
		}
	
	/* Convert the string colors to 8-bit RGBA: */
	GLColor<GLubyte,4> bg=stringBackgroundColor;
	GLColor<GLubyte,4> fg=stringForegroundColor;
	GLColor<GLubyte,4> sbg=selectionBackgroundColor;
	GLColor<GLubyte,4> sfg=selectionForegroundColor;
	
	/* Create an RGBA texture image of appropriate size: */
	GLColor<GLubyte,4>* image=new GLColor<GLubyte,4>[imageWidth*imageHeight];
	GLColor<GLubyte,4>* iPtr=image;
	for(int i=imageWidth*imageHeight;i>0;--i,++iPtr)
		*iPtr=bg;
	
	if(string!=0)
		{
		/* Copy all characters into the texture image: */
		GLsizei index=0;
		for(const char* cPtr=string;*cPtr!=0;++cPtr,++index)
			{
			int charIndex=int(*cPtr)-firstCharacter;
			if(charIndex>=0&&charIndex<numCharacters)
				{
				const CharInfo* ciPtr=&characters[charIndex];
				const unsigned char* rasterLine=&rasterLines[ciPtr->rasterLineOffset];
				const unsigned char* span=&spans[ciPtr->spanOffset];
				
				GLColor<GLubyte,4> tfg=fg;
				if(index>=selectionStart&&index<selectionEnd)
					{
					/* Change the background color to the selection background color: */
					for(int y1=0;y1<imageHeight;++y1)
						{
						iPtr=&image[imageWidth*y1+x];
						for(int x1=x;x1<x+ciPtr->width;++x1,++iPtr)
							*iPtr=sbg;
						}
					
					/* Use the selection foreground color: */
					tfg=sfg;
					}
				
				/* Copy all raster lines: */
				for(int y=baseLineRow-ciPtr->descent;y<baseLineRow+ciPtr->ascent;++y,++rasterLine)
					{
					/* Copy all spans in this line: */
					iPtr=&image[imageWidth*y+x+ciPtr->glyphOffset];
					int numSpans=int(*rasterLine);
					for(int i=0;i<numSpans;++i,++span)
						{
						iPtr+=int((*span)>>3);
						int numPixels=int((*span)&0x07);
						for(int j=0;j<numPixels;++j,++iPtr)
							*iPtr=tfg;
						}
					}
				
				x+=ciPtr->width;
				}
			}
		}
	
	/* Upload the created texture image: */
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS,0);
	glPixelStorei(GL_UNPACK_ROW_LENGTH,0);
	glPixelStorei(GL_UNPACK_SKIP_ROWS,0);
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,textureWidth,textureHeight,0,GL_RGBA,GL_UNSIGNED_BYTE,0);
	if(antialiasing)
		{
		static GLfloat kernel[3]={0.25,0.5,0.25};
		// static GLfloat kernel[5]={0.125,0.2,0.35,0.2,0.125};
		glConvolutionParameteri(GL_SEPARABLE_2D,GL_CONVOLUTION_BORDER_MODE,GL_REPLICATE_BORDER);
		glSeparableFilter2D(GL_SEPARABLE_2D,GL_RGBA8,3,3,GL_LUMINANCE,GL_FLOAT,kernel,kernel);
		glEnable(GL_SEPARABLE_2D);
		glTexSubImage2D(GL_TEXTURE_2D,0,0,0,imageWidth,imageHeight,GL_RGBA,GL_UNSIGNED_BYTE,image);
		glDisable(GL_SEPARABLE_2D);
		}
	else
		glTexSubImage2D(GL_TEXTURE_2D,0,0,0,imageWidth,imageHeight,GL_RGBA,GL_UNSIGNED_BYTE,image);
	
	/* Clean up and return: */
	delete[] image;
	}

void GLFont::loadFont(IO::File& file)
	{
	/* Load the font file header: */
	firstCharacter=file.read<GLint>();
	numCharacters=file.read<GLsizei>();
	maxAscent=file.read<GLshort>();
	maxDescent=file.read<GLshort>();
	maxLeftLap=file.read<GLshort>();
	maxRightLap=file.read<GLshort>();
	
	/* Create the CharInfo structures: */
	characters=new CharInfo[numCharacters];
	for(GLsizei i=0;i<numCharacters;++i)
		characters[i].read(file);
	
	/* Load the raster line descriptors: */
	numRasterLines=file.read<GLsizei>();
	rasterLines=new unsigned char[numRasterLines];
	file.read<unsigned char>(rasterLines,numRasterLines);
	
	/* Load the span descriptors: */
	numSpans=file.read<GLsizei>();
	spans=new unsigned char[numSpans];
	file.read<unsigned char>(spans,numSpans);
	
	/* Calculate the texture height: */
	fontHeight=maxDescent+maxAscent+2;
	baseLine=maxDescent+1;
	for(textureHeight=1;textureHeight<fontHeight;textureHeight<<=1)
		;
	
	/* Calculate the average width of a character box (by only looking at digits): */
	GLint totalWidth=0;
	for(GLint i=0;i<10;++i)
		totalWidth+=characters[i+GLint('0')-firstCharacter].width;
	averageWidth=GLfloat(totalWidth)/(10.0f*GLfloat(fontHeight));
	}

GLFont::GLFont(const char* fontName)
	:firstCharacter(0),numCharacters(0),maxAscent(0),maxDescent(0),
	 maxLeftLap(0),maxRightLap(0),characters(0),
	 numRasterLines(0),rasterLines(0),
	 numSpans(0),spans(0),
	 fontHeight(0),textureHeight(0),
	 textHeight(1.0),hAlignment(Left),vAlignment(Baseline),
	 antialiasing(false)
	{
	/* Read the font from file: */
	char fontFileName[1024];
	try
		{
		/* Try given directory first: */
		snprintf(fontFileName,sizeof(fontFileName),"%s.fnt",fontName);
		IO::FilePtr fontFile(IO::openFile(fontFileName));
		fontFile->setEndianness(Misc::LittleEndian);
		loadFont(*fontFile);
		return;
		}
	catch(...)
		{
		/* Ignore the error and continue: */
		}
	
	/* Try GL font directory set in environment next: */
	if(getenv("GLFONTDIR")!=0)
		{
		try
			{
			/* Try the GLFONTDIR directory next: */
			snprintf(fontFileName,sizeof(fontFileName),"%s/%s.fnt",getenv("GLFONTDIR"),fontName);
			IO::FilePtr fontFile(IO::openFile(fontFileName));
			fontFile->setEndianness(Misc::LittleEndian);
			loadFont(*fontFile);
			return;
			}
		catch(...)
			{
			/* Ignore the error and continue: */
			}
		}
	
	try
		{
		/* Try system-wide GL font directory last: */
		snprintf(fontFileName,sizeof(fontFileName),"%s/%s.fnt",GLSUPPORT_CONFIG_GL_FONT_DIR,fontName);
		IO::FilePtr fontFile(IO::openFile(fontFileName));
		fontFile->setEndianness(Misc::LittleEndian);
		loadFont(*fontFile);
		return;
		}
	catch(...)
		{
		/* Ignore the error and continue: */
		}
	
	/* Give up: */
	Misc::throwStdErr("GLFont::GLFont: Font %s not found",fontName);
	}

GLFont::~GLFont(void)
	{
	delete[] characters;
	delete[] rasterLines;
	delete[] spans;
	}

GLFont::Vector GLFont::calcStringSize(GLsizei stringWidth) const
	{
	/* Return the string's scaled width: */
	return Vector(GLfloat(stringWidth-1)*textHeight/GLfloat(fontHeight-1),textHeight,0.0f);
	}

GLFont::Box GLFont::calcStringBox(GLsizei stringWidth) const
	{
	/* Calculate the string's scaled width: */
	Vector boxSize(GLfloat(stringWidth-1)*textHeight/GLfloat(fontHeight-1),textHeight,0.0f);
	
	/* Calculate the string's bounding box origin: */
	Vector boxOrigin(0.0f,0.0f,0.0f);
	switch(hAlignment)
		{
		case Left:
			boxOrigin[0]=0.0f;
			break;
		
		case Center:
			boxOrigin[0]=-0.5f*boxSize[0];
			break;
		
		case Right:
			boxOrigin[0]=-boxSize[0];
			break;
		}
	switch(vAlignment)
		{
		case Top:
			boxOrigin[1]=-boxSize[1];
			break;
		
		case VCenter:
			boxOrigin[1]=-0.5f*boxSize[1];
			break;
		
		case Baseline:
			boxOrigin[1]=-boxSize[1]*GLfloat(baseLine)/GLfloat(fontHeight);
			break;
		
		case Bottom:
			boxOrigin[1]=0.0f;
			break;
		}
	
	return Box(boxOrigin,boxSize);
	}

GLFont::TBox GLFont::calcStringTexCoords(GLsizei stringWidth,GLsizei textureWidth) const
	{
	/* Calculate the string's texture coordinates: */
	GLVector<GLfloat,2> origin(0.5f/GLfloat(textureWidth),0.5f/GLfloat(textureHeight));
	GLVector<GLfloat,2> size(GLfloat(stringWidth-1)/GLfloat(textureWidth),GLfloat(fontHeight-1)/GLfloat(textureHeight));
	
	return TBox(origin,size);
	}

void GLFont::updateString(GLString& string) const
	{
	/* Calculate the string's texel width: */
	string.texelWidth=calcStringWidth(string.string);
	
	/* Calculate the string's texture width: */
	for(string.textureWidth=1;string.textureWidth<string.texelWidth;string.textureWidth<<=1)
		;
	
	/* Calculate the string's texture coordinate box: */
	string.textureBox=calcStringTexCoords(string.texelWidth,string.textureWidth);
	}

GLint GLFont::calcCharacterPos(const char* string,GLsizei textureWidth,GLfloat texX) const
	{
	/* Convert the texture coordinate to texel space: */
	texX*=GLfloat(textureWidth);
	
	/* Find the character whose box contains the texel-space texture coordinate: */
	GLint result=0;
	if(string!=0)
		{
		const char* cPtr=string;
		GLfloat left=GLfloat(maxLeftLap);
		while(*cPtr!='\0')
			{
			/* Calculate the current character's box extents: */
			int charIndex=int(*cPtr)-firstCharacter;
			GLfloat right=left;
			if(charIndex>=0&&charIndex<numCharacters)
				right+=GLfloat(characters[charIndex].width);
			
			if(texX<(left+right)*0.5f)
				break;
			
			/* Go to the next character: */
			++result;
			++cPtr;
			left=right;
			}
		}
	
	return result;
	}

GLfloat GLFont::calcCharacterTexCoord(const char* string,GLsizei textureWidth,GLint characterPos) const
	{
	/* Find the character's texel position: */
	GLint texelPos=maxLeftLap;
	if(string!=0)
		{
		const char* cPtr=string;
		while(characterPos>0&&*cPtr!='\0')
			{
			/* Calculate the current character's box extents: */
			int charIndex=int(*cPtr)-firstCharacter;
			if(charIndex>=0&&charIndex<numCharacters)
				texelPos+=characters[charIndex].width;
			
			/* Go to the next character: */
			--characterPos;
			++cPtr;
			}
		}
	
	/* Convert the texel position to a texture coordinate: */
	return GLfloat(texelPos)/GLfloat(textureWidth);
	}

void GLFont::uploadStringTexture(const char* string) const
	{
	/* Calculate the string's texel width: */
	GLsizei stringWidth=calcStringWidth(string);
	
	/* Calculate the texture width: */
	GLsizei textureWidth;
	for(textureWidth=1;textureWidth<stringWidth;textureWidth<<=1)
		;
	
	/* Upload the string's texture image: */
	uploadStringTexture(string,stringWidth,textureWidth);
	}

void GLFont::uploadStringTexture(const GLString& string) const
	{
	/* Upload the string's texture image: */
	uploadStringTexture(string.string,string.texelWidth,string.textureWidth);
	}

void GLFont::uploadStringTexture(const char* string,const GLFont::Color& stringBackgroundColor,const GLFont::Color& stringForegroundColor) const
	{
	/* Calculate the string's texel width: */
	GLsizei stringWidth=calcStringWidth(string);
	
	/* Calculate the texture width: */
	GLsizei textureWidth;
	for(textureWidth=1;textureWidth<stringWidth;textureWidth<<=1)
		;
	
	/* Upload the string's texture image: */
	uploadStringTexture(string,stringBackgroundColor,stringForegroundColor,stringWidth,textureWidth);
	}

void GLFont::uploadStringTexture(const GLString& string,const GLFont::Color& stringBackgroundColor,const GLFont::Color& stringForegroundColor) const
	{
	/* Upload the string's texture image: */
	uploadStringTexture(string.string,stringBackgroundColor,stringForegroundColor,string.texelWidth,string.textureWidth);
	}

void GLFont::uploadStringTexture(const char* string,const GLFont::Color& stringBackgroundColor,const GLFont::Color& stringForegroundColor,GLsizei selectionStart,GLsizei selectionEnd,const Color& selectionBackgroundColor,const Color& selectionForegroundColor) const
	{
	/* Calculate the string's texel width: */
	GLsizei stringWidth=calcStringWidth(string);
	
	/* Calculate the texture width: */
	GLsizei textureWidth;
	for(textureWidth=1;textureWidth<stringWidth;textureWidth<<=1)
		;
	
	/* Upload the string's texture image: */
	uploadStringTexture(string,stringBackgroundColor,stringForegroundColor,selectionStart,selectionEnd,selectionBackgroundColor,selectionForegroundColor,stringWidth,textureWidth);
	}

void GLFont::uploadStringTexture(const GLString& string,const GLFont::Color& stringBackgroundColor,const GLFont::Color& stringForegroundColor,GLsizei selectionStart,GLsizei selectionEnd,const Color& selectionBackgroundColor,const Color& selectionForegroundColor) const
	{
	/* Upload the string's texture image: */
	uploadStringTexture(string.string,stringBackgroundColor,stringForegroundColor,selectionStart,selectionEnd,selectionBackgroundColor,selectionForegroundColor,string.texelWidth,string.textureWidth);
	}

void GLFont::drawString(const GLFont::Vector& origin,const char* string) const
	{
	/* Calculate the string's texel width: */
	GLsizei stringWidth=calcStringWidth(string);
	
	/* Calculate the string's bounding box: */
	Box stringBox=calcStringBox(stringWidth);
	stringBox.doOffset(origin);
	
	/* Calculate the texture width: */
	GLsizei textureWidth;
	for(textureWidth=1;textureWidth<stringWidth;textureWidth<<=1)
		;
	
	/* Calculate the string's texture coordinates: */
	TBox stringTexCoord=calcStringTexCoords(stringWidth,textureWidth);
	
	/* Upload the string's texture image: */
	uploadStringTexture(string,backgroundColor,foregroundColor,stringWidth,textureWidth);
	
	/* Render a textured quad: */
	glPushAttrib(GL_TEXTURE_BIT);
	glEnable(GL_TEXTURE_2D);
	glTexEnvMode(GLTexEnvEnums::TEXTURE_ENV,GLTexEnvEnums::MODULATE);
	glColor4f(1.0f,1.0f,1.0f,backgroundColor[3]);
	glBegin(GL_QUADS);
	glNormal3f(0.0,0.0,1.0);
	glTexCoord(stringTexCoord.getCorner(0));
	glVertex(stringBox.getCorner(0));
	glTexCoord(stringTexCoord.getCorner(1));
	glVertex(stringBox.getCorner(1));
	glTexCoord(stringTexCoord.getCorner(3));
	glVertex(stringBox.getCorner(3));
	glTexCoord(stringTexCoord.getCorner(2));
	glVertex(stringBox.getCorner(2));
	glEnd();
	glPopAttrib();
	}
