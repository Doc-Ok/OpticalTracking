/***********************************************************************
FdSet - Class to simplify using sets of file descriptors for the select
and pselect system calls.
Copyright (c) 2008-2013 Oliver Kreylos

This file is part of the Miscellaneous Support Library (Misc).

The Miscellaneous Support Library is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The Miscellaneous Support Library is distributed in the hope that it
will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Miscellaneous Support Library; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#ifndef MISC_FDSET_INCLUDED
#define MISC_FDSET_INCLUDED

#include <sys/types.h>
#ifdef __APPLE__
#include <unistd.h>
#endif

/* Forward declarations: */
namespace Misc {
class Time;
}

namespace Misc {

/* Forward declarations for friend functions: */
class FdSet;
int select(FdSet* readFdSet,FdSet* writeFdSet,FdSet* exceptFdSet,struct timeval* timeout =0);
int select(FdSet* readFdSet,FdSet* writeFdSet,FdSet* exceptFdSet,const Misc::Time& timeout);
int pselect(FdSet* readFdSet,FdSet* writeFdSet,FdSet* exceptFdSet,const struct timespec* timeout =0,const sigset_t* sigmask =0);
int pselect(FdSet* readFdSet,FdSet* writeFdSet,FdSet* exceptFdSet,const Misc::Time& timeout,const sigset_t* sigmask =0);

class FdSet:public fd_set
	{
	/* Elements: */
	private:
	int maxFd; // Highest file descriptor contained in the set
	
	/* Private methods: */
	void update(void) // Updates the maxFd value after a change to the file descriptor set
		{
		/* Find the largest file descriptor in the set smaller than the previous maxFd: */
		while(maxFd>=0&&!FD_ISSET(maxFd,this))
			--maxFd;
		}
	
	/* Constructors and destructors: */
	public:
	FdSet(void) // Creates an empty file descriptor set
		:maxFd(-1)
		{
		/* Clear the file descriptor set: */
		FD_ZERO(this);
		}
	FdSet(int sFd) // Creates a singleton file descriptor set
		:maxFd(sFd)
		{
		/* Clear the file descriptor set and set the given descriptor: */
		FD_ZERO(this);
		FD_SET(sFd,this);
		}
	private:
	FdSet(const FdSet& source); // Prohibit copy constructor
	FdSet& operator=(const FdSet& source); // Prohibit assignment operator
	
	/* Methods: */
	public:
	int getMaxFd(void) const // Returns the largest file descriptor in the set
		{
		return maxFd;
		}
	bool isSet(int fd) const // Returns true if the given file descriptor is in the set
		{
		return FD_ISSET(fd,const_cast<FdSet*>(this)); // const_cast necessary due to bad API
		}
	void clear(void) // Clears the file descriptor set
		{
		FD_ZERO(this);
		maxFd=-1;
		}
	void add(int fd) // Adds the given file descriptor to the set
		{
		FD_SET(fd,this);
		if(maxFd<fd)
			maxFd=fd;
		}
	void remove(int fd) // Removes the given file descriptor from the set
		{
		FD_CLR(fd,this);
		if(maxFd==fd)
			update();
		}
	friend int select(FdSet* readFdSet,FdSet* writeFdSet,FdSet* exceptFdSet,struct timeval* timeout); // Wrapper for select() function
	friend int select(FdSet* readFdSet,FdSet* writeFdSet,FdSet* exceptFdSet,const Misc::Time& timeout); // Ditto, with Misc::Time timeout
	friend int pselect(FdSet* readFdSet,FdSet* writeFdSet,FdSet* exceptFdSet,const struct timespec* timeout,const sigset_t* sigmask); // Wrapper for pselect() function
	friend int pselect(FdSet* readFdSet,FdSet* writeFdSet,FdSet* exceptFdSet,const Misc::Time& timeout,const sigset_t* sigmask); // Ditto, with Misc::Time timeout
	};

}

#endif
