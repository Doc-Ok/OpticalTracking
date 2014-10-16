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

#include <GLMotif/DragWidget.h>

#include <GLMotif/Event.h>

namespace GLMotif {

/***************************
Methods of class DragWidget:
***************************/

void DragWidget::startDragging(Event&)
	{
	if(!dragging)
		{
		/* Start dragging: */
		dragging=true;
		DraggingCallbackData cbData(this,DraggingCallbackData::DRAGGING_STARTED);
		draggingCallbacks.call(&cbData);
		}
	}

void DragWidget::stopDragging(Event&)
	{
	if(dragging)
		{
		/* Stop dragging: */
		dragging=false;
		DraggingCallbackData cbData(this,DraggingCallbackData::DRAGGING_STOPPED);
		draggingCallbacks.call(&cbData);
		}
	}

bool DragWidget::overrideRecipient(Widget* widget,Event& event)
	{
	/* This event belongs to the given widget! */
	return event.setTargetWidget(widget,event.calcWidgetPoint(widget));
	}

DragWidget::~DragWidget(void)
	{
	}

}
