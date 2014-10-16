/***********************************************************************
TCPPipe - Class for high-performance reading/writing from/to connected
TCP sockets.
Copyright (c) 2010-2013 Oliver Kreylos

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

#ifndef COMM_TCPPIPE_INCLUDED
#define COMM_TCPPIPE_INCLUDED

#include <Comm/NetPipe.h>

/* Forward declarations: */
namespace Comm {
class ListeningTCPSocket;
}

namespace Comm {

class TCPPipe:public NetPipe
	{
	/* Elements: */
	private:
	int fd; // File descriptor of the underlying TCP socket
	
	/* Protected methods from IO::File: */
	protected:
	virtual size_t readData(Byte* buffer,size_t bufferSize);
	virtual void writeData(const Byte* buffer,size_t bufferSize);
	
	/* Constructors and destructors: */
	public:
	TCPPipe(const char* hostName,int portId); // Opens a TCP socket connected to the given port on the given host with "DontCare" endianness setting
	TCPPipe(ListeningTCPSocket& listenSocket); // Opens a TCP socket connected to a waiting incoming socket on the given listening socket with "DontCare" endianness setting
	private:
	TCPPipe(const TCPPipe& source); // Prohibit copy constructor
	TCPPipe& operator=(const TCPPipe& source); // Prohibit assignment operator
	public:
	virtual ~TCPPipe(void);
	
	/* Methods from IO::File: */
	virtual int getFd(void) const;
	
	/* Methods from Pipe: */
	virtual bool waitForData(void) const;
	virtual bool waitForData(const Misc::Time& timeout) const;
	virtual void shutdown(bool read,bool write);
	
	/* Methods from NetPipe: */
	virtual int getPortId(void) const;
	virtual std::string getAddress(void) const;
	virtual std::string getHostName(void) const;
	virtual int getPeerPortId(void) const;
	virtual std::string getPeerAddress(void) const;
	virtual std::string getPeerHostName(void) const;
	};

}

#endif
