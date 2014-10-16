/***********************************************************************
ScrolledListBox - Compound widget containing a list box, a vertical, and
an optional horizontal scroll bar.
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

#ifndef GLMOTIF_SCROLLEDLISTBOX_INCLUDED
#define GLMOTIF_SCROLLEDLISTBOX_INCLUDED

#include <GLMotif/Container.h>
#include <GLMotif/ListBox.h>
#include <GLMotif/ScrollBar.h>

namespace GLMotif {

class ScrolledListBox:public Container
	{
	/* Elements: */
	private:
	ListBox* listBox; // Pointer to the list box widget
	ScrollBar* verticalScrollBar; // Pointer to the vertical scroll bar
	ScrollBar* horizontalScrollBar; // Pointer to the horizontal scroll bar
	
	/* Private methods: */
	void listBoxPageChangedCallback(ListBox::PageChangedCallbackData* cbData); // Method called when the list of visible items changes
	void scrollBarCallback(ScrollBar::ValueChangedCallbackData* cbData); // Method called on vertical or horizontal scrolling
	
	/* Constructors and destructors: */
	public:
	ScrolledListBox(const char* sName,Container* sParent,ListBox::SelectionMode sSelectionMode,int sPreferredWidth,int sPreferredPageSize,bool manageChild =true);
	virtual ~ScrolledListBox(void);
	
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
	
	/* New methods: */
	const ListBox* getListBox(void) const // Returns a pointer to the list box widget
		{
		return listBox;
		}
	ListBox* getListBox(void) // Ditto
		{
		return listBox;
		}
	void showHorizontalScrollBar(bool enable); // Shows or hides the horizontal scroll bar
	};

}

#endif
