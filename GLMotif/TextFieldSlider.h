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

#ifndef GLMOTIF_TEXTFIELDSLIDER_INCLUDED
#define GLMOTIF_TEXTFIELDSLIDER_INCLUDED

#include <Misc/CallbackData.h>
#include <Misc/CallbackList.h>
#include <GLMotif/TextField.h>
#include <GLMotif/Slider.h>
#include <GLMotif/Container.h>

namespace GLMotif {

class TextFieldSlider:public Container
	{
	/* Embedded classes: */
	public:
	enum SliderMapping // Enumerated type for mappings from slider positions to values
		{
		LINEAR,EXP10
		};
	enum ValueType // Enumerated type for value types
		{
		UINT,INT,FLOAT
		};
	class ValueChangedCallbackData:public Misc::CallbackData
		{
		/* Embedded classes: */
		public:
		enum ChangeReason // Enumerated type for different change reasons
			{
			CLICKED,DRAGGED,EDITED
			};
		
		/* Elements: */
		TextFieldSlider* slider; // Pointer to the compound slider widget causing the event
		ChangeReason reason; // Reason for this value change
		double value; // Current slider value
		
		/* Constructors and destructors: */
		ValueChangedCallbackData(TextFieldSlider* sSlider,ChangeReason sReason,double sValue)
			:slider(sSlider),reason(sReason),value(sValue)
			{
			}
		};
	
	/* Private methods: */
	private:
	void textFieldValueChangedCallback(TextField::ValueChangedCallbackData* cbData);
	void sliderValueChangedCallback(Slider::ValueChangedCallbackData* cbData);
	
	/* Elements: */
	private:
	TextField* textField; // Pointer to the text field widget
	GLfloat spacing; // Spacing between text field and slider widget
	Slider* slider; // Pointer to the slider widget
	SliderMapping sliderMapping; // Mapping from slider positions to values
	ValueType valueType; // Slider's value type for display in the text field
	double valueMin,valueMax; // Value range
	double valueIncrement; // Value increment in slider position units
	double value; // The current value of the compound slider
	Misc::CallbackList valueChangedCallbacks; // List of callbacks to be called when the slider value changes due to a user interaction
	
	/* Constructors and destructors: */
	public:
	TextFieldSlider(const char* sName,Container* sParent,GLint sCharWidth,GLfloat sShaftLength,bool sManageChild =true);
	virtual ~TextFieldSlider(void);
	
	/* Methods inherited from Widget: */
	virtual Vector calcNaturalSize(void) const;
	virtual ZRange calcZRange(void) const;
	virtual void resize(const Box& newExterior);
	virtual void draw(GLContextData& contextData) const;
	virtual bool findRecipient(Event& event);
	
	/* Methods inherited from Container: */
	virtual void addChild(Widget* newChild);
	virtual void removeChild(Widget* removeChild);
	virtual void requestResize(Widget* child,const Vector& newExteriorSize);
	virtual Widget* getFirstChild(void);
	virtual Widget* getNextChild(Widget* child);
	
	/* New methods: */
	const TextField* getTextField(void) const // Returns the text field widget
		{
		return textField;
		}
	TextField* getTextField(void) // Ditto
		{
		return textField;
		}
	GLfloat getSpacing(void) const // Returns spacing between the text field and slider widgets
		{
		return spacing;
		}
	void setSpacing(GLfloat newSpacing); // Sets the spacing between the text field and slider widgets
	const Slider* getSlider(void) const // Returns the slider widget
		{
		return slider;
		}
	Slider* getSlider(void) // Ditto
		{
		return slider;
		}
	SliderMapping getSliderMapping(void) const // Returns the current slider position mapping
		{
		return sliderMapping;
		}
	void setSliderMapping(SliderMapping newSliderMapping); // Sets the slider position mapping
	ValueType getValueType(void) const // Returns the slider's value type
		{
		return valueType;
		}
	void setValueType(ValueType newValueType); // Sets the slider's value type
	void setValueRange(double newValueMin,double newValueMax,double newValueIncrement); // Sets the slider's value range
	double getValue(void) const // Returns the current slider value
		{
		return value;
		}
	void setValue(double newValue); // Sets the current slider value
	Misc::CallbackList& getValueChangedCallbacks(void) // Returns list of value changed callbacks
		{
		return valueChangedCallbacks;
		}
	};

}

#endif
