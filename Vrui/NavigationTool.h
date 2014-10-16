/***********************************************************************
NavigationTool - Base class for navigation tools.
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

#ifndef VRUI_NAVIGATIONTOOL_INCLUDED
#define VRUI_NAVIGATIONTOOL_INCLUDED

#include <Vrui/Tool.h>

/* Forward declarations: */
namespace Vrui {
class ToolManager;
}

namespace Vrui {

class NavigationToolFactory:public ToolFactory
	{
	/* Constructors and destructors: */
	public:
	NavigationToolFactory(ToolManager& toolManager);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual const char* getButtonFunction(int buttonSlotIndex) const;
	virtual const char* getValuatorFunction(int valuatorSlotIndex) const;
	};

class NavigationTool:public Tool
	{
	/* Elements: */
	private:
	bool active; // Flag if this tool is the currently active navigation tool
	
	/* Constructors and destructors: */
	public:
	NavigationTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	virtual ~NavigationTool(void);
	
	/* New methods: */
	bool isActive(void) const // Returns true if the navigation tool is currently active
		{
		return active;
		}
	bool activate(void); // Activates the tool and returns true if successful
	void deactivate(void); // Deactivates the tool
	};

}

#endif
