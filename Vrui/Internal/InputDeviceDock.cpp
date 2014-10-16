/***********************************************************************
InputDeviceDock - GLMotif widget class to "dock" virtual input devices
in a dialog window for quick access to pre-configured tools.
Copyright (c) 2010 Oliver Kreylos

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

#if 0 // Not yet done

#include <Vrui/Internal/InputDeviceDock.h>

#include <Vrui/Vrui.h>
#include <Vrui/InputDeviceManager.h>
#include <Vrui/InputGraphManager.h>

namespace Vrui {

/********************************
Methods of class InputDeviceDock:
********************************/

InputDeviceDock::InputDeviceDock(const char* sName,GLMotif::Container* sParent,bool sManageChild)
	:GLMotif::Widget(sName,sParent,false),
	 numColumns(1),transformation(GLMotif::WidgetManager::Transformation::identity)
	{
	/* Create the special "shredder" slot: */
	DeviceSlot shredder;
	shredder.device=0;
	shredder.docked=true;
	deviceSlots.push_back(shredder);
	
	/* Create the initial set of device slots: */
	for(int i=0;i<3;++i)
		{
		DeviceSlot newSlot;
		
		/* Create a new device and grab it: */
		newSlot.device=Vrui::addVirtualInputDevice("DockDevice",1,0);
		
		
		newSlot.docked=true;
		deviceSlots.push_back(newSlot);
		}
	
	/* Manage me: */
	if(sManageChild)
		manageChild();
	}

InputDeviceDock::~InputDeviceDock(void)
	{
	}

}

#endif
