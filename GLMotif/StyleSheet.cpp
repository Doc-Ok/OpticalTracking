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

#include <GL/GLFont.h>

#include <GLMotif/StyleSheet.h>

namespace GLMotif {

/***************************
Methods of class StyleSheet:
***************************/

StyleSheet::StyleSheet(void)
	:font(0)
	{
	borderColor=Color(0.667f,0.667f,0.667f);
	bgColor=Color(0.667f,0.667f,0.667f);
	fgColor=Color(0.0f,0.0f,0.0f);
	textfieldBgColor=Color(1.0f,1.0f,0.5f);
	textfieldFgColor=Color(0.0f,0.0f,0.0f);
	selectionBgColor=Color(0.25f,0.25f,0.25f);
	selectionFgColor=Color(1.0f,1.0f,1.0f);
	buttonArmedBackgroundColor=Color(0.5f,0.5f,0.5f);
	toggleButtonToggleColor=Color(0.0f,1.0f,0.0f);
	titlebarBgColor=Color(0.0f,0.0f,0.667f);
	titlebarFgColor=Color(1.0f,1.0f,1.0f);
	sliderHandleColor=Color(0.75f,0.75f,0.75f);
	sliderShaftColor=Color(0.333f,0.333f,0.333f);
	multiClickTime=0.5;
	}

void StyleSheet::setFont(GLFont* sFont)
	{
	font=sFont;
	fontHeight=font->getTextHeight();
	setSize(fontHeight*0.5f);
	}

void StyleSheet::setSize(GLfloat newSize)
	{
	size=newSize;
	containerMarginWidth=0.0f;
	containerSpacing=size;
	menuButtonBorderWidth=0.0f; // size*0.25f;
	labelMarginWidth=0.0f;
	textfieldBorderWidth=size*0.5f;
	textfieldMarginWidth=size*0.5f;
	buttonBorderWidth=size*0.5f;
	buttonMarginWidth=size*0.5f;
	decoratedButtonSpacing=size*0.5f;
	decoratedButtonDecorationSize=size*0.5f;
	popupMarginWidth=size;
	popupTitleSpacing=size;
	titlebarBorderWidth=size*0.5f;
	popupWindowChildBorderWidth=size;
	sliderMarginWidth=(fontHeight+size)*0.25f;
	sliderHandleWidth=fontHeight+size;
	sliderHandleLength=sliderHandleWidth*0.5f;
	sliderHandleHeight=sliderHandleWidth*0.5f;
	sliderShaftWidth=size*0.5f;
	sliderShaftDepth=size*0.5f;
	}

}
