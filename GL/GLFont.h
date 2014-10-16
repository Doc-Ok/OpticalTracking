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

#ifndef GLFONT_INCLUDED
#define GLFONT_INCLUDED

#include <Misc/Endianness.h>
#include <GL/gl.h>
#include <GL/GLColor.h>
#include <GL/GLVector.h>
#include <GL/GLBox.h>
#include <GL/GLString.h>

/* Forward declarations: */
namespace IO {
class File;
}

class GLFont
	{
	/* Embedded classes: */
	public:
	typedef GLColor<GLfloat,4> Color; // Type for color values
	typedef GLVector<GLfloat,3> Vector; // Type for model space vectors and points
	typedef GLBox<GLfloat,3> Box; // Type for model space boxes
	typedef GLBox<GLfloat,2> TBox; // Type for texture space boxes
	
	enum HAlignment
		{
		Left,Center,Right
		};
	
	enum VAlignment
		{
		Top,VCenter,Baseline,Bottom
		};
	
	private:
	struct CharInfo
		{
		/* Elements: */
		public:
		/* Character box description: */
		GLshort width; // Width of character box
		GLshort ascent,descent; // Height of character box above and below baseline
		GLshort glyphOffset; // Offset and width of character glyph in box
		GLsizei rasterLineOffset; // Offset of raster line descriptors in main array
		GLsizei spanOffset; // Offset of span descriptors in main array
		
		/* Methods: */
		void read(IO::File& file); // Reads a CharInfo structure from a font file
		};
	
	/* Elements: */
	GLint firstCharacter; // Index of first character in font
	GLsizei numCharacters; // Number of characters in font
	GLshort maxAscent,maxDescent; // Maximal height and depth of characters
	GLshort maxLeftLap,maxRightLap; // Maximal overlap of a character versus its box
	CharInfo* characters; // Array of CharInfo structures describing all characters
	GLsizei numRasterLines; // Total number of raster lines for all characters
	GLubyte* rasterLines; // Array of number of spans per raster line for all characters
	GLsizei numSpans; // Total number of spans of all raster lines and characters
	GLubyte* spans; // Array of spans
	GLint fontHeight; // Total height of the font (ascent+descent+2 border pixels)
	GLint baseLine; // Position of baseline
	GLsizei textureHeight; // Height of a texture image to hold a single line of text
	GLfloat averageWidth; // Average width of a character box
	
	/* Current font status: */
	GLfloat textHeight; // Scaled height of font
	Color backgroundColor; // Text background color
	Color foregroundColor; // Text foreground color
	HAlignment hAlignment; // Horizontal alignment
	VAlignment vAlignment; // Vertical alignment
	bool antialiasing; // Flag to enable antialiasing
	
	/* Private methods: */
	GLsizei calcStringWidth(const char* string) const; // Calculates the texel width of a string
	void uploadStringTexture(const char* string,GLsizei stringWidth,GLsizei textureWidth) const; // Creates and uploads a texture for a string
	void uploadStringTexture(const char* string,const Color& stringBackgroundColor,const Color& stringForegroundColor,GLsizei stringWidth,GLsizei textureWidth) const; // Creates and uploads a texture for a string using the given colors
	void uploadStringTexture(const char* string,const Color& stringBackgroundColor,const Color& stringForegroundColor,GLsizei selectionStart,GLsizei selectionEnd,const Color& selectionBackgroundColor,const Color& selectionForegroundColor,GLsizei stringWidth,GLsizei textureWidth) const; // Creates and uploads a texture for a string using the given colors, selection range, and selection colors
	void loadFont(IO::File& file); // Loads font from given file
	
	/* Constructors and Destructors: */
	public:
	GLFont(const char* fontName); // Creates a GL font from a font file
	~GLFont(void);
	
	/* Methods: */
	bool isValid(void) const // Checks if the font object was created successfully
		{
		return characters!=0;
		}
	GLfloat getTextPixelHeight(void) const // Returns font's unscaled height
		{
		return GLfloat(fontHeight);
		}
	GLfloat getTextHeight(void) const // Returns the font's scaled height
		{
		return textHeight;
		}
	const Color& getBackgroundColor(void) const // Returns the font's background color
		{
		return backgroundColor;
		}
	const Color& getForegroundColor(void) const // Returns the font's foreground color
		{
		return foregroundColor;
		}
	void setTextHeight(GLfloat newTextHeight) // Sets the font's scaled height
		{
		textHeight=newTextHeight;
		}
	GLfloat getCharacterWidth(void) const // Returns the average scaled character width
		{
		return averageWidth*textHeight;
		}
	template <class InputColorType>
	void setBackgroundColor(const InputColorType& newBackgroundColor) // Sets the text background color
		{
		backgroundColor=newBackgroundColor;
		}
	template <class InputColorType>
	void setForegroundColor(const InputColorType& newForegroundColor) // Sets the text foreground color
		{
		foregroundColor=newForegroundColor;
		}
	void setHAlignment(HAlignment newHAlignment) // Sets the font's horizontal alignment
		{
		hAlignment=newHAlignment;
		}
	void setVAlignment(VAlignment newVAlignment) // Sets the font's vertical alignment
		{
		vAlignment=newVAlignment;
		}
	void setAntialiasing(bool newAntialiasing) // Sets the antialiasing flag
		{
		antialiasing=newAntialiasing;
		}
	Vector calcStringSize(GLsizei stringWidth) const; // Returns the size of the bounding box of a string
	Vector calcStringSize(const char* string) const // Ditto
		{
		return calcStringSize(calcStringWidth(string));
		}
	Vector calcStringSize(const GLString& string) const // Ditto
		{
		return calcStringSize(string.texelWidth);
		}
	Box calcStringBox(GLsizei stringWidth) const; // Returns the bounding box of a string
	Box calcStringBox(const char* string) const // Ditto
		{
		return calcStringBox(calcStringWidth(string));
		}
	Box calcStringBox(const GLString& string) const // Ditto
		{
		return calcStringBox(string.texelWidth);
		}
	TBox calcStringTexCoords(GLsizei stringWidth,GLsizei textureWidth) const; // Calculates the texture coordinates needed to render a string
	TBox calcStringTexCoords(const char* string) const // Ditto
		{
		GLsizei stringWidth=calcStringWidth(string);
		GLsizei textureWidth;
		for(textureWidth=1;textureWidth<stringWidth;textureWidth<<=1)
			;
		
		return calcStringTexCoords(stringWidth,textureWidth);
		}
	void updateString(GLString& string) const; // Updates the font-related data of the given string object
	GLint calcCharacterPos(const char* string,GLsizei textureWidth,GLfloat texX) const; // Returns the index of the string's character which lies underneath the given horizontal texture coordinate
	GLint calcCharacterPos(const char* string,GLfloat texX) const // Ditto
		{
		GLsizei stringWidth=calcStringWidth(string);
		GLsizei textureWidth;
		for(textureWidth=1;textureWidth<stringWidth;textureWidth<<=1)
			;
		
		return calcCharacterPos(string,textureWidth,texX);
		}
	GLint calcCharacterPos(const GLString& string,GLfloat texX) const // Ditto
		{
		return calcCharacterPos(string.string,string.textureWidth,texX);
		}
	GLfloat calcCharacterTexCoord(const char* string,GLsizei textureWidth,GLint characterPos) const; // Returns the texture coordinate of the right side of the character of the given position in the string
	GLfloat calcCharacterTexCoord(const char* string,GLint characterPos) const // Ditto
		{
		GLsizei stringWidth=calcStringWidth(string);
		GLsizei textureWidth;
		for(textureWidth=1;textureWidth<stringWidth;textureWidth<<=1)
			;
		
		return calcCharacterTexCoord(string,textureWidth,characterPos);
		}
	GLfloat calcCharacterTexCoord(const GLString& string,GLint characterPos) const // Ditto
		{
		return calcCharacterTexCoord(string.string,string.textureWidth,characterPos);
		}
	void uploadStringTexture(const char* string) const; // Uploads a string's texture image
	void uploadStringTexture(const GLString& string) const; // Ditto
	void uploadStringTexture(const char* string,const Color& stringBackgroundColor,const Color& stringForegroundColor) const; // Uploads a string's texture image with the given colors
	void uploadStringTexture(const GLString& string,const Color& stringBackgroundColor,const Color& stringForegroundColor) const; // Ditto
	void uploadStringTexture(const char* string,const Color& stringBackgroundColor,const Color& stringForegroundColor,GLsizei selectionStart,GLsizei selectionEnd,const Color& selectionBackgroundColor,const Color& selectionForegroundColor) const; // Uploads a string's texture image with the given colors, selection range, and selection colors
	void uploadStringTexture(const GLString& string,const Color& stringBackgroundColor,const Color& stringForegroundColor,GLsizei selectionStart,GLsizei selectionEnd,const Color& selectionBackgroundColor,const Color& selectionForegroundColor) const; // Ditto
	void drawString(const Vector& origin,const char* string) const; // Draws a simple, one-line string
	};

#endif
