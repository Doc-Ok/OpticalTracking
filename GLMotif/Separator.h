/***********************************************************************
Separator - Class for widgets to visually separate adjacent components
in widget layouts.
Copyright (c) 2008 Oliver Kreylos

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

#ifndef GLMOTIF_SEPARATOR_INCLUDED
#define GLMOTIF_SEPARATOR_INCLUDED

#include <GLMotif/Widget.h>

namespace GLMotif {

class Separator:public Widget
	{
	/* Embedded classes: */
	public:
	enum Orientation // Enumerated type for separator orientations
		{
		VERTICAL,HORIZONTAL
		};
	
	enum Style // Enumerated type for separator styles
		{
		LOWERED,RAISED
		};
	
	/* Elements: */
	private:
	Orientation orientation; // Orientation of the separator
	GLfloat preferredHeight; // Preferred total separator height (assuming vertical separator)
	GLfloat marginWidth; // Width of margin around separator
	Style style; // Separator style
	GLfloat separatorWidth; // Width of separator (assuming vertical separator)
	Box separator; // Box around the separator
	Vector p1,p2; // The two endpoints of the separator's ridge line
	
	/* Constructors and destructors: */
	public:
	Separator(const char* sName,Container* sParent,Orientation sOrientation,GLfloat sPreferredHeight,Style sStyle,bool manageChild =true);
	
	/* Methods inherited from Widget: */
	virtual Vector calcNaturalSize(void) const;
	virtual ZRange calcZRange(void) const;
	virtual void resize(const Box& newExterior);
	virtual void draw(GLContextData& contextData) const;
	
	/* New methods: */
	void setPreferredHeight(GLfloat newPreferredHeight); // Sets the separator's preferred height
	void setMarginWidth(GLfloat newMarginWidth); // Sets the separator's margin width
	void setStyle(Style newStyle); // Sets the separator's style
	void setSeparatorWidth(GLfloat newSeparatorWidth); // Sets the separator's width
	Orientation getOrientation(void) const // Returns the separator's orientation
		{
		return orientation;
		}
	GLfloat getPreferredHeight(void) const // Returns the preferred height
		{
		return preferredHeight;
		}
	GLfloat getMarginWidth(void) const // Returns the margin width
		{
		return marginWidth;
		}
	Style getStyle(void) const // Returns the separator's style
		{
		return style;
		}
	GLfloat getSeparatorWidth(void) const // Returns the separator width
		{
		return separatorWidth;
		}
	};

}

#endif
