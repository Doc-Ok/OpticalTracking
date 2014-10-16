/***********************************************************************
UdevEnumerator - Class to enumerate devices managed by a udev context.
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

#ifndef RAWHID_INTERNAL_UDEVENUMERATOR_INCLUDED
#define RAWHID_INTERNAL_UDEVENUMERATOR_INCLUDED

/* Forward declarations: */
struct udev_enumerate;
namespace RawHID {
class UdevContext;
class UdevListIterator;
}

namespace RawHID {

class UdevEnumerator
	{
	/* Elements: */
	private:
	udev_enumerate* enumerator; // Pointer to the low-level udev enumerator
	
	/* Constructors and destructors: */
	public:
	UdevEnumerator(UdevContext& context); // Creates a new udev enumerator in the given udev context
	private:
	UdevEnumerator(const UdevEnumerator& source); // Prohibit copy constructor
	UdevEnumerator& operator=(const UdevEnumerator& source); // Prohibit assignment operator
	public:
	~UdevEnumerator(void); // Destroys the udev enumerator
	
	/* Methods: */
	udev_enumerate* getEnumerator(void) // Returns the low-level enumerator
		{
		return enumerator;
		}
	void addMatchSubsystem(const char* subsystem); // Adds a subsystem to the enumerator's device matcher
	void scanDevices(void); // Scans all managed devices and creates a list of those passing the current device matcher
	UdevListIterator getDevices(void); // Returns the list of devices in the enumerator
	};

}

#endif
