/***********************************************************************
UdevContext - Class to represent a udev device management context, to
locate hardware devices in the file system.
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

#include <RawHID/Internal/UdevContext.h>

#include <libudev.h>
#include <stdexcept>
#include <Misc/ThrowStdErr.h>
#include <RawHID/Internal/UdevDevice.h>
#include <RawHID/Internal/UdevMonitor.h>

namespace RawHID {

/****************************
Methods of class UdevContext:
****************************/

UdevContext::UdevContext(void)
	:context(0)
	{
	/* Create a new udev context: */
	context=udev_new();
	if(context==0)
		throw std::runtime_error("RawHID::UdevContext::UdevContext: Cannot create udev context");
	}

UdevContext::UdevContext(const UdevContext& source)
	:context(source.context)
	{
	/* Reference the copied context: */
	if(context!=0)
		udev_ref(context);
	}

UdevContext& UdevContext::operator=(const UdevContext& source)
	{
	if(context!=source.context)
		{
		/* Unreference the old and reference the new context: */
		if(context!=0)
			udev_unref(context);
		context=source.context;
		if(context!=0)
			udev_ref(context);
		}
	return *this;
	}

UdevContext::~UdevContext(void)
	{
	/* Release the udev context: */
	udev_unref(context);
	}

UdevDevice UdevContext::getDeviceFromSyspath(const char* syspath)
	{
	udev_device* result=udev_device_new_from_syspath(context,syspath);
	if(result==0)
		Misc::throwStdErr("RawHID::UdevContext::getDeviceFromSyspath: Device for syspath %s not found",syspath);
	return UdevDevice(result);
	}

UdevMonitor UdevContext::getMonitor(void)
	{
	udev_monitor* result=udev_monitor_new_from_netlink(context,"udev");
	if(result==0)
		throw std::runtime_error("RawHID::UdevContext::getMonitor: Cannot create udev monitor");
	return UdevMonitor(result);
	}

}
