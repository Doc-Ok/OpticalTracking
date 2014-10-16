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

#include <RawHID/Device.h>

#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libudev.h>
#include <linux/input.h>
#include <linux/hidraw.h>
#include <iostream>
#include <stdexcept>
#include <Misc/ThrowStdErr.h>
#include <RawHID/BusType.h>

#include <RawHID/Internal/UdevContext.h>
#include <RawHID/Internal/UdevListIterator.h>
#include <RawHID/Internal/UdevEnumerator.h>
#include <RawHID/Internal/UdevDevice.h>

namespace RawHID {

/**************************************
Methods of class Device::DeviceMatcher:
**************************************/

Device::DeviceMatcher::~DeviceMatcher(void)
	{
	}

namespace {

/****************
Helper functions:
****************/

unsigned int readHex(const char*& cPtr)
	{
	unsigned int result=0;
	while((*cPtr>='0'&&*cPtr<='9')||(*cPtr>='A'&&*cPtr<='F')||(*cPtr>='a'&&*cPtr<='f'))
		{
		result<<=4;
		if(*cPtr>='0'&&*cPtr<='9')
			result|=(unsigned int)(*cPtr-'0');
		else if(*cPtr>='A'&&*cPtr<='F')
			result|=(unsigned int)(*cPtr-'A'+10);
		else
			result|=(unsigned int)(*cPtr-'a'+10);
		++cPtr;
		}
	
	return result;
	}

inline BusType convertBusType(int hidBusType)
	{
	switch(hidBusType)
		{
		case BUS_USB:
			return BUSTYPE_USB;
		
		case BUS_HIL:
			return BUSTYPE_HIL;
		
		case BUS_BLUETOOTH:
			return BUSTYPE_BLUETOOTH;
		
		case BUS_VIRTUAL:
			return BUSTYPE_VIRTUAL;
		
		default:
			return BUSTYPE_NONE;
		}
	}

void parseUevent(const char* uevent,BusType& busType,unsigned short& vendorId,unsigned short& productId,std::string& serialNumber)
	{
	busType=BUSTYPE_NONE;
	vendorId=0x0000U;
	productId=0x0000U;
	serialNumber.clear();
	
	while(*uevent!='\0')
		{
		/* Find the end of the key (lines are of form "KEY=<value>"): */
		const char* sPtr;
		for(sPtr=uevent;*sPtr!='\0'&&*sPtr!='\n'&&*sPtr!='=';++sPtr)
			;
		if(*sPtr=='=')
			{
			/* Check the key: */
			if(sPtr-uevent==6&&memcmp(uevent,"HID_ID",6)==0)
				{
				/* Read the HID device's bus type and vendor/product ID: */
				
				/* Skip the '=': */
				++sPtr;
				
				/* Read the bus type and convert it to a RawHID library bus type: */
				busType=convertBusType(readHex(sPtr));
				if(*sPtr==':')
					{
					++sPtr;
					vendorId=(unsigned short)(readHex(sPtr));
					if(*sPtr==':')
						{
						++sPtr;
						productId=(unsigned short)(readHex(sPtr));
						}
					}
				}
			else if(sPtr-uevent==8&&memcmp(uevent,"HID_UNIQ",8)==0)
				{
				/* Read the HID device's serial number: */
				for(++sPtr;*sPtr!='\0'&&*sPtr!='\n';++sPtr)
					serialNumber.push_back(*sPtr);
				}
			}
		
		/* Skip the current line: */
		while(*sPtr!='\0'&&*sPtr!='\n')
			++sPtr;
		if(*sPtr=='\n')
			++sPtr;
		uevent=sPtr;
		}
	}

}

/***********************
Methods of class Device:
***********************/

void Device::initDevice(const char* devnode,int sBusType,unsigned short sVendorId,unsigned short sProductId,const std::string& sSerialNumber)
	{
	/* Try opening the device node file: */
	fd=open(devnode,O_RDWR);
	if(fd<0)
		{
		int err=errno;
		Misc::throwStdErr("RawHID::Device::Device: Cannot open device node file %s due to error %s",devnode,strerror(err));
		}
	
	/* Initialize device state: */
	busType=sBusType;
	vendorId=sVendorId;
	productId=sProductId;
	serialNumber=sSerialNumber;
	}

Device::Device(int busTypeMask,unsigned short sVendorId,unsigned short sProductId,unsigned int index)
	:fd(-1)
	{
	/* Enumerate all devices on the rawhid subsystem: */
	UdevContext context;
	UdevEnumerator enumerator(context);
	enumerator.addMatchSubsystem("hidraw");
	enumerator.scanDevices();
	
	/* Check all items in the enumerator against the given vendor/product ID: */
	for(UdevListIterator dlIt=enumerator.getDevices();dlIt.isValid();++dlIt)
		{
		/* Get the udev device for the current list entry: */
		UdevDevice rawhid=context.getDeviceFromSyspath(dlIt.getName());
		
		/* Get the raw HID device's HID parent: */
		UdevDevice hid=rawhid.getParent("hid");
		if(hid.isValid())
			{
			/* Get the bus type and vendor/product ID from the HID device's uevent sysattr value: */
			BusType devBusType;
			unsigned short devVendorId,devProductId;
			std::string devSerialNumber;
			const char* uevent=hid.getSysattrValue("uevent");
			parseUevent(uevent,devBusType,devVendorId,devProductId,devSerialNumber);
			
			/* Check if the device matches the search parameters: */
			if((busTypeMask&devBusType)!=0x0&&devVendorId==sVendorId&&devProductId==sProductId)
				{
				/* Check if this is the requested device: */
				if(index==0)
					{
					/* Initialize the device: */
					initDevice(rawhid.getDevnode(),devBusType,devVendorId,devProductId,devSerialNumber);
					
					/* Stop searching: */
					break;
					}
				else
					{
					/* Keep looking: */
					--index;
					}
				}
			}
		}
	
	/* Check if device was found: */
	if(fd<0)
		Misc::throwStdErr("RawHID::Device::Device: Device not found");
	}

Device::Device(const Device::DeviceMatcher& deviceMatcher,unsigned int index)
	:fd(-1)
	{
	/* Enumerate all devices on the rawhid subsystem: */
	UdevContext context;
	UdevEnumerator enumerator(context);
	enumerator.addMatchSubsystem("hidraw");
	enumerator.scanDevices();
	
	/* Check all items in the enumerator against the given vendor/product ID: */
	for(UdevListIterator dlIt=enumerator.getDevices();dlIt.isValid();++dlIt)
		{
		/* Get the udev device for the current list entry: */
		UdevDevice rawhid=context.getDeviceFromSyspath(dlIt.getName());
		
		/* Get the raw HID device's HID parent: */
		UdevDevice hid=rawhid.getParent("hid");
		if(hid.isValid())
			{
			/* Get the bus type and vendor/product ID from the HID device's uevent sysattr value: */
			BusType devBusType;
			unsigned short devVendorId,devProductId;
			std::string devSerialNumber;
			const char* uevent=hid.getSysattrValue("uevent");
			parseUevent(uevent,devBusType,devVendorId,devProductId,devSerialNumber);
			
			/* Check if the device matches the search parameters: */
			if(deviceMatcher(devBusType,devVendorId,devProductId))
				{
				/* Check if this is the requested device: */
				if(index==0)
					{
					/* Initialize the device: */
					initDevice(rawhid.getDevnode(),devBusType,devVendorId,devProductId,devSerialNumber);
					
					/* Stop searching: */
					break;
					}
				else
					{
					/* Keep looking: */
					--index;
					}
				}
			}
		}
	
	/* Check if device was found: */
	if(fd<0)
		Misc::throwStdErr("RawHID::Device::Device: Device not found");
	}

Device::Device(int busTypeMask,unsigned short sVendorId,unsigned short sProductId,const std::string& sSerialNumber)
	:fd(-1)
	{
	/* Enumerate all devices on the rawhid subsystem: */
	UdevContext context;
	UdevEnumerator enumerator(context);
	enumerator.addMatchSubsystem("hidraw");
	enumerator.scanDevices();
	
	/* Check all items in the enumerator against the given vendor/product ID: */
	for(UdevListIterator dlIt=enumerator.getDevices();dlIt.isValid();++dlIt)
		{
		/* Get the udev device for the current list entry: */
		UdevDevice rawhid=context.getDeviceFromSyspath(dlIt.getName());
		
		/* Get the raw HID device's HID parent: */
		UdevDevice hid=rawhid.getParent("hid");
		if(hid.isValid())
			{
			/* Get the bus type and vendor/product ID from the HID device's uevent sysattr value: */
			BusType devBusType;
			unsigned short devVendorId,devProductId;
			std::string devSerialNumber;
			const char* uevent=hid.getSysattrValue("uevent");
			parseUevent(uevent,devBusType,devVendorId,devProductId,devSerialNumber);
			
			/* Check if the device matches the search parameters: */
			if((busTypeMask&devBusType)!=0x0&&devVendorId==sVendorId&&devProductId==sProductId&&devSerialNumber==serialNumber)
				{
				/* Initialize the device: */
				initDevice(rawhid.getDevnode(),devBusType,devVendorId,devProductId,devSerialNumber);
				
				/* Stop searching: */
				break;
				}
			}
		}
	
	/* Check if device was found: */
	if(fd<0)
		Misc::throwStdErr("RawHID::Device::Device: Device not found");
	}

Device::Device(const Device::DeviceMatcher& deviceMatcher,const std::string& sSerialNumber)
	:fd(-1)
	{
	/* Enumerate all devices on the rawhid subsystem: */
	UdevContext context;
	UdevEnumerator enumerator(context);
	enumerator.addMatchSubsystem("hidraw");
	enumerator.scanDevices();
	
	/* Check all items in the enumerator against the given vendor/product ID: */
	for(UdevListIterator dlIt=enumerator.getDevices();dlIt.isValid();++dlIt)
		{
		/* Get the udev device for the current list entry: */
		UdevDevice rawhid=context.getDeviceFromSyspath(dlIt.getName());
		
		/* Get the raw HID device's HID parent: */
		UdevDevice hid=rawhid.getParent("hid");
		if(hid.isValid())
			{
			/* Get the bus type and vendor/product ID from the HID device's uevent sysattr value: */
			BusType devBusType;
			unsigned short devVendorId,devProductId;
			std::string devSerialNumber;
			const char* uevent=hid.getSysattrValue("uevent");
			parseUevent(uevent,devBusType,devVendorId,devProductId,devSerialNumber);
			
			/* Check if the device matches the search parameters: */
			if(deviceMatcher(devBusType,devVendorId,devProductId)&&devSerialNumber==serialNumber)
				{
				/* Initialize the device: */
				initDevice(rawhid.getDevnode(),devBusType,devVendorId,devProductId,devSerialNumber);
				
				/* Stop searching: */
				break;
				}
			}
		}
	
	/* Check if device was found: */
	if(fd<0)
		Misc::throwStdErr("RawHID::Device::Device: Device not found");
	}

Device::~Device(void)
	{
	/* Close the device file: */
	close(fd);
	}

size_t Device::readReport(Device::Byte* report,size_t reportSize)
	{
	ssize_t readResult=read(fd,report,reportSize);
	if(readResult<0)
		{
		int err=errno;
		Misc::throwStdErr("RawHID::Device::readReport: Error %s",strerror(err));
		}
	return size_t(readResult);
	}

void Device::readSizedReport(Device::Byte* report,size_t reportSize)
	{
	ssize_t readResult=read(fd,report,reportSize);
	if(readResult<0)
		{
		int err=errno;
		Misc::throwStdErr("RawHID::Device::readSizedReport: Error %s",strerror(err));
		}
	else if(size_t(readResult)!=reportSize)
		throw std::runtime_error("RawHID::Device::readSizedReport: Truncated read");
	}

void Device::writeReport(const Device::Byte* report,size_t reportSize)
	{
	ssize_t writeResult=write(fd,report,reportSize);
	if(writeResult<0)
		{
		int err=errno;
		Misc::throwStdErr("RawHID::Device::writeReport: Error %s",strerror(err));
		}
	else if(size_t(writeResult)!=reportSize)
		Misc::throwStdErr("RawHID::Device::writeReport: Short write, %u instead of %u",size_t(writeResult),reportSize);
	}

size_t Device::readFeatureReport(Device::Byte* report,size_t reportSize)
	{
	int ioctlResult=ioctl(fd,HIDIOCGFEATURE(reportSize),report);
	if(ioctlResult<0)
		{
		int err=errno;
		Misc::throwStdErr("RawHID::Device::readFeatureReport: Error %s",strerror(err));
		}
	return size_t(ioctlResult);
	}

void Device::readSizedFeatureReport(Device::Byte* report,size_t reportSize)
	{
	int ioctlResult=ioctl(fd,HIDIOCGFEATURE(reportSize),report);
	if(ioctlResult<0)
		{
		int err=errno;
		Misc::throwStdErr("RawHID::Device::readSizedFeatureReport: Error %s",strerror(err));
		}
	else if(size_t(ioctlResult)!=reportSize)
		throw std::runtime_error("RawHID::Device::readSizedFeatureReport: Truncated read");
	}

void Device::writeFeatureReport(const Device::Byte* report,size_t reportSize)
	{
	int ioctlResult=ioctl(fd,HIDIOCSFEATURE(reportSize),report);
	if(ioctlResult<0)
		{
		int err=errno;
		Misc::throwStdErr("RawHID::Device::writeFeatureReport: Error %s",strerror(err));
		}
	else if(size_t(ioctlResult)!=reportSize)
		Misc::throwStdErr("RawHID::Device::writeFeatureReport: Short write, %u instead of %u",size_t(ioctlResult),reportSize);
	}

}
