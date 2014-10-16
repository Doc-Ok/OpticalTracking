/***********************************************************************
StandardDirectory - Class to access standard operating system
directories.
Copyright (c) 2010-2014 Oliver Kreylos

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

#ifndef IO_STANDARDDIRECTORY_INCLUDED
#define IO_STANDARDDIRECTORY_INCLUDED

#include <dirent.h>
#include <string>
#include <IO/Directory.h>

namespace IO {

class StandardDirectory:public Directory
	{
	/* Elements: */
	private:
	std::string pathName; // Fully-qualified path name of this directory
	DIR* directory; // Pointer to the directory access structure
	struct dirent* entry; // Pointer to the current directory entry (or 0 at end-of-directory)
	
	/* Constructors and destructors: */
	public:
	StandardDirectory(const char* sPathName); // Opens the directory of the given relative or absolute path name
	private:
	StandardDirectory(const char* sPathName,int); // Ditto; but assumes that given path name is absolute and normalized
	public:
	virtual ~StandardDirectory(void);
	
	/* Methods from Directory: */
	virtual std::string getName(void) const;
	virtual std::string getPath(void) const;
	virtual std::string getPath(const char* relativePath) const;
	virtual bool hasParent(void) const;
	virtual DirectoryPtr getParent(void) const;
	virtual void rewind(void);
	virtual bool readNextEntry(void);
	virtual const char* getEntryName(void) const;
	virtual Misc::PathType getEntryType(void) const;
	virtual Misc::PathType getPathType(const char* relativePath) const;
	virtual FilePtr openFile(const char* fileName,File::AccessMode accessMode =File::ReadOnly) const;
	virtual DirectoryPtr openDirectory(const char* directoryName) const;
	};

}

#endif
