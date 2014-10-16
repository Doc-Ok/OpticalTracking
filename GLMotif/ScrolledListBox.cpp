/***********************************************************************
ScrolledListBox - Compound widget containing a list box, a vertical, and
an optional horizontal scroll bar.
Copyright (c) 2008-2010 Oliver Kreylos

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

#include <GLMotif/ScrolledListBox.h>

#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLVertexTemplates.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/Event.h>

namespace GLMotif {

/********************************
Methods of class ScrolledListBox:
********************************/

void ScrolledListBox::listBoxPageChangedCallback(ListBox::PageChangedCallbackData* cbData)
	{
	/* Update the scroll bars: */
	if(cbData->reasonMask&(ListBox::PageChangedCallbackData::POSITION_CHANGED|ListBox::PageChangedCallbackData::NUMITEMS_CHANGED|ListBox::PageChangedCallbackData::PAGESIZE_CHANGED))
		{
		verticalScrollBar->setPositionRange(0,cbData->numItems,cbData->pageSize);
		verticalScrollBar->setPosition(cbData->position);
		}
	if(horizontalScrollBar!=0&&(cbData->reasonMask&(ListBox::PageChangedCallbackData::HORIZONTALOFFSET_CHANGED|ListBox::PageChangedCallbackData::MAXITEMWIDTH_CHANGED|ListBox::PageChangedCallbackData::LISTWIDTH_CHANGED)))
		{
		GLfloat fw=cbData->listBox->getFont()->getCharacterWidth();
		horizontalScrollBar->setPositionRange(0,int(cbData->maxItemWidth/fw+0.5f),int(cbData->listWidth/fw+0.5f));
		horizontalScrollBar->setPosition(int(cbData->horizontalOffset/fw+0.5f));
		}
	}

void ScrolledListBox::scrollBarCallback(ScrollBar::ValueChangedCallbackData* cbData)
	{
	if(cbData->scrollBar==verticalScrollBar)
		{
		/* Update the list box's position based on the new scroll bar position: */
		listBox->setPosition(cbData->position);
		}
	else if(cbData->scrollBar==horizontalScrollBar)
		{
		/* Update the list box's horizontal offset based on the new scroll bar position: */
		listBox->setHorizontalOffset(GLfloat(cbData->position)*listBox->getFont()->getCharacterWidth());
		}
	}

ScrolledListBox::ScrolledListBox(const char* sName,Container* sParent,ListBox::SelectionMode sSelectionMode,int sPreferredWidth,int sPreferredPageSize,bool sManageChild)
	:Container(sName,sParent,false),
	 listBox(new ListBox("ListBox",this,sSelectionMode,sPreferredWidth,sPreferredPageSize,false)),
	 verticalScrollBar(new ScrollBar("VerticalScrollBar",this,ScrollBar::VERTICAL,true,false)),
	 horizontalScrollBar(0)
	{
	/* Get the style sheet: */
	const StyleSheet* ss=getStyleSheet();
	
	/* Set the compound widget's borders: */
	setBorderWidth(ss->textfieldBorderWidth);
	setBorderType(Widget::LOWERED);
	
	/* Initialize the list box: */
	listBox->setBorderWidth(0.0f);
	listBox->setBorderType(Widget::PLAIN);
	listBox->getPageChangedCallbacks().add(this,&ScrolledListBox::listBoxPageChangedCallback);
	
	/* Initialize the vertical scroll bar: */
	verticalScrollBar->setPositionRange(0,0,listBox->getPageSize());
	verticalScrollBar->getValueChangedCallbacks().add(this,&ScrolledListBox::scrollBarCallback);
	
	/* Manage the children: */
	listBox->manageChild();
	verticalScrollBar->manageChild();
	
	/* Manage me: */
	if(sManageChild)
		manageChild();
	}

ScrolledListBox::~ScrolledListBox(void)
	{
	/* Delete the child widgets: */
	deleteChild(listBox);
	deleteChild(verticalScrollBar);
	deleteChild(horizontalScrollBar);
	}

Vector ScrolledListBox::calcNaturalSize(void) const
	{
	/* Calculate the list box's natural size: */
	Vector result=listBox->calcNaturalSize();
	
	/* Adjust for the vertical scroll bar: */
	Vector vBarSize=verticalScrollBar->calcNaturalSize();
	result[0]+=vBarSize[0];
	if(result[1]<vBarSize[1])
		result[1]=vBarSize[1];
	
	if(horizontalScrollBar!=0)
		{
		/* Adjust for the horizontal scroll bar: */
		Vector hBarSize=horizontalScrollBar->calcNaturalSize();
		if(result[0]<hBarSize[0]+vBarSize[0])
			result[0]=hBarSize[0]+vBarSize[0];
		result[1]+=hBarSize[1];
		}
	
	return calcExteriorSize(result);
	}

ZRange ScrolledListBox::calcZRange(void) const
	{
	/* Calculate the parent class widget's z range: */
	ZRange myZRange=Container::calcZRange();
	
	/* Adjust for the list box: */
	myZRange+=listBox->calcZRange();
	
	/* Adjust for the vertical scroll bar: */
	myZRange+=verticalScrollBar->calcZRange();
	
	if(horizontalScrollBar!=0)
		{
		/* Adjust for the horizontal scroll bar: */
		myZRange+=horizontalScrollBar->calcZRange();
		}
	
	return myZRange;
	}

void ScrolledListBox::resize(const Box& newExterior)
	{
	/* Resize the parent class widget: */
	Container::resize(newExterior);
	
	/* Get the new interior size: */
	Box lbBox=getInterior();
	
	/* Make room for and position the vertical scroll bar: */
	Vector vbSize=verticalScrollBar->calcNaturalSize();
	Box vbBox=lbBox;
	vbBox.origin[0]+=lbBox.size[0]-vbSize[0];
	vbBox.size[0]=vbSize[0];
	lbBox.size[0]-=vbSize[0];
	
	Box hbBox;
	if(horizontalScrollBar!=0)
		{
		/* Make room for and position the horizontal scroll bar: */
		Vector hbSize=horizontalScrollBar->calcNaturalSize();
		hbBox=lbBox;
		hbBox.size[1]=hbSize[1];
		lbBox.origin[1]+=hbSize[1];
		lbBox.size[1]-=hbSize[1];
		vbBox.origin[1]+=hbSize[1];
		vbBox.size[1]-=hbSize[1];
		}
	
	/* Resize the list box and scroll bars: */
	listBox->resize(lbBox);
	verticalScrollBar->resize(vbBox);
	if(horizontalScrollBar!=0)
		horizontalScrollBar->resize(hbBox);
	}

void ScrolledListBox::draw(GLContextData& contextData) const
	{
	/* Draw the parent class widget: */
	Container::draw(contextData);
	
	/* Draw the margin around the child widgets: */
	glColor(backgroundColor);
	
	/* Draw the top left margin part: */
	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(0.0f,0.0f,1.0f);
	glVertex(getInterior().getCorner(2));
	glVertex(getInterior().getCorner(0));
	if(horizontalScrollBar!=0)
		glVertex(horizontalScrollBar->getExterior().getCorner(0));
	glVertex(listBox->getExterior().getCorner(0));
	glVertex(listBox->getExterior().getCorner(2));
	glVertex(listBox->getExterior().getCorner(3));
	glVertex(verticalScrollBar->getExterior().getCorner(3));
	glVertex(getInterior().getCorner(3));
	glEnd();
	
	/* Draw the bottom right margin part: */
	glBegin(GL_TRIANGLE_FAN);
	glVertex(getInterior().getCorner(1));
	glVertex(getInterior().getCorner(3));
	glVertex(verticalScrollBar->getExterior().getCorner(3));
	glVertex(verticalScrollBar->getExterior().getCorner(1));
	glVertex(listBox->getExterior().getCorner(1));
	if(horizontalScrollBar!=0)
		{
		glVertex(horizontalScrollBar->getExterior().getCorner(1));
		glVertex(horizontalScrollBar->getExterior().getCorner(0));
		}
	else
		glVertex(listBox->getExterior().getCorner(0));
	glVertex(getInterior().getCorner(0));
	glEnd();
	
	/* Draw the list box and scroll bars: */
	listBox->draw(contextData);
	verticalScrollBar->draw(contextData);
	if(horizontalScrollBar!=0)
		horizontalScrollBar->draw(contextData);
	}

bool ScrolledListBox::findRecipient(Event& event)
	{
	/* Distribute the question to the child widgets: */
	bool childFound=false;
	if(!childFound)
		childFound=listBox->findRecipient(event);
	if(!childFound)
		childFound=verticalScrollBar->findRecipient(event);
	if(!childFound&&horizontalScrollBar!=0)
		childFound=horizontalScrollBar->findRecipient(event);
	
	/* If no child was found, return ourselves (and ignore any incoming events): */
	if(childFound)
		return true;
	else
		{
		/* Check ourselves: */
		Event::WidgetPoint wp=event.calcWidgetPoint(this);
		if(isInside(wp.getPoint()))
			return event.setTargetWidget(this,wp);
		else
			return false;
		}
	}

void ScrolledListBox::addChild(Widget* newChild)
	{
	/* Only the dedicated children can call this, and they get managed in the constructor, so ignore this */
	}

void ScrolledListBox::removeChild(Widget* removeChild)
	{
	/* This should never be called, so simply ignore it */
	}

void ScrolledListBox::requestResize(Widget* child,const Vector& newExteriorSize)
	{
	if(isManaged)
		{
		/* Gather the old and new sizes of the child widgets: */
		Vector lbSize=child==listBox?newExteriorSize:listBox->getExterior().size;
		Vector vbSize=child==verticalScrollBar?newExteriorSize:verticalScrollBar->getExterior().size;
		Vector hbSize(0.0f,0.0f,0.0f);
		if(horizontalScrollBar!=0)
			hbSize=child==horizontalScrollBar?newExteriorSize:horizontalScrollBar->getExterior().size;
		
		/* Assemble the new interior size: */
		if(lbSize[0]<hbSize[0])
			lbSize[0]=hbSize[0];
		if(lbSize[1]<vbSize[1])
			lbSize[1]=vbSize[1];
		lbSize[0]+=vbSize[0];
		lbSize[1]+=hbSize[1];
		
		/* Resize the widget: */
		parent->requestResize(this,calcExteriorSize(lbSize));
		}
	else
		{
		/* Just resize the child: */
		child->resize(Box(child->getExterior().origin,newExteriorSize));
		}
	}

Widget* ScrolledListBox::getFirstChild(void)
	{
	/* Return the list box: */
	return listBox;
	}

Widget* ScrolledListBox::getNextChild(Widget* child)
	{
	/* Return the next child: */
	if(child==listBox)
		return verticalScrollBar;
	else if(child==verticalScrollBar)
		return horizontalScrollBar;
	else
		return 0;
	}

void ScrolledListBox::showHorizontalScrollBar(bool enable)
	{
	if(enable&&horizontalScrollBar==0)
		{
		/* Create a horizontal scroll bar: */
		horizontalScrollBar=new ScrollBar("HorizontalScrollBar",this,ScrollBar::HORIZONTAL,false);
		horizontalScrollBar->getValueChangedCallbacks().add(this,&ScrolledListBox::scrollBarCallback);
		
		/* Add it to the widget's layout: */
		Vector lbSize=listBox->getExterior().size;
		Vector vbSize=verticalScrollBar->getExterior().size;
		Vector hbSize=horizontalScrollBar->calcNaturalSize();
		if(lbSize[0]<hbSize[0])
			lbSize[0]=hbSize[0];
		if(lbSize[1]<vbSize[1])
			lbSize[1]=vbSize[1];
		lbSize[0]+=vbSize[0];
		lbSize[1]+=hbSize[1];
		
		/* Resize the widget: */
		if(isManaged)
			parent->requestResize(this,calcExteriorSize(lbSize));
		else
			resize(Box(Vector(0.0f,0.0f,0.0f),calcExteriorSize(lbSize)));
		}
	else if(!enable&&horizontalScrollBar!=0)
		{
		/* Delete the horizontal scroll bar: */
		deleteChild(horizontalScrollBar);
		
		/* Remove it from the widget's layout: */
		Vector lbSize=listBox->getExterior().size;
		Vector vbSize=verticalScrollBar->getExterior().size;
		if(lbSize[1]<vbSize[1])
			lbSize[1]=vbSize[1];
		lbSize[0]+=vbSize[0];
		
		/* Resize the widget: */
		if(isManaged)
			parent->requestResize(this,calcExteriorSize(lbSize));
		else
			resize(Box(Vector(0.0f,0.0f,0.0f),calcExteriorSize(lbSize)));
		}
	}

}
