/***********************************************************************
MemMappedFile - Class for read/write access to memory-mapped files using
the File abstraction; mostly for simplified resource management.
Copyright (c) 2011 Oliver Kreylos

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

#include <IO/MemMappedFile.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <Misc/ThrowStdErr.h>

namespace IO {

/******************************
Methods of class MemMappedFile:
******************************/

void MemMappedFile::openFile(const char* fileName,File::AccessMode accessMode,int flags,int mode)
	{
	/* Adjust flags according to access mode: */
	switch(accessMode)
		{
		case NoAccess:
			flags&=~(O_RDONLY|O_WRONLY|O_RDWR|O_CREAT|O_TRUNC|O_APPEND);
			break;
		
		case ReadOnly:
			flags&=~(O_WRONLY|O_RDWR|O_CREAT|O_TRUNC|O_APPEND);
			flags|=O_RDONLY;
			break;
		
		case WriteOnly:
			flags&=~(O_RDONLY|O_RDWR);
			flags|=O_WRONLY;
			break;
		
		case ReadWrite:
			flags&=~(O_RDONLY|O_WRONLY);
			flags|=O_RDWR;
			break;
		}
	
	/* Open the file: */
	int fd=open(fileName,flags,mode);
	
	/* Check for errors and throw an exception: */
	if(fd<0)
		{
		int errorCode=errno;
		throw OpenError(Misc::printStdErrMsg("IO::MemMappedFile: Unable to open file %s for %s due to error %d",fileName,getAccessModeName(accessMode),errorCode));
		}
	
	/* Get the file's total size: */
	struct stat statBuffer;
	if(fstat(fd,&statBuffer)<0)
		{
		close(fd);
		throw OpenError(Misc::printStdErrMsg("IO::MemMappedFile: Unable to determine size of file %s",fileName));
		}
	memSize=statBuffer.st_size;
	
	/* Memory-map the file: */
	int prot;
	switch(accessMode)
		{
		case ReadOnly:
			prot=PROT_READ;
			break;
		
		case WriteOnly:
			prot=PROT_WRITE;
			break;
		
		case ReadWrite:
			prot=PROT_READ|PROT_WRITE;
			break;
		
		default:
			prot=0x0;
		}
	memBase=mmap(0,memSize,prot,MAP_SHARED,fd,0);
	if(memBase==MAP_FAILED)
		{
		close(fd);
		throw OpenError(Misc::printStdErrMsg("IO::MemMappedFile: Unable to memory-map file %s",fileName));
		}
	
	/* Close the file again: */
	close(fd);
	
	/* Re-allocate the buffered file's buffers: */
	setReadBuffer(memSize,static_cast<Byte*>(memBase),false);
	canReadThrough=false;
	setWriteBuffer(memSize,static_cast<Byte*>(memBase),false);
	canWriteThrough=false;
	
	/* Pretend putting the file data into the read buffer: */
	appendReadBufferData(memSize);
	readPos=memSize;
	}

MemMappedFile::MemMappedFile(const char* fileName,File::AccessMode accessMode)
	:SeekableFile(),
	 memBase(0),memSize(0)
	{
	/* Create flags and mode to open the file: */
	int flags=O_CREAT;
	if(accessMode==WriteOnly)
		flags|=O_TRUNC;
	mode_t mode=S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH;
	
	/* Open the file: */
	openFile(fileName,accessMode,flags,mode);
	}

MemMappedFile::MemMappedFile(const char* fileName,File::AccessMode accessMode,int flags,int mode)
	:SeekableFile(),
	 memBase(0),memSize(0)
	{
	/* Open the file: */
	openFile(fileName,accessMode,flags,mode);
	}

MemMappedFile::~MemMappedFile(void)
	{
	/* Release the buffered file's buffers: */
	setReadBuffer(0,0,false);
	setWriteBuffer(0,0,false);
	
	if(memBase!=0)
		{
		/* Unmap the file: */
		if(munmap(memBase,memSize)<0)
			throw Error(Misc::printStdErrMsg("IO::MemMappedFile: Fatal error while unmapping memory"));
		}
	}

size_t MemMappedFile::resizeReadBuffer(size_t newReadBufferSize)
	{
	/* Ignore it and return the full memory size: */
	return memSize;
	}

void MemMappedFile::resizeWriteBuffer(size_t newWriteBufferSize)
	{
	/* Ignore it */
	}

SeekableFile::Offset MemMappedFile::getSize(void) const
	{
	/* Return the file size: */
	return memSize;
	}

}
