/***********************************************************************
TextControlEvent - Class to notify widgets of events controlling text
entry (cursor movement, cut & paste, etc.).
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

#ifndef GLMOTIF_TEXTCONTROLEVENT_INCLUDED
#define GLMOTIF_TEXTCONTROLEVENT_INCLUDED

namespace GLMotif {

class TextControlEvent
	{
	/* Embedded classes: */
	public:
	enum EventType // Enumerated type for events
		{
		CURSOR_LEFT,CURSOR_RIGHT,
		CURSOR_WORD_LEFT,CURSOR_WORD_RIGHT,
		CURSOR_START,CURSOR_END,
		CURSOR_UP,CURSOR_DOWN,
		CURSOR_PAGE_UP,CURSOR_PAGE_DOWN,
		CURSOR_TEXT_START,CURSOR_TEXT_END,
		DELETE,BACKSPACE,
		CUT,COPY,PASTE,
		CONFIRM,
		EVENTTYPE_END
		};
	
	/* Elements: */
	public:
	EventType event; // Type of this event
	bool selection; // Flag whether a cursor movement affects the cursor or the selection
	
	/* Constructors and destructors: */
	public:
	TextControlEvent(void) // Constructs uninitialied event
		:event(EVENTTYPE_END),selection(false)
		{
		}
	TextControlEvent(EventType sEvent,bool sSelection =false)
		:event(sEvent),selection(sSelection)
		{
		}
	};

}

#endif
