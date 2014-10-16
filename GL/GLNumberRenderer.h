/***********************************************************************
GLNumberRenderer - Class to render numbers using a HUD-like font.
Copyright (c) 2010-2013 Oliver Kreylos

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

#ifndef GLNUMBERRENDERER_INCLUDED
#define GLNUMBERRENDERER_INCLUDED

#include <GL/gl.h>
#include <GL/GLVector.h>
#include <GL/GLObject.h>

class GLNumberRenderer:public GLObject
	{
	/* Embedded classes: */
	public:
	typedef GLVector<GLfloat,3> Vector; // Type for points and vectors
	
	private:
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		GLuint displayListBase; // Base index of display list to render digits, minus signs, and decimal points
		
		/* Constructors and destructors: */
		public:
		DataItem(void);
		virtual ~DataItem(void);
		};
	
	/* Elements: */
	GLfloat fontHeight; // Height of font in model-space units
	bool proportional; // Flag if the font's characters have different widths
	GLfloat charWidths[12]; // Width of the font's characters
	
	/* Private methods: */
	GLfloat calcStringWidth(const char* string) const; // Returns the width of the given string, which only contains digits, decimal points, and minus signs
	void drawString(const char* string,GLContextData& contextData) const; // Draws the given string, with same restrictions
	void drawString(const char* string,const Vector& pos,int horizAlign,int vertAlign,GLContextData& contextData) const; // Draws string at given position, with same restrictions
	
	/* Constructors and destructors: */
	public:
	GLNumberRenderer(GLfloat sFontHeight,bool sProportional); // Creates a number renderer for the given font size and proportional spacing flag
	
	/* Methods from class GLObject: */
	virtual void initContext(GLContextData& contextData) const;
	
	/* New methods: */
	GLfloat getFontHeight(void) const // Returns the height of the renderer's characters
		{
		return fontHeight;
		}
	bool isProportional(void) const // Returns true if the renderer's font is proportional
		{
		return proportional;
		}
	GLfloat calcNumberWidth(unsigned int number) const; // Calculates the width of rendering the given number in model-space units
	GLfloat calcNumberWidth(int number) const; // Ditto, for signed number
	GLfloat calcNumberWidth(double number,int precision) const; // Ditto, for floating-point number
	void drawNumber(unsigned int number,GLContextData& contextData) const; // Draws the given integer number at the current origin; translates modelview matrix to the right
	void drawNumber(int number,GLContextData& contextData) const; // Ditto, using signed number
	void drawNumber(double number,int precision,GLContextData& contextData) const; // Ditto, using floating-point number and given precision
	
	/* Methods to draw positioned and aligned numbers (horizAlign: -1 = left, 0 = centered, 1 = right, vertAlign: -1 = bottom, 0 = center, 1 = top): */
	void drawNumber(const Vector& pos,unsigned int number,GLContextData& contextData,int horizAlign =-1,int vertAlign =-1) const; // Draws given unsigned integer number at given position, leaves modelview matrix alone
	void drawNumber(const Vector& pos,int number,GLContextData& contextData,int horizAlign =-1,int vertAlign =-1) const; // Ditto, using signed integer number
	void drawNumber(const Vector& pos,double number,int precision,GLContextData& contextData,int horizAlign =-1,int vertAlign =-1) const; // Ditto, using floating-point number and given precision
	};

#endif
