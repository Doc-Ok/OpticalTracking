/***********************************************************************
OpenFile - Convenience function to open files of several types using the
IO::File abstraction and distribute among a cluster via a multicast
pipe.
Copyright (c) 2011 Oliver Kreylos

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

#include <Cluster/OpenFile.h>

#include <string.h>
#include <Misc/ThrowStdErr.h>
#include <Misc/FileNameExtensions.h>
#include <IO/StandardFile.h>
#include <IO/GzipFilter.h>
#include <IO/SeekableFilter.h>
#include <IO/StandardDirectory.h>
#include <Comm/HttpFile.h>
#include <Cluster/Multiplexer.h>
#include <Cluster/StandardFile.h>
#include <Cluster/TCPPipe.h>
#include <Cluster/StandardDirectory.h>

namespace Cluster {

IO::FilePtr openFile(Multiplexer* multiplexer,const char* fileName,IO::File::AccessMode accessMode)
	{
	IO::FilePtr result;
	
	/* Open the base file: */
	if(strncmp(fileName,"http://",7)==0)
		{
		if(accessMode==IO::File::WriteOnly||accessMode==IO::File::ReadWrite)
			Misc::throwStdErr("Cluster::openFile: Write access to HTTP files not supported");
		
		if(multiplexer==0)
			{
			/* Open a non-shared remote file via the HTTP/1.1 protocol: */
			result=new Comm::HttpFile(fileName);
			}
		else if(multiplexer->isMaster())
			{
			/* Open a master-side shared TCP pipe: */
			Comm::HttpFile::URLParts urlParts=Comm::HttpFile::splitUrl(fileName);
			Comm::PipePtr pipe=new TCPPipeMaster(multiplexer,urlParts.serverName.c_str(),urlParts.portNumber);
			
			/* Open an HTTP file over the shared TCP pipe: */
			result=new Comm::HttpFile(urlParts,pipe);
			}
		else
			{
			/* Open a slave-side shared TCP pipe: */
			Comm::HttpFile::URLParts urlParts=Comm::HttpFile::splitUrl(fileName);
			Comm::PipePtr pipe=new TCPPipeSlave(multiplexer,urlParts.serverName.c_str(),urlParts.portNumber);
			
			/* Open an HTTP file over the shared TCP pipe: */
			result=new Comm::HttpFile(urlParts,pipe);
			}
		}
	else
		{
		if(multiplexer==0)
			{
			/* Open a non-shared standard file: */
			result=new IO::StandardFile(fileName,accessMode);
			}
		else if(multiplexer->isMaster())
			{
			/* Open a master-side shared standard file: */
			result=new StandardFileMaster(multiplexer,fileName,accessMode);
			}
		else
			{
			/* Open a slave-side shared standard file: */
			result=new StandardFileSlave(multiplexer,fileName,accessMode);
			}
		}
	
	/* Check if the file name has the .gz extension: */
	if(Misc::hasCaseExtension(fileName,".gz"))
		{
		/* Wrap a gzip filter around the base file: */
		result=new IO::GzipFilter(result);
		}
	
	/* Return the open file: */
	return result;
	}

IO::SeekableFilePtr openSeekableFile(Multiplexer* multiplexer,const char* fileName,IO::File::AccessMode accessMode)
	{
	/* Open a potentially non-seekable file first: */
	IO::FilePtr file=openFile(multiplexer,fileName,accessMode);
	
	/* Check if the file is already seekable: */
	IO::SeekableFilePtr result=file;
	if(result==0)
		{
		/* Wrap a seekable filter around the base file: */
		result=new IO::SeekableFilter(file);
		}
	
	return result;
	}

IO::DirectoryPtr openDirectory(Multiplexer* multiplexer,const char* directoryName)
	{
	if(multiplexer==0)
		{
		/* Open a non-shared standard directory: */
		return new IO::StandardDirectory(directoryName);
		}
	else if(multiplexer->isMaster())
		{
		/* Open a master-side shared standard directory: */
		return new Cluster::StandardDirectoryMaster(multiplexer,directoryName);
		}
	else
		{
		/* Open a slave-side shared standard directory: */
		return new Cluster::StandardDirectorySlave(multiplexer,directoryName);
		}
	}

}
