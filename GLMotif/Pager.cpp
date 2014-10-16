/***********************************************************************
Pager - Container class to arrange children as individual pages in a
"flipbook" of sorts.
Copyright (c) 2013-2014 Oliver Kreylos

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

#include <GLMotif/Pager.h>

#include <stdio.h>
#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLVertexTemplates.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/Event.h>

namespace GLMotif {

/**********************
Methods of class Pager:
**********************/

void Pager::pageButtonSelectCallback(Button::SelectCallbackData* cbData)
	{
	/* Find the index of the selected page button: */
	GLint pageButtonIndex;
	for(pageButtonIndex=0;pageButtonIndex<GLint(pageButtons.size())&&pageButtons[pageButtonIndex]!=cbData->button;++pageButtonIndex)
		;
	
	/* Select the button's page: */
	setCurrentChildIndex(pageButtonIndex);
	
	/* Call the page changed callbacks: */
	PageChangedCallbackData myCbData(this,currentChildIndex);
	pageChangedCallbacks.call(&myCbData);
	}

Pager::Pager(const char* sName,Container* sParent,bool sManageChild)
	:Container(sName,sParent,false),
	 nextChildIndex(0),nextPageIndex(0),addingPageButton(false),
	 currentChildIndex(0)
	{
	/* Get the style sheet: */
	const StyleSheet* ss=getStyleSheet();
	
	/* Set the child widget margin: */
	marginWidth=ss->containerMarginWidth;
	
	/* Manage this widget if requested: */
	if(sManageChild)
		manageChild();
	}

Pager::~Pager(void)
	{
	/* Delete the child widgets: */
	for(WidgetList::iterator cIt=children.begin();cIt!=children.end();++cIt)
		deleteChild(*cIt);
	
	/* Delete the page selection buttons: */
	for(ButtonList::iterator pbIt=pageButtons.begin();pbIt!=pageButtons.end();++pbIt)
		deleteChild(*pbIt);
	}

Vector Pager::calcNaturalSize(void) const
	{
	/* Calculate the size of the page button box: */
	Vector buttonSize(0.0f,0.0f,0.0f);
	for(ButtonList::const_iterator pbIt=pageButtons.begin();pbIt!=pageButtons.end();++pbIt)
		{
		/* Get the button's size: */
		Vector s=(*pbIt)->calcNaturalSize();
		
		/* Increment the box width: */
		buttonSize[0]+=s[0];
		
		/* Adjust the box height: */
		if(buttonSize[1]<s[1])
			buttonSize[1]=s[1];
		}
	
	/* Calculate the size of the child widget area: */
	Vector childSize(0.0f,0.0f,0.0f);
	for(WidgetList::const_iterator cIt=children.begin();cIt!=children.end();++cIt)
		{
		/* Get the child's size: */
		Vector s=(*cIt)->calcNaturalSize();
		
		/* Adjust the box width and height: */
		for(int i=0;i<2;++i)
			if(childSize[i]<s[i])
				childSize[i]=s[i];
		}
	
	/* Calcuate the total interior size: */
	Vector result=childSize;
	result[0]+=2.0f*marginWidth;
	result[1]+=2.0f*marginWidth;
	if(result[0]<buttonSize[0])
		result[0]=buttonSize[0];
	result[1]+=buttonSize[1];
	return calcExteriorSize(result);
	}

ZRange Pager::calcZRange(void) const
	{
	/* Calculate the parent class widget's z range: */
	ZRange myZRange=Container::calcZRange();
	
	/* Calculate the combined z range of all children and page selection buttons: */
	for(WidgetList::const_iterator cIt=children.begin();cIt!=children.end();++cIt)
		myZRange+=(*cIt)->calcZRange();
	for(ButtonList::const_iterator pbIt=pageButtons.begin();pbIt!=pageButtons.end();++pbIt)
		myZRange+=(*pbIt)->calcZRange();
	
	return myZRange;
	}

void Pager::resize(const Box& newExterior)
	{
	/* Resize the parent class widget: */
	Container::resize(newExterior);
	
	/* Calculate the natural size of the page button box: */
	Vector buttonSize(0.0f,0.0f,0.0f);
	for(ButtonList::iterator pbIt=pageButtons.begin();pbIt!=pageButtons.end();++pbIt)
		{
		/* Get the button's size: */
		Vector s=(*pbIt)->calcNaturalSize();
		
		/* Increment the box width: */
		buttonSize[0]+=s[0];
		
		/* Adjust the box height: */
		if(buttonSize[1]<s[1])
			buttonSize[1]=s[1];
		}
	
	/* Resize and reposition the page buttons: */
	buttonBox=getInterior();
	buttonBox.origin[1]+=buttonBox.size[1]-buttonSize[1];
	buttonBox.size[1]=buttonSize[1];
	Box pbBox=buttonBox;
	for(ButtonList::iterator pbIt=pageButtons.begin();pbIt!=pageButtons.end();++pbIt)
		{
		/* Get the button's natural size: */
		Vector s=(*pbIt)->calcNaturalSize();
		
		/* Resize the current button: */
		pbBox.size[0]=s[0];
		(*pbIt)->resize(pbBox);
		
		/* Advance the button box position to the next button: */
		pbBox.origin[0]+=pbBox.size[0];
		}
	
	/* Resize and reposition all child widgets: */
	childBox=getInterior();
	childBox.size[1]-=buttonSize[1];
	childBox.doInset(Vector(marginWidth,marginWidth,0.0f));
	for(WidgetList::iterator cIt=children.begin();cIt!=children.end();++cIt)
		(*cIt)->resize(childBox);
	}

void Pager::draw(GLContextData& contextData) const
	{
	/* Draw the parent class widget: */
	Container::draw(contextData);
	
	/* Bail out if there are no children: */
	if(children.empty())
		return;
	
	/* Draw a margin around the page buttons and the child widget area: */
	glBegin(GL_TRIANGLE_FAN);
	glColor(backgroundColor);
	glNormal3f(0.0f,0.0f,1.0f);
	glVertex(getInterior().getCorner(0));
	glVertex(getInterior().getCorner(1));
	glVertex(childBox.getCorner(1));
	glVertex(childBox.getCorner(0));
	glVertex(childBox.getCorner(2));
	glVertex(buttonBox.getCorner(0));
	glVertex(buttonBox.getCorner(2));
	glVertex(getInterior().getCorner(2));
	glEnd();
	
	glBegin(GL_TRIANGLE_FAN);
	glVertex(getInterior().getCorner(3));
	glVertex(getInterior().getCorner(2));
	glVertex(buttonBox.getCorner(2));
	for(ButtonList::const_iterator pbIt=pageButtons.begin();pbIt!=pageButtons.end();++pbIt)
		glVertex((*pbIt)->getExterior().getCorner(3));
	glVertex(buttonBox.getCorner(3));
	glVertex(buttonBox.getCorner(1));
	glVertex(getInterior().getCorner(1));
	glEnd();
	
	glBegin(GL_TRIANGLE_FAN);
	glVertex(childBox.getCorner(3));
	glVertex(childBox.getCorner(1));
	glVertex(getInterior().getCorner(1));
	glVertex(buttonBox.getCorner(1));
	for(ButtonList::const_reverse_iterator pbIt=pageButtons.rbegin();pbIt!=pageButtons.rend();++pbIt)
		glVertex((*pbIt)->getExterior().getCorner(1));
	glVertex(buttonBox.getCorner(0));
	glVertex(childBox.getCorner(2));
	glEnd();
	
	/* Fill the empty space next to the page buttons: */
	glBegin(GL_QUADS);
	glVertex(pageButtons.back()->getExterior().getCorner(1));
	glVertex(buttonBox.getCorner(1));
	glVertex(buttonBox.getCorner(3));
	glVertex(pageButtons.back()->getExterior().getCorner(3));
	glEnd();
	
	/* Draw the page buttons: */
	for(ButtonList::const_iterator pbIt=pageButtons.begin();pbIt!=pageButtons.end();++pbIt)
		(*pbIt)->draw(contextData);
	
	/* Draw the currently displayed child widget: */
	children[currentChildIndex]->draw(contextData);
	}

bool Pager::findRecipient(Event& event)
	{
	/* Distribute the question to the currently displayed child widget: */
	bool childFound=children[currentChildIndex]->findRecipient(event);
	
	/* Distribute the question to the page button widgets: */
	for(ButtonList::iterator pbIt=pageButtons.begin();!childFound&&pbIt!=pageButtons.end();++pbIt)
		childFound=(*pbIt)->findRecipient(event);
	
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

void Pager::addChild(Widget* newChild)
	{
	/* Do nothing if the new child is actually a page button: */
	if(addingPageButton)
		return;
	
	/* Add the child to the list at the selected position: */
	children.insert(children.begin()+nextChildIndex,newChild);
	
	/* Create a new page button: */
	char pbName[20];
	snprintf(pbName,sizeof(pbName),"PageButton%u",nextPageIndex);
	++nextPageIndex;
	Button* newPageButton;
	addingPageButton=true;
	if(!nextPageName.empty())
		{
		newPageButton=new Button(pbName,this,nextPageName.c_str());
		nextPageName="";
		}
	else
		{
		/* Create a default page button label: */
		char pbLabel[20];
		snprintf(pbLabel,sizeof(pbLabel),"Page %u",nextPageIndex);
		newPageButton=new Button(pbName,this,pbLabel);
		}
	addingPageButton=false;
	newPageButton->setBorderWidth(newPageButton->getBorderWidth()*0.5f);
	newPageButton->getSelectCallbacks().add(this,&Pager::pageButtonSelectCallback);
	
	/* Add the page button to the list: */
	pageButtons.insert(pageButtons.begin()+nextChildIndex,newPageButton);
	
	/* Select the just-added child: */
	setCurrentChildIndex(nextChildIndex);
	
	/* Prepare for the next page: */
	nextChildIndex=GLint(children.size());
	
	if(isManaged)
		{
		/* Try to resize the widget to accomodate the new child: */
		parent->requestResize(this,calcNaturalSize());
		}
	}

void Pager::removeChild(Widget* removeChild)
	{
	/* Find the given widget in the list of children: */
	WidgetList::iterator cIt;
	unsigned int childIndex=0;
	for(cIt=children.begin();cIt!=children.end()&&*cIt!=removeChild;++cIt,++childIndex)
		;
	if(cIt!=children.end())
		{
		/* Remove the child from the list: */
		children.erase(cIt);
		
		/* Remove the child's associated page button from the list: */
		deleteChild(pageButtons[childIndex]);
		pageButtons.erase(pageButtons.begin()+childIndex);
		
		/* Update the child insertion position: */
		if(nextChildIndex>GLint(children.size()))
			nextChildIndex=GLint(children.size());
		
		if(isManaged)
			{
			/* Try to resize the widget to calculate the new layout: */
			parent->requestResize(this,calcNaturalSize());
			}
		}
	}

void Pager::requestResize(Widget* child,const Vector& newExteriorSize)
	{
	/* Just grant the request if nothing really changed: */
	if(!isManaged)
		{
		/* Just resize the child: */
		child->resize(Box(child->getExterior().origin,newExteriorSize));
		}
	else if(newExteriorSize[0]==child->getExterior().size[0]&&newExteriorSize[1]==child->getExterior().size[1])
		{
		/* Resize the child in its previous box: */
		child->resize(child->getExterior());
		
		/* Invalidate the visual representation: */
		update();
		}
	else
		{
		/* Calculate the new natural grid size: */
		
		/* Calculate the size of the page button box: */
		Vector buttonSize(0.0f,0.0f,0.0f);
		for(ButtonList::const_iterator pbIt=pageButtons.begin();pbIt!=pageButtons.end();++pbIt)
			{
			/* Get the button's size: */
			Vector s=*pbIt!=child?(*pbIt)->calcNaturalSize():newExteriorSize;
			
			/* Increment the box width: */
			buttonSize[0]+=s[0];
			
			/* Adjust the box height: */
			if(buttonSize[1]<s[1])
				buttonSize[1]=s[1];
			}
		
		/* Calculate the size of the child widget area: */
		Vector childSize(0.0f,0.0f,0.0f);
		for(WidgetList::const_iterator cIt=children.begin();cIt!=children.end();++cIt)
			{
			/* Get the child's size: */
			Vector s=*cIt!=child?(*cIt)->calcNaturalSize():newExteriorSize;
			
			/* Adjust the box width and height: */
			for(int i=0;i<2;++i)
				if(childSize[i]<s[i])
					childSize[i]=s[i];
			}
		
		/* Calcuate the new overall size: */
		Vector newSize=childSize;
		newSize[0]+=2.0f*marginWidth;
		newSize[1]+=2.0f*marginWidth;
		if(newSize[0]<buttonSize[0])
			newSize[0]=buttonSize[0];
		newSize[1]+=buttonSize[1];
		
		/* Try to resize the widget: */
		parent->requestResize(this,calcExteriorSize(newSize));
		}
	}

Widget* Pager::getFirstChild(void)
	{
	if(children.empty())
		return 0;
	else
		return children.front();
	}

Widget* Pager::getNextChild(Widget* child)
	{
	/* Search the given widget in the children list: */
	WidgetList::iterator childIt;
	for(childIt=children.begin();childIt!=children.end();++childIt)
		if(*childIt==child)
			{
			/* Return the child after the found one: */
			++childIt;
			break;
			}
	
	/* If the iterator is valid, return its content; otherwise, return null: */
	if(childIt!=children.end())
		return *childIt;
	else
		return 0;
	}

void Pager::setNextChildIndex(GLint newNextChildIndex)
	{
	/* Set the insertion index: */
	if(newNextChildIndex>GLint(children.size()))
		newNextChildIndex=GLint(children.size());
	nextChildIndex=newNextChildIndex;
	}

void Pager::setNextPageName(const char* newNextPageName)
	{
	/* Set the next page name: */
	nextPageName=newNextPageName;
	}

void Pager::setMarginWidth(GLfloat newMarginWidth)
	{
	/* Set the margin width: */
	marginWidth=newMarginWidth;
	
	if(isManaged)
		{
		/* Try to resize the widget to accomodate the new setting: */
		parent->requestResize(this,calcNaturalSize());
		}
	}

void Pager::setCurrentChildIndex(GLint newCurrentChildIndex)
	{
	if(newCurrentChildIndex>GLint(children.size())-1)
		newCurrentChildIndex=GLint(children.size())-1;
	
	/* Get the style sheet: */
	const StyleSheet& ss=*getStyleSheet();
	
	/* Un-"arm" the previously selected path button: */
	pageButtons[currentChildIndex]->setBorderType(Widget::RAISED);
	pageButtons[currentChildIndex]->setBackgroundColor(ss.bgColor);
	pageButtons[currentChildIndex]->setArmedBackgroundColor(ss.buttonArmedBackgroundColor);
	
	/* Select the new page button: */
	pageButtons[newCurrentChildIndex]->setBorderType(Widget::LOWERED);
	pageButtons[newCurrentChildIndex]->setBackgroundColor(ss.buttonArmedBackgroundColor);
	pageButtons[newCurrentChildIndex]->setArmedBackgroundColor(ss.bgColor);
	
	/* Select the new child widget: */
	currentChildIndex=newCurrentChildIndex;
	
	/* Invalidate the visual representation: */
	update();
	}

}
