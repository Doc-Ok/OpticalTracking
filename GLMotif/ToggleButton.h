/***********************************************************************
ToggleButton - Class for buttons displaying a binary (on/off) status.
Copyright (c) 2001-2010 Oliver Kreylos

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

#ifndef GLMOTIF_TOGGLEBUTTON_INCLUDED
#define GLMOTIF_TOGGLEBUTTON_INCLUDED

#include <Misc/CallbackData.h>
#include <Misc/CallbackList.h>
#include <GLMotif/DecoratedButton.h>

namespace GLMotif {

class ToggleButton:public DecoratedButton
	{
	/* Embedded classes: */
	public:
	enum ToggleType // Enumerated type for different toggle button types
		{
		TOGGLE_BUTTON,RADIO_BUTTON
		};
	
	class ValueChangedCallbackData:public Misc::CallbackData
		{
		/* Elements: */
		public:
		ToggleButton* toggle; // Pointer to the toggle button widget causing the event
		bool set; // Current toggle value
		
		/* Constructors and destructors: */
		ValueChangedCallbackData(ToggleButton* sToggle,bool sSet)
			:toggle(sToggle),set(sSet)
			{
			}
		};
	
	/* Elements: */
	protected:
	ToggleType toggleType; // Type of toggle button
	GLfloat toggleBorderWidth; // Size of border around toggle
	GLfloat toggleWidth; // Size of toggle
	Color toggleColor; // Color to draw the toggle decoration when set
	Vector toggleOuter[4]; // Four outer toggle points
	Vector toggleInner[4]; // Four inner toggle points
	bool set; // Flag if the toggle is set
	Misc::CallbackList valueChangedCallbacks; // List of callbacks to be called when the button changes state due to a user interaction
	
	/* Protected methods inherited from Button: */
	virtual void setArmed(bool newArmed);
	virtual void select(void);
	
	/* Protected methods inherited from DecoratedButton: */
	virtual void drawDecoration(GLContextData& contextData) const;
	
	/* New protected methods: */
	void positionToggle(void);
	void setSet(bool newSet);
	
	/* Constructors and destructors: */
	public:
	ToggleButton(const char* sName,Container* sParent,const char* sLabel,const GLFont* sFont,bool manageChild =true); // Deprecated
	ToggleButton(const char* sName,Container* sParent,const char* sLabel,bool manageChild =true);
	
	/* Methods inherited from Widget: */
	virtual ZRange calcZRange(void) const;
	virtual void resize(const Box& newExterior);
	
	/* New methods: */
	ToggleType getToggleType(void) const
		{
		return toggleType;
		}
	bool getToggle(void) const
		{
		return set;
		}
	void setToggleType(ToggleType newToggleType);
	void setToggleBorderWidth(GLfloat newToggleBorderWidth);
	void setToggleWidth(GLfloat newToggleBorderWidth);
	void setToggleColor(const Color& newToggleColor)
		{
		toggleColor=newToggleColor;
		}
	void setToggle(bool newSet)
		{
		setSet(newSet);
		}
	Misc::CallbackList& getValueChangedCallbacks(void) // Returns list of callbacks called when the toggle button's state changes
		{
		return valueChangedCallbacks;
		}
	};

}

#endif
