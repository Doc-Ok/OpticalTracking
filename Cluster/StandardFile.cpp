/***********************************************************************
StandardFile - Pair of classes for high-performance cluster-transparent
reading/writing from/to standard operating system files.
Copyright (c) 2011-2013 Oliver Kreylos

This file is part of the Cluster Abstraction Library (Cluster).

The Cluster Abstraction Library is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Cluster Abstraction Library is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Cluster Abstraction Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <Cluster/StandardFile.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <Misc/ThrowStdErr.h>
#include <Cluster/Packet.h>
#include <Cluster/Multiplexer.h>

#ifdef __APPLE__
#define lseek64 lseek
#endif

namespace Cluster {

/***********************************
Methods of class StandardFileMaster:
***********************************/

size_t StandardFileMaster::readData(IO::File::Byte* buffer,size_t bufferSize)
	{
	/* Collect error codes: */
	int errorType=0;
	int errorCode=0;
	size_t readSize=0;
	
	/* Check if file needs to be repositioned: */
	if(filePos!=readPos)
		{
		/* Set the file position and check for seek errors: */
		if(lseek64(fd,readPos,SEEK_SET)<0)
			errorType=1; // Seek error
		}
	
	if(errorType==0)
		{
		/* Read more data from source: */
		ssize_t readResult;
		do
			{
			readResult=::read(fd,buffer,bufferSize);
			}
		while(readResult<0&&(errno==EAGAIN||errno==EWOULDBLOCK||errno==EINTR));
		
		/* Check for errors: */
		if(readResult>0)
			readSize=size_t(readResult);
		else if(readResult==0)
			errorType=2; // End of file
		else
			{
			errorType=3; // Fatal error
			errorCode=errno;
			}
		}
	
	/* Check for errors: */
	if(errorType==0)
		{
		if(isReadCoupled())
			{
			/* Forward the just-read data to the slaves: */
			Packet* packet=multiplexer->newPacket();
			packet->packetSize=readSize;
			memcpy(packet->packet,buffer,readSize);
			multiplexer->sendPacket(pipeId,packet);
			}
		
		/* Advance the read pointer: */
		readPos+=readSize;
		filePos=readPos;
		
		return readSize;
		}
	else
		{
		if(isReadCoupled())
			{
			/* Send an error indicator (empty packet followed by status packet) to the slaves: */
			Packet* packet=multiplexer->newPacket();
			packet->packetSize=0;
			multiplexer->sendPacket(pipeId,packet);
			packet=multiplexer->newPacket();
			{
			Packet::Writer writer(packet);
			writer.write<int>(errorType);
			writer.write<int>(errorCode);
			}
			multiplexer->sendPacket(pipeId,packet);
			}
		
		/* Throw an exception: */
		if(errorType==1)
			throw SeekError(readPos);
		else if(errorType==3)
			throw Error(Misc::printStdErrMsg("Cluster::StandardFile: Fatal error %d while reading from file",errorCode));
		
		/* Only reached in case of end-of-file: */
		filePos=readPos;
		return 0;
		}
	}

void StandardFileMaster::writeData(const IO::File::Byte* buffer,size_t bufferSize)
	{
	/* Collect error codes: */
	int errorType=0;
	int errorCode=0;
	size_t numBytesWritten=0;
	
	/* Check if file needs to be repositioned: */
	if(filePos!=writePos)
		{
		/* Set the file position and check for seek errors: */
		if(lseek64(fd,writePos,SEEK_SET)<0)
			errorType=1; // Seek error
		}
	
	/* Invalidate the read buffer to prevent reading stale data: */
	flushReadBuffer();
	
	/* Write all data in the given buffer: */
	while(errorType==0&&bufferSize>0)
		{
		ssize_t writeResult=::write(fd,buffer,bufferSize);
		if(writeResult>0)
			{
			/* Prepare to write more data: */
			buffer+=writeResult;
			bufferSize-=writeResult;
			
			/* Advance the write pointer: */
			writePos+=writeResult;
			filePos=writePos;
			
			numBytesWritten+=writeResult;
			}
		else if(writeResult<0&&(errno==EAGAIN||errno==EWOULDBLOCK||errno==EINTR))
			{
			/* Do nothing and try again */
			}
		else if(writeResult==0)
			{
			/* Sink has reached end-of-file: */
			errorType=2;
			errorCode=int(bufferSize);
			}
		else
			{
			/* Unknown error; probably fatal: */
			errorType=3;
			errorCode=errno;
			}
		}
	
	if(isWriteCoupled())
		{
		/* Send a status packet to the slaves: */
		Packet* packet=multiplexer->newPacket();
		{
		Packet::Writer writer(packet);
		writer.write<int>(errorType);
		writer.write<int>(errorCode);
		writer.write<int>(int(numBytesWritten));
		}
		multiplexer->sendPacket(pipeId,packet);
		}
	
	/* Handle errors: */
	if(errorType==1)
		throw SeekError(writePos);
	else if(errorType==2)
		throw WriteError(errorCode);
	else if(errorType==3)
		throw Error(Misc::printStdErrMsg("Cluster::StandardFile: Fatal error %d while writing to file",errorCode));
	}

void StandardFileMaster::openFile(const char* fileName,IO::File::AccessMode accessMode,int flags,int mode)
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
	errno=0;
	fd=open(fileName,flags,mode);
	int errorCode=fd<0?errno:0;
	
	/* Send a status message to the slaves: */
	Packet* statusPacket=multiplexer->newPacket();
	{
	Packet::Writer writer(statusPacket);
	writer.write<int>(errorCode);
	}
	multiplexer->sendPacket(pipeId,statusPacket);
	
	/* Check for errors: */
	if(errorCode!=0)
		{
		/* Throw an exception: */
		throw OpenError(Misc::printStdErrMsg("Cluster::StandardFile: Unable to open file %s for %s due to error %d",fileName,getAccessModeName(accessMode),errorCode));
		}
	
	/* Install a read buffer the size of a multicast packet: */
	canReadThrough=false;
	if(accessMode==ReadOnly||accessMode==ReadWrite)
		IO::SeekableFile::resizeReadBuffer(Packet::maxPacketSize);
	}

StandardFileMaster::StandardFileMaster(Multiplexer* sMultiplexer,const char* fileName,IO::File::AccessMode accessMode)
	:IO::SeekableFile(disableRead(accessMode)),ClusterPipe(sMultiplexer),
	 fd(-1),
	 filePos(0)
	{
	/* Create flags and mode to open the file: */
	int flags=O_CREAT;
	if(accessMode==WriteOnly)
		flags|=O_TRUNC;
	mode_t mode=S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH;
	
	/* Open the file: */
	openFile(fileName,accessMode,flags,mode);
	}

StandardFileMaster::StandardFileMaster(Multiplexer* sMultiplexer,const char* fileName,IO::File::AccessMode accessMode,int flags,int mode)
	:SeekableFile(disableRead(accessMode)),ClusterPipe(sMultiplexer),
	 fd(-1),
	 filePos(0)
	{
	/* Open the file: */
	openFile(fileName,accessMode,flags,mode);
	}

StandardFileMaster::~StandardFileMaster(void)
	{
	/* Flush the write buffer, and then close the file: */
	flush();
	if(fd>=0)
		close(fd);
	}

int StandardFileMaster::getFd(void) const
	{
	Misc::throwStdErr("Cluster::StandardFile::getFd: Cannot query file descriptor");
	
	/* Just to make compiler happy: */
	return -1;
	}

size_t StandardFileMaster::resizeReadBuffer(size_t newReadBufferSize)
	{
	/* Ignore the change and return the size of a multicast packet: */
	return Packet::maxPacketSize;
	}

IO::SeekableFile::Offset StandardFileMaster::getSize(void) const
	{
	/* Get the file's total size: */
	struct stat statBuffer;
	int statResult=fstat(fd,&statBuffer);
	Offset fileSize=statBuffer.st_size;
	
	if(isReadCoupled())
		{
		/* Send a status message to the slaves: */
		Packet* statusPacket=multiplexer->newPacket();
		{
		Packet::Writer writer(statusPacket);
		writer.write<int>(statResult);
		writer.write<Offset>(fileSize);
		}
		multiplexer->sendPacket(pipeId,statusPacket);
		}
	
	/* Check for errors: */
	if(statResult<0)
		throw Error(Misc::printStdErrMsg("Cluster::StandardFile: Error while determining file size"));
	
	/* Return the file size: */
	return fileSize;
	}

/**********************************
Methods of class StandardFileSlave:
**********************************/

size_t StandardFileSlave::readData(IO::File::Byte* buffer,size_t bufferSize)
	{
	if(isReadCoupled())
		{
		/* Receive a data packet from the master: */
		Packet* newPacket=multiplexer->receivePacket(pipeId);
		
		/* Check for error conditions: */
		if(newPacket->packetSize!=0)
			{
			/* Install the new packet as the file's read buffer: */
			if(packet!=0)
				multiplexer->deletePacket(packet);
			packet=newPacket;
			setReadBuffer(Packet::maxPacketSize,reinterpret_cast<Byte*>(packet->packet),false);
			
			/* Advance the read pointer: */
			readPos+=packet->packetSize;
			
			return packet->packetSize;
			}
		else
			{
			/* Read the status packet: */
			multiplexer->deletePacket(newPacket);
			newPacket=multiplexer->receivePacket(pipeId);
			Packet::Reader reader(newPacket);
			int errorType=reader.read<int>();
			int errorCode=reader.read<int>();
			multiplexer->deletePacket(newPacket);
			
			/* Handle the error: */
			if(errorType==1)
				throw SeekError(readPos);
			else if(errorType==3)
				throw Error(Misc::printStdErrMsg("Cluster::StandardFile: Fatal error %d while reading from file",errorCode));
			
			/* Only reached in case of end-of-file packet: */
			return 0;
			}
		}
	else
		{
		/* Indicate end-of-file; slave shouldn't have been reading in decoupled state: */
		return 0;
		}
	}

void StandardFileSlave::writeData(const IO::File::Byte* buffer,size_t bufferSize)
	{
	/* Invalidate the read buffer to prevent reading stale data: */
	flushReadBuffer();
	
	if(isWriteCoupled())
		{
		/* Receive a status packet from the master: */
		Packet* statusPacket=multiplexer->receivePacket(pipeId);
		Packet::Reader reader(statusPacket);
		int errorType=reader.read<int>();
		int errorCode=reader.read<int>();
		int numBytesWritten=reader.read<int>();
		multiplexer->deletePacket(statusPacket);
		
		/* Handle errors: */
		if(errorType==1)
			throw SeekError(writePos);
		else
			{
			/* Advance the write pointer in case partial data was written: */
			writePos+=numBytesWritten;
			
			if(errorType==2)
				throw WriteError(errorCode);
			else if(errorType==3)
				throw Error(Misc::printStdErrMsg("Cluster::StandardFile: Fatal error %d while writing to file",errorCode));
			}
		}
	}

StandardFileSlave::StandardFileSlave(Multiplexer* sMultiplexer,const char* fileName,IO::File::AccessMode accessMode)
	:IO::SeekableFile(disableRead(accessMode)),ClusterPipe(sMultiplexer),
	 packet(0)
	{
	/* Read the status packet from the master node: */
	Packet* statusPacket=multiplexer->receivePacket(pipeId);
	Packet::Reader reader(statusPacket);
	int errorCode=reader.read<int>();
	multiplexer->deletePacket(statusPacket);
	
	/* Check for errors: */
	if(errorCode!=0)
		{
		/* Throw an exception: */
		throw OpenError(Misc::printStdErrMsg("Cluster::StandardFile: Unable to open file %s for %s due to error %d",fileName,getAccessModeName(accessMode),errorCode));
		}
	
	canReadThrough=false;
	}

StandardFileSlave::~StandardFileSlave(void)
	{
	/* Delete the current multicast packet: */
	if(packet!=0)
		{
		multiplexer->deletePacket(packet);
		setReadBuffer(0,0,false);
		}
	}

int StandardFileSlave::getFd(void) const
	{
	Misc::throwStdErr("Cluster::StandardFile::getFd: Cannot query file descriptor");
	
	/* Just to make compiler happy: */
	return -1;
	}

size_t StandardFileSlave::getReadBufferSize(void) const
	{
	/* Return the size of a multicast packet: */
	return Packet::maxPacketSize;
	}

size_t StandardFileSlave::resizeReadBuffer(size_t newReadBufferSize)
	{
	/* Ignore the change and return the size of a multicast packet: */
	return Packet::maxPacketSize;
	}

IO::SeekableFile::Offset StandardFileSlave::getSize(void) const
	{
	if(isReadCoupled())
		{
		/* Receive a status message from the master: */
		Packet* statusPacket=multiplexer->receivePacket(pipeId);
		Packet::Reader reader(statusPacket);
		int statResult=reader.read<int>();
		Offset fileSize=reader.read<Offset>();
		multiplexer->deletePacket(statusPacket);
		
		/* Check for errors: */
		if(statResult<0)
			throw Error(Misc::printStdErrMsg("Cluster::StandardFile: Error while determining file size"));
		
		/* Return the file size: */
		return fileSize;
		}
	else
		{
		/* Return empty size; slave shouldn't have been querying the file size in decoupled state: */
		return 0;
		}
	}

}
