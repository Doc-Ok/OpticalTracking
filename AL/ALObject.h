/***********************************************************************
ALObject - Base class for objects that store OpenAL context-specific
data.
Copyright (c) 2008 Oliver Kreylos

This file is part of the OpenAL Support Library (ALSupport).

The OpenAL Support Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The OpenAL Support Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the OpenAL Support Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef ALOBJECT_INCLUDED
#define ALOBJECT_INCLUDED

/* Forward declarations: */
class ALContextData;

class ALObject
	{
	/* Embedded classes: */
	public:
	struct DataItem // Base class for context data items
		{
		/* Constructors and destructors: */
		public:
		virtual ~DataItem(void) // Called to release any OpenAL resources allocated by the context data item
			{
			}
		};
	
	/* Constructors and destructors: */
	public:
	ALObject(void); // Marks the object for context initialization
	virtual ~ALObject(void); // Destroys the object and its associated context data item
	
	/* Methods: */
	virtual void initContext(ALContextData& contextData) const =0; // Method called before an AL object is rendered for the first time in the given OpenAL context
	};

#endif
