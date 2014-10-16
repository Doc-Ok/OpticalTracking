/***********************************************************************
Widget - Base class for GLMotif UI components.
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

#ifndef GLMOTIF_WIDGET_INCLUDED
#define GLMOTIF_WIDGET_INCLUDED

#include <GLMotif/Types.h>

/* Forward declarations: */
class GLContextData;
namespace GLMotif {
struct StyleSheet;
class Event;
class TextEvent;
class TextControlEvent;
class WidgetManager;
class Container;
}

namespace GLMotif {

class Widget
	{
	/* Embedded classes: */
	public:
	enum BorderType // Enumerated type to select different ways to draw a widget's border
		{
		PLAIN,RAISED,LOWERED
		};
	
	/* Elements: */
	protected:
	Container* parent; // Pointer to widget's parent (must be a container)
	bool isManaged; // Flag if the child is currently managed by its parent
	private:
	char* name; // Widget's name (unique amongst siblings)
	Box exterior; // Exterior rectangle of widget (including border)
	GLfloat borderWidth; // Width of border around widget
	BorderType borderType; // Type of border around widget
	Box interior; // Interior rectangle of widget (excluding border)
	ZRange zRange; // Z range of the widget with respect to its exterior
	bool enabled; // Flag if the widget is enabled, i.e., reacts to events
	protected:
	Color borderColor; // Color of widget's border
	Color backgroundColor; // Color of widget's background
	Color foregroundColor; // Color of widget's foreground
	
	/* "Protected" methods: */
	public:
	void unmanageChild(void); // Removes the widget from its parent container; must only be called by parent
	
	/* Constructors and destructors: */
	public:
	Widget(const char* sName,Container* sParent,bool sManageChild =true);
	virtual ~Widget(void);
	
	/* Methods: */
	const char* getName(void) const
		{
		return name;
		}
	const Container* getParent(void) const // Returns a widget's parent
		{
		return parent;
		}
	Container* getParent(void) // Ditto
		{
		return parent;
		}
	void reparent(Container* newParent,bool manageChild =true); // Changes the parent widget of a widget; overrides basic widget settings with those from new parent
	void manageChild(void); // Adds a child to its parent container
	const Widget* getRoot(void) const; // Returns the root of a widget tree (a popup widget)
	Widget* getRoot(void); // Ditto
	virtual const WidgetManager* getManager(void) const; // Returns a pointer to the widget manager
	virtual WidgetManager* getManager(void); // Ditto
	const StyleSheet* getStyleSheet(void) const; // Returns a pointer to the widget manager's style sheet
	const Box& getExterior(void) const
		{
		return exterior;
		}
	GLfloat getBorderWidth(void) const
		{
		return borderWidth;
		}
	BorderType getBorderType(void) const
		{
		return borderType;
		}
	const Box& getInterior(void) const
		{
		return interior;
		}
	ZRange getZRange(void) const
		{
		return zRange;
		}
	bool isEnabled(void) const
		{
		return enabled;
		}
	const Color& getBorderColor(void) const // Returns widget's border color
		{
		return borderColor;
		}
	const Color& getBackgroundColor(void) const // Returns widget's background color
		{
		return backgroundColor;
		}
	const Color& getForegroundColor(void) const // Returns widget's foreground color
		{
		return foregroundColor;
		}
	Vector calcExteriorSize(const Vector& interiorSize) const; // Converts requested interior size to exterior size
	virtual Vector calcNaturalSize(void) const =0; // Returns the preferred exterior size of a widget
	virtual ZRange calcZRange(void) const; // Calculates the z range of a widget
	virtual void resize(const Box& newExterior); // Moves and resizes a widget to the given exterior size
	virtual Vector calcHotSpot(void) const; // Returns a "hot spot" for the widget (usually its center)
	virtual void setBorderWidth(GLfloat newBorderWidth); // Changes a widget's border width
	virtual void setBorderType(BorderType newBorderType); // Changes a widget's border type
	virtual void setBorderColor(const Color& newBorderColor); // Changes a widget's border color
	virtual void setBackgroundColor(const Color& newBackgroundColor); // Changes a widget's background color
	virtual void setForegroundColor(const Color& newForegroundColor); // Changes a widget's foreground color
	virtual void update(void); // Method called whenever a widget changes its visual representation, to facilitate render caching
	virtual void draw(GLContextData& contextData) const; // Draws the widget
	
	/* User interaction events: */
	bool isInside(const Point& p) const; // Tests whether a point is inside the widget's bounding box
	Scalar intersectRay(const Ray& ray,Point& intersection) const; // Intersects a ray with a widget's center plane
	virtual void setEnabled(bool newEnabled); // Enables or disables the widget
	virtual bool findRecipient(Event& event); // Determines which widget is to receive a localized event
	virtual void pointerButtonDown(Event& event);
	virtual void pointerButtonUp(Event& event);
	virtual void pointerMotion(Event& event);
	virtual bool giveTextFocus(void); // Method to give text entry focus to a widget; must return false if widget does not accept focus
	virtual void takeTextFocus(void); // Method to indicate that text entry focus was taken away from the widget
	virtual void textEvent(const TextEvent& event);
	virtual void textControlEvent(const TextControlEvent& event);
	};

}

#endif
