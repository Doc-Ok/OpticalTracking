/***********************************************************************
Blind - Class for dummy widgets (space-filling placeholders, actually).
Copyright (c) 2001-2005 Oliver Kreylos

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

#ifndef GLMOTIF_BLIND_INCLUDED
#define GLMOTIF_BLIND_INCLUDED

#include <GLMotif/Widget.h>

namespace GLMotif {

class Blind:public Widget
	{
	/* Elements: */
	protected:
	Vector preferredSize; // The widget's preferred size
	
	/* Constructors and destructors: */
	public:
	Blind(const char* sName,Container* sParent,bool manageChild =true);
	
	/* Methods inherited from Widget: */
	virtual Vector calcNaturalSize(void) const;
	virtual void draw(GLContextData& contextData) const;
	
	/* New methods: */
	void setPreferredSize(const Vector& newPreferredSize); // Sets a new preferred size
	};

}

#endif
