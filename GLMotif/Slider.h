/***********************************************************************
Slider - Class for horizontal or vertical sliders.
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

#ifndef GLMOTIF_SLIDER_INCLUDED
#define GLMOTIF_SLIDER_INCLUDED

#include <vector>
#include <Misc/CallbackData.h>
#include <Misc/CallbackList.h>
#include <Misc/TimerEventScheduler.h>
#include <GLMotif/Widget.h>
#include <GLMotif/DragWidget.h>

namespace GLMotif {

class Slider:public Widget,public DragWidget
	{
	/* Embedded classes: */
	public:
	enum Orientation // Enumerated type for slider orientations
		{
		HORIZONTAL,VERTICAL
		};
	
	class ValueChangedCallbackData:public Misc::CallbackData
		{
		/* Embedded classes: */
		public:
		enum ChangeReason // Enumerated type for different change reasons
			{
			CLICKED,DRAGGED
			};
		
		/* Elements: */
		Slider* slider; // Pointer to the slider widget causing the event
		ChangeReason reason; // Reason for this value change
		GLfloat value; // Current slider value
		
		/* Constructors and destructors: */
		ValueChangedCallbackData(Slider* sSlider,ChangeReason sReason,GLfloat sValue)
			:slider(sSlider),reason(sReason),value(sValue)
			{
			}
		};
	
	/* Elements: */
	protected:
	GLfloat marginWidth; // Width of margin around slider and shaft
	Orientation orientation; // Slider orientation
	GLfloat sliderWidth; // Width of slider handle (assuming vertical slider)
	GLfloat sliderLength; // Length of slider handle (assuming vertical slider)
	GLfloat sliderHeight; // Height of slider handle
	GLfloat shaftWidth; // Width of shaft (assuming vertical slider)
	GLfloat shaftLength; // Length of shaft (assuming vertical slider)
	GLfloat shaftDepth; // Depth of shaft
	Box sliderBox; // Position and size of slider handle
	Color sliderColor; // Color of slider handle
	Box shaftBox; // Position and size of shaft
	Color shaftColor; // Color of shaft
	GLfloat valueMin,valueMax,valueIncrement; // Value range and increment
	std::vector<GLfloat> notchValues; // Values of "notches" along the slider to simplify selection of special values
	std::vector<GLfloat> notchPositions; // Positions of notches
	GLfloat value; // Currently selected value
	Misc::CallbackList valueChangedCallbacks; // List of callbacks to be called when the slider value changes due to a user interaction
	
	int isClicking; // Flag if the slider is currently waiting for click repeat timer events, and whether it's decrementing (<0) or incrementing (>0)
	double nextClickEventTime; // Time at which the next click-repeat event was scheduled
	GLfloat dragOffset; // Offset between pointer position and slider origin during dragging
	GLfloat lastDragPos; // Position of dragged slider handle at last frame
	
	/* Protected methods: */
	void positionShaft(void); // Positions the shaft inside the widget
	void positionNotches(void); // Calculates the positions of all slider notches
	void positionSlider(void); // Positions the slider handle inside the widget
	void decrement(void); // Decrements the slider value by the current granularity
	void increment(void); // Increments the slider value by the current granularity
	void clickRepeatTimerEventCallback(Misc::TimerEventScheduler::CallbackData* cbData); // Callback for click-repeat timer events
	
	/* Constructors and destructors: */
	public:
	Slider(const char* sName,Container* sParent,Orientation sOrientation,GLfloat sSliderWidth,GLfloat sShaftLength,bool sManageChild =true); // Deprecated
	Slider(const char* sName,Container* sParent,Orientation sOrientation,GLfloat sShaftLength,bool sManageChild =true);
	virtual ~Slider(void);
	
	/* Methods inherited from Widget: */
	virtual Vector calcNaturalSize(void) const;
	virtual ZRange calcZRange(void) const;
	virtual void resize(const Box& newExterior);
	virtual void draw(GLContextData& contextData) const;
	virtual bool findRecipient(Event& event);
	virtual void pointerButtonDown(Event& event);
	virtual void pointerButtonUp(Event& event);
	virtual void pointerMotion(Event& event);
	
	/* New methods: */
	void setMarginWidth(GLfloat newMarginWidth); // Changes the margin width
	void setSliderColor(const Color& newSliderColor) // Changes color of slider handle
		{
		sliderColor=newSliderColor;
		}
	void setShaftColor(const Color& newShaftColor) // Changes color of shaft
		{
		shaftColor=newShaftColor;
		}
	GLfloat getValue(void) const // Returns the current slider value
		{
		return value;
		}
	void addNotch(GLfloat newNotchValue); // Adds a notch to the slider
	void removeNotch(GLfloat notchValue); // Removes a notch from the slider
	void setValue(GLfloat newValue); // Changes the current slider value
	void setValueRange(GLfloat newValueMin,GLfloat newValueMax,GLfloat newValueIncrement); // Changes the slider value range
	Misc::CallbackList& getValueChangedCallbacks(void) // Returns list of value changed callbacks
		{
		return valueChangedCallbacks;
		}
	};

}

#endif
