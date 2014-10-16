/***********************************************************************
ConfigDescriptor - Class representing a USB configuration descriptor.
Copyright (c) 2010-2011 Oliver Kreylos

This file is part of the USB Support Library (USB).

The USB Support Library is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as published
by the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

The USB Support Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the USB Support Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef USB_CONFIGDESCRIPTOR_INCLUDED
#define USB_CONFIGDESCRIPTOR_INCLUDED

/* Forward declarations: */
struct libusb_config_descriptor;

namespace USB {

class ConfigDescriptor
	{
	/* Elements: */
	private:
	libusb_config_descriptor* descriptor; // Configuration descriptor pointer from the USB library
	
	/* Constructors and destructors: */
	public:
	ConfigDescriptor(libusb_config_descriptor* sDescriptor) // Wraps given descriptor pointer
		:descriptor(sDescriptor)
		{
		}
	private:
	ConfigDescriptor(const ConfigDescriptor& source); // Prohibit copy constructor
	ConfigDescriptor& operator=(const ConfigDescriptor& source); // Prohibit assignment operator
	public:
	~ConfigDescriptor(void); // Destroys the configuration descriptor
	
	/* Methods: */
	libusb_config_descriptor* getDescriptor(void) const // Returns the descriptor pointer
		{
		return descriptor;
		}
	};

}

#endif
