/***********************************************************************
Container - Base class for GLMotif UI components that contain other
components.
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

#ifndef GLMOTIF_CONTAINER_INCLUDED
#define GLMOTIF_CONTAINER_INCLUDED

#include <GLMotif/Widget.h>

namespace GLMotif {

class Container:public Widget
	{
	/* Protected methods: */
	protected:
	static void deleteChild(Widget* child) // Safely unmanages and deletes a child widget
		{
		if(child!=0)
			{
			child->unmanageChild();
			delete child;
			}
		}
	
	/* Constructors and destructors: */
	public:
	Container(const char* sName,Container* sParent,bool manageChild =true);
	
	/* New methods: */
	virtual void addChild(Widget* newChild) =0; // Adds a new child to the container
	virtual void removeChild(Widget* removeChild) =0; // Removes a child from the container
	virtual void requestResize(Widget* child,const Vector& newExteriorSize) =0; // Allows a child to request a resize of itself
	virtual Widget* getFirstChild(void) =0; // Returns pointer to the first child widget; returns null if container has no children
	virtual Widget* getNextChild(Widget* child) =0; // Returns pointer to next child after the given one; returns null at end of list or when given widget is not a child
	Widget* findChild(const char* childName); // Returns pointer to a direct child of the given name; returns null if child of given name does not exist
	Widget* findDescendant(const char* descendantPath); // Returns pointer to a descendent of the given path, widget names are separated by forward slashes; returns null if final child does not exist; throws exception if any intermediate path node does not exist
	};

}

#endif
