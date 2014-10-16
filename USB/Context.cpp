/***********************************************************************
Context - Class representing libusb library contexts.
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

#include <USB/Context.h>

#include <libusb-1.0/libusb.h>
#include <Misc/ThrowStdErr.h>

namespace USB {

/************************
Methods of class Context:
************************/

void* Context::eventHandlingThreadMethod(void)
	{
	Threads::Thread::setCancelState(Threads::Thread::CANCEL_ENABLE);
	// Threads::Thread::setCancelType(Threads::Thread::CANCEL_ASYNCHRONOUS);
	
	while(true)
		{
		/* Block until the next USB event and handle it: */
		libusb_handle_events(context);
		}
	
	return 0;
	}

Context::Context(void)
	:context(0)
	{
	/* Initialize the context: */
	if(libusb_init(&context)!=0)
		Misc::throwStdErr("USB::Context::Context: Error initializing USB context");
	}

Context::~Context(void)
	{
	if(!eventHandlingThread.isJoined())
		{
		/* Stop the event handling thread: */
		eventHandlingThread.cancel();
		eventHandlingThread.join();
		}
	
	/* Destroy the context: */
	if(context!=0)
		libusb_exit(context);
	}

void Context::setDebugLevel(int newDebugLevel)
	{
	libusb_set_debug(context,newDebugLevel);
	}

void Context::startEventHandling(void)
	{
	if(eventHandlingThread.isJoined())
		{
		/* Start the event handling thread: */
		eventHandlingThread.start(this,&Context::eventHandlingThreadMethod);
		}
	}

void Context::stopEventHandling(void)
	{
	if(!eventHandlingThread.isJoined())
		{
		/* Stop the event handling thread: */
		eventHandlingThread.cancel();
		eventHandlingThread.join();
		}
	}

void Context::processEvents(void)
	{
	/* Check if events are already handled in the background: */
	if(eventHandlingThread.isJoined())
		{
		/* Block until the next USB event and handle it: */
		libusb_handle_events(context);
		}
	}

}
