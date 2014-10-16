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

#include <GLMotif/TextEvent.h>

#include <string.h>

namespace GLMotif {

/**************************
Methods of class TextEvent:
**************************/

TextEvent::TextEvent(const char* sText)
	:textLength(strlen(sText)),
	 text(new char[textLength+1])
	{
	memcpy(text,sText,textLength+1);
	}

TextEvent::TextEvent(const TextEvent& source)
	:textLength(source.textLength),
	 text(new char[textLength+1])
	{
	memcpy(text,source.text,textLength+1);
	}

TextEvent& TextEvent::operator=(const TextEvent& source)
	{
	if(this!=&source)
		{
		if(textLength!=source.textLength)
			{
			delete[] text;
			textLength=source.textLength;
			text=new char[textLength+1];
			}
		memcpy(text,source.text,textLength+1);
		}
	return *this;
	}

}
