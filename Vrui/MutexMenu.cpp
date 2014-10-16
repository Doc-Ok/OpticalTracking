/***********************************************************************
MutexMenu - Class to represent UI menus with mutually exclusive access
by menu interaction tools.
Copyright (c) 2004-2007 Oliver Kreylos

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

#include <Vrui/MutexMenu.h>

namespace Vrui {

/**************************
Methods of class MutexMenu:
**************************/

MutexMenu::MutexMenu(GLMotif::PopupMenu* sMenuPopup)
	:menuPopup(sMenuPopup),
	 activeTool(0)
	{
	}

MutexMenu::~MutexMenu(void)
	{
	/* Nothing to be done, incidentally */
	}

bool MutexMenu::lockMenu(const Tool* tool)
	{
	/* Check if the menu is already locked: */
	if(activeTool!=0)
		return false;
	
	/* Lock the menu for the calling tool: */
	activeTool=tool;
	return true;
	}

void MutexMenu::unlockMenu(const Tool* tool)
	{
	/* Check if called by the currently active tool: */
	if(activeTool==tool)
		{
		/* Unlock the menu: */
		activeTool=0;
		}
	}

}
