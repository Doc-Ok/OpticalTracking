/***********************************************************************
Glyph - Class for widgets containing only a glyph.
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

#ifndef GLMOTIF_GLYPH_INCLUDED
#define GLMOTIF_GLYPH_INCLUDED

#include <GLMotif/GlyphGadget.h>
#include <GLMotif/Widget.h>

namespace GLMotif {

class Glyph:public Widget
	{
	/* Elements: */
	protected:
	GlyphGadget glyph; // The widget's glyph
	
	/* Constructors and destructors: */
	public:
	Glyph(const char* sName,Container* sParent,GlyphGadget::GlyphType glyphType,GlyphGadget::Depth depth,bool sManageChild =true);
	
	/* Methods inherited from VRWidget: */
	virtual Vector calcNaturalSize(void) const;
	virtual ZRange calcZRange(void) const;
	virtual void resize(const Box& newExterior);
	virtual void setForegroundColor(const Color& newForegroundColor);
	virtual void draw(GLContextData& contextData) const;
	
	/* New methods: */
	void setGlyphType(GlyphGadget::GlyphType newGlyphType);
	void setDepth(GlyphGadget::Depth newDepth);
	void setGlyphSize(GLfloat newGlyphSize);
	void setBevelSize(GLfloat newBevelSize);
	};

}

#endif
