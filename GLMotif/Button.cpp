/***********************************************************************
Button - Base class for GLMotif UI components reacting to push events.
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

#include <GLMotif/Button.h>

#include <GLMotif/StyleSheet.h>
#include <GLMotif/Event.h>
#include <GLMotif/Container.h>

namespace GLMotif {

/***********************
Methods of class Button:
***********************/

void Button::setArmed(bool newArmed)
	{
	if(newArmed&&!isArmed)
		{
		/* Arm the button: */
		savedBorderType=getBorderType();
		savedBackgroundColor=backgroundColor;
		Label::setBorderType(savedBorderType!=Widget::LOWERED?Widget::LOWERED:Widget::RAISED); // Need to use base class method here
		Label::setBackgroundColor(armedBackgroundColor); // Need to use base class method here
		isArmed=true;
		}
	else if(!newArmed&&isArmed)
		{
		/* Disarm the button: */
		Label::setBorderType(savedBorderType); // Need to use base class method here
		Label::setBackgroundColor(savedBackgroundColor); // Need to use base class method here
		isArmed=false;
		}
	
	/* Call the arm callbacks: */
	ArmCallbackData cbData(this,isArmed);
	armCallbacks.call(&cbData);
	}

void Button::select(void)
	{
	/* Call the select callbacks: */
	SelectCallbackData cbData(this);
	selectCallbacks.call(&cbData);
	}

Button::Button(const char* sName,Container* sParent,const char* sLabel,const GLFont* sFont,bool sManageChild)
	:Label(sName,sParent,sLabel,sFont,false),
	 isArmed(false)
	{
	/* Get the style sheet: */
	const StyleSheet* ss=getStyleSheet();
	
	/* Button defaults to raised border: */
	setBorderType(Widget::RAISED);
	setBorderWidth(ss->buttonBorderWidth);
	
	/* Button defaults to some margin: */
	setMarginWidth(ss->buttonMarginWidth);
	
	/* Button defaults to centered text: */
	setHAlignment(GLFont::Center);
	
	/* Set the armed background color: */
	armedBackgroundColor=ss->buttonArmedBackgroundColor;
	
	/* Manage me: */
	if(sManageChild)
		manageChild();
	}

Button::Button(const char* sName,Container* sParent,const char* sLabel,bool sManageChild)
	:Label(sName,sParent,sLabel,false),
	 isArmed(false)
	{
	/* Get the style sheet: */
	const StyleSheet* ss=getStyleSheet();
	
	/* Button defaults to raised border: */
	setBorderType(Widget::RAISED);
	setBorderWidth(ss->buttonBorderWidth);
	
	/* Button defaults to some margin: */
	setMarginWidth(ss->buttonMarginWidth);
	
	/* Button defaults to centered text: */
	setHAlignment(GLFont::Center);
	
	/* Set the armed background color: */
	armedBackgroundColor=ss->buttonArmedBackgroundColor;
	
	/* Manage me: */
	if(sManageChild)
		manageChild();
	}

ZRange Button::calcZRange(void) const
	{
	/* Calculate the parent class widget's z range: */
	ZRange myZRange=Label::calcZRange();
	
	/* Adjust for the popping in/out when arming/disarming: */
	myZRange+=ZRange(getExterior().origin[2]-getBorderWidth(),getExterior().origin[2]+getBorderWidth());
	
	return myZRange;
	}
	
void Button::pointerButtonDown(Event&)
	{
	/* Ignore the event if the button is disabled: */
	if(!isEnabled())
		return;
	
	/* Arm the button: */
	setArmed(true);
	}

void Button::pointerButtonUp(Event& event)
	{
	/* Ignore the event if the button is disabled: */
	if(!isEnabled())
		return;
	
	/* Select if the event is for us: */
	if(event.getTargetWidget()==this)
		select();
	
	/* Disarm the button: */
	setArmed(false);
	}

void Button::pointerMotion(Event& event)
	{
	/* Ignore the event if the button is disabled: */
	if(!isEnabled())
		return;
	
	if(!event.isPressed())
		return;
	
	/* Check if the new pointer position is still inside the button: */
	if(event.getTargetWidget()==this)
		{
		/* Arm the button: */
		setArmed(true);
		}
	else
		{
		/* Disarm the button: */
		setArmed(false);
		}
	}

void Button::setBorderType(Widget::BorderType newBorderType)
	{
	if(isArmed)
		{
		/* Store the new border type to be set when the button is unarmed: */
		savedBorderType=newBorderType;
		}
	else
		{
		/* Set the new border type using the base class method: */
		Label::setBorderType(newBorderType);
		}
	}

void Button::setBackgroundColor(const Color& newBackgroundColor)
	{
	if(isArmed)
		{
		/* Store the new background color to be set when the button is unarmed: */
		savedBackgroundColor=newBackgroundColor;
		}
	else
		{
		/* Set the new background color using the base class method: */
		Label::setBackgroundColor(newBackgroundColor);
		}
	}

void Button::setArmedBackgroundColor(const Color& newArmedBackgroundColor)
	{
	/* Store the new armed color: */
	armedBackgroundColor=newArmedBackgroundColor;
	
	if(isArmed)
		{
		/* Set the new armed background color right away: */
		Label::setBackgroundColor(armedBackgroundColor);
		}
	}

}
