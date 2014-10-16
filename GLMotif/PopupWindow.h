/***********************************************************************
PopupWindow - Class for main windows with a draggable title bar and an
optional close button.
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

#ifndef GLMOTIF_POPUPWINDOW_INCLUDED
#define GLMOTIF_POPUPWINDOW_INCLUDED

#define GLMOTIF_POPUPWINDOW_USE_RENDERCACHE 0

#include <Misc/CallbackData.h>
#include <Misc/CallbackList.h>
#if GLMOTIF_POPUPWINDOW_USE_RENDERCACHE
#include <GL/gl.h>
#include <GL/GLObject.h>
#endif
#include <GLMotif/Container.h>

/* Forward declarations: */
class GLFont;
namespace GLMotif {
class TitleBar;
class NewButton;
}

namespace GLMotif {

#if GLMOTIF_POPUPWINDOW_USE_RENDERCACHE
class PopupWindow:public Container,public GLObject
#else
class PopupWindow:public Container
#endif
	{
	/* Embedded classes: */
	public:
	struct CallbackData:public Misc::CallbackData // Base class for popup window events
		{
		/* Elements: */
		public:
		PopupWindow* popupWindow; // Widget that generated the event
		
		/* Constructors and destructors: */
		CallbackData(PopupWindow* sPopupWindow)
			:popupWindow(sPopupWindow)
			{
			}
		};
	
	struct ResizeCallbackData:public CallbackData // Class for window resize events
		{
		/* Embedded classes: */
		public:
		enum WindowBorders // Enumerated type for window borders
			{
			BORDER_LEFT=0x1,BORDER_RIGHT=0x2,BORDER_BOTTOM=0x4,BORDER_TOP=0x8
			};
		
		/* Elements: */
		public:
		int borderMask; // Bit mask indicating which window borders are being dragged
		
		/* Constructors and destructors: */
		public:
		ResizeCallbackData(PopupWindow* sPopupWindow,int sBorderMask)
			:CallbackData(sPopupWindow),
			 borderMask(sBorderMask)
			{
			}
		};
	
	struct CloseCallbackData:public CallbackData // Class for window close events
		{
		/* Constructors and destructors: */
		public:
		CloseCallbackData(PopupWindow* sPopupWindow)
			:CallbackData(sPopupWindow)
			{
			}
		};
	
	#if GLMOTIF_POPUPWINDOW_USE_RENDERCACHE
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		public:
		GLuint displayListId; // ID of display list caching the rendering of all child widgets
		unsigned int version; // Version number of display list
		
		/* Constructors and destructors: */
		DataItem(void)
			:displayListId(glGenLists(1)),version(0)
			{
			}
		virtual ~DataItem(void)
			{
			glDeleteLists(displayListId,1);
			}
		};
	#endif
	
	/* Elements: */
	protected:
	WidgetManager* manager; // Pointer to the widget manager
	TitleBar* titleBar; // Pointer to the title bar widget
	NewButton* hideButton; // Optional button to hide the popup window
	NewButton* closeButton; // Optional button to close the popup window
	int resizableMask; // Bit mask whether the window can be resized horizontally (0x1) and/or vertically (0x2)
	GLfloat childBorderWidth; // Width of border around child widget
	Widget* child; // Single child of the popup window
	Misc::CallbackList resizeCallbacks; // List of callbacks called when the window is resized
	Misc::CallbackList closeCallbacks; // List of callbacks called when the window is closed via the close button
	
	bool isResizing; // Flag if the window is currently being resized by the user
	int resizeBorderMask; // Bit mask of which borders are being dragged 1 - left, 2 - right, 4 - bottom, 8 - top
	GLfloat resizeOffset[2]; // Offset from the initial resizing position to the relevant border
	
	#if GLMOTIF_POPUPWINDOW_USE_RENDERCACHE
	private:
	unsigned int version; // Version number of visual representation of popup window and child widgets
	#endif
	
	/* Protected methods: */
	protected:
	void hideButtonCallback(Misc::CallbackData* cbData);
	void closeButtonCallback(Misc::CallbackData* cbData);
	
	/* Constructors and destructors: */
	public:
	PopupWindow(const char* sName,WidgetManager* sManager,const char* sTitleString,const GLFont* font); // Deprecated
	PopupWindow(const char* sName,WidgetManager* sManager,const char* sTitleString);
	virtual ~PopupWindow(void); // Pops down the popup window and destroys it
	
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
	virtual Vector calcHotSpot(void) const;
	#if GLMOTIF_POPUPWINDOW_USE_RENDERCACHE
	virtual void update(void);
	#endif
	virtual void draw(GLContextData& contextData) const;
	virtual bool findRecipient(Event& event);
	virtual void pointerButtonDown(Event& event);
	virtual void pointerButtonUp(Event& event);
	virtual void pointerMotion(Event& event);
	
	/* Methods inherited from Container: */
	virtual void addChild(Widget* newChild);
	virtual void removeChild(Widget* removeChild);
	virtual void requestResize(Widget* child,const Vector& newExteriorSize);
	virtual Widget* getFirstChild(void);
	virtual Widget* getNextChild(Widget* child);
	
	#if GLMOTIF_POPUPWINDOW_USE_RENDERCACHE
	/* Methods inherited from GLObject: */
	virtual void initContext(GLContextData& contextData) const;
	#endif
	
	/* New methods: */
	void setTitleBorderWidth(GLfloat newTitleBorderWidth); // Changes the title border width
	void setTitleBarColor(const Color& newTitleBarColor); // Sets the color of the title bar
	void setTitleBarTextColor(const Color& newTitleBarTextColor); // Sets the text color of the title bar
	void setTitleString(const char* newTitleString); // Changes the title label string
	void setHideButton(bool enable); // Adds or removes the optional hide button
	void setCloseButton(bool enable); // Adds or removes the optional close button
	void setResizableFlags(bool horizontal,bool vertical); // Sets whether the popup window can be resized interactively
	void setChildBorderWidth(GLfloat newChildBorderWidth); // Changes the border width around the child widget
	const char* getTitleString(void) const; // Returns the current title label string
	const Widget* getChild(void) const // Returns the popup window's child
		{
		return child;
		}
	Widget* getChild(void) // Ditto
		{
		return child;
		}
	static void popDownFunction(Misc::CallbackData* cbData); // Default callback function that simply pops down, but does not delete, the popup window; cbData must be derived form PopupWindow::CallbackData
	static void deleteFunction(Misc::CallbackData* cbData); // Default callback function that simply deletes the popup window; cbData must be derived from PopupWindow::CallbackData
	Misc::CallbackList& getResizeCallbacks(void) // Returns list of callbacks called when the window is resized
		{
		return resizeCallbacks;
		}
	Misc::CallbackList& getCloseCallbacks(void) // Returns list of callbacks called when the close button is pressed
		{
		return closeCallbacks;
		}
	void popDownOnClose(void); // Convenience method to pop down the popup window when the close button is selected
	void deleteOnClose(void); // Convenience method to delete the popup window when the close button is selected
	virtual void close(void); // Convenience method to safely close and destroy the popup window from within a callback
	};

}

#endif
