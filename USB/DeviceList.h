/***********************************************************************
DeviceList - Class representing lists of USB devices resulting from
device enumeration.
Copyright (c) 2010-2013 Oliver Kreylos

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

#ifndef USB_DEVICELIST_INCLUDED
#define USB_DEVICELIST_INCLUDED

#include <stddef.h>
#include <USB/VendorProductId.h>

/* Forward declarations: */
struct libusb_device;
struct libusb_device_descriptor;
namespace USB {
class Context;
}

namespace USB {

class DeviceList
	{
	/* Elements: */
	private:
	size_t numDevices; // Number of devices in list
	libusb_device** deviceList; // Array of device structure pointers as returned by device enumeration function
	
	/* Constructors and destructors: */
	public:
	DeviceList(void) // Creates empty device list
		:numDevices(0),deviceList(0)
		{
		}
	DeviceList(const Context& context); // Creates a device list by enumerating all devices in the given context
	private:
	DeviceList(const DeviceList& source); // Prohibit copy constructor
	DeviceList& operator=(const DeviceList& source); // Prohibit assignment operator
	public:
	~DeviceList(void); // Destroys the device list
	
	/* Methods: */
	size_t getNumDevices(void) const // Returns the number of devices in the list
		{
		return numDevices;
		}
	libusb_device* getDevice(size_t index) const // Returns the device of the given index
		{
		return deviceList[index];
		}
	libusb_device_descriptor& getDeviceDescriptor(size_t index,libusb_device_descriptor& descriptor) const; // Returns a device descriptor for the device of the given index
	VendorProductId getVendorProductId(size_t index) const; // Returns the USB ID of the device of the given index
	size_t getNumDevices(unsigned short idVendor,unsigned short idProduct) const; // Returns the number of USB devices with the given vendor / product ID
	template <class DeviceMatcherParam>
	size_t getNumDevices(const DeviceMatcherParam& deviceMatcher) const; // Returns the number of USB devices matching the given functor
	libusb_device* getDevice(unsigned short idVendor,unsigned short idProduct,size_t index =0) const; // Returns index-th USB devices with the given vendor / product ID, or NULL
	template <class DeviceMatcherParam>
	libusb_device* getDevice(const DeviceMatcherParam& deviceMatcher,size_t index =0) const; // Returns index-th USB device matching the given functor
	libusb_device* getParent(libusb_device* device) const; // Returns the parent (hub) device of the given device
	};

}

#if !defined(USB_DEVICELIST_IMPLEMENTATION)
#include <USB/DeviceList.icpp>
#endif

#endif
