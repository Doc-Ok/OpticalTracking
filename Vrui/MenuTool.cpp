/***********************************************************************
MenuTool - Base class for menu selection tools.
Copyright (c) 2004-2010 Oliver Kreylos

This file is part of the Virtual Reality User Interface Library (Vrui).

The Virtual Reality User Interface Library is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The Virtual Reality User Interface Library is distributed in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Virtual Reality User Interface Library; if not, write to the
Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#include <Vrui/MenuTool.h>

#include <Vrui/MutexMenu.h>
#include <Vrui/ToolManager.h>
#include <Vrui/Internal/Vrui.h>

namespace Vrui {

/********************************
Methods of class MenuToolFactory:
********************************/

MenuToolFactory::MenuToolFactory(ToolManager& toolManager)
	:ToolFactory("MenuTool",toolManager)
	{
	/* Insert class into class hierarchy: */
	ToolFactory* toolFactory=toolManager.loadClass("UserInterfaceTool");
	toolFactory->addChildClass(this);
	addParentClass(toolFactory);
	}

const char* MenuToolFactory::getName(void) const
	{
	return "Menu Handler";
	}

const char* MenuToolFactory::getButtonFunction(int) const
	{
	/* By default, menu tools only use a single button: */
	return "Show Menu";
	}

/*************************
Methods of class MenuTool:
*************************/

bool MenuTool::activate(void)
	{
	/* Can't activate unless there is a menu: */
	if(menu==0)
		return false;
	
	/* Try locking the menu: */
	if(menu->lockMenu(this))
		{
		/* Call activation callbacks: */
		ActivationCallbackData cbData(this);
		activationCallbacks.call(&cbData);
		
		/* Activate the tool: */
		active=true;
		}
	
	return active;
	}

void MenuTool::deactivate(void)
	{
	if(active)
		{
		/* Call deactivation callbacks: */
		DeactivationCallbackData cbData(this);
		deactivationCallbacks.call(&cbData);
		
		/* Deactivate the tool: */
		menu->unlockMenu(this);
		active=false;
		}
	}

MenuTool::MenuTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment)
	:UserInterfaceTool(factory,inputAssignment),
	 menu(0),active(false)
	{
	}

void MenuTool::setMenu(MutexMenu* newMenu)
	{
	menu=newMenu;
	}

}
