/***********************************************************************
WidgetAlgorithms - Functions to perform algorithms on trees of widgets.
Copyright (c) 2004-2010 Oliver Kreylos

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

#include <GLMotif/WidgetAlgorithms.h>

#include <GLMotif/Widget.h>
#include <GLMotif/Container.h>

namespace GLMotif {

Widget* getPreviousWidget(Widget* widget)
	{
	/* Find the widget's preceding sibling: */
	Container* parent=widget->getParent();
	if(parent==0)
		return 0;
	Widget* w1=0;
	for(Widget* w2=parent->getFirstChild();w2!=widget;w1=w2,w2=parent->getNextChild(w2))
		;
	widget=w1;
	
	/* Go up the widget hierarchy if the widget is invalid, otherwise traverse into containers: */
	if(widget==0)
		widget=parent;
	else
		{
		parent=dynamic_cast<Container*>(widget);
		while(parent!=0&&parent->getFirstChild()!=0)
			{
			/* Find the container's last child: */
			Widget* w1=0;
			for(Widget* w2=parent->getFirstChild();w2!=0;w1=w2,w2=parent->getNextChild(w2))
				;
			if(w1!=0)
				{
				widget=w1;
				parent=dynamic_cast<Container*>(widget);
				}
			}
		}
	
	return widget;
	}

Widget* getNextWidget(Widget* widget)
	{
	/* Traverse into containers first, then to siblings: */
	Container* parent=widget->getParent();
	if(dynamic_cast<Container*>(widget)!=0)
		{
		parent=dynamic_cast<Container*>(widget);
		widget=parent->getFirstChild();
		}
	else if(parent!=0)
		widget=parent->getNextChild(widget);
	else
		widget=0;
	
	/* Go up the widget hierarchy while the widget is invalid: */
	while(widget==0&&parent!=0)
		{
		widget=parent;
		parent=widget->getParent();
		if(parent!=0)
			widget=parent->getNextChild(widget);
		else
			widget=0;
		}
	
	return widget;
	}

}
