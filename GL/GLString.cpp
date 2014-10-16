/***********************************************************************
GLString - Class to represent strings with the additional data required
to render said strings using a texture-based font.
Copyright (c) 2010 Oliver Kreylos

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

#include <GL/GLString.h>

#include <string.h>
#include <GL/GLFont.h>

/*************************
Methods of class GLString:
*************************/

GLString::GLString(const char* sString,const GLFont& font)
	:length(strlen(sString)),
	 string(new char[length+1])
	{
	/* Copy the source string: */
	memcpy(string,sString,length);
	string[length]='\0';
	
	/* Update the string's font-related data: */
	font.updateString(*this);
	}

GLString::GLString(const char* sStringBegin,const char* sStringEnd,const GLFont& font)
	:length(sStringEnd-sStringBegin),
	 string(new char[length+1])
	{
	/* Copy the source string: */
	memcpy(string,sStringBegin,length);
	string[length]='\0';
	
	/* Update the string's font-related data: */
	font.updateString(*this);
	}

GLString::GLString(const GLString& source)
	:length(source.length),
	 string(source.string!=0?new char[length+1]:0),
	 texelWidth(source.texelWidth),textureWidth(source.textureWidth),
	 textureBox(source.textureBox)
	{
	if(source.string!=0)
		{
		/* Copy the source string: */
		memcpy(string,source.string,length);
		string[length]='\0';
		}
	}

GLString& GLString::operator=(const GLString& source)
	{
	if(this!=&source)
		{
		/* Copy the source string: */
		delete[] string;
		length=source.length;
		string=source.string!=0?new char[length+1]:0;
		if(source.string!=0)
			{
			memcpy(string,source.string,length);
			string[length]='\0';
			}
		
		/* Copy the source's texture-related data: */
		texelWidth=source.texelWidth;
		textureWidth=source.textureWidth;
		textureBox=source.textureBox;
		}
	
	return *this;
	}

GLString::~GLString(void)
	{
	delete[] string;
	}

void GLString::setString(const char* newString,const GLFont& font)
	{
	/* Copy the new string: */
	delete[] string;
	length=strlen(newString);
	string=new char[length+1];
	memcpy(string,newString,length);
	string[length]='\0';
	
	/* Update the string's font-related data: */
	font.updateString(*this);
	}

void GLString::setString(const char* newStringBegin,const char* newStringEnd,const GLFont& font)
	{
	/* Copy the new string: */
	delete[] string;
	length=newStringEnd-newStringBegin;
	string=new char[length+1];
	memcpy(string,newStringBegin,length);
	string[length]='\0';
	
	/* Update the string's font-related data: */
	font.updateString(*this);
	}

void GLString::adoptString(char* newString,const GLFont& font)
	{
	/* Adopt the new string: */
	delete[] string;
	length=strlen(newString);
	string=newString;
	
	/* Update the string's font-related data: */
	font.updateString(*this);
	}

void GLString::adoptString(GLsizei newLength,char* newString,const GLFont& font)
	{
	/* Adopt the new string: */
	delete[] string;
	length=newLength;
	string=newString;
	string[length]='\0';
	
	/* Update the string's font-related data: */
	font.updateString(*this);
	}

void GLString::setFont(const GLFont& font)
	{
	/* Update the string's font-related data: */
	font.updateString(*this);
	}
