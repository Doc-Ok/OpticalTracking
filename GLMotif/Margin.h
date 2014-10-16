/***********************************************************************
Margin - Class to create a margin around a widget, or pad a widget's
natural size into a larger frame.
Copyright (c) 2008-2010 Oliver Kreylos

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

#ifndef GLMOTIF_MARGIN_INCLUDED
#define GLMOTIF_MARGIN_INCLUDED

#include <GLMotif/Alignment.h>
#include <GLMotif/SingleChildContainer.h>

namespace GLMotif {

class Margin:public SingleChildContainer
	{
	/* Elements: */
	protected:
	GLfloat marginWidth; // Minimal width of margin around child widget
	Alignment alignment; // Alignment for child widget in oversized frame
	
	/* Protected methods inherited from SingleChildContainer: */
	virtual Vector calcInteriorSize(const Vector& childSize) const;
	virtual Box calcChildBox(const Box& interior) const;
	
	/* Constructors and destructors: */
	public:
	Margin(const char* sName,Container* sParent,bool manageChild =true);
	
	/* Methods inherited from Widget: */
	virtual void draw(GLContextData& contextData) const;
	
	/* New methods: */
	GLfloat getMarginWidth(void) const // Returns the margin's minimal margin width
		{
		return marginWidth;
		}
	const Alignment& getAlignment(void) const // Returns the child alignment
		{
		return alignment;
		}
	void setMarginWidth(GLfloat newMarginWidth); // Changes the margin width
	void setAlignment(const Alignment& newAlignment); // Changes the child alignment
	};

}

#endif
