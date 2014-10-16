/***********************************************************************
UdevContext - Class to represent a udev device management context, to
locate hardware devices in the file system.
Copyright (c) 2014 Oliver Kreylos

This file is part of the Raw HID Support Library (RawHID).

The Raw HID Support Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Raw HID Support Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Raw HID Support Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef RAWHID_INTERNAL_UDEVCONTEXT_INCLUDED
#define RAWHID_INTERNAL_UDEVCONTEXT_INCLUDED

/* Forward declarations: */
struct udev;
namespace RawHID {
class UdevDevice;
class UdevMonitor;
}

namespace RawHID {

class UdevContext
	{
	friend class UdevEnumerator;
	
	/* Elements: */
	private:
	udev* context; // Pointer to the low-level udev context
	
	/* Constructors and destructors: */
	public:
	UdevContext(void); // Creates a new udev context; throws exception if context cannot be created
	UdevContext(udev* sContext) // Creates context from low-level udev context; does not reference
		:context(sContext)
		{
		}
	UdevContext(const UdevContext& source); // Copy constructor
	UdevContext& operator=(const UdevContext& source); // Assignment operator
	~UdevContext(void); // Destroys the udev context
	
	/* Methods: */
	UdevDevice getDeviceFromSyspath(const char* syspath); // Returns a udev device object for the given path in the sys file system
	UdevMonitor getMonitor(void); // Returns a new udev monitor
	};

}

#endif
