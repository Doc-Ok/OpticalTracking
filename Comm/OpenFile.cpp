/***********************************************************************
OpenFile - Convenience functions to open files of several types using
the File abstraction.
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

#include <Comm/OpenFile.h>

#include <string.h>
#include <Misc/ThrowStdErr.h>
#include <Misc/FileNameExtensions.h>
#include <IO/StandardFile.h>
#include <IO/GzipFilter.h>
#include <IO/SeekableFilter.h>
#include <Comm/HttpFile.h>

namespace Comm {

IO::FilePtr openFile(const char* fileName,IO::File::AccessMode accessMode)
	{
	IO::FilePtr result;
	
	/* Open the base file: */
	if(strncmp(fileName,"http://",7)==0)
		{
		if(accessMode==IO::File::WriteOnly||accessMode==IO::File::ReadWrite)
			Misc::throwStdErr("Comm::openFile: Write access to HTTP files not supported");
		
		/* Open a remote file via the HTTP/1.1 protocol: */
		result=new HttpFile(fileName);
		}
	else
		{
		/* Open a standard file: */
		result=new IO::StandardFile(fileName,accessMode);
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

IO::SeekableFilePtr openSeekableFile(const char* fileName,IO::File::AccessMode accessMode)
	{
	/* Open a potentially non-seekable file first: */
	IO::FilePtr file=openFile(fileName,accessMode);
	
	/* Check if the file is already seekable: */
	IO::SeekableFilePtr result=file;
	if(result==0)
		{
		/* Wrap a seekable filter around the base file: */
		result=new IO::SeekableFilter(file);
		}
	
	return result;
	}

}
