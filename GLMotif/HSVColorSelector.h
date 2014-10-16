/***********************************************************************
HSVColorSelector - Class for widgets to display and select colors based
on the HSV color model.
Copyright (c) 2012-2013 Oliver Kreylos

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

#ifndef GLMOTIF_HSVCOLORSELECTOR_INCLUDED
#define GLMOTIF_HSVCOLORSELECTOR_INCLUDED

#include <Misc/CallbackData.h>
#include <Misc/CallbackList.h>
#include <GL/gl.h>
#include <GLMotif/Container.h>
#include <GLMotif/DragWidget.h>
#include <GLMotif/Slider.h>

namespace GLMotif {

class HSVColorSelector:public Container,public DragWidget
	{
	/* Embedded classes: */
	public:
	class ValueChangedCallbackData:public Misc::CallbackData
		{
		/* Elements: */
		public:
		HSVColorSelector* colorSelector; // Pointer to the color selector widget causing the event
		Color newColor; // New RGB color value
		
		/* Constructors and destructors: */
		ValueChangedCallbackData(HSVColorSelector* sColorSelector,const Color& sNewColor)
			:colorSelector(sColorSelector),newColor(sNewColor)
			{
			}
		};
	
	/* Elements: */
	protected:
	GLfloat marginWidth; // Width of margin around color hexagon
	GLfloat preferredSize; // Preferred size (diameter) of color hexagon
	GLfloat indicatorSize; // Size of current-color indicator
	Slider* slider; // Pointer to the slider widget
	Box hexBox; // Box containing the color hexagon
	Vector hexCenter; // Center point of the color hexagon
	GLfloat hexRadius; // Radius of the color hexagon
	GLfloat currentValue; // Current value (brightness) of the color hexagon
	GLfloat currentColorPos[2]; // Position of current color inside color hexagon, normalized to [-1, 1]^2
	bool snapping; // Flag whether color values during the current dragging operation are snapped to 7 "pure" colors
	Misc::CallbackList valueChangedCallbacks; // List of callbacks to be called when the slider value changes due to a user interaction
	
	/* Private methods: */
	private:
	void sliderDraggingCallback(DraggingCallbackData* cbData);
	void sliderValueChangedCallback(Slider::ValueChangedCallbackData* cbData);
	
	/* Constructors and destructors: */
	public:
	HSVColorSelector(const char* sName,Container* sParent,bool manageChild =true);
	virtual ~HSVColorSelector(void);
	
	/* Methods inherited from Widget: */
	virtual Vector calcNaturalSize(void) const;
	virtual ZRange calcZRange(void) const;
	virtual void resize(const Box& newExterior);
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
	
	/* New methods: */
	void setMarginWidth(GLfloat newMarginWidth); // Sets the size of the margin around the color hexagon
	void setPreferredSize(GLfloat newPreferredSize); // Sets the minimum diameter of the color hexagon
	void setIndicatorSize(GLfloat newIndicatorSize); // Sets the size of the current-color indicator
	Color getCurrentColor(void) const; // Returns the currently selected color in RGB
	void setCurrentColor(const Color& newColor); // Sets the current color as RGB
	Misc::CallbackList& getValueChangedCallbacks(void) // Returns list of value changed callbacks
		{
		return valueChangedCallbacks;
		}
	};

}

#endif
