/***********************************************************************
PopupMenu - Class for top-level GLMotif UI components that act as menus
and only require a single down-motion-up sequence to select an entry.
Copyright (c) 2001-2014 Oliver Kreylos

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

#ifndef GLMOTIF_POPUPMENU_INCLUDED
#define GLMOTIF_POPUPMENU_INCLUDED

#include <GLMotif/Popup.h>

namespace GLMotif {

class PopupMenu:public Popup
	{
	/* Elements: */
	protected:
	Widget* foundWidget; // True recipient of the next Event
	Widget* armedWidget; // Child that currently has the fake pointerButtonDown event
	
	/* Constructors and destructors: */
	public:
	PopupMenu(const char* sName,WidgetManager* sManager);
	
	/* Methods inherited from Widget: */
	virtual Vector calcHotSpot(void) const;
	virtual bool findRecipient(Event& event);
	virtual void pointerButtonDown(Event& event);
	virtual void pointerButtonUp(Event& event);
	virtual void pointerMotion(Event& event);
	};

}

#endif
