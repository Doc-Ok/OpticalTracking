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

#ifndef VRUI_MENUTOOL_INCLUDED
#define VRUI_MENUTOOL_INCLUDED

#include <Misc/CallbackData.h>
#include <Misc/CallbackList.h>
#include <Vrui/UserInterfaceTool.h>

/* Forward declarations: */
namespace Vrui {
class MutexMenu;
class ToolManager;
}

namespace Vrui {

class MenuToolFactory:public ToolFactory
	{
	/* Constructors and destructors: */
	public:
	MenuToolFactory(ToolManager& toolManager);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual const char* getButtonFunction(int buttonSlotIndex) const;
	};

class MenuTool:public UserInterfaceTool
	{
	/* Embedded classes: */
	public:
	class CallbackData:public Misc::CallbackData // Base class for menu tool callback data structures
		{
		/* Elements: */
		public:
		MenuTool* tool; // Tool that caused the event
		
		/* Constructors and destructors: */
		CallbackData(MenuTool* sTool)
			:tool(sTool)
			{
			}
		};
	
	class ActivationCallbackData:public CallbackData // Event data structure sent to activation callbacks
		{
		/* Constructors and destructors: */
		public:
		ActivationCallbackData(MenuTool* sTool)
			:CallbackData(sTool)
			{
			}
		};
	
	class DeactivationCallbackData:public CallbackData // Event data structure sent to deactivation callbacks
		{
		/* Constructors and destructors: */
		public:
		DeactivationCallbackData(MenuTool* sTool)
			:CallbackData(sTool)
			{
			}
		};
	
	/* Elements: */
	protected:
	MutexMenu* menu; // Menu associated with this tool
	private:
	Misc::CallbackList activationCallbacks; // List of callbacks for activation events
	Misc::CallbackList deactivationCallbacks; // List of callbacks for deactivation events
	bool active; // Flag if the menu tool is currently active
	
	/* Protected methods: */
	protected:
	bool isActive(void) const // Returns true if the menu tool is currently active
		{
		return active;
		}
	bool activate(void); // Returns true if the menu tool could be activated
	void deactivate(void); // Deactivates the menu tool
	
	/* Constructors and destructors: */
	public:
	MenuTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	
	/* New methods: */
	MutexMenu* getMenu(void) const // Returns the menu associated with this tool
		{
		return menu;
		}
	virtual void setMenu(MutexMenu* newMenu); // Sets the menu associated with this tool
	Misc::CallbackList& getActivationCallbacks(void) // Returns list of activation callbacks
		{
		return activationCallbacks;
		}
	Misc::CallbackList& getDeactivationCallbacks(void) // Returns list of deactivation callbacks
		{
		return deactivationCallbacks;
		}
	};

}

#endif
