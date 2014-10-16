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

#ifndef IO_FILEMONITOR_INCLUDED
#define IO_FILEMONITOR_INCLUDED

#include <stddef.h>
#include <string>
#include <Misc/HashTable.h>
#include <Threads/Mutex.h>
#include <Threads/Thread.h>

/* Forward declarations: */
namespace Misc {
template <class ParameterParam>
class FunctionCall;
}

namespace IO {

class FileMonitor
	{
	/* Embedded classes: */
	public:
	typedef int Cookie; // Data type for cookies identifying monitored files or directories; values are unique inside the same FileMonitor object
	
	enum EventType // Enumerated type for events; can be or-ed together for event masks
		{
		/* Events for the monitored file/directory itself, or for a file/directory contained in the monitored directory: */
		Accessed=0x1, // File/directory was accessed
		Modified=0x2, // File/directory was modified
		AttributesChanged=0x4, // File/directory's attributes changed
		Opened=0x8, // File/directory was opened
		ClosedWrite=0x10, // File/directory was closed after being opened for writing
		ClosedNoWrite=0x20, // File/directory was closed after not being opened for writing
		Closed=0x30, // File/directory was closed, after being opened for writing or not
		
		/* Events for the monitored directory itself, or for a directory contained in the monitored directory: */
		Created=0x40, // File/directory was created inside directory
		MovedFrom=0x80, // File/directory was moved out of directory
		MovedTo=0x100, // File/directory was moved into directory
		Moved=0x180, // File/directory was moved out of or into directory
		Deleted=0x200, // File/directory was deleted inside directory
		
		/* Events for the monitored file/directory itself: */
		SelfMoved=0x400, // Monitored file/directory itself was moved
		SelfDeleted=0x800, // Monitored file/directory itself was deleted
		AllEvents=0xfff, // Mask for all events that can be monitored
		
		/* Events that are reported even if they were not requested: */
		Unmounted=0x1000 // The file system containing the monitored file/directory was unmounted
		};
	
	enum EventModifiers // Enumerated types for modifier flags that can be or-ed onto event masks
		{
		DontFollowLinks=0x2000, // If monitored path is a symbolic link, monitor link itself and not file/directory to which it points
		IgnoreUnlinkedFiles=0x4000 // Don't report events for already-unlinked files/directories inside a monitored directory
		};
	
	struct Event // Structure describing an event passed to event callbacks
		{
		/* Elements: */
		public:
		Cookie cookie; // Watch cookie identifying the monitored file/directory
		int eventMask; // Mask of event types that occurred
		bool directory; // Subject of this event is a directory
		unsigned int moveCookie; // Cookie to relate the MovedFrom and MovedTo events caused by a rename or move
		std::string name; // For watched directories: name of file/directory inside watched directory to which event applies
		};
	
	typedef Misc::FunctionCall<const Event&> EventCallback; // Type for event callback functions
	
	private:
	typedef Misc::HashTable<Cookie,EventCallback*> EventCallbackMap; // Type for hash tables mapping from watch cookies to event callbacks
	
	/* Elements: */
	private:
	int fd; // Low-level file descriptor connecting to the OS' file monitoring service
	Threads::Mutex eventCallbacksMutex; // Mutex serializing access to the event callback hash table
	EventCallbackMap eventCallbacks; // Hash table from monitor cookies to event callbacks
	#ifndef __linux__
	Cookie nextCookie; // Dummy cookie to be returned when adding paths, to keep up the appearance of working
	#endif
	size_t eventBufferSize; // Size of the current event buffer
	char* eventBuffer; // Current event buffer
	Threads::Thread eventHandlingThread; // Background thread receiving monitoring events from the OS and dispatching callbacks to clients
	
	/* Private methods: */
	void* eventHandlingThreadMethod(void); // Method for the background event handling thread
	
	/* Constructors and destructors: */
	public:
	FileMonitor(void); // Creates an empty file monitor
	private:
	FileMonitor(const FileMonitor& source); // Prohibit copy constructor
	FileMonitor& operator=(const FileMonitor& source); // Prohibit assignment operator
	public:
	~FileMonitor(void); // Releases all resources allocated by this file monitor
	
	/* Methods: */
	void startEventHandling(void); // Starts background event handling; resets the file monitor to block on empty queue
	void stopEventHandling(void); // Stops background event handling
	void startPolling(void); // Sets the file monitor to allow polling; ignored if background event handling is active
	void stopPolling(void); // Resets the file monitor to block on empty event queue
	bool processEvents(void); // Processes all pending events that have occurred; blocks until at least one event occurred if not in polling mode; returns true if any event callbacks were called
	Cookie addPath(const char* pathName,int eventMask,EventCallback* eventCallback); // Adds a file or directory to the monitor's watch list with the given event mask; given callback is adopted by object and called from background thread when event occurs; returns cookie unique for the given path inside this FileMonitor object
	void removePath(Cookie pathCookie); // Removes a file or directory from the monitor's watch list based on a cookie previously returned by addPath
	};

}

#endif
