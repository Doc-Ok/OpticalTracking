/***********************************************************************
Device - Class representing a human interface device for raw access.
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

#ifndef RAWHID_DEVICE_INCLUDED
#define RAWHID_DEVICE_INCLUDED

#include <string>
#include <Misc/SizedTypes.h>

namespace RawHID {

class Device
	{
	/* Embedded classes: */
	public:
	class DeviceMatcher // Abstract base class to match devices based on bus type and vendor and product ID
		{
		/* Constructors and destructors: */
		public:
		virtual ~DeviceMatcher(void);
		
		/* Methods: */
		virtual bool operator()(int busType,unsigned short vendorId,unsigned short productId) const =0; // Returns true if a device with the given parameters matches the search
		};
	
	typedef Misc::UInt8 Byte; // Type for report data bytes
	
	/* Elements: */
	private:
	int fd; // Device's file descriptor
	int busType; // Type of bus to which the device is connected
	unsigned short vendorId,productId; // Device's vendor/product ID
	std::string serialNumber; // Device's unique serial number
	
	/* Private methods: */
	void initDevice(const char* devnode,int sBusType,unsigned short sVendorId,unsigned short sProductId,const std::string& sSerialNumber); // Initializes the device
	
	/* Constructors and destructors: */
	public:
	Device(int busTypeMask,unsigned short sVendorId,unsigned short sProductId,unsigned int index); // Opens the index-th device matching the given product/vendor ID on any of the given bus types
	Device(const DeviceMatcher& deviceMatcher,unsigned int index); // Opens the index-th device matching the given device matcher
	Device(int busTypeMask,unsigned short sVendorId,unsigned short sProductId,const std::string& sSerialNumber); // Opens the device matching the given product/vendor ID and serial number on any of the given bus types
	Device(const DeviceMatcher& deviceMatcher,const std::string& sSerialNumber); // Opens the device matching the given serial number and device matcher
	private:
	Device(const Device& source); // Prohibit copy constructor
	Device& operator=(const Device& source); // Prohibit assignment operator
	public:
	virtual ~Device(void); // Closes the device
	
	/* Methods: */
	int getFd(void) const // Returns the file descriptor
		{
		return fd;
		}
	int getBusType(void) const // Returns the bus type
		{
		return busType;
		}
	unsigned short getVendorId(void) const // Returns the vendor ID
		{
		return vendorId;
		}
	unsigned short getProductId(void) const // Returns the product ID
		{
		return productId;
		}
	const std::string& getSerialNumber(void) const // Returns the serial number
		{
		return serialNumber;
		}
	
	/* Raw HID report methods: */
	size_t readReport(Byte* report,size_t reportSize); // Reads a raw HID report from the device; if device uses numbered reports, first byte of returned buffer is report number; returns size of read report
	void readSizedReport(Byte* report,size_t reportSize); // Reads a raw HID report from the device; if device uses numbered reports, first byte of returned buffer is report number; throws exception if report size does not match
	void writeReport(const Byte* report,size_t reportSize); // Writes a raw HID report to the device; first byte of report is report number, or 0 if device does not use numbered reports
	size_t readFeatureReport(Byte* report,size_t reportSize); // Reads a raw HID feature report from the device; first byte of report is report number, or 0 if device does not use numbered reports; returns size of read feature report
	void readSizedFeatureReport(Byte* report,size_t reportSize); // Reads a raw HID feature report from the device; first byte of report is report number, or 0 if device does not use numbered reports; throws exception if report size does not match
	void writeFeatureReport(const Byte* report,size_t reportSize); // Writes a raw HID feature report to the device; first byte of report is report number, or 0 if device does not use numbered reports
	};

}

#endif
