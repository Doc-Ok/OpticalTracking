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

#ifndef MISC_FILETESTS_INCLUDED
#define MISC_FILETESTS_INCLUDED

namespace Misc {

enum PathType // Enumerated type for filesystem object types
	{
	PATHTYPE_DOES_NOT_EXIST,
	PATHTYPE_FILE,PATHTYPE_DIRECTORY,
	PATHTYPE_CHARACTER_DEVICE,PATHTYPE_BLOCK_DEVICE,
	PATHTYPE_NAMED_PIPE,
	PATHTYPE_SYMBOLIC_LINK,
	PATHTYPE_SOCKET,
	PATHTYPE_UNKNOWN
	};

bool doesPathExist(const char* path); // Returns true if a filesystem object exists at the given path
PathType getPathType(const char* path); // Returns the type of the filesystem object at the given path, or PATHTYPE_DOES_NOT_EXIST if the object does not exist
bool isPathFile(const char* path); // Returns true if the given path points to a regular file
bool isFileReadable(const char* path); // Returns true of the given path points to a regular file that can be read by the current user
bool isFileWriteable(const char* path); // Returns true of the given path points to a regular file that can be written by the current user
bool isPathDirectory(const char* path); // Returns true if the given path points to a directory

}

#endif
