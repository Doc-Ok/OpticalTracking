/***********************************************************************
TextEvent - Class to notify widgets of text entry events.
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

#ifndef GLMOTIF_TEXTEVENT_INCLUDED
#define GLMOTIF_TEXTEVENT_INCLUDED

namespace GLMotif {

class TextEvent
	{
	/* Elements: */
	private:
	int textLength; // The length of the entered text
	char* text; // The entered text as an ASCII string
	
	/* Constructors and destructors: */
	public:
	TextEvent(void) // Creates an empty text event
		:textLength(0),text(new char[1])
		{
		text[0]='\0';
		}
	TextEvent(char sT) // Creates a single-character text event
		:textLength(1),text(new char[2])
		{
		text[0]=sT;
		text[1]='\0';
		}
	TextEvent(const char* sText); // Creates a multi-character text event
	TextEvent(const TextEvent& source); // Copy constructor
	TextEvent& operator=(const TextEvent& source); // Assignment operator
	~TextEvent(void)
		{
		delete[] text;
		}
	
	/* Methods: */
	int getTextLength(void) const // Returns the length of the entered text
		{
		return textLength;
		}
	const char* getText(void) const // Returns the entered text
		{
		return text;
		}
	};

}

#endif
