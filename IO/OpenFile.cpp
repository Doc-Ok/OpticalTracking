/***********************************************************************
OpenFile - Convenience functions to open files of several types using
the File abstraction.
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

#include <IO/OpenFile.h>

#include <Misc/ThrowStdErr.h>
#include <Misc/FileNameExtensions.h>
#include <IO/StandardFile.h>
#include <IO/GzipFilter.h>
#include <IO/SeekableFilter.h>
#include <IO/StandardDirectory.h>

namespace IO {

FilePtr openFile(const char* fileName,File::AccessMode accessMode)
	{
	FilePtr result;
	
	/* Open the base file: */
	result=new StandardFile(fileName,accessMode);
	
	/* Check if the file name has the .gz extension: */
	if(Misc::hasCaseExtension(fileName,".gz"))
		{
		/* Wrap a gzip filter around the base file: */
		result=new GzipFilter(result);
		}
	
	/* Return the open file: */
	return result;
	}

SeekableFilePtr openSeekableFile(const char* fileName,File::AccessMode accessMode)
	{
	/* Open a potentially non-seekable file first: */
	FilePtr file=openFile(fileName,accessMode);
	
	/* Check if the file is already seekable: */
	SeekableFilePtr result=file;
	if(result==0)
		{
		/* Wrap a seekable filter around the base file: */
		result=new SeekableFilter(file);
		}
	
	return result;
	}

DirectoryPtr openDirectory(const char* directoryName)
	{
	return new StandardDirectory(directoryName);
	}

}
