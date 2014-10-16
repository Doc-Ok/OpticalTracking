/***********************************************************************
Pager - Container class to arrange children as individual pages in a
"flipbook" of sorts.
Copyright (c) 2013-2014 Oliver Kreylos

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

#ifndef GLMOTIF_PAGER_INCLUDED
#define GLMOTIF_PAGER_INCLUDED

#include <string>
#include <vector>
#include <Misc/CallbackData.h>
#include <Misc/CallbackList.h>
#include <GL/gl.h>
#include <GLMotif/Container.h>
#include <GLMotif/Button.h>

namespace GLMotif {

class Pager:public Container
	{
	/* Embedded classes: */
	private:
	typedef std::vector<Widget*> WidgetList; // Data type for list of child widgets
	typedef std::vector<Button*> ButtonList; // Data type for list of page selection buttons
	
	public:
	class PageChangedCallbackData:public Misc::CallbackData // Callback data passed to callbacks when the currently displayed page changes due to user interaction
		{
		/* Elements: */
		public:
		Pager* pager; // Pointer to the pager widget causing the event
		GLint newCurrentChildIndex; // Index of new displayed page
		
		/* Constructors and destructors: */
		PageChangedCallbackData(Pager* sPager,GLint sNewCurrentChildIndex)
			:pager(sPager),newCurrentChildIndex(sNewCurrentChildIndex)
			{
			}
		};
	
	/* Elements: */
	GLfloat marginWidth; // Width of margin around table
	WidgetList children; // List of child widgets, displayed one per page
	GLint nextChildIndex; // Index at which to insert the next child into the list
	unsigned int nextPageIndex; // Unique index to be assigned to the next page
	std::string nextPageName; // Name that will be assigned to the page created for the next added child
	bool addingPageButton; // Flag to indicate that a new page button is currently being added to the widget
	ButtonList pageButtons; // List of buttons to select individual pages
	Box buttonBox; // Box around the page buttons
	Box childBox; // Box around the child widget area
	GLint currentChildIndex; // Index of the currently displayed child widget
	Misc::CallbackList pageChangedCallbacks; // List of callbacks to be called when the currently displayed page changes due to user interaction
	
	/* Private methods: */
	void pageButtonSelectCallback(Button::SelectCallbackData* cbData);
	
	/* Constructors and destructors: */
	public:
	Pager(const char* sName,Container* sParent,bool sManageChild =true);
	virtual ~Pager(void);
	
	/* Methods from Widget: */
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
	void setNextChildIndex(GLint newNextChildIndex); // Sets the index at which to insert the next child
	void setNextPageName(const char* newNextPageName); // Sets the page name to assign to the next inserted child
	void setMarginWidth(GLfloat newMarginWidth); // Changes the margin width around the child widgets
	void setCurrentChildIndex(GLint newCurrentChildIndex); // Selects the page of the given index
	Misc::CallbackList& getPageChangedCallbacks(void) // Returns list of page changed callbacks
		{
		return pageChangedCallbacks;
		}
	};

}

#endif
