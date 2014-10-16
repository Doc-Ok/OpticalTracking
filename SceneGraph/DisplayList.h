/***********************************************************************
DisplayList - Base class for objects that use OpenGL display lists for
rendering.
Copyright (c) 2009 Oliver Kreylos

This file is part of the Simple Scene Graph Renderer (SceneGraph).

The Simple Scene Graph Renderer is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Simple Scene Graph Renderer is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Simple Scene Graph Renderer; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef SCENEGRAPH_DISPLAYLIST_INCLUDED
#define SCENEGRAPH_DISPLAYLIST_INCLUDED

#include <GL/gl.h>
#include <GL/GLObject.h>

namespace SceneGraph {

class DisplayList:public GLObject
	{
	/* Embedded classes: */
	private:
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		public:
		GLuint displayListId; // ID of display list
		unsigned int version; // Version number of geometry stored in display list
		
		/* Constructors and destructors: */
		DataItem(void); // Creates a display list
		virtual ~DataItem(void); // Destroys the display list
		};
	
	/* Elements: */
	unsigned int version; // Version number of geometry in the main object
	
	/* Protected methods: */
	protected:
	virtual void createList(GLContextData& contextData) const =0; // Uploads geometry into the display list
	
	/* Constructors and destructors: */
	public:
	DisplayList(void)
		:version(0)
		{
		}
	
	/* Methods from GLObject: */
	virtual void initContext(GLContextData& contextData) const;
	
	/* New methods: */
	void update(void); // Invalidates the geometry stored in the display list
	void glRenderAction(GLContextData& contextData) const; // Updates and renders the display list
	};

}

#endif
