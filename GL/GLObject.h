/***********************************************************************
GLObject - Base class for objects that store OpenGL context-specific
data.
Copyright (c) 2006-2013 Oliver Kreylos

This file is part of the OpenGL Support Library (GLSupport).

The OpenGL Support Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The OpenGL Support Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the OpenGL Support Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef GLOBJECT_INCLUDED
#define GLOBJECT_INCLUDED

/* Forward declarations: */
class GLContextData;

class GLObject
	{
	/* Embedded classes: */
	public:
	struct DataItem // Base class for context data items
		{
		/* Constructors and destructors: */
		public:
		virtual ~DataItem(void) // Called to release any OpenGL resources allocated by the context data item
			{
			}
		};
	
	/* Protected methods: */
	protected:
	void dependsOn(const GLObject* thing) const; // Method declaring that this GLObject depends on another GLObject being initialized before it in every context
	void init(void); // Marks the object for context initialization if not done automatically in the GLObject constructor
	
	/* Constructors and destructors: */
	public:
	GLObject(bool autoInit =true); // Marks the object for context initialization if the given flag is true; otherwise, init() method must be called at some later point
	virtual ~GLObject(void); // Destroys the object and its associated context data item
	
	/* Methods: */
	virtual void initContext(GLContextData& contextData) const =0; // Method called before a GL object is rendered for the first time in the given OpenGL context
	};

#endif
