/***********************************************************************
LocatorToolAdapter - Adapter class to connect a generic locator tool to
application functionality.
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

#ifndef VRUI_LOCATORTOOLADAPTER_INCLUDED
#define VRUI_LOCATORTOOLADAPTER_INCLUDED

#include <Vrui/LocatorTool.h>

/* Forward declarations: */
namespace Misc {
class CallbackData;
}

namespace Vrui {

class LocatorToolAdapter
	{
	/* Elements: */
	private:
	LocatorTool* tool; // Pointer to locator tool associated with this adapter
	
	/* Constructors and destructors: */
	public:
	LocatorToolAdapter(LocatorTool* sTool); // Creates an adapter connected to the given locator tool
	virtual ~LocatorToolAdapter(void); // Destroys an adapter
	
	/* Methods: */
	LocatorTool* getTool(void) const // Returns pointer to locator tool
		{
		return tool;
		}
	virtual void storeState(Misc::ConfigurationFileSection& configFileSection) const; // Stores the tool adapter's state to the given configuration file
	virtual void getName(std::string& name) const; // Returns a descriptive name for the tool adapter
	virtual void motionCallback(LocatorTool::MotionCallbackData* cbData); // Callback called when moving
	virtual void buttonPressCallback(LocatorTool::ButtonPressCallbackData* cbData); // Callback called when button is pressed
	virtual void buttonReleaseCallback(LocatorTool::ButtonReleaseCallbackData* cbData); // Callback called when button is released
	};

}

#endif
