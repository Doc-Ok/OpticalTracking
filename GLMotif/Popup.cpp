/***********************************************************************
Popup - Class for top-level GLMotif UI components.
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

#include <GLMotif/Popup.h>

#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLVertexTemplates.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/Event.h>
#include <GLMotif/WidgetManager.h>
#include <GLMotif/Label.h>

namespace GLMotif {

/**********************
Methods of class Popup:
**********************/

Popup::Popup(const char* sName,WidgetManager* sManager)
	:Container(sName,0,false),manager(sManager),
	 title(0),child(0)
	{
	/* Get the style sheet: */
	const StyleSheet* ss=manager->getStyleSheet();
	
	/* Set the popup window's default layout: */
	marginWidth=ss->popupMarginWidth;
	titleSpacing=ss->popupTitleSpacing;
	
	setBorderWidth(0.0f);
	setBorderType(Widget::PLAIN);
	setBorderColor(ss->borderColor);
	setBackgroundColor(ss->bgColor);
	setForegroundColor(ss->fgColor);
	}

Popup::~Popup(void)
	{
	/* Pop down the widget: */
	manager->popdownWidget(this);
	
	/* Unmanage and delete the title bar: */
	deleteChild(title);
	
	/* Unmanage and delete the child widget: */
	deleteChild(child);
	}

Vector Popup::calcNaturalSize(void) const
	{
	Vector result(2.0f*marginWidth,2.0f*marginWidth,0.0f);
	GLfloat maxWidth=0.0;
	
	if(title!=0)
		{
		/* Calculate the label's natural size: */
		Vector titleSize=title->calcNaturalSize();
		if(maxWidth<titleSize[0])
			maxWidth=titleSize[0];
		result[1]+=titleSize[1]+titleSpacing;
		}
	
	if(child!=0)
		{
		/* Calculate the first child's natural size: */
		Vector childSize=child->calcNaturalSize();
		if(maxWidth<childSize[0])
			maxWidth=childSize[0];
		result[1]+=childSize[1];
		}
	result[0]+=maxWidth;
	
	return calcExteriorSize(result);
	}

ZRange Popup::calcZRange(void) const
	{
	/* Calculate the parent class widget's z range: */
	ZRange myZRange=Container::calcZRange();
	
	/* Check the title widget: */
	if(title!=0)
		myZRange+=title->calcZRange();
	
	/* Check the child widget: */
	if(child!=0)
		myZRange+=child->calcZRange();
	
	/* Adjust the minimum z value to accomodate the popup's back side: */
	myZRange.first-=getBorderWidth();
	
	return myZRange;
	}

void Popup::resize(const Box& newExterior)
	{
	#if 0
	/* Center the widget around the origin: */
	Box exterior=newExterior;
	exterior.origin[0]-=0.5f*exterior.size[0];
	exterior.origin[1]-=0.5f*exterior.size[1];
	#endif
	
	/* Resize the parent class widget: */
	Container::resize(newExterior);
	
	/* Resize the title: */
	GLfloat titleHeight=0.0f;
	if(title!=0)
		{
		Box titleRect=getInterior();
		titleRect.origin[0]+=marginWidth;
		titleRect.size[0]-=2.0f*marginWidth;
		titleHeight=title->calcNaturalSize()[1];
		titleRect.origin[1]+=titleRect.size[1]-marginWidth-titleHeight;
		titleRect.size[1]=titleHeight;
		title->resize(titleRect);
		
		/* Account for spacing between title and child: */
		titleHeight+=titleSpacing;
		}
	
	/* Resize the child: */
	if(child!=0)
		{
		Box childRect=getInterior();
		childRect.origin[0]+=marginWidth;
		childRect.size[0]-=2.0f*marginWidth;
		childRect.origin[1]+=marginWidth;
		childRect.size[1]-=2.0f*marginWidth+titleHeight;
		child->resize(childRect);
		}
	
	/* Resize the parent class widget again to calculate the correct z range: */
	Container::resize(newExterior);
	}

void Popup::draw(GLContextData& contextData) const
	{
	/* Draw the parent class widget: */
	Container::draw(contextData);
	
	/* Draw the widget's back side: */
	Box back=getExterior().offset(Vector(0.0,0.0,getZRange().first));
	glColor(borderColor);
	glBegin(GL_QUADS);
	glNormal3f(0.0f,0.0f,-1.0f);
	glVertex(back.getCorner(0));
	glVertex(back.getCorner(2));
	glVertex(back.getCorner(3));
	glVertex(back.getCorner(1));
	glNormal3f(0.0f,-1.0f,0.0f);
	glVertex(back.getCorner(0));
	glVertex(back.getCorner(1));
	glVertex(getExterior().getCorner(1));
	glVertex(getExterior().getCorner(0));
	glNormal3f(1.0f,0.0f,0.0f);
	glVertex(back.getCorner(1));
	glVertex(back.getCorner(3));
	glVertex(getExterior().getCorner(3));
	glVertex(getExterior().getCorner(1));
	glNormal3f(0.0f,1.0f,0.0f);
	glVertex(back.getCorner(3));
	glVertex(back.getCorner(2));
	glVertex(getExterior().getCorner(2));
	glVertex(getExterior().getCorner(3));
	glNormal3f(-1.0f,0.0f,0.0f);
	glVertex(back.getCorner(2));
	glVertex(back.getCorner(0));
	glVertex(getExterior().getCorner(0));
	glVertex(getExterior().getCorner(2));
	glEnd();
	
	/* Draw the margin and title separator: */
	glColor(backgroundColor);
	
	/* Draw the top left margin part: */
	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(0.0f,0.0f,1.0f);
	glVertex(getInterior().getCorner(2));
	glVertex(getInterior().getCorner(0));
	if(child!=0)
		{
		glVertex(child->getExterior().getCorner(0));
		glVertex(child->getExterior().getCorner(2));
		}
	if(title!=0)
		{
		glVertex(title->getExterior().getCorner(0));
		glVertex(title->getExterior().getCorner(2));
		glVertex(title->getExterior().getCorner(3));
		}
	else if(child!=0)
		glVertex(child->getExterior().getCorner(3));
	glVertex(getInterior().getCorner(3));
	glEnd();
	
	/* Draw the bottom right margin part: */
	glBegin(GL_TRIANGLE_FAN);
	glVertex(getInterior().getCorner(1));
	glVertex(getInterior().getCorner(3));
	if(title!=0)
		{
		glVertex(title->getExterior().getCorner(3));
		glVertex(title->getExterior().getCorner(1));
		}
	if(child!=0)
		{
		glVertex(child->getExterior().getCorner(3));
		glVertex(child->getExterior().getCorner(1));
		glVertex(child->getExterior().getCorner(0));
		}
	else if(title!=0)
		glVertex(title->getExterior().getCorner(0));
	glVertex(getInterior().getCorner(0));
	glEnd();
	
	/* Draw the title separator: */
	if(title!=0&&child!=0)
		{
		glBegin(GL_QUADS);
		glVertex(child->getExterior().getCorner(2));
		glVertex(child->getExterior().getCorner(3));
		glVertex(title->getExterior().getCorner(1));
		glVertex(title->getExterior().getCorner(0));
		glEnd();
		}
	
	/* Draw the title and child: */
	if(title!=0)
		title->draw(contextData);
	if(child!=0)
		child->draw(contextData);
	}

bool Popup::findRecipient(Event& event)
	{
	/* Distribute the question to the child widget: */
	if(child==0||!child->findRecipient(event))
		{
		/* Check ourselves: */
		Event::WidgetPoint wp=event.calcWidgetPoint(this);
		if(isInside(wp.getPoint()))
			return event.setTargetWidget(this,wp);
		else
			return false;
		}
	else
		return true;
	}

void Popup::addChild(Widget* newChild)
	{
	if(newChild!=title)
		{
		/* Delete the current child: */
		deleteChild(child);
		
		/* Add the new child: */
		child=newChild;
		
		/* Resize the widget: */
		resize(Box(Vector(0.0f,0.0f,0.0f),calcNaturalSize()));
		}
	}

void Popup::removeChild(Widget* removeChild)
	{
	/* Check if the given widget is really the child: */
	if(child!=0&&child==removeChild)
		{
		/* Tell the child that it is being removed: */
		child->unmanageChild();
		
		/* Remove the child: */
		child=0;
		
		/* Resize the widget: */
		resize(Box(Vector(0.0f,0.0f,0.0f),calcNaturalSize()));
		}
	}

void Popup::requestResize(Widget* child,const Vector&)
	{
	/* Calculate the new exterior size: */
	Vector size(2.0f*marginWidth,2.0f*marginWidth,0.0f);
	GLfloat maxWidth=0.0;
	
	if(title!=0)
		{
		/* Calculate the label's natural size: */
		Vector titleSize=title->calcNaturalSize();
		if(maxWidth<titleSize[0])
			maxWidth=titleSize[0];
		size[1]+=titleSize[1]+titleSpacing;
		}
	
	if(child!=0)
		{
		/* Calculate the first child's natural size: */
		Vector childSize=child->calcNaturalSize();
		if(maxWidth<childSize[0])
			maxWidth=childSize[0];
		size[1]+=childSize[1];
		}
	size[0]+=maxWidth;
	size=calcExteriorSize(size);
	
	/* Resize the widget: */
	resize(Box(Vector(0.0f,0.0f,0.0f),size));
	}

Widget* Popup::getFirstChild(void)
	{
	/* Return the only child: */
	return child;
	}

Widget* Popup::getNextChild(Widget*)
	{
	/* Since there is only one child, always return null: */
	return 0;
	}

void Popup::setMarginWidth(GLfloat newMarginWidth)
	{
	/* Change the margin width: */
	marginWidth=newMarginWidth;
	
	/* Resize the widget: */
	resize(Box(Vector(0.0f,0.0f,0.0f),calcNaturalSize()));
	}

void Popup::setTitleSpacing(GLfloat newTitleSpacing)
	{
	/* Change the title spacing: */
	titleSpacing=newTitleSpacing;
	
	/* Resize the widget: */
	resize(Box(Vector(0.0f,0.0f,0.0f),calcNaturalSize()));
	}

void Popup::setTitle(const char* titleString,const GLFont* font)
	{
	/* Delete the current title: */
	deleteChild(title);
	
	if(titleString!=0&&font!=0)
		{
		/* Create a new title widget: */
		title=new Label("_Title",this,titleString,font,false);
		title->setHAlignment(GLFont::Center);
		title->manageChild();
		}
	else
		title=0;
	
	/* Resize the widget: */
	resize(Box(Vector(0.0f,0.0f,0.0f),calcNaturalSize()));
	}

void Popup::setTitle(const char* titleString)
	{
	/* Delete the current title: */
	deleteChild(title);
	
	if(titleString!=0)
		{
		/* Create a new title widget: */
		title=new Label("_Title",this,titleString,false);
		title->setHAlignment(GLFont::Center);
		title->manageChild();
		}
	else
		title=0;
	
	/* Resize the widget: */
	resize(Box(Vector(0.0f,0.0f,0.0f),calcNaturalSize()));
	}

}
