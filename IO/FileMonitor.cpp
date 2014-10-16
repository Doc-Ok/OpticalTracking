/***********************************************************************
FileMonitor - Class to monitor a set of files and/or directories and
send callbacks on any changes to any of the monitored files or
directories.
Copyright (c) 2012 Oliver Kreylos

This file is part of the I/O Support Library (IO).

The I/O Support Library is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as published
by the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

The I/O Support Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the I/O Support Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <IO/FileMonitor.h>

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#ifdef __linux__
#include <sys/inotify.h>
#endif
#include <Misc/ThrowStdErr.h>
#include <Misc/FunctionCalls.h>

namespace IO {

/****************************
Methods of class FileMonitor:
****************************/

void* FileMonitor::eventHandlingThreadMethod(void)
	{
	/* Enable thread cancellation: */
	Threads::Thread::setCancelState(Threads::Thread::CANCEL_ENABLE);
	
	/* Process events until cancelled: */
	while(true)
		processEvents();
	
	return 0;
	}

FileMonitor::FileMonitor(void)
	:fd(-1),
	 eventCallbacks(17),
	 eventBufferSize(256),eventBuffer(new char[eventBufferSize])
	{
	#ifdef __linux__
	
	/* Create the inotify instance: */
	fd=inotify_init();
	if(fd<0)
		{
		int error=errno;
		Misc::throwStdErr("IO::FileMonitor::FileMonitor: Cannot create FileMonitor object due to error %d",error);
		}
	#else
	
	/* Initialize the dummy cookie: */
	nextCookie=0;
	
	#endif
	}

FileMonitor::~FileMonitor(void)
	{
	/* Shut down the background event handling thread: */
	stopEventHandling();
	
	/* Delete the event buffer: */
	delete[] eventBuffer;
	
	/* Delete all event callbacks: */
	for(EventCallbackMap::Iterator ecIt=eventCallbacks.begin();!ecIt.isFinished();++ecIt)
		delete ecIt->getDest();
	
	#ifdef __linux__
	/* Close the inotify instance: */
	close(fd);
	#endif
	}

void FileMonitor::startEventHandling(void)
	{
	/* Disable polling if it was enabled: */
	stopPolling();
	
	/* Start the background thread if it is not already running: */
	if(eventHandlingThread.isJoined())
		eventHandlingThread.start(this,&FileMonitor::eventHandlingThreadMethod);
	}

void FileMonitor::stopEventHandling(void)
	{
	/* Shut down the background event handling thread if it is running: */
	if(!eventHandlingThread.isJoined())
		{
		eventHandlingThread.cancel();
		eventHandlingThread.join();
		}
	}

void FileMonitor::startPolling(void)
	{
	/* Ignore if background event handling thread is running: */
	if(eventHandlingThread.isJoined())
		{
		#ifdef __linux__
		/* Set the file descriptor to non-blocking: */
		int fileFlags=fcntl(fd,F_GETFL);
		if(fileFlags<0)
			{
			int error=errno;
			Misc::throwStdErr("IO::FileMonitor::startPolling: Caught error %d",error);
			}
		fileFlags|=O_NONBLOCK;
		if(fcntl(fd,F_SETFL,fileFlags)<0)
			{
			int error=errno;
			Misc::throwStdErr("IO::FileMonitor::startPolling: Caught error %d",error);
			}
		#endif
		}
	}

void FileMonitor::stopPolling(void)
	{
	/* Ignore if background event handling thread is running: */
	if(eventHandlingThread.isJoined())
		{
		#ifdef __linux__
		/* Set the file descriptor to blocking: */
		int fileFlags=fcntl(fd,F_GETFL);
		if(fileFlags<0)
			{
			int error=errno;
			Misc::throwStdErr("IO::FileMonitor::stopPolling: Caught error %d",error);
			}
		fileFlags&=~O_NONBLOCK;
		if(fcntl(fd,F_SETFL,fileFlags)<0)
			{
			int error=errno;
			Misc::throwStdErr("IO::FileMonitor::stopPolling: Caught error %d",error);
			}
		#endif
		}
	}

bool FileMonitor::processEvents(void)
	{
	#ifdef __linux__
	
	/* Wait for the next event: */
	ssize_t readResult;
	while(true)
		{
		/* Try reading into the current event buffer: */
		readResult=read(fd,eventBuffer,eventBufferSize);
		
		/* Check if the event buffer was too small to hold the next pending event: */
		if(readResult==0||(readResult<0&&errno==EINVAL))
			{
			/* Resize the event buffer and try again: */
			delete[] eventBuffer;
			eventBufferSize=(eventBufferSize*3)/2;
			eventBuffer=new char[eventBufferSize];
			}
		else
			break;
		}
	
	/* Read and handle all event structures just read: */
	bool dispatchedEvent=false;
	char* ebPtr=eventBuffer;
	while(readResult>0)
		{
		/* Get a pointer to the next event structure in the buffer: */
		inotify_event* eventStruct=reinterpret_cast<inotify_event*>(ebPtr);
		
		/* Check if the event needs to be reported: */
		if(eventStruct->wd>=0&&(eventStruct->mask&IN_IGNORED)==0x0)
			{
			/* Lock the event callback list and find the recipient for this event: */
			Threads::Mutex::Lock eventCallbacksLock(eventCallbacksMutex);
			EventCallbackMap::Iterator ecIt=eventCallbacks.findEntry(eventStruct->wd);
			if(!ecIt.isFinished())
				{
				/***********************************************************
				Assemble an event reporting structure and call the callback:
				***********************************************************/
				
				Event event;
				
				/* Copy the watch identifer: */
				event.cookie=eventStruct->wd;
				
				/* Convert inotify's internal event types and flags to FileMonitor's: */
				event.eventMask=0x0;
				if(eventStruct->mask&IN_ACCESS)
					event.eventMask|=Accessed;
				if(eventStruct->mask&IN_MODIFY)
					event.eventMask|=Modified;
				if(eventStruct->mask&IN_ATTRIB)
					event.eventMask|=AttributesChanged;
				if(eventStruct->mask&IN_OPEN)
					event.eventMask|=Opened;
				if(eventStruct->mask&IN_CLOSE_WRITE)
					event.eventMask|=ClosedWrite;
				if(eventStruct->mask&IN_CLOSE_NOWRITE)
					event.eventMask|=ClosedNoWrite;
				if(eventStruct->mask&IN_CREATE)
					event.eventMask|=Created;
				if(eventStruct->mask&IN_MOVED_FROM)
					event.eventMask|=MovedFrom;
				if(eventStruct->mask&IN_MOVED_TO)
					event.eventMask|=MovedTo;
				if(eventStruct->mask&IN_DELETE)
					event.eventMask|=Deleted;
				if(eventStruct->mask&IN_MOVE_SELF)
					event.eventMask|=SelfMoved;
				if(eventStruct->mask&IN_DELETE_SELF)
					event.eventMask|=SelfDeleted;
				if(eventStruct->mask&IN_UNMOUNT)
					event.eventMask|=Unmounted;
				
				/* Check if the event is for a directory: */
				event.directory=(eventStruct->mask&IN_ISDIR)!=0x0;
				
				/* Copy the move cookie: */
				event.moveCookie=eventStruct->cookie;
				
				/* Copy the optional file name: */
				if(eventStruct->len!=0)
					event.name=ebPtr+sizeof(inotify_event);
				
				/* Call the callback: */
				(*ecIt->getDest())(event);
				dispatchedEvent=true;
				}
			}
		
		/* Go to the next event in the buffer: */
		size_t eventSize=sizeof(inotify_event)+eventStruct->len;
		readResult-=eventSize;
		ebPtr+=eventSize;
		}
	
	return dispatchedEvent;
	
	#else
	
	/* Pretend nothing happened: */
	return false;
	
	#endif
	}

FileMonitor::Cookie FileMonitor::addPath(const char* pathName,int eventMask,FileMonitor::EventCallback* eventCallback)
	{
	#ifdef __linux__
	
	/* Convert the given event mask to inotify's internal types and flags: */
	uint32_t em=0x0U;
	if(eventMask&Accessed)
		em|=IN_ACCESS;
	if(eventMask&Modified)
		em|=IN_MODIFY;
	if(eventMask&AttributesChanged)
		em|=IN_ATTRIB;
	if(eventMask&Opened)
		em|=IN_OPEN;
	if(eventMask&ClosedWrite)
		em|=IN_CLOSE_WRITE;
	if(eventMask&ClosedNoWrite)
		em|=IN_CLOSE_NOWRITE;
	if(eventMask&Created)
		em|=IN_CREATE;
	if(eventMask&MovedFrom)
		em|=IN_MOVED_FROM;
	if(eventMask&MovedTo)
		em|=IN_MOVED_TO;
	if(eventMask&Deleted)
		em|=IN_DELETE;
	if(eventMask&SelfMoved)
		em|=IN_MOVE_SELF;
	if(eventMask&SelfDeleted)
		em|=IN_DELETE_SELF;
	
	if(eventMask&DontFollowLinks)
		em|=IN_DONT_FOLLOW;
	#if 0 // Not supported on older Linux kernels
	if(eventMask&IgnoreUnlinkedFiles)
		em|=IN_EXCL_UNLINK;
	#endif
	
	/* Lock the event callback list: */
	Threads::Mutex::Lock eventCallbacksLock(eventCallbacksMutex);
	
	/* Add the given path to inotify's watch list: */
	Cookie cookie=inotify_add_watch(fd,pathName,eventMask);
	if(cookie<0)
		{
		int error=errno;
		Misc::throwStdErr("IO::FileMonitor::addPath: Could not monitor path %s due to error %d",pathName,error);
		}
	
	/* Enter the cookie and event callback into the callback list: */
	EventCallbackMap::Iterator ecIt=eventCallbacks.findEntry(cookie);
	if(!ecIt.isFinished())
		{
		/*******************************************************************
		Someone else already registered a watch for the given path. This
		could be handled using a multi-value hash table and stored masks;
		for now, simply delete the old callback and remember the new one:
		*******************************************************************/
		
		delete ecIt->getDest();
		ecIt->getDest()=eventCallback;
		}
	else
		{
		/* Add the new event callback to the list: */
		eventCallbacks[cookie]=eventCallback;
		}
	
	/* Return the OS-supplied cookie to the caller: */
	return cookie;
	
	#else
	
	/* Lock the event callback list: */
	Threads::Mutex::Lock eventCallbacksLock(eventCallbacksMutex);
	
	/* Store the callback with a new dummy cookie in the callback map: */
	Cookie cookie=nextCookie;
	++nextCookie;
	
	eventCallbacks[cookie]=eventCallback;
	
	/* Return the dummy cookie: */
	return cookie;
	
	#endif
	}

void FileMonitor::removePath(FileMonitor::Cookie pathCookie)
	{
	/* Lock the event callback list: */
	Threads::Mutex::Lock eventCallbacksLock(eventCallbacksMutex);
	
	/* Find the event callback associated with the given cookie: */
	EventCallbackMap::Iterator ecIt=eventCallbacks.findEntry(pathCookie);
	if(!ecIt.isFinished())
		{
		/* Remove the event callback from the event callback list: */
		delete ecIt->getDest();
		eventCallbacks.removeEntry(ecIt);
		
		#ifdef __linux__
		/* Remove the watch from inotify's watch list: */
		inotify_rm_watch(fd,pathCookie);
		#endif
		}
	}

}
