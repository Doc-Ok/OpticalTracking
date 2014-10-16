/***********************************************************************
SingleChildContainer - Base class for containers that contain at most
one child.
Copyright (c) 2008-2010 Oliver Kreylos

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

#ifndef GLMOTIF_SINGLECHILDCONTAINER_INCLUDED
#define GLMOTIF_SINGLECHILDCONTAINER_INCLUDED

#include <GLMotif/Container.h>

namespace GLMotif {

class SingleChildContainer:public Container
	{
	/* Elements: */
	protected:
	Widget* child; // Pointer to the child widget, or 0
	
	/* Protected methods: */
	virtual Vector calcInteriorSize(const Vector& childSize) const; // Calculates the preferred interior size of the widget, based on the size of the child widget
	virtual Box calcChildBox(const Box& interior) const; // Calculates the position and the size of the child widget based on the given widget interior; only called if there is a child
	
	/* Constructors and destructors: */
	public:
	SingleChildContainer(const char* sName,Container* sParent,bool manageChild =true); // Creates container with no child
	virtual ~SingleChildContainer(void); // Deletes the container and the child widget
	
	/* Methods inherited from Widget: */
	virtual Vector calcNaturalSize(void) const;
	virtual ZRange calcZRange(void) const;
	virtual void resize(const Box& newExterior);
	virtual void draw(GLContextData& contextData) const;
	virtual bool findRecipient(Event& event);
	
	/* Methods inherited from Container: */
	virtual void addChild(Widget* newChild);
	virtual void removeChild(Widget* removeChild);
	virtual void requestResize(Widget* child,const Vector& newExteriorSize);
	virtual Widget* getFirstChild(void);
	virtual Widget* getNextChild(Widget* child);
	};

}

#endif


