/***********************************************************************
UdevMonitor - Class to represent a udev event monitor, to receive
notification of device plug-in/removal.
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

#ifndef RAWHID_INTERNAL_UDEVMONITOR_INCLUDED
#define RAWHID_INTERNAL_UDEVMONITOR_INCLUDED

#include <Misc/FdSet.h>

/* Forward declarations: */
struct udev_monitor;
namespace RawHID {
class UdevContext;
class UdevDevice;
}

namespace RawHID {

class UdevMonitor
	{
	/* Elements: */
	private:
	udev_monitor* monitor; // Pointer to the low-level udev monitor
	int fd; // The monitor's event file descriptor
	bool listening; // Flag if the monitor is already listening to events
	
	/* Constructors and destructors: */
	public:
	UdevMonitor(void) // Creates an invalid monitor
		:monitor(0),fd(-1),listening(false)
		{
		}
	UdevMonitor(udev_monitor* sMonitor); // Creates monitor from low-level udev monitor; does not reference
	UdevMonitor(const UdevMonitor& source); // Copy constructor
	UdevMonitor& operator=(const UdevMonitor& source); // Assignment operator
	~UdevMonitor(void); // Destroys the udev monitor
	
	/* Methods: */
	bool isValid(void) const // Returns true if the monitor is valid
		{
		return monitor!=0;
		}
	UdevContext getContext(void); // Returns the udev context to which this monitor belongs
	void addSubsystemFilter(const char* subsystem,const char* deviceType); // Adds notifications for the given subsystem and device type
	void addTagFilter(const char* tag); // Adds notifications for the given tag
	void removeFilters(void); // Remove all filters from the monitor
	void listen(void); // Starts listening to events on the selected subsystem(s) or tag(s)
	void addEvent(Misc::FdSet& fdSet) const // Adds the monitor's event socket to the given wait set
		{
		fdSet.add(fd);
		}
	void removeEvent(Misc::FdSet& fdSet) const // Removes the monitor's event socket from the given wait set
		{
		fdSet.remove(fd);
		}
	bool isTriggered(const Misc::FdSet& fdSet) const // Returns true if the monitor's event socket has an event pending in the given wait set
		{
		return fdSet.isSet(fd);
		}
	UdevDevice receiveDeviceEvent(void); // Suspends calling thread until a device event occurs and returns a device structure representing the event with action string set; returns invalid device if there are no pending events, or an error occurred
	};

}

#endif
