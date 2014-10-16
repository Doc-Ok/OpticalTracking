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

#ifndef VRUI_MUTEXMENU_INCLUDED
#define VRUI_MUTEXMENU_INCLUDED

/* Forward declarations: */
namespace GLMotif {
class PopupMenu;
}
namespace Vrui {
class Tool;
}

namespace Vrui {

class MutexMenu
	{
	/* Elements: */
	private:
	GLMotif::PopupMenu* menuPopup; // Pointer to the top level menu widget
	const Tool* activeTool; // Pointer to the tool currently "owning" the menu
	
	/* Constructors and destructors: */
	public:
	MutexMenu(GLMotif::PopupMenu* sMenuPopup); // Creates a mutex menu shell for the given menu widget
	~MutexMenu(void); // Destroys the mutex menu shell
	
	/* Methods: */
	GLMotif::PopupMenu* getPopup(void) const // Returns the menu widget associated with the shell
		{
		return menuPopup;
		}
	bool lockMenu(const Tool* tool); // Returns true if the given tool can have access to the menu
	void unlockMenu(const Tool* tool); // Releases access to the menu if called by the currently active tool
	const Tool* getActiveTool(void) const // Returns the currently active tool
		{
		return activeTool;
		}
	};

}

#endif
