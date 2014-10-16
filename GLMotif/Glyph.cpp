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

#include <GLMotif/Glyph.h>

#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/Container.h>

namespace GLMotif {

/**********************
Methods of class Glyph:
**********************/

Glyph::Glyph(const char* sName,Container* sParent,GlyphGadget::GlyphType glyphType,GlyphGadget::Depth depth,bool sManageChild)
	:Widget(sName,sParent,false),
	 glyph(glyphType,depth,0.0f)
	{
	/* Get the style sheet: */
	const StyleSheet* ss=getStyleSheet();
	
	/* Glyph defaults to no border: */
	setBorderWidth(0.0f);
	
	/* Set the glyph sizes: */
	glyph.setGlyphSize(ss->size*0.25f);
	glyph.setBevelSize(ss->size*0.25f);
	glyph.setGlyphColor(foregroundColor);
	
	/* Manage me: */
	if(sManageChild)
		manageChild();
	}

Vector Glyph::calcNaturalSize(void) const
	{
	GLfloat gs=glyph.getPreferredBoxSize();
	return calcExteriorSize(Vector(gs,gs,0.0f));
	}

ZRange Glyph::calcZRange(void) const
	{
	/* Calculate the parent class widget's z range: */
	ZRange myZRange=Widget::calcZRange();
	
	/* Adjust for the glyph: */
	myZRange+=glyph.calcZRange();
	
	return myZRange;
	}

void Glyph::resize(const Box& newExterior)
	{
	/* Resize the parent class widget: */
	Widget::resize(newExterior);
	
	/* Reposition the glyph: */
	glyph.setGlyphBox(getInterior());
	}

void Glyph::setForegroundColor(const Color& newForegroundColor)
	{
	/* Set the glyph's color: */
	glyph.setGlyphColor(newForegroundColor);
	
	/* Invalidate the visual representation: */
	update();
	}

void Glyph::draw(GLContextData& contextData) const
	{
	/* Draw the glyph: */
	glColor(backgroundColor);
	glyph.draw(contextData);
	}

void Glyph::setGlyphType(GlyphGadget::GlyphType newGlyphType)
	{
	/* Set the glyph type: */
	glyph.setGlyphType(newGlyphType);
	
	if(isManaged)
		{
		/* Try adjusting the widget size to accomodate the new preferred size: */
		parent->requestResize(this,calcNaturalSize());
		}
	else
		resize(Box(Vector(0.0f,0.0f,0.0f),calcNaturalSize()));
	}

void Glyph::setDepth(GlyphGadget::Depth newDepth)
	{
	/* Set the glyph depth: */
	glyph.setDepth(newDepth);
	
	if(isManaged)
		{
		/* Try adjusting the widget size to accomodate the new preferred size: */
		parent->requestResize(this,calcNaturalSize());
		}
	else
		resize(Box(Vector(0.0f,0.0f,0.0f),calcNaturalSize()));
	}

void Glyph::setGlyphSize(GLfloat newGlyphSize)
	{
	/* Set the glyph size: */
	glyph.setGlyphSize(newGlyphSize);
	
	if(isManaged)
		{
		/* Try adjusting the widget size to accomodate the new preferred size: */
		parent->requestResize(this,calcNaturalSize());
		}
	else
		resize(Box(Vector(0.0f,0.0f,0.0f),calcNaturalSize()));
	}

void Glyph::setBevelSize(GLfloat newBevelSize)
	{
	/* Set the bevel size: */
	glyph.setBevelSize(newBevelSize);
	
	if(isManaged)
		{
		/* Try adjusting the widget size to accomodate the new preferred size: */
		parent->requestResize(this,calcNaturalSize());
		}
	else
		resize(Box(Vector(0.0f,0.0f,0.0f),calcNaturalSize()));
	}

}

