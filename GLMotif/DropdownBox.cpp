/***********************************************************************
DropdownBox - Class for labels that show one string out of a list of
strings and allow changing the selection by choosing from a pop-up list.
Copyright (c) 2006-2014 Oliver Kreylos

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

#include <GLMotif/DropdownBox.h>

#include <stdio.h>
#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLNormalTemplates.h>
#include <GL/GLVertexTemplates.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/Event.h>
#include <GLMotif/WidgetManager.h>
#include <GLMotif/Container.h>
#include <GLMotif/Popup.h>
#include <GLMotif/Button.h>
#include <GLMotif/RowColumn.h>

namespace GLMotif {

/****************************
Methods of class DropdownBox:
****************************/

void DropdownBox::itemSelectedCallbackWrapper(Misc::CallbackData* cbData,void* userData)
	{
	/* Extract the widget pointers: */
	Button::SelectCallbackData* cbStruct=static_cast<Button::SelectCallbackData*>(cbData);
	DropdownBox* thisPtr=static_cast<DropdownBox*>(userData);
	
	/* Get the selected button's index: */
	int newSelectedItem=thisPtr->items->getChildIndex(cbStruct->button);
	if(thisPtr->selectedItem!=newSelectedItem)
		{
		/* Change the selected item: */
		int oldSelectedItem=thisPtr->selectedItem;
		thisPtr->selectedItem=newSelectedItem;
		thisPtr->setString(cbStruct->button->getString());
		
		/* Call the value changed callbacks: */
		ValueChangedCallbackData cbData(thisPtr,oldSelectedItem,thisPtr->selectedItem);
		thisPtr->valueChangedCallbacks.call(&cbData);
		}
	}

DropdownBox::DropdownBox(const char* sName,Container* sParent,bool sManageChild)
	:Label(sName,sParent,"",false),
	 popup(0),items(0),isPopped(false),
	 foundChild(0),armedChild(0),
	 arrow(GlyphGadget::FANCY_ARROW_DOWN,GlyphGadget::IN,0.0f),
	 numItems(0),
	 selectedItem(-1)
	{
	const GLMotif::StyleSheet& ss=*getManager()->getStyleSheet();
	
	/* Dropdown box defaults to raised border: */
	setBorderType(Widget::RAISED);
	setBorderWidth(ss.buttonBorderWidth);
	
	/* Dropdown box defaults to some margin: */
	setMarginWidth(ss.buttonMarginWidth);
	
	/* Set the arrow sizes: */
	spacing=ss.buttonBorderWidth+2.0f*ss.buttonMarginWidth;
	arrow.setGlyphSize(ss.size*0.25f);
	arrow.setBevelSize(ss.size*0.25f);
	arrow.setGlyphColor(backgroundColor);
	popupExtrudeSize=ss.size*4.0f;
	
	/* Set the label insets: */
	setInsets(0.0f,arrow.getPreferredBoxSize()+spacing);
	
	/* Create a pop-up containing the item labels: */
	popup=new Popup("Popup",getManager());
	popup->setBorderWidth(ss.buttonBorderWidth);
	popup->setBorderType(Widget::PLAIN);
	popup->setBorderColor(borderColor);
	popup->setBackgroundColor(backgroundColor);
	popup->setForegroundColor(foregroundColor);
	popup->setMarginWidth(0.0f);
	
	/* Create a container for the item labels: */
	items=new RowColumn("Items",popup,false);
	items->setBorderWidth(0.0f);
	items->setOrientation(RowColumn::VERTICAL);
	items->setNumMinorWidgets(1);
	items->setMarginWidth(0.0f);
	items->setSpacing(ss.buttonBorderWidth);
	
	items->manageChild();
	
	/* Manage me: */
	if(sManageChild)
		manageChild();
	}

DropdownBox::DropdownBox(const char* sName,Container* sParent,const std::vector<std::string>& sItems,bool sManageChild)
	:Label(sName,sParent,sItems[0].c_str(),false),
	 popup(0),items(0),isPopped(false),
	 foundChild(0),armedChild(0),
	 arrow(GlyphGadget::FANCY_ARROW_DOWN,GlyphGadget::IN,0.0f),
	 numItems(sItems.size()),
	 selectedItem(0)
	{
	const GLMotif::StyleSheet& ss=*getManager()->getStyleSheet();
	
	/* Dropdown box defaults to raised border: */
	setBorderType(Widget::RAISED);
	setBorderWidth(ss.buttonBorderWidth);
	
	/* Dropdown box defaults to some margin: */
	setMarginWidth(ss.buttonMarginWidth);
	
	/* Set the arrow sizes: */
	spacing=ss.buttonBorderWidth+2.0f*ss.buttonMarginWidth;
	arrow.setGlyphSize(ss.size*0.25f);
	arrow.setBevelSize(ss.size*0.25f);
	arrow.setGlyphColor(backgroundColor);
	popupExtrudeSize=ss.size*4.0f;
	
	/* Set the label insets: */
	setInsets(0.0f,arrow.getPreferredBoxSize()+spacing);
	
	/* Create a pop-up containing the item labels: */
	popup=new Popup("Popup",getManager());
	popup->setBorderWidth(ss.buttonBorderWidth);
	popup->setBorderType(Widget::PLAIN);
	popup->setBorderColor(borderColor);
	popup->setBackgroundColor(backgroundColor);
	popup->setForegroundColor(foregroundColor);
	popup->setMarginWidth(0.0f);
	
	/* Create a container for the item labels: */
	items=new RowColumn("Items",popup,false);
	items->setBorderWidth(0.0f);
	items->setOrientation(RowColumn::VERTICAL);
	items->setNumMinorWidgets(1);
	items->setMarginWidth(0.0f);
	items->setSpacing(ss.buttonBorderWidth);
	
	/* Create a button for each list item: */
	for(int i=0;i<numItems;++i)
		{
		char itemButtonName[40];
		snprintf(itemButtonName,sizeof(itemButtonName),"ItemButton%d",i);
		Button* button=new Button(itemButtonName,items,sItems[i].c_str());
		button->setBorderType(Widget::PLAIN);
		button->setBorderWidth(0.0f);
		button->setHAlignment(GLFont::Left);
		button->getSelectCallbacks().add(itemSelectedCallbackWrapper,this);
		}
	
	items->manageChild();
	
	/* Manage me: */
	if(sManageChild)
		manageChild();
	}

DropdownBox::~DropdownBox(void)
	{
	/* Delete the item label popup: */
	delete popup;
	}

Vector DropdownBox::calcNaturalSize(void) const
	{
	/* Calculate the maximum width of all items: */
	Vector result(0.0f,label.getFont()->getTextHeight(),0.0f);
	if(items!=0)
		for(int i=0;i<numItems;++i)
			{
			Vector itemSize=static_cast<Button*>(items->getChild(i))->getLabel().calcNaturalSize();
			for(int j=0;j<2;++j)
				if(result[j]<itemSize[j])
					result[j]=itemSize[j];
			}
	
	/* Return size of largest list item plus margin and dropdown arrow: */
	if(result[1]<arrow.getPreferredBoxSize())
		result[1]=arrow.getPreferredBoxSize();
	result[0]+=2.0f*marginWidth+leftInset+rightInset;
	result[1]+=2.0f*marginWidth;
	
	return calcExteriorSize(result);
	}

ZRange DropdownBox::calcZRange(void) const
	{
	/* Calculate the parent class widget's z range: */
	ZRange myZRange=Label::calcZRange();
	
	/* Adjust for the dropdown arrow: */
	myZRange+=arrow.calcZRange();
	
	return myZRange;
	}
	
void DropdownBox::resize(const Box& newExterior)
	{
	/* Resize the parent class widget: */
	Label::resize(newExterior);
	
	/* Position the dropdown arrow: */
	Box arrowBox=getInterior().inset(Vector(marginWidth,marginWidth,0.0f));
	arrowBox.origin[0]+=arrowBox.size[0]-arrow.getPreferredBoxSize();
	arrowBox.size[0]=arrow.getPreferredBoxSize();
	arrow.setGlyphBox(arrowBox);
	
	if(popup!=0)
		{
		/* Resize the popup to match the interior of the dropdown box: */
		Box popupBox=popup->getExterior();
		popupBox.size[0]=getExterior().size[0]-arrowBox.size[0]-spacing;
		popup->resize(popupBox);
		}
	}

void DropdownBox::setBackgroundColor(const Color& newBackgroundColor)
	{
	/* Call the base class method: */
	Label::setBackgroundColor(newBackgroundColor);
	
	/* Set the dropdown arrow's background color: */
	arrow.setGlyphColor(newBackgroundColor);
	}

void DropdownBox::draw(GLContextData& contextData) const
	{
	/* Draw the base class widget: */
	Widget::draw(contextData);
	
	/* Draw the margin and label separator: */
	glColor(backgroundColor);
	
	/* Draw the top left margin part: */
	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(0.0f,0.0f,1.0f);
	glVertex(getInterior().getCorner(2));
	glVertex(getInterior().getCorner(0));
	glVertex(label.getLabelBox().getCorner(0));
	glVertex(label.getLabelBox().getCorner(2));
	glVertex(label.getLabelBox().getCorner(3));
	glVertex(arrow.getGlyphBox().getCorner(2));
	glVertex(arrow.getGlyphBox().getCorner(3));
	glVertex(getInterior().getCorner(3));
	glEnd();
	
	/* Draw the bottom right margin part: */
	glBegin(GL_TRIANGLE_FAN);
	glVertex(getInterior().getCorner(1));
	glVertex(getInterior().getCorner(3));
	glVertex(arrow.getGlyphBox().getCorner(3));
	glVertex(arrow.getGlyphBox().getCorner(1));
	glVertex(arrow.getGlyphBox().getCorner(0));
	glVertex(label.getLabelBox().getCorner(1));
	glVertex(label.getLabelBox().getCorner(0));
	glVertex(getInterior().getCorner(0));
	glEnd();
	
	/* Draw the label separator: */
	glBegin(GL_QUADS);
	glVertex(label.getLabelBox().getCorner(3));
	glVertex(label.getLabelBox().getCorner(1));
	glVertex(arrow.getGlyphBox().getCorner(0));
	glVertex(arrow.getGlyphBox().getCorner(2));
	glEnd();
	
	/* Draw the dropdown arrow: */
	arrow.draw(contextData);
	
	/* Draw the label: */
	label.draw(contextData);
	}

bool DropdownBox::findRecipient(Event& event)
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

void DropdownBox::pointerButtonDown(Event& event)
	{
	/* "Repair" the incoming event: */
	event.overrideTargetWidget(foundChild);
	
	/* Pop up the secondary top-level widget: */
	if(numItems>0&&!isPopped&&popup!=0)
		{
		/* Calculate the popup's transformation: */
		Vector offset=getInterior().getCorner(0);
		Button* child=static_cast<Button*>(items->getChild(selectedItem));
		Vector popupHotSpot=child->getInterior().getCorner(0);
		offset[0]-=popupHotSpot[0];
		offset[1]-=popupHotSpot[1];
		offset[2]-=popupHotSpot[2];
		offset[2]-=popup->getZRange().first;
		getManager()->popupSecondaryWidget(this,popup,offset);
		isPopped=true;
		
		/* Calculate the extended "hit box" around the popup: */
		popupHitBox=popup->getExterior();
		ZRange popupZRange=popup->getZRange();
		popupHitBox.origin[2]=popupZRange.first;
		popupHitBox.size[2]=popupZRange.second-popupZRange.first;
		popupHitBox.doOffset(offset);
		popupHitBox.doOutset(Vector(popupExtrudeSize,popupExtrudeSize,popupExtrudeSize));
		
		/* Find a potential event recipient in the popup: */
		if(popup->findRecipient(event))
			{
			armedChild=event.getTargetWidget();
			armedChild->pointerButtonDown(event);
			}
		else
			armedChild=0;
		}
	}

void DropdownBox::pointerButtonUp(Event& event)
	{
	/* "Repair" the incoming event: */
	event.overrideTargetWidget(foundChild);
	
	/* Disarm the armed child: */
	if(armedChild!=0)
		{
		armedChild->pointerButtonUp(event);
		armedChild=0;
		}
	
	/* Pop down the secondary top-level widget: */
	if(isPopped)
		{
		popup->getManager()->popdownWidget(popup);
		isPopped=false;
		}
	}

void DropdownBox::pointerMotion(Event& event)
	{
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

void DropdownBox::setSpacing(GLfloat newSpacing)
	{
	/* Set the new spacing: */
	spacing=newSpacing;
	
	/* Adjust the label position: */
	setInsets(0.0f,arrow.getPreferredBoxSize()+spacing);
	
	/* Try adjusting the widget size to accomodate the new setting: */
	if(isManaged)
		parent->requestResize(this,calcNaturalSize());
	else
		resize(Box(Vector(0.0f,0.0f,0.0f),calcNaturalSize()));
	}

void DropdownBox::setArrowBorderSize(GLfloat newArrowBorderSize)
	{
	/* Adjust the arrow glyph: */
	arrow.setBevelSize(newArrowBorderSize);
	
	/* Adjust the label position: */
	setInsets(0.0f,arrow.getPreferredBoxSize()+spacing);
	
	/* Try adjusting the widget size to accomodate the new setting: */
	if(isManaged)
		parent->requestResize(this,calcNaturalSize());
	else
		resize(Box(Vector(0.0f,0.0f,0.0f),calcNaturalSize()));
	}

void DropdownBox::setArrowSize(GLfloat newArrowSize)
	{
	/* Adjust the arrow glyph: */
	arrow.setGlyphSize(newArrowSize);
	
	/* Adjust the label position: */
	setInsets(0.0f,arrow.getPreferredBoxSize()+spacing);
	
	/* Try adjusting the widget size to accomodate the new setting: */
	if(isManaged)
		parent->requestResize(this,calcNaturalSize());
	else
		resize(Box(Vector(0.0f,0.0f,0.0f),calcNaturalSize()));
	}

void DropdownBox::setPopupExtrudeSize(GLfloat newPopupExtrudeSize)
	{
	popupExtrudeSize=newPopupExtrudeSize;
	}

const Widget* DropdownBox::getItemWidget(int item) const
	{
	return items->getChild(item);
	}

const char* DropdownBox::getItem(int item) const
	{
	return static_cast<Button*>(items->getChild(item))->getString();
	}

void DropdownBox::clearItems(void)
	{
	/* Remove all buttons from the drop-down list: */
	while(numItems>0)
		{
		items->removeWidgets(numItems-1);
		--numItems;
		}
	
	/* Reset the selected item: */
	selectedItem=-1;
	
	/* Resize the widget to accomodate the new list: */
	if(isManaged)
		parent->requestResize(this,calcNaturalSize());
	else
		resize(Box(Vector(0.0f,0.0f,0.0f),calcNaturalSize()));
	}

void DropdownBox::addItem(const char* newItem)
	{
	/* Get the style sheet: */
	// const GLMotif::StyleSheet& ss=*getManager()->getStyleSheet();
	
	/* Create a new button in the drop-down list: */
	char itemButtonName[40];
	snprintf(itemButtonName,sizeof(itemButtonName),"ItemButton%d",numItems);
	Button* button=new Button(itemButtonName,items,newItem);
	button->setBorderType(Widget::PLAIN);
	button->setBorderWidth(0.0f);
	button->setHAlignment(GLFont::Left);
	button->getSelectCallbacks().add(itemSelectedCallbackWrapper,this);
	++numItems;
	
	/* Resize the widget to accomodate the new item: */
	if(isManaged)
		parent->requestResize(this,calcNaturalSize());
	else
		resize(Box(Vector(0.0f,0.0f,0.0f),calcNaturalSize()));
	}

void DropdownBox::setSelectedItem(int newSelectedItem)
	{
	if(newSelectedItem>=0&&newSelectedItem<numItems&&selectedItem!=newSelectedItem)
		{
		/* Set the selected item: */
		selectedItem=newSelectedItem;
		
		/* Change the displayed label: */
		setString(static_cast<Button*>(items->getChild(selectedItem))->getString());
		}
	}

}
