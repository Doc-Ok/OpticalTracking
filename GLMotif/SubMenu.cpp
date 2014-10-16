/***********************************************************************
SubMenu - Subclass of RowColumn that only contains buttons and acts as a
sub-menu of another pop-up menu. This class replicates Menu's API but is
not related because it implies a different selection strategy in widget
managers.
Copyright (c) 2007-2008 Oliver Kreylos

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

#include <stdio.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/Event.h>
#include <GLMotif/WidgetManager.h>
#include <GLMotif/Label.h>
#include <GLMotif/Button.h>

#include <GLMotif/SubMenu.h>

namespace GLMotif {

/************************
Methods of class SubMenu:
************************/

void SubMenu::childrenSelectCallbackWrapper(Misc::CallbackData* callbackData,void* userData)
	{
	/* Extract the widget pointers: */
	Button::SelectCallbackData* cbStruct=static_cast<Button::SelectCallbackData*>(callbackData);
	SubMenu* thisPtr=static_cast<SubMenu*>(userData);
	
	/* Call the entry select callbacks: */
	EntrySelectCallbackData cbData(thisPtr,cbStruct->button,cbStruct);
	thisPtr->entrySelectCallbacks.call(&cbData);
	}

SubMenu::SubMenu(const char* sName,Container* sParent,bool sManageChild)
	:RowColumn(sName,sParent,false)
	{
	/* Set default layout for menus: */
	setMarginWidth(0.0f);
	
	/* Manage me: */
	if(sManageChild)
		manageChild();
	}

void SubMenu::addChild(Widget* newChild)
	{
	/* Get the style sheet: */
	const StyleSheet* ss=getStyleSheet();
	
	/* Set the child's border: */
	newChild->setBorderWidth(ss->menuButtonBorderWidth);
	
	/* If the child is a label: Left-align its text */
	Label* newLabel=dynamic_cast<Label*>(newChild);
	if(newLabel!=0)
		newLabel->setHAlignment(GLFont::Left);
	
	/* Call the parent class widget's routine: */
	RowColumn::addChild(newChild);
	
	/* If the child is a button: Set the child's select callback to point to us: */
	Button* newButton=dynamic_cast<Button*>(newChild);
	if(newButton!=0)
		newButton->getSelectCallbacks().add(childrenSelectCallbackWrapper,this);
	}

void SubMenu::addEntry(const char* newEntryLabel)
	{
	/* Create a new button: */
	char newButtonName[40];
	snprintf(newButtonName,sizeof(newButtonName),"_SubMenuButton%d",int(children.size()));
	new Button(newButtonName,this,newEntryLabel);
	}

int SubMenu::getEntryIndex(const Button* entry) const
	{
	int result=-1;
	int index=0;
	for(WidgetList::const_iterator chIt=children.begin();chIt!=children.end();++chIt,++index)
		if(*chIt==entry)
			{
			result=index;
			break;
			}
	
	return result;
	}

}
