/***********************************************************************
GlyphGadget - Helper class to render assorted glyphs as part of other
widgets.
Copyright (c) 2010 Oliver Kreylos

This file is part of the GLMotif Widget Library (GLMotif).

The GLMotif Widget Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GLMotif Widget Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the GLMotif Widget Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef GLMOTIF_GLYPHGADGET_INCLUDED
#define GLMOTIF_GLYPHGADGET_INCLUDED

#include <GL/gl.h>
#include <GLMotif/Types.h>

/* Forward declarations: */
class GLContextData;

namespace GLMotif {

class GlyphGadget
	{
	/* Embedded classes: */
	public:
	enum GlyphType // Enumerated type for glyph types
		{
		NONE,
		SQUARE,DIAMOND,
		LOW_BAR,CROSS,
		SIMPLE_ARROW_LEFT,SIMPLE_ARROW_DOWN,SIMPLE_ARROW_RIGHT,SIMPLE_ARROW_UP,
		FANCY_ARROW_LEFT,FANCY_ARROW_DOWN,FANCY_ARROW_RIGHT,FANCY_ARROW_UP
		};
	
	enum Depth // Enumerated type for glyph engraving or embossing
		{
		IN,OUT
		};
	
	/* Elements: */
	private:
	GlyphType glyphType; // Glyph type
	Depth depth; // Glyph depth
	GLfloat glyphSize; // Size of glyph
	GLfloat bevelSize; // Size of bevel around glyph
	Box glyphBox; // Box around the glyph; also defined glyph's base plane
	Color glyphColor; // Color for the glyph; margin color is inherited from current state
	int numVertices; // Number of vertices in glyph
	int innerStart; // Start index of inner (engraved or embossed) vertex ring
	Vector* glyphVertices; // Array of vertices to render the glyph
	int numNormals; // Number of normal vectors in glyph
	Vector* glyphNormals; // Array of normal vectors to render the glyph
	
	/* Private methods: */
	void layoutGlyph(void); // Creates vertices and normals to render the glyph
	
	/* Constructors and destructors: */
	public:
	GlyphGadget(void); // Creates a default glyph
	GlyphGadget(GlyphType sGlyphType,Depth sDepth,GLfloat sGlyphSize); // Creates a glyph of the given type, style, and size
	~GlyphGadget(void);
	
	/* Methods: */
	GLfloat getGlyphSize(void) const // Returns the glyph's size
		{
		return glyphSize;
		}
	GLfloat getBevelSize(void) const // Returns the glyph's bevel size
		{
		return bevelSize;
		}
	const Box& getGlyphBox(void) const // Returns the glyph box
		{
		return glyphBox;
		}
	const Color& getGlyphColor(void) const // Returns the glyph's color
		{
		return glyphColor;
		}
	GLfloat getPreferredBoxSize(void) const; // Returns the glyph's preferred box size
	ZRange calcZRange(void) const; // Returns the range of z values of the glyph
	void setGlyphType(GlyphType newGlyphType); // Sets the glyph's type
	void setDepth(Depth newDepth); // Sets the glyph's depth
	void setGlyphSize(GLfloat newGlyphSize); // Sets the glyph's size
	void setBevelSize(GLfloat newBevelSize); // Sets the glyph's bevel size
	void setGlyphBox(const Box& newGlyphBox); // Repositions the glyph
	void setGlyphColor(const Color& newGlyphColor); // Sets the glyph's color
	void draw(GLContextData& contextData) const; // Draws the glyph
	};

}

#endif
