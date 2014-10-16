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

#include <RawHID/Internal/UdevListIterator.h>

#include <libudev.h>

namespace RawHID {

/*********************************
Methods of class UdevListIterator:
*********************************/

const char* UdevListIterator::getName(void) const
	{
	return udev_list_entry_get_name(entry);
	}

const char* UdevListIterator::getValue(void) const
	{
	return udev_list_entry_get_value(entry);
	}

UdevListIterator& UdevListIterator::operator++(void)
	{
	/* Advance the iterator: */
	entry=udev_list_entry_get_next(entry);
	
	return *this;
	}

UdevListIterator UdevListIterator::find(const char* name) const
	{
	/* Find the entry: */
	udev_list_entry* foundEntry=udev_list_entry_get_by_name(entry,name);
	
	/* Return a new iterator: */
	return UdevListIterator(foundEntry);
	}

}
