/***********************************************************************
Popup - Class for top-level GLMotif UI components.
Copyright (c) 2001-2010 Oliver Kreylos

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

#ifndef GLMOTIF_POPUP_INCLUDED
#define GLMOTIF_POPUP_INCLUDED

#include <GLMotif/Container.h>

/* Forward declarations: */
class GLFont;
namespace GLMotif {
class Label;
}

namespace GLMotif {

class Popup:public Container
	{
	/* Elements: */
	protected:
	WidgetManager* manager; // Pointer to the widget manager
	GLfloat marginWidth; // Width of margin around title and child widget
	GLfloat titleSpacing; // Height of spacing between title and child widget
	Label* title; // Label widget for the popup title (optional)
	Widget* child; // Single child of the popup
	
	/* Constructors and destructors: */
	public:
	Popup(const char* sName,WidgetManager* sManager);
	virtual ~Popup(void); // Pops down the widget if it is currently managed by the widget manager and destroys it
	
	/* Methods inherited from Widget: */
	virtual const WidgetManager* getManager(void) const
		{
		return manager;
		}
	virtual WidgetManager* getManager(void)
		{
		return manager;
		}
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
	
	/* New methods: */
	void setMarginWidth(GLfloat newMarginWidth); // Changes the margin width
	void setTitleSpacing(GLfloat newTitleSpacing); // Changes the title spacing
	void setTitle(const char* titleString,const GLFont* font); // Changes the title label string; deprecated
	void setTitle(const char* titleString); // Changes the title label string
	const Widget* getChild(void) const // Returns the popup's child
		{
		return child;
		}
	Widget* getChild(void) // Ditto
		{
		return child;
		}
	};

}

#endif
