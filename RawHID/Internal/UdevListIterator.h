/***********************************************************************
UdevListIterator - Class to iterate through lists of udev name/value
pairs.
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

#ifndef RAWHID_INTERNAL_UDEVLISTITERATOR_INCLUDED
#define RAWHID_INTERNAL_UDEVLISTITERATOR_INCLUDED

/* Forward declarations: */
struct udev_list_entry;

namespace RawHID {

class UdevListIterator
	{
	/* Elements: */
	private:
	udev_list_entry* entry; // Pointer to the current list entry
	
	/* Constructors and destructors: */
	public:
	UdevListIterator(void) // Creates iterator to end of list
		:entry(0)
		{
		}
	UdevListIterator(udev_list_entry* sEntry) // Creates iterator for the given udev list
		:entry(sEntry)
		{
		}
	
	/* Methods: */
	bool isValid(void) const // Returns true if the iterator points to a valid list entry
		{
		return entry!=0;
		}
	const char* getName(void) const; // Returns the name of the pointed-to entry
	const char* getValue(void) const; // Returns the value of the pointed-to entry
	UdevListIterator& operator++(void); // Increments the iterator to the next list entry
	UdevListIterator find(const char* name) const; // Returns a new iterator pointing to the entry of the given name, or the end of the list
	};

}

#endif
