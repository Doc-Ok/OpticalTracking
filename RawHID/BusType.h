/***********************************************************************
BusType - Enumerated type for bus types to which a human interface
device can be connected.
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

#ifndef RAWHID_BUSTYPE_INCLUDED
#define RAWHID_BUSTYPE_INCLUDED

namespace RawHID {

enum BusType
	{
	BUSTYPE_NONE=0x0,
	BUSTYPE_USB=0x1,
	BUSTYPE_HIL=0x2,
	BUSTYPE_BLUETOOTH=0x4,
	BUSTYPE_VIRTUAL=0x8,
	BUSTYPE_ANY=0xf
	};

}

#endif
