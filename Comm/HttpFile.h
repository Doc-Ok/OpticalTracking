/***********************************************************************
HttpFile - Class for high-performance reading from remote files using
the HTTP/1.1 protocol.
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

#ifndef COMM_HTTPFILE_INCLUDED
#define COMM_HTTPFILE_INCLUDED

#include <string>
#include <IO/File.h>
#include <Comm/Pipe.h>

namespace Comm {

class HttpFile:public IO::File
	{
	/* Embedded classes: */
	public:
	struct URLParts // Structure to hold the three components of an HTTP URL
		{
		/* Elements: */
		public:
		std::string serverName; // Server host name
		int portNumber; // Server port number
		std::string resourcePath; // Absolute resource path
		};
	
	/* Elements: */
	private:
	PipePtr pipe; // Pipe connected to the HTTP server
	bool chunked; // Flag whether the file is transfered in chunks
	bool haveEof; // Flag if the zero-sized EOF chunk was already seen
	bool fixedSize; // Flag whether the file's size is known a-priori
	size_t unreadSize; // Number of unread bytes in the current chunk or the entire fixed-size file
	bool gzipped; // Flag whether the HTTP payload has been gzip-compressed for transmission
	
	/* Protected methods from IO::File: */
	protected:
	virtual size_t readData(Byte* buffer,size_t bufferSize);
	
	/* Private methods: */
	private:
	void init(const URLParts& urlParts);
	
	/* Constructors and destructors: */
	public:
	HttpFile(const char* fileUrl); // Opens file of the given URL over a private server connection
	HttpFile(const URLParts& urlParts,PipePtr sPipe); // Opens file of the given URL over the existing server connection
	virtual ~HttpFile(void); // Closes the HTTP file
	
	/* Methods from IO::File: */
	virtual size_t getReadBufferSize(void) const;
	virtual size_t resizeReadBuffer(size_t newReadBufferSize);
	
	/* New methods: */
	static URLParts splitUrl(const char* url); // Splits the given HTTP URL into its components
	bool isGzipped(void) const // Returns true if the file's contents are gzip-compressed
		{
		return gzipped;
		}
	};

}

#endif
