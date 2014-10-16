/***********************************************************************
TitleBar - Class for title bars used to move popup windows in 3D space.
Mostly a wrapper class around label; moving functionality must be
mplemented by the surrounding toolkit in collaboration with a widget
manager.
Copyright (c) 2005 Oliver Kreylos

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

#include <GLMotif/StyleSheet.h>

#include <GLMotif/TitleBar.h>

namespace GLMotif {

/*************************
Methods of class TitleBar:
*************************/

TitleBar::TitleBar(const char* sName,Container* sParent,const char* sLabel,const GLFont* sFont,bool sManageChild)
	:Label(sName,sParent,sLabel,sFont,false)
	{
	/* Get the style sheet: */
	const StyleSheet* ss=getStyleSheet();
	
	/* TitleBar defaults to raised border: */
	setBorderType(Widget::RAISED);
	setBorderWidth(ss->titlebarBorderWidth);
	
	setBorderColor(ss->titlebarBgColor);
	setBackgroundColor(ss->titlebarBgColor);
	setForegroundColor(ss->titlebarFgColor);
	
	setMarginWidth(ss->titlebarBorderWidth);
	setHAlignment(GLFont::Left);
	
	/* Manage me: */
	if(sManageChild)
		manageChild();
	}

TitleBar::TitleBar(const char* sName,Container* sParent,const char* sLabel,bool sManageChild)
	:Label(sName,sParent,sLabel,false)
	{
	/* Get the style sheet: */
	const StyleSheet* ss=getStyleSheet();
	
	/* TitleBar defaults to raised border: */
	setBorderType(Widget::RAISED);
	setBorderWidth(ss->titlebarBorderWidth);
	
	setBorderColor(ss->titlebarBgColor);
	setBackgroundColor(ss->titlebarBgColor);
	setForegroundColor(ss->titlebarFgColor);
	
	setMarginWidth(ss->titlebarBorderWidth);
	setHAlignment(GLFont::Left);
	
	/* Manage me: */
	if(sManageChild)
		manageChild();
	}

}
