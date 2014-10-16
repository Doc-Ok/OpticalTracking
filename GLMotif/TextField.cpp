/***********************************************************************
TextField - Class for labels displaying values as text.
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

#include <string.h>
#include <stdio.h>
#include <Misc/PrintInteger.h>
#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLVertexTemplates.h>
#include <GLMotif/Event.h>
#include <GLMotif/TextEvent.h>
#include <GLMotif/TextControlEvent.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/Container.h>
#include <GLMotif/WidgetManager.h>

#include <GLMotif/TextField.h>

namespace GLMotif {

/**************************
Methods of class TextField:
**************************/

char* TextField::createFormatString(char* buffer)
	{
	char* bufPtr=buffer;
	*bufPtr++='%';
	if(fieldWidth>=0)
		*bufPtr++='*';
	if(precision>=0)
		{
		*bufPtr++='.';
		*bufPtr++='*';
		}
	switch(floatFormat)
		{
		case FIXED:
			*bufPtr++='f';
			break;
		
		case SCIENTIFIC:
			*bufPtr++='e';
			break;
		
		case SMART:
			*bufPtr++='g';
			break;
		}
	*bufPtr++='\0';
	return buffer;
	}

void TextField::setCursorPos(int newCursorPos)
	{
	/* Set and check the cursor position: */
	cursorPos=newCursorPos;
	if(cursorPos<0)
		cursorPos=0;
	if(cursorPos>label.getLength())
		cursorPos=label.getLength();
	
	/* Calculate the model-space cursor position: */
	cursorModelPos=label.calcCharacterPos(cursorPos);
	}

void TextField::insert(int insertLength,const char* insert)
	{
	/* Insert the event's text in place of the current selection: */
	int selectionStart=anchorPos;
	int selectionEnd=cursorPos;
	if(selectionStart>selectionEnd)
		{
		int t=selectionStart;
		selectionStart=selectionEnd;
		selectionEnd=t;
		}
	int newLabelLength=selectionStart; // Length of old pre-selection text
	newLabelLength+=insertLength; // Length of inserted text
	newLabelLength+=label.getLength()-selectionEnd; // Length of old post-selection text
	char* newLabel=new char[newLabelLength+1];
	memcpy(newLabel,label.getString(),selectionStart); // Copy old pre-selection text
	memcpy(newLabel+selectionStart,insert,insertLength); // Copy inserted text
	memcpy(newLabel+selectionStart+insertLength,label.getString()+selectionEnd,label.getLength()-selectionEnd+1); // Copy old post-selection text and string terminator
	
	/* Update the label: */
	label.adoptString(newLabelLength,newLabel);
	edited=true;
	
	/* Adjust the label position: */
	positionLabel();
	
	/* Update the selection range to insert behind the just-inserted text: */
	setCursorPos(selectionStart+insertLength);
	anchorPos=cursorPos;
	}

TextField::TextField(const char* sName,Container* sParent,const GLFont* sFont,GLint sCharWidth,bool sManageChild)
	:Label(sName,sParent,"",sFont,false),
	 charWidth(sCharWidth),
	 fieldWidth(-1),precision(-1),floatFormat(SMART),
	 editable(false),focus(false),anchorPos(0),cursorPos(0),cursorModelPos(0.0f),buttonDownTime(0.0),edited(false)
	{
	/* Get the style sheet: */
	const StyleSheet* ss=getStyleSheet();
	
	setBorderWidth(ss->textfieldBorderWidth);
	setBorderType(Widget::LOWERED);
	setBackgroundColor(ss->textfieldBgColor);
	setForegroundColor(ss->textfieldFgColor);
	setMarginWidth(ss->textfieldMarginWidth);
	setHAlignment(GLFont::Right);
	
	/* Manage me: */
	if(sManageChild)
		manageChild();
	}

TextField::TextField(const char* sName,Container* sParent,GLint sCharWidth,bool sManageChild)
	:Label(sName,sParent,"",false),
	 charWidth(sCharWidth),
	 fieldWidth(-1),precision(-1),floatFormat(SMART),
	 editable(false),focus(false),anchorPos(0),cursorPos(0),cursorModelPos(0.0f),buttonDownTime(0.0),edited(false)
	{
	/* Get the style sheet: */
	const StyleSheet* ss=getStyleSheet();
	
	setBorderWidth(ss->textfieldBorderWidth);
	setBorderType(Widget::LOWERED);
	setBackgroundColor(ss->textfieldBgColor);
	setForegroundColor(ss->textfieldFgColor);
	setMarginWidth(ss->textfieldMarginWidth);
	setHAlignment(GLFont::Right);
	
	/* Manage me: */
	if(sManageChild)
		manageChild();
	}

TextField::~TextField(void)
	{
	if(focus)
		{
		/* Tell the widget's manager to release this widget's text entry focus: */
		WidgetManager* manager=getManager();
		if(manager!=0)
			manager->releaseFocus(this);
		}
	}

void TextField::draw(GLContextData& contextData) const
	{
	if(editable&&focus)
		{
		/* Call the base class method: */
		Widget::draw(contextData);
		
		/* Draw the label margin: */
		glColor(backgroundColor);
		glBegin(GL_QUAD_STRIP);
		glNormal3f(0.0f,0.0f,1.0f);
		glVertex(label.getLabelBox().getCorner(0));
		glVertex(getInterior().getCorner(0));
		glVertex(label.getLabelBox().getCorner(1));
		glVertex(getInterior().getCorner(1));
		glVertex(label.getLabelBox().getCorner(3));
		glVertex(getInterior().getCorner(3));
		glVertex(label.getLabelBox().getCorner(2));
		glVertex(getInterior().getCorner(2));
		glVertex(label.getLabelBox().getCorner(0));
		glVertex(getInterior().getCorner(0));
		glEnd();
		
		/* Draw the label itself: */
		if(anchorPos!=cursorPos)
			{
			GLsizei start=GLsizei(anchorPos);
			GLsizei end=GLsizei(cursorPos);
			if(start>end)
				{
				GLsizei t=start;
				start=end;
				end=t;
				}
			label.draw(start,end,getManager()->getStyleSheet()->selectionBgColor,getManager()->getStyleSheet()->selectionFgColor,contextData);
			}
		else
			label.draw(contextData);
		
		/* Draw the cursor: */
		GLfloat x0=cursorModelPos-marginWidth;
		GLfloat x1=cursorModelPos;
		GLfloat x2=cursorModelPos+marginWidth;
		GLfloat y0=getInterior().origin[1];
		GLfloat y1=y0+2.0f*marginWidth;
		GLfloat y2=getInterior().origin[1]+getInterior().size[1];
		GLfloat y3=y2-2.0f*marginWidth;
		GLfloat z0=getInterior().origin[2];
		GLfloat z1=z0+marginWidth;
		
		glColor(borderColor);
		glBegin(GL_TRIANGLES);
		glNormal3f(-0.6667,0.3333,0.6667);
		glVertex3f(x1,y1,z0);
		glVertex3f(x0,y0,z0);
		glVertex3f(x1,y0,z1);
		
		glNormal3f(0.6667,0.3333,0.6667);
		glVertex3f(x2,y0,z0);
		glVertex3f(x1,y1,z0);
		glVertex3f(x1,y0,z1);
		
		glNormal3f(0.0f,-1.0f,0.0f);
		glVertex3f(x0,y0,z0);
		glVertex3f(x2,y0,z0);
		glVertex3f(x1,y0,z1);
		
		glNormal3f(-0.6667,-0.3333,0.6667);
		glVertex3f(x0,y2,z0);
		glVertex3f(x1,y3,z0);
		glVertex3f(x1,y2,z1);
		
		glNormal3f(0.6667,-0.3333,0.6667);
		glVertex3f(x1,y3,z0);
		glVertex3f(x2,y2,z0);
		glVertex3f(x1,y2,z1);
		
		glNormal3f(0.0f,1.0f,0.0f);
		glVertex3f(x2,y2,z0);
		glVertex3f(x0,y2,z0);
		glVertex3f(x1,y2,z1);
		glEnd();
		}
	else
		{
		/* Call the base class method: */
		Label::draw(contextData);
		}
	}

void TextField::pointerButtonDown(Event& event)
	{
	/* Bail out if the text field is not editable: */
	if(!editable)
		return;
	
	/* Request text entry focus: */
	WidgetManager* manager=getManager();
	if(manager!=0&&(focus||manager->requestFocus(this)))
		{
		/* Check if this was a double-click: */
		double time=getManager()->getTime();
		lastPointerPos=event.getWidgetPoint().getPoint()[0];
		if(time-buttonDownTime<getManager()->getStyleSheet()->multiClickTime)
			{
			/* Select the entire text field: */
			anchorPos=0;
			setCursorPos(label.getLength());
			}
		else
			{
			/* Move the cursor to the selected character: */
			setCursorPos(label.calcCharacterIndex(lastPointerPos));
			anchorPos=cursorPos;
			}
		
		focus=true;
		buttonDownTime=time;
		
		/* Invalidate the visual representation: */
		label.invalidate();
		update();
		}
	}

void TextField::pointerButtonUp(Event& event)
	{
	}

void TextField::pointerMotion(Event& event)
	{
	/* Bail out if the text field is not editable or the pointer button is not pressed: */
	if(!editable||!event.isPressed())
		return;
	
	/* Move the cursor to the selected character: */
	GLfloat pointerPos=event.calcWidgetPoint(this).getPoint()[0];
	if(lastPointerPos!=pointerPos)
		setCursorPos(label.calcCharacterIndex(pointerPos));
	lastPointerPos=pointerPos;
	
	/* Invalidate the visual representation: */
	label.invalidate();
	update();
	}

bool TextField::giveTextFocus(void)
	{
	if(editable)
		{
		#if 0
		/* Adjust the selection range: */
		anchorPos=0;
		setCursorPos(label.getLength());
		#endif
		
		focus=true;
		
		/* Invalidate the visual representation: */
		label.invalidate();
		update();
		}
	
	/* Accept focus if the text field is editable: */
	return editable;
	}

void TextField::takeTextFocus(void)
	{
	focus=false;
	
	/* Call value changed callbacks if the text field has been edited: */
	if(edited)
		{
		/* Call the value changed callbacks: */
		ValueChangedCallbackData cbData(this,label.getString(),false);
		valueChangedCallbacks.call(&cbData);
		
		/* Clear the flag: */
		edited=false;
		}
	
	/* Invalidate the visual representation: */
	label.invalidate();
	update();
	}

void TextField::textEvent(const TextEvent& event)
	{
	/* Bail out if the text field is not editable: */
	if(!editable)
		return;
	
	/* Insert the event's text: */
	insert(event.getTextLength(),event.getText());
	
	/* Invalidate the visual representation: */
	label.invalidate();
	update();
	}

void TextField::textControlEvent(const TextControlEvent& event)
	{
	/* Bail out if the text field is not editable: */
	if(!editable)
		return;
	
	/* Handle the event: */
	switch(event.event)
		{
		case TextControlEvent::CURSOR_LEFT:
		case TextControlEvent::CURSOR_WORD_LEFT:
			setCursorPos(cursorPos-1);
			if(!event.selection)
				anchorPos=cursorPos;
			break;
		
		case TextControlEvent::CURSOR_RIGHT:
		case TextControlEvent::CURSOR_WORD_RIGHT:
			setCursorPos(cursorPos+1);
			if(!event.selection)
				anchorPos=cursorPos;
			break;
		
		case TextControlEvent::CURSOR_START:
		case TextControlEvent::CURSOR_UP:
		case TextControlEvent::CURSOR_PAGE_UP:
		case TextControlEvent::CURSOR_TEXT_START:
			setCursorPos(0);
			if(!event.selection)
				anchorPos=cursorPos;
			break;
		
		case TextControlEvent::CURSOR_END:
		case TextControlEvent::CURSOR_DOWN:
		case TextControlEvent::CURSOR_PAGE_DOWN:
		case TextControlEvent::CURSOR_TEXT_END:
			setCursorPos(label.getLength());
			if(!event.selection)
				anchorPos=cursorPos;
			break;
		
		case TextControlEvent::DELETE:
		case TextControlEvent::BACKSPACE:
		case TextControlEvent::CUT:
			if(cursorPos==anchorPos)
				{
				/* Select the character right of the cursor if deleting on empty selection: */
				if(event.event==TextControlEvent::DELETE&&cursorPos<label.getLength())
					++cursorPos;
				
				/* Select the character left of the cursor if backspacing on empty selection: */
				if(event.event==TextControlEvent::BACKSPACE&&cursorPos>0)
					--cursorPos;
				}
			
			if(cursorPos!=anchorPos)
				{
				if(event.event==TextControlEvent::CUT)
					{
					/* Save the current selection: */
					int selectionStart=anchorPos;
					int selectionEnd=cursorPos;
					if(selectionStart>selectionEnd)
						{
						int t=selectionStart;
						selectionStart=selectionEnd;
						selectionEnd=t;
						}
					getManager()->setTextBuffer(selectionEnd-selectionStart,label.getString()+selectionStart);
					}
				
				/* Remove the current selection: */
				insert(0,0);
				}
			break;
		
		case TextControlEvent::COPY:
			if(cursorPos!=anchorPos)
				{
				/* Save the current selection: */
				int selectionStart=anchorPos;
				int selectionEnd=cursorPos;
				if(selectionStart>selectionEnd)
					{
					int t=selectionStart;
					selectionStart=selectionEnd;
					selectionEnd=t;
					}
				getManager()->setTextBuffer(selectionEnd-selectionStart,label.getString()+selectionStart);
				}
			break;
		
		case TextControlEvent::PASTE:
			/* Insert the widget manager's text buffer: */
			insert(getManager()->getTextBufferLength(),getManager()->getTextBuffer());
			break;
		
		case TextControlEvent::CONFIRM:
			{
			/* Call value changed callbacks whether or not the text field has been edited: */
			ValueChangedCallbackData cbData(this,label.getString(),true);
			valueChangedCallbacks.call(&cbData);
			
			/* Clear the edit flag: */
			edited=false;
			
			/* Give up the text entry focus: */
			getManager()->releaseFocus(this);
			focus=false;
			
			/* Invalidate the visual representation: */
			update();
			break;
			}
		
		case TextControlEvent::EVENTTYPE_END:
			/* Just to make compiler happy... */
			break;
		}
	
	/* Invalidate the visual representation: */
	label.invalidate();
	update();
	}

Vector TextField::calcNaturalSize(void) const
	{
	/* Return the size of the text box plus margin: */
	Vector result(GLfloat(charWidth)*label.getFont()->getCharacterWidth(),label.getFont()->getTextHeight(),0.0f);
	result[0]+=2.0f*marginWidth+leftInset+rightInset;
	result[1]+=2.0f*marginWidth;
	
	return calcExteriorSize(result);
	}

void TextField::resize(const Box& newExterior)
	{
	/* Resize the parent class widget: */
	Label::resize(newExterior);
	
	/* Calculate the new character width of the text field: */
	GLfloat width=getInterior().size[0]-2.0f*marginWidth-leftInset-rightInset;
	int newCharWidth=int(width/label.getFont()->getCharacterWidth());
	
	/* Call the layout changed callbacks: */
	LayoutChangedCallbackData cbData(this,newCharWidth);
	layoutChangedCallbacks.call(&cbData);
	}

void TextField::setString(const char* newLabelBegin,const char* newLabelEnd)
	{
	/* Update the label text: */
	label.setString(newLabelBegin,newLabelEnd);
	
	/* Adjust the label position: */
	positionLabel();
	
	/* Check the selection range of editable text fields: */
	if(editable&&focus)
		{
		setCursorPos(label.getLength());
		anchorPos=0;
		}
	
	/* Invalidate the visual representation: */
	update();
	}

void TextField::setCharWidth(GLint newCharWidth)
	{
	/* Set the width: */
	charWidth=newCharWidth;
	
	if(isManaged)
		{
		/* Try adjusting the widget size to accomodate the new margin width: */
		parent->requestResize(this,calcNaturalSize());
		}
	else
		resize(Box(Vector(0.0f,0.0f,0.0f),calcNaturalSize()));
	}

void TextField::setFieldWidth(GLint newFieldWidth)
	{
	/* Set the field width: */
	fieldWidth=newFieldWidth;
	}

void TextField::setPrecision(GLint newPrecision)
	{
	/* Set the precision: */
	precision=newPrecision;
	}

void TextField::setFloatFormat(TextField::FloatFormat newFloatFormat)
	{
	/* Set the floating-point formatting mode: */
	floatFormat=newFloatFormat;
	}

void TextField::setEditable(bool newEditable)
	{
	/* Set the text field's editable flag: */
	editable=newEditable;
	}

void TextField::setSelection(int newAnchorPos,int newCursorPos)
	{
	/* Bail out if the text field is not editable or doesn't have the focus: */
	if(!editable) // ||!focus)
		return;
	
	/* Set the selection range: */
	setCursorPos(newCursorPos);
	anchorPos=newAnchorPos;
	if(anchorPos<0)
		anchorPos=0;
	if(anchorPos>label.getLength())
		anchorPos=label.getLength();
	
	/* Invalidate the visual representation: */
	label.invalidate();
	update();
	}

template <class ValueParam>
void TextField::setValue(const ValueParam& value)
	{
	}

template <>
void TextField::setValue<int>(const int& value)
	{
	char valueString[81];
	if(fieldWidth>=0)
		{
		char* vsPtr=Misc::print(value,valueString+80);
		for(int width=(valueString+80)-vsPtr;width<fieldWidth&&width<80;++width)
			*(--vsPtr)=' ';
		setString(vsPtr);
		}
	else
		setString(Misc::print(value,valueString+11));
	}

template <>
void TextField::setValue<unsigned int>(const unsigned int& value)
	{
	char valueString[81];
	if(fieldWidth>=0)
		{
		char* vsPtr=Misc::print(value,valueString+80);
		for(int width=(valueString+80)-vsPtr;width<fieldWidth&&width<80;++width)
			*(--vsPtr)=' ';
		setString(vsPtr);
		}
	else
		setString(Misc::print(value,valueString+10));
	}

template <>
void TextField::setValue<float>(const float& value)
	{
	char format[10],valueString[80];
	if(fieldWidth>=0)
		{
		if(precision>=0)
			snprintf(valueString,sizeof(valueString),createFormatString(format),fieldWidth,precision,value);
		else
			snprintf(valueString,sizeof(valueString),createFormatString(format),fieldWidth,value);
		}
	else
		{
		if(precision>=0)
			snprintf(valueString,sizeof(valueString),createFormatString(format),precision,value);
		else
			snprintf(valueString,sizeof(valueString),createFormatString(format),value);
		}
	setString(valueString);
	}

template <>
void TextField::setValue<double>(const double& value)
	{
	char format[10],valueString[80];
	if(fieldWidth>=0)
		{
		if(precision>=0)
			snprintf(valueString,sizeof(valueString),createFormatString(format),fieldWidth,precision,value);
		else
			snprintf(valueString,sizeof(valueString),createFormatString(format),fieldWidth,value);
		}
	else
		{
		if(precision>=0)
			snprintf(valueString,sizeof(valueString),createFormatString(format),precision,value);
		else
			snprintf(valueString,sizeof(valueString),createFormatString(format),value);
		}
	setString(valueString);
	}

}
