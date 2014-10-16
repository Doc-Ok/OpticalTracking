/***********************************************************************
ScrollBar - Class for horizontal or vertical scroll bars, to be used as
a component by scrolling widgets like list boxes.
Copyright (c) 2008-2012 Oliver Kreylos

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

#ifndef GLMOTIF_SCROLLBAR_INCLUDED
#define GLMOTIF_SCROLLBAR_INCLUDED

#include <Misc/CallbackData.h>
#include <Misc/CallbackList.h>
#include <Misc/TimerEventScheduler.h>
#include <GL/gl.h>
#include <GLMotif/GlyphGadget.h>
#include <GLMotif/Widget.h>
#include <GLMotif/DragWidget.h>

namespace GLMotif {

class ScrollBar:public Widget,public DragWidget
	{
	/* Embedded classes: */
	public:
	enum Orientation // Enumerated type for scroll bar orientations
		{
		HORIZONTAL,VERTICAL
		};
	
	class ValueChangedCallbackData:public Misc::CallbackData
		{
		/* Embedded classes: */
		public:
		enum ChangeReason // Enumerated type for different change reasons
			{
			ITEM_UP,ITEM_DOWN,PAGE_UP,PAGE_DOWN,DRAGGED
			};
		
		/* Elements: */
		ScrollBar* scrollBar; // Pointer to the scroll bar widget causing the event
		ChangeReason reason; // Reason for this value change
		int position; // Current scroll bar position
		
		/* Constructors and destructors: */
		ValueChangedCallbackData(ScrollBar* sScrollBar,ChangeReason sReason,int sPosition)
			:scrollBar(sScrollBar),reason(sReason),position(sPosition)
			{
			}
		};
	
	/* Elements: */
	protected:
	Orientation orientation; // Scroll bar orientation
	bool reverse; // Flag whether the scroll bar direction is reversed
	GLfloat scrollBarWidth; // Width of scroll bar (assuming vertical scroll bar)
	GLfloat bevelWidth; // Width of bevel for arrow buttons and scroll bar handle
	Box arrowBox[2]; // Position and size of the decrement and increment buttons
	Box arrowBevelBox[2]; // Position and size of the interior of the arrow buttons' bevels
	GlyphGadget arrows[2]; // The two arrow glyphs
	GLfloat shaftDepth; // Depth of scroll bar shaft
	Color shaftColor; // Color of scroll bar shaft
	Box shaftBox; // Position and size of the scroll bar shaft
	Color handleColor; // Color of scroll bar handle
	int positionMin,positionMax; // Range of scroll bar position values
	int pageSize; // Number of positions visible in the scrolled widget
	int position; // Current position
	Box handleBox; // Position and size of the scroll bar handle
	Box handleBottomBox; // Position and size of bottom of scroll bar handle
	Box handleBevelBox; // Position and size of the interior of the handle's bevel
	Misc::CallbackList valueChangedCallbacks; // List of callbacks to be called when the scroll bar position changes due to a user interaction
	
	int armedArrowIndex; // Index of the arrow that is currently armed; -1 if none armed
	bool isClicking; // Flag if the scroll bar is currently waiting for click repeat timer events
	int clickPositionIncrement; // Position increment for each timer event
	ValueChangedCallbackData::ChangeReason clickChangeReason; // Change reason for each following click event
	double nextClickEventTime; // Time at which the next click-repeat event was scheduled
	GLfloat dragOffset; // Offset between pointer position and handle origin during dragging
	
	/* Protected methods: */
	void positionButtonsAndShaft(void); // Positions the arrow buttons and scroll bar shaft inside the widget
	void positionHandle(void); // Positions the scroll bar handle inside the widget
	void drawBeveledBox(const Box& base,const Box& bevel) const; // Draws the bevel around a box
	void clickRepeatTimerEventCallback(Misc::TimerEventScheduler::CallbackData* cbData); // Callback for click-repeat timer events
	void scheduleClickRepeat(int increment,ValueChangedCallbackData::ChangeReason reason,double interval); // Schedules a click-repeat timer event
	
	/* Constructors and destructors: */
	public:
	ScrollBar(const char* sName,Container* sParent,Orientation sOrientation,bool sReverse,bool sManageChild =true);
	virtual ~ScrollBar(void);
	
	/* Methods inherited from Widget: */
	virtual Vector calcNaturalSize(void) const;
	virtual ZRange calcZRange(void) const;
	virtual void resize(const Box& newExterior);
	virtual void setBackgroundColor(const Color& newBackgroundColor);
	virtual void draw(GLContextData& contextData) const;
	virtual bool findRecipient(Event& event);
	virtual void pointerButtonDown(Event& event);
	virtual void pointerButtonUp(Event& event);
	virtual void pointerMotion(Event& event);
	virtual bool giveTextFocus(void);
	virtual void textControlEvent(const TextControlEvent& event);
	
	/* New methods: */
	void setBevelWidth(GLfloat newBevelWidth); // Changes width of bevel around arrow buttons and scroll bar handle
	void setShaftColor(const Color& newShaftColor) // Changes color of shaft
		{
		shaftColor=newShaftColor;
		}
	void setHandleColor(const Color& newHandleColor) // Changes color of scroll bar handle
		{
		handleColor=newHandleColor;
		}
	int getPosition(void) const // Returns the current scroll bar position
		{
		return position;
		}
	void setPosition(int newPosition); // Changes the current scroll bar position
	void setPositionRange(int newPositionMin,int newPositionMax,int newPageSize); // Changes the scroll bar position range
	Misc::CallbackList& getValueChangedCallbacks(void) // Returns list of value changed callbacks
		{
		return valueChangedCallbacks;
		}
	};

}

#endif
