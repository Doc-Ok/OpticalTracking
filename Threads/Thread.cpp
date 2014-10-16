/***********************************************************************
Thread - Wrapper class for pthreads threads, mostly providing more
convenient thread starting methods and "resource allocation as creation"
paradigm.
Copyright (c) 2005-2012 Oliver Kreylos

This file is part of the Portable Threading Library (Threads).

The Portable Threading Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Portable Threading Library is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Portable Threading Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <Threads/Thread.h>

namespace Threads {

/*******************************
Static elements of class Thread:
*******************************/

Thread Thread::rootThreadObject(0);
pthread_key_t Thread::threadObjectKey;

/***********************
Methods of class Thread:
***********************/

void Thread::initialize(void)
	{
	/* Create the thread object storage key: */
	pthread_key_create(&threadObjectKey,0);
	
	/* Install the main thread object: */
	pthread_setspecific(threadObjectKey,&rootThreadObject);
	}

void Thread::deinitialize(void)
	{
	/* Destroy the thread object storage key: */
	pthread_key_delete(threadObjectKey);
	}

}
