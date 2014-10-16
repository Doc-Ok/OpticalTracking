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

#ifndef GLSTRING_INCLUDED
#define GLSTRING_INCLUDED

#include <GL/gl.h>
#include <GL/GLBox.h>

/* Forward declarations: */
class GLFont;

class GLString
	{
	friend class GLFont;
	
	/* Embedded classes: */
	public:
	typedef GLBox<GLfloat,2> TBox; // Type for texture space boxes
	
	/* Elements: */
	private:
	GLsizei length; // The string's length in characters
	char* string; // The string's C-style representation
	protected:
	GLsizei texelWidth; // The string's width in texels (based on its current font)
	GLsizei textureWidth; // The width of the string's texture image (based on its current font)
	TBox textureBox; // Texture coordinates to render the string
	
	/* Constructors and destructors: */
	public:
	GLString(void) // Dummy constructor
		:length(0),string(0)
		{
		}
	GLString(const char* sString,const GLFont& font); // Creates string from C-style string and associates it with the given font
	GLString(const char* sStringBegin,const char* sStringEnd,const GLFont& font); // Creates string from character array and associates it with the given font
	GLString(const GLString& source); // Copies the given string
	GLString& operator=(const GLString& source); // Ditto
	~GLString(void); // Destroys the string
	
	/* Methods: */
	GLsizei getLength(void) const // Returns the string's length in characters
		{
		return length;
		}
	const char* getString(void) const // Returns the C-style string representation
		{
		return string;
		}
	GLsizei getTexelWidth(void) const // Returns the string's width in texel space
		{
		return texelWidth;
		}
	GLsizei getTextureWidth(void) const // Returns the string's texture image width
		{
		return textureWidth;
		}
	const TBox& getTextureBox(void) const // Returns the string's texture coordinate box
		{
		return textureBox;
		}
	void setString(const char* newString,const GLFont& font); // Replaces the string and associates it with the given font
	void setString(const char* newStringBegin,const char* newStringEnd,const GLFont& font); // Ditto, from character array
	void adoptString(char* newString,const GLFont& font); // Takes ownership of the new[]-allocated string and associates it with the given font
	void adoptString(GLsizei newLength,char* newString,const GLFont& font); // Ditto, if length of new string is already known
	void setFont(const GLFont& font); // Re-associates the string with the given font
	};

#endif
