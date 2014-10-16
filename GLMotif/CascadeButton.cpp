/***********************************************************************
CascadeButton - Class for buttons that pop up secondary top-level
GLMotif UI components.
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

#include <GLMotif/CascadeButton.h>

#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLNormalTemplates.h>
#include <GL/GLVertexTemplates.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/Event.h>
#include <GLMotif/WidgetManager.h>
#include <GLMotif/Container.h>
#include <GLMotif/Popup.h>

#define GLMOTIF_CASCADEBUTTON_CENTER_POPUPS 1

namespace GLMotif {

/******************************
Methods of class CascadeButton:
******************************/

void CascadeButton::setArmed(bool newArmed)
	{
	/* Call the base class widget's setArmed method: */
	DecoratedButton::setArmed(newArmed);
	
	/* Pop the secondary top level widget up or down: */
	if(isArmed&&!isPopped&&popup!=0)
		{
		/* Calculate the popup's transformation: */
		Vector offset=getExterior().getCorner(1);
		#if GLMOTIF_CASCADEBUTTON_CENTER_POPUPS
		offset[1]+=getExterior().size[1]*0.5f;
		#else
		offset[1]+=getExterior().size[1];
		#endif
		Vector popupHotSpot=popup->getChild()->getExterior().getCorner(0);
		#if GLMOTIF_CASCADEBUTTON_CENTER_POPUPS
		popupHotSpot[1]+=popup->getChild()->getExterior().size[1]*0.5f;
		#else
		popupHotSpot[1]+=popup->getChild()->getExterior().size[1];
		#endif
		for(int i=0;i<3;++i)
			offset[i]-=popupHotSpot[i];
		offset[2]+=getZRange().second-popup->getChild()->getZRange().first;
		getManager()->popupSecondaryWidget(this,popup,offset);
		isPopped=true;
		
		/* Calculate the extended "hit box" around the popup: */
		popupHitBox=popup->getExterior();
		ZRange popupZRange=popup->getZRange();
		popupHitBox.origin[2]=popupZRange.first;
		popupHitBox.size[2]=popupZRange.second-popupZRange.first;
		popupHitBox.doOffset(offset);
		popupHitBox.doOutset(Vector(popupExtrudeSize,popupExtrudeSize,popupExtrudeSize));
		}
	else if(!isArmed&&isPopped)
		{
		popup->getManager()->popdownWidget(popup);
		isPopped=false;
		}
	}

void CascadeButton::drawDecoration(GLContextData& contextData) const
	{
	/* Draw the cascade button arrow: */
	glColor(backgroundColor);
	arrow.draw(contextData);
	}

CascadeButton::CascadeButton(const char* sName,Container* sParent,const char* sLabel,const GLFont* sFont,bool sManageChild)
	:DecoratedButton(sName,sParent,sLabel,sFont,false),
	 popup(0),isPopped(false),
	 foundChild(0),armedChild(0),
	 arrow(GlyphGadget::FANCY_ARROW_RIGHT,GlyphGadget::IN,0.0f)
	{
	/* Get the style sheet: */
	const StyleSheet* ss=getStyleSheet();
	
	/* Set the arrow sizes: */
	arrow.setGlyphSize(ss->size*0.25f);
	arrow.setBevelSize(ss->size*0.25f);
	arrow.setGlyphColor(backgroundColor);
	popupExtrudeSize=ss->size*4.0f;
	
	/* Set the decoration position and size: */
	setDecorationPosition(DecoratedButton::DECORATION_RIGHT);
	GLfloat width=arrow.getPreferredBoxSize();
	setDecorationSize(Vector(width,width,0.0f));
	
	/* Manage me: */
	if(sManageChild)
		manageChild();
	}

CascadeButton::CascadeButton(const char* sName,Container* sParent,const char* sLabel,bool sManageChild)
	:DecoratedButton(sName,sParent,sLabel,false),
	 popup(0),isPopped(false),
	 foundChild(0),armedChild(0),
	 arrow(GlyphGadget::FANCY_ARROW_RIGHT,GlyphGadget::IN,0.0f)
	{
	/* Get the style sheet: */
	const StyleSheet* ss=getStyleSheet();
	
	/* Set the arrow sizes: */
	arrow.setGlyphSize(ss->size*0.25f);
	arrow.setBevelSize(ss->size*0.25f);
	arrow.setGlyphColor(backgroundColor);
	popupExtrudeSize=ss->size*4.0f;
	
	/* Set the decoration position and size: */
	setDecorationPosition(DecoratedButton::DECORATION_RIGHT);
	GLfloat width=arrow.getPreferredBoxSize();
	setDecorationSize(Vector(width,width,0.0f));
	
	/* Manage me: */
	if(sManageChild)
		manageChild();
	}

CascadeButton::~CascadeButton(void)
	{
	delete popup;
	}

ZRange CascadeButton::calcZRange(void) const
	{
	/* Calculate the parent class widget's z range: */
	ZRange myZRange=DecoratedButton::calcZRange();
	
	/* Adjust for the cascade arrow: */
	myZRange+=arrow.calcZRange();
	
	return myZRange;
	}
	
void CascadeButton::resize(const Box& newExterior)
	{
	/* Resize the parent class widget: */
	DecoratedButton::resize(newExterior);
	
	/* Position the cascade arrow: */
	arrow.setGlyphBox(decorationBox);
	}

void CascadeButton::setBackgroundColor(const Color& newBackgroundColor)
	{
	/* Call the base class method: */
	DecoratedButton::setBackgroundColor(newBackgroundColor);
	
	/* Let the arrow glyph track the background color: */
	arrow.setGlyphColor(newBackgroundColor);
	}

bool CascadeButton::findRecipient(Event& event)
	{
	bool result=false;
	
	/* Find the event's point in our coordinate system: */
	Event::WidgetPoint wp=event.calcWidgetPoint(this);
	
	/* If the point is inside our bounding box, put us down as recipient: */
	if(isInside(wp.getPoint()))
		result=event.setTargetWidget(this,wp);
	
	/* If the popup is popped up, redirect the question: */
	foundChild=0;
	if(isPopped)
		{
		bool popupResult=popup->findRecipient(event);
		if(popupResult)
			{
			foundChild=event.getTargetWidget();
			event.overrideTargetWidget(this);
			result=true;
			}
		else if(popupHitBox.isInside(Vector(wp.getPoint().getComponents())))
			result|=event.setTargetWidget(this,wp);
		}
	
	return result;
	}

void CascadeButton::pointerButtonDown(Event& event)
	{
	/* Ignore the event if the button is disabled: */
	if(!isEnabled())
		return;
	
	/* "Repair" the incoming event: */
	event.overrideTargetWidget(foundChild);
	
	/* Arm the button: */
	setArmed(true);
	
	/* Find a potential event recipient in the popup: */
	if(popup->findRecipient(event))
		{
		armedChild=event.getTargetWidget();
		armedChild->pointerButtonDown(event);
		}
	else
		armedChild=0;
	}

void CascadeButton::pointerButtonUp(Event& event)
	{
	/* Ignore the event if the button is disabled: */
	if(!isEnabled())
		return;
	
	/* "Repair" the incoming event: */
	event.overrideTargetWidget(foundChild);
	
	/* Disarm the armed child: */
	if(armedChild!=0)
		{
		armedChild->pointerButtonUp(event);
		armedChild=0;
		}
	
	setArmed(false);
	}

void CascadeButton::pointerMotion(Event& event)
	{
	/* Ignore the event if the button is disabled: */
	if(!isEnabled())
		return;
	
	/* "Repair" the incoming event: */
	event.overrideTargetWidget(foundChild);
	
	/* Arm/disarm children as we go by sending fake button events: */
	if(event.isPressed()&&foundChild!=armedChild)
		{
		if(armedChild!=0)
			armedChild->pointerButtonUp(event);
		armedChild=foundChild;
		if(armedChild!=0)
			armedChild->pointerButtonDown(event);
		}
	else if(armedChild!=0)
		armedChild->pointerMotion(event);
	}

void CascadeButton::setPopup(Popup* newPopup)
	{
	if(isPopped)
		{
		popup->getManager()->popdownWidget(popup);
		isPopped=false;
		}
	delete popup;
	popup=newPopup;
	}

void CascadeButton::setArrowBorderSize(GLfloat newArrowBorderSize)
	{
	/* Adjust the arrow glyph: */
	arrow.setBevelSize(newArrowBorderSize);
	
	/* Set the decoration width: */
	GLfloat width=arrow.getPreferredBoxSize();
	setDecorationSize(Vector(width,width,0.0f));
	}

void CascadeButton::setArrowSize(GLfloat newArrowSize)
	{
	/* Adjust the arrow glyph: */
	arrow.setGlyphSize(newArrowSize);
	
	/* Set the decoration width: */
	GLfloat width=arrow.getPreferredBoxSize();
	setDecorationSize(Vector(width,width,0.0f));
	}

void CascadeButton::setPopupExtrudeSize(GLfloat newPopupExtrudeSize)
	{
	popupExtrudeSize=newPopupExtrudeSize;
	}

}
