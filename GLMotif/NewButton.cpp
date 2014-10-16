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

#include <GLMotif/NewButton.h>

#include <GLMotif/StyleSheet.h>
#include <GLMotif/Event.h>
#include <GLMotif/Blind.h>
#include <GLMotif/Label.h>
#include <GLMotif/Container.h>

namespace GLMotif {

/**************************
Methods of class NewButton:
**************************/

void NewButton::setArmed(bool newArmed)
	{
	if(newArmed&&!isArmed)
		{
		/* Arm the button: */
		savedBorderType=getBorderType();
		savedBackgroundColor=backgroundColor;
		SingleChildContainer::setBorderType(savedBorderType!=Widget::LOWERED?Widget::LOWERED:Widget::RAISED); // Need to use base class method here
		SingleChildContainer::setBackgroundColor(armedBackgroundColor); // Need to use base class method here
		if(child!=0)
			child->setBackgroundColor(armedBackgroundColor);
		
		isArmed=true;
		}
	else if(!newArmed&&isArmed)
		{
		/* Disarm the button: */
		SingleChildContainer::setBorderType(savedBorderType); // Need to use base class method here
		SingleChildContainer::setBackgroundColor(savedBackgroundColor); // Need to use base class method here
		if(child!=0)
			child->setBackgroundColor(savedBackgroundColor);
		
		isArmed=false;
		}
	
	/* Call the arm callbacks: */
	ArmCallbackData cbData(this,isArmed);
	armCallbacks.call(&cbData);
	}

void NewButton::select(void)
	{
	/* Call the select callbacks: */
	SelectCallbackData cbData(this);
	selectCallbacks.call(&cbData);
	}

NewButton::NewButton(const char* sName,Container* sParent,bool sManageChild)
	:SingleChildContainer(sName,sParent,false),
	 isArmed(false)
	{
	/* Get the style sheet: */
	const StyleSheet* ss=getStyleSheet();
	
	/* Button defaults to raised border: */
	setBorderType(Widget::RAISED);
	setBorderWidth(ss->buttonBorderWidth);
	
	/* Set the armed background color: */
	armedBackgroundColor=ss->buttonArmedBackgroundColor;
	
	/* Manage me: */
	if(sManageChild)
		manageChild();
	}

NewButton::NewButton(const char* sName,Container* sParent,const Vector& preferredSize,bool sManageChild)
	:SingleChildContainer(sName,sParent,false),
	 isArmed(false)
	{
	/* Get the style sheet: */
	const StyleSheet* ss=getStyleSheet();
	
	/* Button defaults to raised border: */
	setBorderType(Widget::RAISED);
	setBorderWidth(ss->buttonBorderWidth);
	
	/* Set the armed background color: */
	armedBackgroundColor=ss->buttonArmedBackgroundColor;
	
	/* Create the blind child: */
	Blind* blind=new Blind("ButtonBlind",this);
	blind->setBorderType(Widget::PLAIN);
	blind->setBorderWidth(0.0f);
	blind->setPreferredSize(preferredSize);
	
	/* Manage me: */
	if(sManageChild)
		manageChild();
	}

NewButton::NewButton(const char* sName,Container* sParent,const char* sLabel,bool sManageChild)
	:SingleChildContainer(sName,sParent,false),
	 isArmed(false)
	{
	/* Get the style sheet: */
	const StyleSheet* ss=getStyleSheet();
	
	/* Button defaults to raised border: */
	setBorderType(Widget::RAISED);
	setBorderWidth(ss->buttonBorderWidth);
	
	/* Set the armed background color: */
	armedBackgroundColor=ss->buttonArmedBackgroundColor;
	
	/* Create the label child widget: */
	Label* label=new Label("ButtonLabel",this,sLabel);
	label->setBorderType(Widget::PLAIN);
	label->setBorderWidth(0.0f);
	
	/* Button defaults to some margin: */
	label->setMarginWidth(ss->buttonMarginWidth);
	
	/* Button defaults to centered text: */
	label->setHAlignment(GLFont::Center);
	
	/* Manage me: */
	if(sManageChild)
		manageChild();
	}

ZRange NewButton::calcZRange(void) const
	{
	/* Calculate the parent class widget's z range: */
	ZRange myZRange=SingleChildContainer::calcZRange();
	
	/* Adjust for the popping in/out when arming/disarming: */
	myZRange+=ZRange(getExterior().origin[2]-getBorderWidth(),getExterior().origin[2]+getBorderWidth());
	
	return myZRange;
	}
	
bool NewButton::findRecipient(Event& event)
	{
	/* Check ourselves: */
	Event::WidgetPoint wp=event.calcWidgetPoint(this);
	if(isInside(wp.getPoint()))
		return event.setTargetWidget(this,wp);
	else
		return false;
	}

void NewButton::pointerButtonDown(Event&)
	{
	/* Arm the button: */
	setArmed(true);
	}

void NewButton::pointerButtonUp(Event& event)
	{
	/* Select if the event is for us: */
	if(event.getTargetWidget()==this)
		select();
	
	/* Disarm the button: */
	setArmed(false);
	}

void NewButton::pointerMotion(Event& event)
	{
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

void NewButton::setBorderType(Widget::BorderType newBorderType)
	{
	if(isArmed)
		{
		/* Store the new border type to be set when the button is unarmed: */
		savedBorderType=newBorderType;
		}
	else
		{
		/* Set the new border type using the base class method: */
		SingleChildContainer::setBorderType(newBorderType);
		}
	}

void NewButton::setForegroundColor(const Color& newForegroundColor)
	{
	/* Call the base class method: */
	SingleChildContainer::setForegroundColor(newForegroundColor);
	
	/* Set the child's foreground color: */
	if(child!=0)
		child->setForegroundColor(newForegroundColor);
	}

void NewButton::setBackgroundColor(const Color& newBackgroundColor)
	{
	if(isArmed)
		{
		/* Store the new background color to be set when the button is unarmed: */
		savedBackgroundColor=newBackgroundColor;
		}
	else
		{
		/* Set the new background color using the base class method: */
		SingleChildContainer::setBackgroundColor(newBackgroundColor);
		if(child!=0)
			child->setBackgroundColor(newBackgroundColor);
		}
	}

void NewButton::setArmedBackgroundColor(const Color& newArmedBackgroundColor)
	{
	/* Store the new armed color: */
	armedBackgroundColor=newArmedBackgroundColor;
	
	if(isArmed)
		{
		/* Set the new armed background color right away: */
		SingleChildContainer::setBackgroundColor(armedBackgroundColor);
		if(child!=0)
			child->setBackgroundColor(armedBackgroundColor);
		}
	}

}
