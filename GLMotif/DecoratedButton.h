/***********************************************************************
DecoratedButton - Class for buttons with decoration around their labels.
Copyright (c) 2001-2010 Oliver Kreylos

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

#ifndef GLMOTIF_DECORATEDBUTTON_INCLUDED
#define GLMOTIF_DECORATEDBUTTON_INCLUDED

#include <GLMotif/Button.h>

namespace GLMotif {

class DecoratedButton:public Button
	{
	/* Embedded classes: */
	protected:
	enum DecorationPosition // Positions for decorations
		{
		DECORATION_LEFT,DECORATION_RIGHT
		};
	
	/* Elements: */
	Vector decorationSize; // Natural size of the decoration
	DecorationPosition decorationPosition; // Position of decoration
	GLfloat spacing; // Spacing between decoration and label
	Box decorationBox; // Box containing decoration
	
	/* Protected methods: */
	virtual void drawDecoration(GLContextData& contextData) const =0; // Draws the decoration
	
	/* Constructors and destructors: */
	public:
	DecoratedButton(const char* sName,Container* sParent,const char* sLabel,const GLFont* sFont,bool manageChild =true); // Deprecated
	DecoratedButton(const char* sName,Container* sParent,const char* sLabel,bool manageChild =true);
	
	/* Methods inherited from Widget: */
	virtual Vector calcNaturalSize(void) const;
	virtual void resize(const Box& newExterior);
	virtual void draw(GLContextData& contextData) const;
	
	/* New methods: */
	void setDecorationSize(const Vector& newDecorationSize);
	void setDecorationPosition(DecorationPosition newDecorationPosition);
	void setSpacing(GLfloat newSpacing);
	};

}

#endif
