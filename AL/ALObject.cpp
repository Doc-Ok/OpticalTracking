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

#include <AL/ALContextData.h>

#include <AL/ALObject.h>

/*************************
Methods of class ALObject:
*************************/

ALObject::ALObject(void)
	{
	/* Mark the object for context initialization: */
	ALContextData::initThing(this);
	}

ALObject::~ALObject(void)
	{
	/* Mark the object's context data item for destruction: */
	ALContextData::destroyThing(this);
	}
