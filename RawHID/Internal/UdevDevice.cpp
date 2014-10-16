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

#include <RawHID/Internal/UdevDevice.h>

#include <libudev.h>

#include <RawHID/Internal/UdevContext.h>
#include <RawHID/Internal/UdevListIterator.h>

namespace RawHID {

/***************************
Methods of class UdevDevice:
***************************/

UdevDevice::UdevDevice(const UdevDevice& source)
	:device(source.device)
	{
	/* Reference the device: */
	if(device!=0)
		udev_device_ref(device);
	}

UdevDevice& UdevDevice::operator=(const UdevDevice& source)
	{
	if(device!=source.device)
		{
		if(device!=0)
			udev_device_unref(device);
		device=source.device;
		if(device!=0)
			udev_device_ref(device);
		}
	
	return *this;
	}

UdevDevice::~UdevDevice(void)
	{
	if(device!=0)
		udev_device_unref(device);
	}

UdevContext UdevDevice::getContext(void)
	{
	/* Get the udev context and reference it: */
	udev* context=udev_device_get_udev(device);
	udev_ref(context);
	
	/* Return a context object: */
	return UdevContext(context);
	}

const char* UdevDevice::getAction(void)
	{
	return udev_device_get_action(device);
	}

const char* UdevDevice::getDevnode(void)
	{
	return udev_device_get_devnode(device);
	}

const char* UdevDevice::getSubsystem(void)
	{
	return udev_device_get_subsystem(device);
	}

const char* UdevDevice::getType(void)
	{
	return udev_device_get_devtype(device);
	}

UdevDevice UdevDevice::getParent(void)
	{
	/* Get the parent device: */
	udev_device* parent=udev_device_get_parent(device);
	
	/* The parent device is not supposed to be unref'ed, so let's explicitly ref it here so that a later unref won't destroy it: */
	if(parent!=0)
		udev_device_ref(parent);
	
	return UdevDevice(parent);
	}

UdevDevice UdevDevice::getParent(const char* subsystem)
	{
	/* Get the parent device: */
	udev_device* parent=udev_device_get_parent_with_subsystem_devtype(device,subsystem,0);
	
	/* The parent device is not supposed to be unref'ed, so let's explicitly ref it here so that a later unref won't hurt: */
	if(parent!=0)
		udev_device_ref(parent);
	
	return UdevDevice(parent);
	}

UdevDevice UdevDevice::getParent(const char* subsystem,const char* deviceType)
	{
	/* Get the parent device: */
	udev_device* parent=udev_device_get_parent_with_subsystem_devtype(device,subsystem,deviceType);
	
	/* The parent device is not supposed to be unref'ed, so let's explicitly ref it here so that a later unref won't hurt: */
	if(parent!=0)
		udev_device_ref(parent);
	
	return UdevDevice(parent);
	}

const char* UdevDevice::getSysattrValue(const char* sysattr)
	{
	return udev_device_get_sysattr_value(device,sysattr);
	}

UdevListIterator UdevDevice::getProperties(void)
	{
	return UdevListIterator(udev_device_get_properties_list_entry(device));
	}

UdevListIterator UdevDevice::getTags(void)
	{
	return UdevListIterator(udev_device_get_tags_list_entry(device));
	}

UdevListIterator UdevDevice::getSysattrs(void)
	{
	return UdevListIterator(udev_device_get_sysattr_list_entry(device));
	}

}
