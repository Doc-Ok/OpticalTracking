/***********************************************************************
StyleSheet - Structure providing defaults for the look and feel of user
interface widgets.
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

#ifndef GLMOTIF_STYLESHEET_INCLUDED
#define GLMOTIF_STYLESHEET_INCLUDED

#include <GLMotif/Types.h>

/* Forward declarations: */
class GLFont;

namespace GLMotif {

struct StyleSheet
	{
	/* Elements: */
	public:
	GLfloat size; // Base size for borders, margins, etc.
	Color borderColor; // Border color for general widgets
	Color bgColor; // Background color for general widgets
	Color fgColor; // Foreground color for general widgets
	GLfloat containerMarginWidth; // Width of margin around children in containers
	GLfloat containerSpacing; // Width of spacing between children in containers
	GLfloat menuButtonBorderWidth; // Width of border to assign to button widgets that are children of menus
	GLfloat labelMarginWidth; // Margin width for labels and derived widgets
	GLFont* font; // Default font for widgets displaying text
	GLfloat fontHeight; // Height of default font
	GLfloat textfieldBorderWidth; // Border width for text fields and derived widgets
	GLfloat textfieldMarginWidth; // Margin width for text fields and derived widgets
	Color textfieldBgColor; // Background color for text fields displaying values
	Color textfieldFgColor; // Foreground color for text fields displaying values
	Color selectionBgColor; // Background color for selected text items
	Color selectionFgColor; // Foreground color for selected text items
	GLfloat buttonBorderWidth; // Border width for buttons and derived widgets
	GLfloat buttonMarginWidth; // Margin width for buttons and derived widgets
	Color buttonArmedBackgroundColor; // Background color for armed buttons and derived classes
	GLfloat decoratedButtonSpacing; // Spacing between button text and decoration in decorated buttons and derived widgets
	GLfloat decoratedButtonDecorationSize; // Size of button decoration for decorated buttons and derived widgets
	Color toggleButtonToggleColor; // Color to indicate selected toggle buttons or derived widgets
	GLfloat popupMarginWidth; // Margin width for popups and derived classes
	GLfloat popupTitleSpacing; // Spacing between title and child in popups and derived classes
	GLfloat titlebarBorderWidth; // Width of border around title bars
	Color titlebarBgColor; // Background color for title bars
	Color titlebarFgColor; // Foreground color for title bars
	GLfloat popupWindowChildBorderWidth; // Border width for child windows of popup windows and derived widgets 
	GLfloat sliderMarginWidth; // Width of margin around slider and handle in sliders and derived widgets
	GLfloat sliderHandleWidth; // Width of slider handle (assuming vertical slider)
	GLfloat sliderHandleLength; // Length of slider handle along slider axis
	GLfloat sliderHandleHeight; // Height (in z direction) of slider handle
	Color sliderHandleColor; // Color for slider handles
	GLfloat sliderShaftWidth; // Width of slider shaft (assuming vertical slider)
	GLfloat sliderShaftDepth; // Depth (in z direction) of slider shaft
	Color sliderShaftColor; // Color for slider shafts
	double multiClickTime; // Maximum multi-click time interval in seconds
	
	/* Constructors and destructors: */
	StyleSheet(void); // Creates an uninitialized style sheet
	
	/* Methods: */
	void setFont(GLFont* sFont); // Initializes the style sheet with reasonable defaults based on the given font
	void setSize(GLfloat newSize); // Sets style sheet settings depending on a general size value
	};

}

#endif
