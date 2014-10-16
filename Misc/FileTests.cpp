/***********************************************************************
FileTests - Helper functions to query information about files and
directories in an operating system-independent manner.
Copyright (c) 2010-2011 Oliver Kreylos

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

#include <Misc/FileTests.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace Misc {

bool doesPathExist(const char* path)
	{
	/* Stat the path: */
	struct stat statBuffer;
	return stat(path,&statBuffer)==0;
	}

PathType getPathType(const char* path)
	{
	/* Stat the path: */
	struct stat statBuffer;
	if(stat(path,&statBuffer)==0)
		{
		/* Return the appropriate file type: */
		if(S_ISREG(statBuffer.st_mode))
			return PATHTYPE_FILE;
		if(S_ISDIR(statBuffer.st_mode))
			return PATHTYPE_DIRECTORY;
		if(S_ISCHR(statBuffer.st_mode))
			return PATHTYPE_CHARACTER_DEVICE;
		if(S_ISBLK(statBuffer.st_mode))
			return PATHTYPE_CHARACTER_DEVICE;
		if(S_ISFIFO(statBuffer.st_mode))
			return PATHTYPE_NAMED_PIPE;
		if(S_ISLNK(statBuffer.st_mode))
			return PATHTYPE_SYMBOLIC_LINK;
		if(S_ISSOCK(statBuffer.st_mode))
			return PATHTYPE_SOCKET;
		
		/* This should never happen: */
		return PATHTYPE_UNKNOWN;
		}
	else
		return PATHTYPE_DOES_NOT_EXIST;
	}

bool isPathFile(const char* path)
	{
	/* Stat the path: */
	struct stat statBuffer;
	if(stat(path,&statBuffer)==0)
		{
		/* Check if the object is a regular file: */
		return S_ISREG(statBuffer.st_mode);
		}
	else
		return false;
	}

bool isFileReadable(const char* path)
	{
	/* Stat the path: */
	struct stat statBuffer;
	if(stat(path,&statBuffer)==0)
		{
		/* Check if the object is a regular file: */
		if(S_ISREG(statBuffer.st_mode))
			{
			/* Check if the file is readable: */
			if(geteuid()==statBuffer.st_uid) // Is user the owner?
				return (statBuffer.st_mode&S_IRUSR)!=0x0;
			if(getegid()==statBuffer.st_gid) // Is user in the same group as owner?
				return (statBuffer.st_mode&S_IRGRP)!=0x0;
			return (statBuffer.st_mode&S_IROTH)!=0x0;
			}
		else
			return false;
		}
	else
		return false;
	}

bool isFileWriteable(const char* path)
	{
	/* Stat the path: */
	struct stat statBuffer;
	if(stat(path,&statBuffer)==0)
		{
		/* Check if the object is a regular file: */
		if(S_ISREG(statBuffer.st_mode))
			{
			/* Check if the file is writeable: */
			if(geteuid()==statBuffer.st_uid) // Is user the owner?
				return (statBuffer.st_mode&S_IWUSR)!=0x0;
			if(getegid()==statBuffer.st_gid) // Is user in the same group as owner?
				return (statBuffer.st_mode&S_IWGRP)!=0x0;
			return (statBuffer.st_mode&S_IWOTH)!=0x0;
			}
		else
			return false;
		}
	else
		return false;
	}

bool isPathDirectory(const char* path)
	{
	/* Stat the path: */
	struct stat statBuffer;
	if(stat(path,&statBuffer)==0)
		{
		/* Check if the object is a directory: */
		return S_ISDIR(statBuffer.st_mode);
		}
	else
		return false;
	}

}
