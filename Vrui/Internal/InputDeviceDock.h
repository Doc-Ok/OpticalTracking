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

#ifndef VRUI_INTERNAL_INPUTDEVICEDOCK_INCLUDED
#define VRUI_INTERNAL_INPUTDEVICEDOCK_INCLUDED

#if 0 // Not yet done

#include <vector>
#include <GLMotif/WidgetManager.h>
#include <GLMotif/Widget.h>

namespace Vrui {

class InputDeviceDock:public GLMotif::Widget
	{
	/* Embedded classes: */
	private:
	struct DeviceSlot // Structure to represent a device slot
		{
		/* Elements: */
		public:
		InputDevice* device; // Pointer to this slot's input device
		bool docked; // Flag if this slot's device is currently docked in the slot
		GLMotif::Box box; // Slot's bounding box in current widget layout
		};
	
	/* Elements: */
	private:
	std::vector<DeviceSlot> deviceSlots; // List of device slots
	int numColumns; // Number of device slots per row in current widget layout
	GLMotif::WidgetManager::Transformation transformation; // Current transformation from widget space to world space
	
	/* Constructors and destructors: */
	public:
	InputDeviceDock(const char* sName,GLMotif::Container* sParent,bool sManageChild =true);
	virtual ~InputDeviceDock(void);
	
	#if 0
	/* Methods from GLMotif::Widget: */
	virtual GLMotif::Vector calcNaturalSize(void) const;
	virtual GLMotif::ZRange calcZRange(void) const;
	virtual void resize(const GLMotif::Box& newExterior);
	virtual void draw(GLContextData& contextData) const;
	#endif
	};

}

#endif

#endif
