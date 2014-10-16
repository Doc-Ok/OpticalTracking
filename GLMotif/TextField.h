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

#ifndef GLMOTIF_TEXTFIELD_INCLUDED
#define GLMOTIF_TEXTFIELD_INCLUDED

#include <Misc/CallbackData.h>
#include <Misc/CallbackList.h>
#include <GLMotif/Label.h>

namespace GLMotif {

class TextField:public Label
	{
	/* Embedded classes: */
	public:
	enum FloatFormat // Enumerated type for floating-point number formatting modes
		{
		FIXED,SCIENTIFIC,SMART
		};
	
	class LayoutChangedCallbackData:public Misc::CallbackData // Data sent when the displayed character width of the text field changes
		{
		/* Elements: */
		public:
		TextField* textField; // Pointer to the text field widget causing the event
		int charWidth; // The new width of the text field in average characters
		
		/* Constructors and destructors: */
		LayoutChangedCallbackData(TextField* sTextField,int sCharWidth)
			:textField(sTextField),charWidth(sCharWidth)
			{
			}
		};
	
	class ValueChangedCallbackData:public Misc::CallbackData
		{
		/* Elements: */
		public:
		TextField* textField; // Pointer to the text field widget causing the event
		const char* value; // The new text field contents
		bool confirmed; // Flag whether the value change was due to a "confirmation" event (e.g., Enter key press)
		
		/* Constructors and destructors: */
		ValueChangedCallbackData(TextField* sTextField,const char* sValue,bool sConfirmed)
			:textField(sTextField),value(sValue),confirmed(sConfirmed)
			{
			}
		};
	
	/* Elements: */
	protected:
	GLint charWidth; // Fixed width of the text field's interior in average character widths
	GLint fieldWidth,precision; // Field width and precision for numerical values (negative values disable that formatting feature)
	FloatFormat floatFormat; // Formatting mode for floating-point numbers (default is smart)
	bool editable; // Flag whether the text field reacts to text and text control events
	bool focus; // Flag if an editable text field is currently ready to receive text entry events
	int anchorPos,cursorPos; // Current selection range or cursor position of an editable text field
	GLfloat cursorModelPos; // Model-space cursor position
	double buttonDownTime; // Last time the pointer button was pressed on the text field
	GLfloat lastPointerPos; // Last horizontal pointer position during click/drag event handling
	bool edited; // Flag if the text field has been edited since the last value change callback
	Misc::CallbackList layoutChangedCallbacks; // List of callbacks to be called when the displayed character width of the text field changes
	Misc::CallbackList valueChangedCallbacks; // List of callbacks to be called when the text field changes value due to a user interaction
	
	/* Protected methods: */
	char* createFormatString(char* buffer); // Creates a printf format string for floating-point numbers in the given buffer
	void setCursorPos(int newCursorPos); // Sets the cursor position
	void insert(int insertLength,const char* insert); // Replaces the current selection with the given string
	
	/* Constructors and destructors: */
	public:
	TextField(const char* sName,Container* sParent,const GLFont* sFont,GLint sCharWidth,bool manageChild =true); // Deprecated
	TextField(const char* sName,Container* sParent,GLint sCharWidth,bool manageChild =true);
	virtual ~TextField(void);
	
	/* Methods inherited from Widget: */
	virtual Vector calcNaturalSize(void) const;
	virtual void resize(const Box& newExterior);
	virtual void draw(GLContextData& contextData) const;
	virtual void pointerButtonDown(Event& event);
	virtual void pointerButtonUp(Event& event);
	virtual void pointerMotion(Event& event);
	virtual bool giveTextFocus(void);
	virtual void takeTextFocus(void);
	virtual void textEvent(const TextEvent& event);
	virtual void textControlEvent(const TextControlEvent& event);
	
	/* Methods inherited from Label: */
	using Label::setString;
	virtual void setString(const char* newLabelBegin,const char* newLabelEnd);
	
	/* New methods: */
	GLint getCharWidth(void) const // Returns current text field size in characters
		{
		return charWidth;
		}
	void setCharWidth(GLint newCharWidth); // Sets the text field's new width
	GLint getFieldWidth(void) const // Returns field width for numerical values
		{
		return fieldWidth;
		}
	void setFieldWidth(GLint newFieldWidth); // Sets new field width for numerical values
	GLint getPrecision(void) const // Returns precision for numerical values
		{
		return precision;
		}
	void setPrecision(GLint newPrecision); // Sets precision for numerical values
	FloatFormat getFloatFormat(void) const // Returns floating-point formatting mode
		{
		return floatFormat;
		}
	void setFloatFormat(FloatFormat newFloatFormat); // Sets new floating-point formatting mode
	bool isEditable(void) const // Returns true if the text field can be edited via events
		{
		return editable;
		}
	void setEditable(bool newEditable); // Sets whether the text field can be edited via events
	bool hasFocus(void) const // Returns true if the text field currently has the input focus
		{
		return focus;
		}
	void setSelection(int newAnchorPos,int newCursorPos); // Sets the selection range or cursor position of an editable text field
	template <class ValueParam>
	void setValue(const ValueParam& value); // Sets the text field to the given value; works for int, unsigned int, float, and double
	Misc::CallbackList& getLayoutChangedCallbacks(void) // Returns list of callbacks called when the text field's displayed field width changes
		{
		return layoutChangedCallbacks;
		}
	Misc::CallbackList& getValueChangedCallbacks(void) // Returns list of callbacks called when the text field's value changes
		{
		return valueChangedCallbacks;
		}
	};

}

#endif
