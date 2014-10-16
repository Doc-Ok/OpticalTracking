/***********************************************************************
TextFieldSlider - Compound widget containing a slider and a text field
to display and edit the slider value.
Copyright (c) 2010 Oliver Kreylos

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

#include <GLMotif/TextFieldSlider.h>

#include <stdlib.h>
#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLVertexTemplates.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/Event.h>

namespace GLMotif {

/********************************
Methods of class TextFieldSlider:
********************************/

void TextFieldSlider::textFieldValueChangedCallback(TextField::ValueChangedCallbackData* cbData)
	{
	/* Get the text field's new value: */
	setValue(atof(textField->getString()));
	
	/* Call the value changed callbacks: */
	ValueChangedCallbackData myCbData(this,ValueChangedCallbackData::EDITED,value);
	valueChangedCallbacks.call(&myCbData);
	}

void TextFieldSlider::sliderValueChangedCallback(Slider::ValueChangedCallbackData* cbData)
	{
	/* Get the slider's new value: */
	switch(sliderMapping)
		{
		case LINEAR:
			setValue(double(slider->getValue()));
			break;
		
		case EXP10:
			setValue(Math::pow(10.0,double(slider->getValue())));
			break;
		}
	
	/* Call the value changed callbacks: */
	ValueChangedCallbackData::ChangeReason reason=cbData->reason==Slider::ValueChangedCallbackData::CLICKED?ValueChangedCallbackData::CLICKED:ValueChangedCallbackData::DRAGGED;
	ValueChangedCallbackData myCbData(this,reason,value);
	valueChangedCallbacks.call(&myCbData);
	}

TextFieldSlider::TextFieldSlider(const char* sName,Container* sParent,GLint sCharWidth,GLfloat sShaftLength,bool sManageChild)
	:Container(sName,sParent,false),
	 textField(new TextField("TextField",this,sCharWidth,false)),
	 slider(new Slider("Slider",this,Slider::HORIZONTAL,sShaftLength,false)),
	 sliderMapping(LINEAR),valueType(FLOAT),
	 valueMin(0.0),valueMax(1000.0),valueIncrement(1.0),value(500.0)
	{
	/* Get the style sheet: */
	const StyleSheet* ss=getStyleSheet();
	
	/* Initialize the compound widget: */
	setBorderWidth(0.0f);
	setBorderType(PLAIN);
	spacing=ss->size;
	
	/* Initialize the text field: */
	textField->setEditable(true);
	textField->getValueChangedCallbacks().add(this,&TextFieldSlider::textFieldValueChangedCallback);
	
	/* Initialize the slider: */
	slider->getValueChangedCallbacks().add(this,&TextFieldSlider::sliderValueChangedCallback);
	
	/* Manage the children: */
	textField->manageChild();
	slider->manageChild();
	
	/* Manage me: */
	if(sManageChild)
		manageChild();
	}

TextFieldSlider::~TextFieldSlider(void)
	{
	/* Delete the child widgets: */
	deleteChild(textField);
	deleteChild(slider);
	}

Vector TextFieldSlider::calcNaturalSize(void) const
	{
	/* Calculate the text field's natural size: */
	Vector result=textField->calcNaturalSize();
	
	/* Insert the spacer: */
	result[0]+=spacing;
	
	/* Calculate the slider's natural size: */
	Vector sliderSize=slider->calcNaturalSize();
	
	/* Adjust the widget size: */
	result[0]+=sliderSize[0];
	if(result[1]<sliderSize[1])
		result[1]=sliderSize[1];
	
	return calcExteriorSize(result);
	}

ZRange TextFieldSlider::calcZRange(void) const
	{
	/* Calculate the parent class widget's z range: */
	ZRange myZRange=Container::calcZRange();
	
	/* Adjust for the text field: */
	myZRange+=textField->calcZRange();
	
	/* Adjust for the slider: */
	myZRange+=slider->calcZRange();
	
	return myZRange;
	}

void TextFieldSlider::resize(const Box& newExterior)
	{
	/* Resize the parent class widget: */
	Container::resize(newExterior);
	
	/* Position the text field: */
	Box textFieldBox=getInterior();
	textFieldBox.size=textField->calcNaturalSize();
	textFieldBox.origin[1]+=(getInterior().size[1]-textFieldBox.size[1])*0.5f;
	textField->resize(textFieldBox);
	
	/* Position the slider: */
	Box sliderBox=getInterior();
	sliderBox.size=slider->calcNaturalSize();
	sliderBox.origin[0]+=textFieldBox.size[0]+spacing;
	sliderBox.origin[1]+=(getInterior().size[1]-sliderBox.size[1])*0.5f;
	sliderBox.size[0]=getInterior().size[0]-textFieldBox.size[0]-spacing;
	slider->resize(sliderBox);
	}

void TextFieldSlider::draw(GLContextData& contextData) const
	{
	/* Draw the parent class widget: */
	Container::draw(contextData);
	
	/* Draw the margin around the child widgets: */
	glColor(backgroundColor);
	GLfloat midx=getInterior().origin[0]+textField->getExterior().size[0]+spacing*0.5f;
	GLfloat y1=getInterior().origin[1];
	GLfloat y2=y1+getInterior().size[1];
	glBegin(GL_TRIANGLES);
	glNormal3f(0.0f,0.0f,1.0f);
	glVertex(getInterior().getCorner(0));
	glVertex(getInterior().getCorner(0));
	glVertex(midx,y1,getInterior().origin[2]);
	glVertex(getInterior().getCorner(3));
	glVertex(getInterior().getCorner(2));
	glVertex(midx,y2,getInterior().origin[2]);
	glEnd();
	
	/* Draw the margin around the text field: */
	glBegin(GL_QUAD_STRIP);
	glVertex(textField->getExterior().getCorner(0));
	glVertex(getInterior().getCorner(0));
	glVertex(textField->getExterior().getCorner(1));
	glVertex(midx,y1,getInterior().origin[2]);
	glVertex(textField->getExterior().getCorner(3));
	glVertex(midx,y2,getInterior().origin[2]);
	glVertex(textField->getExterior().getCorner(2));
	glVertex(getInterior().getCorner(2));
	glVertex(textField->getExterior().getCorner(0));
	glVertex(getInterior().getCorner(0));
	glEnd();
	
	/* Draw the margin around the slider: */
	glBegin(GL_QUAD_STRIP);
	glVertex(slider->getExterior().getCorner(1));
	glVertex(getInterior().getCorner(1));
	glVertex(slider->getExterior().getCorner(3));
	glVertex(getInterior().getCorner(3));
	glVertex(slider->getExterior().getCorner(2));
	glVertex(midx,y2,getInterior().origin[2]);
	glVertex(slider->getExterior().getCorner(0));
	glVertex(midx,y1,getInterior().origin[2]);
	glVertex(slider->getExterior().getCorner(1));
	glVertex(getInterior().getCorner(1));
	glEnd();
	
	/* Draw the child widgets: */
	textField->draw(contextData);
	slider->draw(contextData);
	}

bool TextFieldSlider::findRecipient(Event& event)
	{
	/* Distribute the question to the child widgets: */
	bool childFound=false;
	if(!childFound)
		childFound=textField->findRecipient(event);
	if(!childFound)
		childFound=slider->findRecipient(event);
	
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

void TextFieldSlider::addChild(Widget* newChild)
	{
	/* Only the dedicated children can call this, and they get managed in the constructor, so ignore this */
	}

void TextFieldSlider::removeChild(Widget* removeChild)
	{
	/* This should never be called, so simply ignore it */
	}

void TextFieldSlider::requestResize(Widget* child,const Vector& newExteriorSize)
	{
	if(isManaged)
		{
		/* Adjust the widget's layout: */
		Vector size=child==textField?newExteriorSize:textField->getExterior().size;
		size[0]+=spacing;
		Vector sSize=child==slider?newExteriorSize:slider->getExterior().size;
		size[0]+=sSize[0];
		if(size[1]<sSize[1])
			size[1]=sSize[1];
		
		/* Resize the widget: */
		parent->requestResize(this,calcExteriorSize(size));
		}
	else
		{
		/* Just resize the child: */
		child->resize(Box(child->getExterior().origin,newExteriorSize));
		}
	}

Widget* TextFieldSlider::getFirstChild(void)
	{
	/* Return the text field: */
	return textField;
	}

Widget* TextFieldSlider::getNextChild(Widget* child)
	{
	/* Return the next child: */
	if(child==textField)
		return slider;
	else
		return 0;
	}

void TextFieldSlider::setSpacing(GLfloat newSpacing)
	{
	/* Set the spacing: */
	spacing=newSpacing;
	
	/* Adjust the widget's layout: */
	Vector size=textField->getExterior().size;
	size[0]+=spacing;
	Vector sSize=slider->getExterior().size;
	size[0]+=sSize[0];
	if(size[0]<sSize[1])
		size[0]=sSize[1];
	
	/* Resize the widget: */
	if(isManaged)
		parent->requestResize(this,calcExteriorSize(size));
	else
		resize(Box(Vector(0.0f,0.0f,0.0f),calcExteriorSize(size)));
	}

void TextFieldSlider::setSliderMapping(SliderMapping newSliderMapping)
	{
	/* Set the slider mapping: */
	sliderMapping=newSliderMapping;
	
	/* Update the slider: */
	switch(sliderMapping)
		{
		case LINEAR:
			slider->setValueRange(valueMin,valueMax,valueIncrement);
			slider->setValue(value);
			break;
		
		case EXP10:
			slider->setValueRange(Math::log10(valueMin),Math::log10(valueMax),valueIncrement);
			slider->setValue(Math::log10(value));
			break;
		}
	}

void TextFieldSlider::setValueType(TextFieldSlider::ValueType newValueType)
	{
	/* Set the value type: */
	valueType=newValueType;
	
	/* Update the text field: */
	switch(valueType)
		{
		case UINT:
			textField->setValue<unsigned int>(value>0.0?(unsigned int)(value+0.5):0);
			break;
		
		case INT:
			textField->setValue<int>(int(value+0.5));
			break;
		
		case FLOAT:
			textField->setValue<double>(value);
			break;
		}
	}

void TextFieldSlider::setValueRange(double newValueMin,double newValueMax,double newValueIncrement)
	{
	/* Set the value range: */
	valueMin=newValueMin;
	valueMax=newValueMax;
	valueIncrement=newValueIncrement;
	
	/* Check the value against the range: */
	bool valueChanged=false;
	if(value<valueMin)
		{
		value=valueMin;
		valueChanged=true;
		}
	if(value>valueMax)
		{
		value=valueMax;
		valueChanged=true;
		}
	if(valueChanged)
		{
		/* Update the text field: */
		switch(valueType)
			{
			case UINT:
				textField->setValue<unsigned int>(value>0.0?(unsigned int)(value+0.5):0);
				break;
			
			case INT:
				textField->setValue<int>(int(value+0.5));
				break;
			
			case FLOAT:
				textField->setValue<double>(value);
				break;
			}
		}
	
	/* Update the slider: */
	switch(sliderMapping)
		{
		case LINEAR:
			slider->setValueRange(valueMin,valueMax,valueIncrement);
			slider->setValue(value);
			break;
		
		case EXP10:
			slider->setValueRange(Math::log10(valueMin),Math::log10(valueMax),valueIncrement);
			slider->setValue(Math::log10(value));
			break;
		}
	}

void TextFieldSlider::setValue(double newValue)
	{
	/* Set the value: */
	value=newValue;
	
	/* Check the value against the valid range: */
	if(value<valueMin)
		value=valueMin;
	if(value>valueMax)
		value=valueMax;
	
	/* Update the text field: */
	switch(valueType)
		{
		case UINT:
			textField->setValue<unsigned int>(value>0.0?(unsigned int)(value+0.5):0);
			break;
		
		case INT:
			textField->setValue<int>(int(value+0.5));
			break;
		
		case FLOAT:
			textField->setValue<double>(value);
			break;
		}
	
	/* Update the slider: */
	switch(sliderMapping)
		{
		case LINEAR:
			slider->setValue(value);
			break;
		
		case EXP10:
			slider->setValue(Math::log10(value));
			break;
		}
	}

}
