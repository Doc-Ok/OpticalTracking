/***********************************************************************
Pipe - Base class derived from IO::File for files representing pipes
supporting waiting and automatic endianness negotiation.
Copyright (c) 2011 Oliver Kreylos

This file is part of the Portable Communications Library (Comm).

The Portable Communications Library is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The Portable Communications Library is distributed in the hope that it
will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Portable Communications Library; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#ifndef COMM_PIPE_INCLUDED
#define COMM_PIPE_INCLUDED

#include <Misc/Autopointer.h>
#include <IO/File.h>

/* Forward declarations: */
namespace Misc {
class Time;
}

namespace Comm {

class Pipe:public IO::File
	{
	/* Constructors and destructors: */
	public:
	Pipe(void)
		{
		}
	Pipe(AccessMode sAccessMode)
		:IO::File(sAccessMode)
		{
		}
	
	/* New methods: */
	virtual void negotiateEndianness(void); // Negotiates the pipe's endianness with the other end via "receiver makes it right"
	virtual bool waitForData(void) const =0; // Waits for incoming data on the pipe; returns true if data is ready
	virtual bool waitForData(const Misc::Time& timeout) const =0; // Waits for incoming data on the pipe until timeout; returns true if data is ready
	virtual void shutdown(bool read,bool write); // Shuts down reading and/or writing part of pipe
	};

typedef Misc::Autopointer<Pipe> PipePtr; // Type for pointers to reference-counted pipe objects

}

#endif
