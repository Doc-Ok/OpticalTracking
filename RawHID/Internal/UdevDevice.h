/***********************************************************************
UdevDevice - Class to represent a udev device.
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

#ifndef RAWHID_INTERNAL_UDEVDEVICE_INCLUDED
#define RAWHID_INTERNAL_UDEVDEVICE_INCLUDED

/* Forward declarations: */
struct udev_device;
namespace RawHID {
class UdevContext;
class UdevListIterator;
}

namespace RawHID {

class UdevDevice
	{
	/* Elements: */
	private:
	udev_device* device; // Pointer to the low-level udev device
	
	/* Constructors and destructors: */
	public:
	UdevDevice(void) // Creates an invalid device
		:device(0)
		{
		}
	UdevDevice(udev_device* sDevice) // Creates device from low-level udev device; does not reference
		:device(sDevice)
		{
		}
	UdevDevice(const UdevDevice& source); // Copy constructor
	UdevDevice& operator=(const UdevDevice& source); // Assignment operator
	~UdevDevice(void); // Dereferences the low-level device
	
	/* Methods: */
	bool isValid(void) const // Returns true if the device is valid
		{
		return device!=0;
		}
	udev_device* getDevice(void) // Returns the low-level udev device
		{
		return device;
		}
	UdevContext getContext(void); // Returns the udev context in which the device was created
	const char* getAction(void); // Returns the udev action that happened to a device in a monitor event; returns 0 if there is no action
	const char* getDevnode(void); // Returns the device's device node path
	const char* getSubsystem(void); // Returns the device's subsystem
	const char* getType(void); // Returns the device's type
	UdevDevice getParent(void); // Returns the device's parent
	UdevDevice getParent(const char* subsystem); // Returns the first parent of the device that matches the given subsystem
	UdevDevice getParent(const char* subsystem,const char* deviceType); // Returns the first parent of the device that matches the given subsystem and device type
	const char* getSysattrValue(const char* sysattr); // Returns the value of the device's given system attribute
	UdevListIterator getProperties(void); // Returns the list of the device's properties
	UdevListIterator getTags(void); // Returns the list of the device's tags
	UdevListIterator getSysattrs(void); // Returns the list of the device's system attributes
	};

}

#endif
