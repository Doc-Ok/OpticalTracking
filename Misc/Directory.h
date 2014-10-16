/***********************************************************************
Directory - Wrapper class for the unistd DIR interface with exception
safety.
Copyright (c) 2010 Oliver Kreylos

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

#ifndef MISC_DIRECTORY_INCLUDED
#define MISC_DIRECTORY_INCLUDED

#include <dirent.h>
#ifndef _DIRENT_HAVE_D_TYPE
#include <string>
#endif
#include <stdexcept>
#include <Misc/ThrowStdErr.h>
#include <Misc/FileTests.h>

namespace Misc {

class Directory
	{
	/* Embedded classes: */
	public:
	class OpenError:public std::runtime_error // Exception class to report directory opening errors
		{
		/* Constructors and destructors: */
		public:
		OpenError(const char* directoryName)
			:std::runtime_error(printStdErrMsg("Misc::Directory: Error opening directory %s",directoryName))
			{
			}
		};
	
	/* Elements: */
	private:
	#ifndef _DIRENT_HAVE_D_TYPE
	std::string directoryName; // Name given when directory was opened; necessary to determine file types without D_TYPE field
	#endif
	DIR* directory; // Pointer to the directory structure
	struct dirent* entry; // Pointer to the current directory entry (or 0 at end-of-directory)
	
	/* Constructors and destructors: */
	public:
	Directory(const char* sDirectoryName); // Opens the given directory
	~Directory(void); // Closes the directory
	
	/* Methods: */
	bool eod(void) const // Returns true if all directory entries have been read
		{
		return entry==0;
		}
	bool readNextEntry(void); // Reads the next directory entry; returns false if there are no more entries
	const char* getEntryName(void) const // Returns the name of the current entry
		{
		return entry->d_name;
		}
	PathType getEntryType(void) const; // Returns the filesystem object type of the current entry
	};

}

#endif
