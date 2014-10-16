/***********************************************************************
NewButton - New base class for GLMotif UI components reacting to push
events, implemented as container widgets with a single child.
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

#ifndef GLMOTIF_NEWBUTTON_INCLUDED
#define GLMOTIF_NEWBUTTON_INCLUDED

#include <Misc/CallbackData.h>
#include <Misc/CallbackList.h>
#include <GLMotif/SingleChildContainer.h>

namespace GLMotif {

class NewButton:public SingleChildContainer
	{
	/* Embedded classes: */
	public:
	class CallbackData:public Misc::CallbackData // Base class for button events
		{
		/* Elements: */
		public:
		NewButton* button; // Pointer to the button widget that caused the event
		
		/* Constructors and destructors: */
		CallbackData(NewButton* sButton)
			:button(sButton)
			{
			}
		};
	
	class ArmCallbackData:public CallbackData // Class for for callback data sent when a button changes arm state
		{
		/* Elements: */
		public:
		bool isArmed; // New arm state of the button
		
		/* Constructors and destructors: */
		ArmCallbackData(NewButton* sButton,bool sIsArmed)
			:CallbackData(sButton),isArmed(sIsArmed)
			{
			}
		};
	
	class SelectCallbackData:public CallbackData // Class for callback data sent when a button is selected
		{
		/* Constructors and destructors: */
		public:
		SelectCallbackData(NewButton* sButton)
			:CallbackData(sButton)
			{
			}
		};
	
	/* Elements: */
	protected:
	Color armedBackgroundColor; // Background color while the button is armed
	bool isArmed; // Flag if the button is "armed," i.e., is touched by a selecting pointing device
	BorderType savedBorderType; // The button's border type right before the button was armed
	Color savedBackgroundColor; // Background color right before the button was armed
	Misc::CallbackList armCallbacks; // List of callbacks to be called when the button is armed/disarmed
	Misc::CallbackList selectCallbacks; // List of callbacks to be called when the button is selected
	
	/* Protected methods: */
	virtual void setArmed(bool newArmed); // Changes the "armed" state of the button
	virtual void select(void); // Is called when the button is selected
	
	/* Constructors and destructors: */
	public:
	NewButton(const char* sName,Container* sParent,bool manageChild =true); // Creates a button without a child; interior is rendered as solid color
	NewButton(const char* sName,Container* sParent,const Vector& preferredSize,bool manageChild =true); // Creates a button with a blind as a child
	NewButton(const char* sName,Container* sParent,const char* sLabel,bool manageChild =true); // Creates a button with a label as a child
	
	/* Methods inherited from Widget: */
	virtual std::pair<GLfloat,GLfloat> calcZRange(void) const;
	virtual bool findRecipient(Event& event);
	virtual void pointerButtonDown(Event& event);
	virtual void pointerButtonUp(Event& event);
	virtual void pointerMotion(Event& event);
	virtual void setBorderType(BorderType newBorderType);
	virtual void setForegroundColor(const Color& newForegroundColor);
	virtual void setBackgroundColor(const Color& newBackgroundColor);
	
	/* New methods: */
	const Color& getArmedBackgroundColor(void) const // Returns the background color used when the button is armed
		{
		return armedBackgroundColor;
		}
	void setArmedBackgroundColor(const Color& newArmedBackgroundColor); // Sets the background color to use when the button is armed
	Misc::CallbackList& getArmCallbacks(void) // Returns the list of arm callbacks
		{
		return armCallbacks;
		}
	Misc::CallbackList& getSelectCallbacks(void) // Returns the list of select callbacks
		{
		return selectCallbacks;
		}
	};

}

#endif
