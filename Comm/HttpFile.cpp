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

#include <Comm/HttpFile.h>

#include <string.h>
#include <string>
#include <Misc/ThrowStdErr.h>
#include <Misc/Time.h>
#include <IO/ValueSource.h>
#include <Comm/TCPPipe.h>

namespace Comm {

namespace {

/****************
Helper functions:
****************/

size_t parseChunkHeader(Comm::Pipe& pipe)
	{
	/* Read the next chunk header: */
	size_t chunkSize=0;
	int digit;
	while(true)
		{
		digit=pipe.getChar();
		if(digit>='0'&&digit<='9')
			chunkSize=(chunkSize<<4)+(digit-'0');
		else if(digit>='A'&&digit<='F')
			chunkSize=(chunkSize<<4)+(digit-'A'+10);
		else if(digit>='a'&&digit<='f')
			chunkSize=(chunkSize<<4)+(digit-'a'+10);
		else
			break;
		}

	/* Skip the rest of the chunk header: */
	while(digit!='\r')
		digit=pipe.getChar();
	if(pipe.getChar()!='\n')
		Misc::throwStdErr("HttpFile::readData: Malformed HTTP chunk header");
	
	return chunkSize;
	}

}

/*************************
Methods of class HttpFile:
*************************/

size_t HttpFile::readData(IO::File::Byte* buffer,size_t bufferSize)
	{
	/* Read depending on the reply body's transfer encoding: */
	if(chunked)
		{
		/* Check if the current chunk is finished: */
		if(unreadSize==0)
			{
			/* Bail out if the EOF chunk has already been read: */
			if(haveEof)
				return 0;
			
			/* Skip the chunk footer: */
			if(pipe->getChar()!='\r'||pipe->getChar()!='\n')
				Misc::throwStdErr("Comm::HttpFile: Malformed HTTP chunk footer");
			
			/* Parse the next chunk header: */
			unreadSize=parseChunkHeader(*pipe);
			}
		
		/* Set the EOF flag if this chunk has size zero: */
		if(unreadSize==0)
			{
			haveEof=true;
			return 0;
			}
		
		/* Read more data directly from the pipe's read buffer: */
		void* pipeBuffer;
		size_t pipeSize=pipe->readInBuffer(pipeBuffer,unreadSize);
		setReadBuffer(pipeSize,static_cast<Byte*>(pipeBuffer),false);
		
		/* Reduce the unread data size and return the read size: */
		unreadSize-=pipeSize;
		return pipeSize;
		}
	else if(fixedSize)
		{
		/* Check for end-of-file: */
		if(unreadSize==0)
			return 0;
		
		/* Read more data directly from the pipe's read buffer: */
		void* pipeBuffer;
		size_t pipeSize=pipe->readInBuffer(pipeBuffer,unreadSize);
		setReadBuffer(pipeSize,static_cast<Byte*>(pipeBuffer),false);
		
		/* Reduce the unread data size and return the read size: */
		unreadSize-=pipeSize;
		return pipeSize;
		}
	else
		{
		/* Read more data directly from the pipe's read buffer: */
		void* pipeBuffer;
		size_t pipeSize=pipe->readInBuffer(pipeBuffer);
		setReadBuffer(pipeSize,static_cast<Byte*>(pipeBuffer),false);
		
		/* Return the read size: */
		return pipeSize;
		}
	}

void HttpFile::init(const HttpFile::URLParts& urlParts)
	{
	/* Assemble the GET request: */
	std::string request;
	request.append("GET");
	request.push_back(' ');
	request.append(urlParts.resourcePath);
	request.push_back(' ');
	request.append("HTTP/1.1\r\n");
	
	request.append("Host: ");
	request.append(urlParts.serverName);
	request.push_back(':');
	int pn=urlParts.portNumber;
	char buf[10];
	char* bufPtr=buf;
	do
		{
		*(bufPtr++)=char(pn%10+'0');
		pn/=10;
		}
	while(pn!=0);
	while(bufPtr!=buf)
		request.push_back(*(--bufPtr));
	request.append("\r\n");
	
	#if 0
	request.append("Accept: text/html\r\n");
	#endif
	
	request.append("\r\n");
	
	/* Send the GET request: */
	pipe->writeRaw(request.data(),request.size());
	pipe->flush();
	
	/* Wait for the server's reply: */
	if(!pipe->waitForData(Misc::Time(30,0)))
		throw OpenError(Misc::printStdErrMsg("Comm::HttpFile: Timeout while waiting for reply from server \"%s\" on port %d",urlParts.serverName.c_str(),urlParts.portNumber));
	
	{
	/* Attach a value source to the pipe to parse the server's reply: */
	IO::ValueSource reply(pipe);
	reply.setPunctuation("()<>@,;:\\/[]?={}\r");
	reply.setQuotes("\"");
	reply.skipWs();
	
	/* Read the status line: */
	if(!reply.isLiteral("HTTP")||!reply.isLiteral('/'))
		throw OpenError(Misc::printStdErrMsg("Comm::HttpFile: Malformed HTTP reply from server \"%s\" on port %d",urlParts.serverName.c_str(),urlParts.portNumber));
	reply.skipString();
	unsigned int statusCode=reply.readUnsignedInteger();
	if(statusCode!=200)
		throw OpenError(Misc::printStdErrMsg("Comm::HttpFile: HTTP error %d while opening resource \"%s\" on server \"%s\" on port %d",statusCode,urlParts.resourcePath.c_str(),urlParts.serverName.c_str(),urlParts.portNumber));
	reply.readLine();
	reply.skipWs();
	
	/* Parse reply options until the first empty line: */
	while(!reply.eof()&&reply.peekc()!='\r')
		{
		/* Read the option tag: */
		std::string option=reply.readString();
		if(reply.isLiteral(':'))
			{
			/* Handle the option value: */
			if(option=="Transfer-Encoding")
				{
				/* Parse the comma-separated list of transfer encodings: */
				while(true)
					{
					std::string coding=reply.readString();
					if(coding=="chunked")
						chunked=true;
					else
						{
						/* Skip the transfer extension: */
						while(reply.isLiteral(';'))
							{
							reply.skipString();
							if(!reply.isLiteral('='))
								throw OpenError(Misc::printStdErrMsg("Comm::HttpFile: Malformed HTTP reply from server \"%s\" on port %d",urlParts.serverName.c_str(),urlParts.portNumber));
							reply.skipString();
							}
						}
					if(reply.eof()||reply.peekc()!=',')
						break;
					while(!reply.eof()&&reply.peekc()==',')
						reply.readChar();
					}
				}
			else if(option=="Content-Length")
				{
				if(!chunked)
					{
					fixedSize=true;
					unreadSize=reply.readUnsignedInteger();
					}
				}
			}
		
		/* Skip the rest of the line: */
		reply.skipLine();
		reply.skipWs();
		}
	
	/* Read the CR/LF pair: */
	if(reply.getChar()!='\r'||reply.getChar()!='\n')
		throw OpenError(Misc::printStdErrMsg("Comm::HttpFile: Malformed HTTP reply from server \"%s\" on port %d",urlParts.serverName.c_str(),urlParts.portNumber));
	}
	
	if(chunked)
		{
		/* Read the first chunk header: */
		unreadSize=parseChunkHeader(*pipe);
		haveEof=unreadSize==0;
		}
	}

HttpFile::HttpFile(const char* fileUrl)
	:IO::File(),
	 chunked(false),haveEof(false),
	 fixedSize(false),
	 unreadSize(0),
	 gzipped(false)
	{
	/* Parse the URL to determine server name, port, and absolute resource location: */
	URLParts urlParts=splitUrl(fileUrl);
	
	/* Connect to the HTTP server: */
	pipe=new Comm::TCPPipe(urlParts.serverName.c_str(),urlParts.portNumber);
	
	/* Initialize the HTTP parser: */
	init(urlParts);
	}

HttpFile::HttpFile(const HttpFile::URLParts& urlParts,Comm::PipePtr sPipe)
	:IO::File(),
	 pipe(sPipe),
	 chunked(false),haveEof(false),
	 fixedSize(false),
	 unreadSize(0),
	 gzipped(false)
	{
	/* Initialize the HTTP parser: */
	init(urlParts);
	}

HttpFile::~HttpFile(void)
	{
	/* Skip all unread parts of the HTTP reply body: */
	if(chunked)
		{
		if(!haveEof)
			{
			/* Skip all leftover chunks: */
			while(true)
				{
				/* Skip the rest of the current chunk: */
				pipe->skip<char>(unreadSize);
				
				/* Skip the chunk footer: */
				if(pipe->getChar()!='\r'||pipe->getChar()!='\n')
					Misc::throwStdErr("Comm::HttpFile: Malformed HTTP chunk footer");
				
				/* Parse the next chunk header: */
				unreadSize=parseChunkHeader(*pipe);
				if(unreadSize==0)
					break;
				}
			}
		
		/* Skip any optional message trailers: */
		while(pipe->getChar()!='\r')
			{
			/* Skip the line: */
			while(pipe->getChar()!='\r')
				;
			if(pipe->getChar()!='\n')
				Misc::throwStdErr("Comm::HttpFile: Malformed HTTP body trailer");
			}
		if(pipe->getChar()!='\n')
			Misc::throwStdErr("Comm::HttpFile: Malformed HTTP body trailer");
		}
	else if(fixedSize)
		{
		/* Skip the rest of the fixed-size message body: */
		pipe->skip<char>(unreadSize);
		}
	
	/* Release the read buffer: */
	setReadBuffer(0,0,false);
	}

size_t HttpFile::getReadBufferSize(void) const
	{
	/* Return pipe's read buffer size, since we're sharing it: */
	return pipe->getReadBufferSize();
	}

size_t HttpFile::resizeReadBuffer(size_t newReadBufferSize)
	{
	/* Ignore the request and return pipe's read buffer size, since we're sharing it: */
	return pipe->getReadBufferSize();
	}

HttpFile::URLParts HttpFile::splitUrl(const char* url)
	{
	URLParts result;
	
	/* Parse the URL to determine server name, port, and absolute resource location: */
	const char* uPtr=url;
	
	/* Skip the protocol identifier: */
	if(strncmp(uPtr,"http://",7)==0)
		uPtr+=7;
	
	/* Server name is terminated by colon, slash, or NUL: */
	const char* serverStart=uPtr;
	while(*uPtr!='\0'&&*uPtr!=':'&&*uPtr!='/')
		++uPtr;
	result.serverName=std::string(serverStart,uPtr);
	
	/* Get the port number: */
	result.portNumber=80;
	if(*uPtr==':')
		{
		++uPtr;
		result.portNumber=0;
		while(*uPtr>='0'&&*uPtr<='9')
			{
			result.portNumber=result.portNumber*10+int(*uPtr-'0');
			++uPtr;
			}
		}
	
	/* Get the absolute resource path: */
	if(*uPtr=='/')
		{
		/* Retrieve the absolute path: */
		result.resourcePath=uPtr;
		}
	else
		{
		/* Use the root resource if no path is specified: */
		result.resourcePath.push_back('/');
		}
	
	return result;
	}

}
