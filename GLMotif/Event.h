/***********************************************************************
Event - Class to provide widgets with information they need to handle
events.
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

#ifndef GLMOTIF_EVENT_INCLUDED
#define GLMOTIF_EVENT_INCLUDED

#include <Math/Constants.h>
#include <GLMotif/Types.h>

/* Forward declarations: */
namespace GLMotif {
class Widget;
}

namespace GLMotif {

class Event
	{
	/* Embedded classes: */
	private:
	enum WorldLocationType // Ways to specify event locations
		{
		NONE,POINT,RAY
		};
	
	public:
	class WidgetPoint // Class for results of widget point queries
		{
		friend class Event;
		
		/* Elements: */
		private:
		Scalar lambda; // Ray parameter if event's world location is a ray
		Point point; // Point coordinates
		
		/* Constructors and destructors: */
		public:
		WidgetPoint(void) // Creates uninitialized widget point
			:lambda(Math::Constants<Scalar>::max)
			{
			}
		
		/* Methods: */
		const Point& getPoint(void) const // Returns the widget point
			{
			return point;
			}
		};
	
	/* Elements: */
	private:
	WorldLocationType worldLocationType; // Type of world location in this event
	Point worldLocationPoint; // World location specified as a point
	Ray worldLocationRay; // World location specified as a ray
	bool buttonState; // Pointer button state right before the event occured (true=pressed)
	Widget* targetWidget; // Widget used to calculate widget location; intended recipient of event
	WidgetPoint widgetPoint; // Widget location of this event
	
	/* Constructors and destructors: */
	public:
	Event(bool sButtonState); // Creates event without world location
	Event(const Point& sWorldLocationPoint,bool sButtonState); // Creates event with world location as point
	Event(const Ray& sWorldLocationRay,bool sButtonState); // Creates event with world location as ray
	
	/* Methods: */
	void setWorldLocation(const Point& newWorldLocationPoint) // Sets the world location to a point
		{
		worldLocationType=POINT;
		worldLocationPoint=newWorldLocationPoint;
		}
	void setWorldLocation(const Ray& newWorldLocationRay) // Sets the world location to a ray
		{
		worldLocationType=RAY;
		worldLocationRay=newWorldLocationRay;
		}
	bool isPressed(void) const // Returns true if the pointer button was pressed right before the event occurred
		{
		return buttonState;
		}
	const Widget* getTargetWidget(void) const // Returns the target widget
		{
		return targetWidget;
		}
	Widget* getTargetWidget(void) // Ditto
		{
		return targetWidget;
		}
	const WidgetPoint& getWidgetPoint(void) const // Returns the stored widget point
		{
		return widgetPoint;
		}
	bool setTargetWidget(Widget* newTargetWidget); // Sets the event's target widget; returns true if the setting was successful
	bool setTargetWidget(Widget* newTargetWidget,const WidgetPoint& newWidgetPoint); // Ditto, if the widget point is already known
	void overrideTargetWidget(Widget* newTargetWidget) // Overrides the target widget without changing the widget point
		{
		targetWidget=newTargetWidget;
		}
	WidgetPoint calcWidgetPoint(const Widget* widget) const; // Returns event point in widget's coordinate system
	};

}

#endif
