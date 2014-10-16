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

#include <RawHID/Internal/UdevMonitor.h>

#include <unistd.h>
#include <fcntl.h>
#include <libudev.h>
#include <stdexcept>
#include <Misc/ThrowStdErr.h>
#include <RawHID/Internal/UdevContext.h>
#include <RawHID/Internal/UdevDevice.h>

namespace RawHID {

/****************************
Methods of class UdevMonitor:
****************************/

UdevMonitor::UdevMonitor(udev_monitor* sMonitor)
	:monitor(sMonitor),fd(-1),listening(false)
	{
	/* Get the monitor's file descriptor: */
	if(monitor!=0)
		fd=udev_monitor_get_fd(monitor);
	}

UdevMonitor::UdevMonitor(const UdevMonitor& source)
	:monitor(source.monitor),fd(source.fd),listening(source.listening)
	{
	/* Reference the copied monitor: */
	if(monitor!=0)
		udev_monitor_ref(monitor);
	}

UdevMonitor& UdevMonitor::operator=(const UdevMonitor& source)
	{
	if(monitor!=source.monitor)
		{
		/* Unreference the old and reference the new monitor: */
		if(monitor!=0)
			udev_monitor_unref(monitor);
		monitor=source.monitor;
		if(monitor!=0)
			udev_monitor_ref(monitor);
		fd=source.fd;
		listening=source.listening;
		}
	return *this;
	}

UdevMonitor::~UdevMonitor(void)
	{
	/* Release the udev monitor: */
	udev_monitor_unref(monitor);
	}

UdevContext UdevMonitor::getContext(void)
	{
	/* Get the context and reference it: */
	udev* context=udev_monitor_get_udev(monitor);
	udev_ref(context);
	
	/* Return a context object: */
	return UdevContext(context);
	}

void UdevMonitor::addSubsystemFilter(const char* subsystem,const char* deviceType)
	{
	/* Add a subsystem filter: */
	if(udev_monitor_filter_add_match_subsystem_devtype(monitor,subsystem,deviceType)!=0)
		throw std::runtime_error("RawHID::UdevMonitor::addSubsystemFilter: Internal libudev error");
	
	/* If the monitor is already listening, update its filters: */
	if(listening)
		if(udev_monitor_filter_update(monitor)!=0)
			throw std::runtime_error("RawHID::UdevMonitor::addSubsystemFilter: Internal libudev error");
	}

void UdevMonitor::addTagFilter(const char* tag)
	{
	/* Add a tag filter: */
	if(udev_monitor_filter_add_match_tag(monitor,tag)!=0)
		throw std::runtime_error("RawHID::UdevMonitor::addTagFilter: Internal libudev error");
	
	/* If the monitor is already listening, update its filters: */
	if(listening)
		if(udev_monitor_filter_update(monitor)!=0)
			throw std::runtime_error("RawHID::UdevMonitor::addTagFilter: Internal libudev error");
	}

void UdevMonitor::removeFilters(void)
	{
	/* Remove all filters: */
	if(udev_monitor_filter_remove(monitor)!=0)
		throw std::runtime_error("RawHID::UdevMonitor::removeFilters: Internal libudev error");
	
	/* If the monitor is already listening, update its filters: */
	if(listening)
		if(udev_monitor_filter_update(monitor)!=0)
			throw std::runtime_error("RawHID::UdevMonitor::removeFilters: Internal libudev error");
	}

void UdevMonitor::listen(void)
	{
	/* Bail out if the monitor is already listening: */
	if(listening)
		return;
	
	/* Start listening: */
	if(udev_monitor_enable_receiving(monitor)!=0)
		throw std::runtime_error("RawHID::UdevMonitor::listen: Internal libudev error");
	listening=true;
	
	/* Set the monitor's event socket to blocking: */
	int fdFlags=fcntl(fd,F_GETFL);
	fdFlags&=~O_NONBLOCK;
	fcntl(fd,F_SETFL,fdFlags);
	}

UdevDevice UdevMonitor::receiveDeviceEvent(void)
	{
	/* Wait for and receive a device event: */
	udev_device* result=udev_monitor_receive_device(monitor);
	
	/* Return an event object (which might be invalid in case of error or non-blocking event socket): */
	return UdevDevice(result);
	}

}
