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

#ifndef GLMOTIF_DROPDOWNBOX_INCLUDED
#define GLMOTIF_DROPDOWNBOX_INCLUDED

#include <string>
#include <vector>
#include <Misc/CallbackData.h>
#include <Misc/CallbackList.h>
#include <GL/gl.h>
#include <GLMotif/GlyphGadget.h>
#include <GLMotif/Label.h>

/* Forward declarations: */
namespace GLMotif {
class Popup;
class RowColumn;
}

namespace GLMotif {

class DropdownBox:public Label
	{
	/* Embedded classes: */
	public:
	class CallbackData:public Misc::CallbackData // Base class for callback data sent by dropdown boxes
		{
		/* Elements: */
		public:
		DropdownBox* dropdownBox; // Pointer to the dropdown box that caused the event
		
		/* Constructors and destructors: */
		CallbackData(DropdownBox* sDropdownBox)
			:dropdownBox(sDropdownBox)
			{
			}
		};
	
	class ValueChangedCallbackData:public CallbackData
		{
		/* Elements: */
		public:
		int oldSelectedItem; // Index of the previously selected item
		int newSelectedItem; // Index of the new selected item
		
		/* Constructors and destructors: */
		ValueChangedCallbackData(DropdownBox* sDropdownBox,int sOldSelectedItem,int sNewSelectedItem)
			:CallbackData(sDropdownBox),
			 oldSelectedItem(sOldSelectedItem),
			 newSelectedItem(sNewSelectedItem)
			{
			}
		
		/* Methods: */
		const Widget* getItemWidget(void) const // Returns the new selected item
			{
			return dropdownBox->getItemWidget(newSelectedItem);
			}
		const char* getItem(void) const // Returns the name of the new selected item
			{
			return dropdownBox->getItem(newSelectedItem);
			}
		};
	
	/* Elements: */
	protected:
	Popup* popup; // Window to pop up when button is selected
	RowColumn* items; // Rowcolumn widget containing the list item labels
	bool isPopped; // Flag if the popup window is displayed
	Box popupHitBox; // Extended "hit box" around the popup window to keep it popped even when the interaction pointer moves slightly outside
	Widget* foundChild; // Widget that responded to the last findRecipient call
	Widget* armedChild; // Currently armed widget
	GLfloat spacing; // Spacing between label and arrow glyph
	GlyphGadget arrow; // The dropdown arrow
	GLfloat popupExtrudeSize; // Amount of extrusion for the popup's hit box
	int numItems; // Number of items in the list
	Misc::CallbackList valueChangedCallbacks; // List of callbacks to be called when a different list item is selected
	
	int selectedItem; // Index of currently selected list item
	
	/* Protected methods: */
	static void itemSelectedCallbackWrapper(Misc::CallbackData* callbackData,void* userData); // Callback that is called when a list item is selected
	
	/* Constructors and destructors: */
	public:
	DropdownBox(const char* sName,Container* sParent,bool manageChild =true); // Creates an empty drop-down box
	DropdownBox(const char* sName,Container* sParent,const std::vector<std::string>& sItems,bool manageChild =true); // Creates a drop-down box for the given vector of items
	~DropdownBox(void);
	
	/* Methods inherited from Widget: */
	virtual Vector calcNaturalSize(void) const;
	virtual ZRange calcZRange(void) const;
	virtual void resize(const Box& newExterior);
	virtual void setBackgroundColor(const Color& newBackgroundColor);
	virtual void draw(GLContextData& contextData) const;
	virtual bool findRecipient(Event& event);
	virtual void pointerButtonDown(Event& event);
	virtual void pointerButtonUp(Event& event);
	virtual void pointerMotion(Event& event);
	
	/* New methods: */
	void setSpacing(GLfloat newSpacing);
	void setArrowBorderSize(GLfloat newArrowBorderSize);
	void setArrowSize(GLfloat newArrowSize);
	void setPopupExtrudeSize(GLfloat newPopupExtrudeSize);
	int getNumItems(void) const // Returns the number of items in the drop-down list
		{
		return numItems;
		}
	const Widget* getItemWidget(int item) const; // Returns the widget representing the item of the given index
	const char* getItem(int item) const; // Returns the item of the given index
	void clearItems(void); // Removes all items from the drop-down list
	void addItem(const char* newItem); // Adds an item to the drop-down list
	int getSelectedItem(void) const // Returns the index of the currently selected item
		{
		return selectedItem;
		}
	void setSelectedItem(int newSelectedItem); // Sets the currently selected item
	Misc::CallbackList& getValueChangedCallbacks(void) // Returns list of value changed callbacks
		{
		return valueChangedCallbacks;
		}
	};

}

#endif
