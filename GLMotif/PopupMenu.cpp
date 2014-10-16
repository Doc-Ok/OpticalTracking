/***********************************************************************
PopupMenu - Class for top-level GLMotif UI components that act as menus
and only require a single down-motion-up sequence to select an entry.
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

#include <GLMotif/Event.h>

#include <GLMotif/PopupMenu.h>

namespace GLMotif {

/**************************
Methods of class PopupMenu:
**************************/

PopupMenu::PopupMenu(const char* sName,WidgetManager* sManager)
	:Popup(sName,sManager),
	 foundWidget(0),armedWidget(0)
	{
	}

Vector PopupMenu::calcHotSpot(void) const
	{
	Vector result=Widget::calcHotSpot();
	result[0]=getExterior().origin[0]+getExterior().size[0]-marginWidth*0.5f;
	return result;
	}

bool PopupMenu::findRecipient(Event& event)
	{
	/* Call the parent class method: */
	bool result=Popup::findRecipient(event);
	if(result&&event.getTargetWidget()!=this)
		{
		/* Override the event's target widget to intercept all subsequent events: */
		foundWidget=event.getTargetWidget();
		event.overrideTargetWidget(this);
		}
	else
		foundWidget=0;
	
	return result;
	}

void PopupMenu::pointerButtonDown(Event& event)
	{
	/* "Repair" the incoming event: */
	event.overrideTargetWidget(foundWidget);
	
	/* Store the found widget and arm it: */
	armedWidget=foundWidget;
	if(armedWidget!=0)
		armedWidget->pointerButtonDown(event);
	}

void PopupMenu::pointerButtonUp(Event& event)
	{
	/* "Repair" the incoming event: */
	event.overrideTargetWidget(foundWidget);
	
	/* Disarm the currently armed widget: */
	if(armedWidget!=0)
		{
		armedWidget->pointerButtonUp(event);
		armedWidget=0;
		}
	}

void PopupMenu::pointerMotion(Event& event)
	{
	/* "Repair" the incoming event: */
	event.overrideTargetWidget(foundWidget);
	
	/* Arm/disarm widgets as we go by sending fake button events: */
	if(event.isPressed()&&foundWidget!=armedWidget)
		{
		if(armedWidget!=0)
			armedWidget->pointerButtonUp(event);
		armedWidget=foundWidget;
		if(armedWidget!=0)
			armedWidget->pointerButtonDown(event);
		}
	else if(armedWidget!=0)
		armedWidget->pointerMotion(event);
	}

}
