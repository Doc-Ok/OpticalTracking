/***********************************************************************
DragWidget - Mix-in class for GLMotif UI components reacting to dragging
events.
Copyright (c) 2001-2012 Oliver Kreylos

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

#ifndef GLMOTIF_DRAGWIDGET_INCLUDED
#define GLMOTIF_DRAGWIDGET_INCLUDED

#include <Misc/CallbackData.h>
#include <Misc/CallbackList.h>

/* Forward declarations: */
namespace GLMotif {
class Event;
class Widget;
}

namespace GLMotif {

class DragWidget
	{
	/* Embedded classes: */
	public:
	class CallbackData:public Misc::CallbackData // Base class for callback data sent by drag widgets
		{
		/* Elements: */
		public:
		DragWidget* dragWidget; // Pointer to the drag widget that sent the callback
		
		/* Constructors and destructors: */
		CallbackData(DragWidget* sDragWidget)
			:dragWidget(sDragWidget)
			{
			}
		};
	
	class DraggingCallbackData:public CallbackData // Class for callback data sent while a drag widget is dragged
		{
		/* Embedded classes: */
		public:
		enum DraggingCallbackReason // Enumerated type for different callback reasons
			{
			DRAGGING_STARTED,DRAGGING_STOPPED
			};
		
		/* Elements: */
		DraggingCallbackReason reason; // Reason for this dragging callback
		
		/* Constructors and destructors: */
		DraggingCallbackData(DragWidget* sDragWidget,DraggingCallbackReason sReason)
			:CallbackData(sDragWidget),reason(sReason)
			{
			}
		};
	
	/* Elements: */
	protected:
	Misc::CallbackList draggingCallbacks; // List of callbacks to be called when the widget starts/stops being dragged
	bool dragging; // Flag if the widget is currently being dragged
	
	/* Protected methods: */
	void startDragging(Event& event); // Starts dragging the widget
	void stopDragging(Event& event); // Stops dragging the widget
	bool overrideRecipient(Widget* widget,Event& event); // Puts the given widget as target into the event
	
	/* Constructors and destructors: */
	public:
	DragWidget(void)
		:dragging(false)
		{
		}
	virtual ~DragWidget(void);
	
	/* New methods: */
	bool isDragging(void) const // Returns true if the widget is currently being dragged
		{
		return dragging;
		}
	Misc::CallbackList& getDraggingCallbacks(void) // Returns list of dragging callbacks
		{
		return draggingCallbacks;
		}
	};

}

#endif
