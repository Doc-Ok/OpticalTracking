/***********************************************************************
Directory - Base class to access directory-like objects in a generic
fashion.
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

#ifndef IO_DIRECTORY_INCLUDED
#define IO_DIRECTORY_INCLUDED

#include <string>
#include <stdexcept>
#include <Misc/RefCounted.h>
#include <Misc/Autopointer.h>
#include <Misc/FileTests.h>
#include <IO/File.h>

/* Forward declarations: */
namespace IO {
class Directory;
}

namespace IO {

typedef Misc::Autopointer<Directory> DirectoryPtr; // Type for pointers to reference-counted directory objects

class Directory:public Misc::RefCounted
	{
	/* Embedded classes: */
	public:
	class OpenError:public std::runtime_error // Exception class to report directory opening errors
		{
		/* Constructors and destructors: */
		public:
		OpenError(const char* directoryName);
		};
	
	/* Protected methods: */
	protected:
	static std::string& normalizePath(std::string& path,unsigned int prefixLength); // Normalizes the given path in-place, prefixLength is the length of the path prefix that denotes the root directory; throws OpenError exception if path dips below root directory
	static std::string::const_iterator getLastComponent(const std::string& path,unsigned int prefixLength); // Returns iterator to the beginning of the last component of the given path name, or the prefix in case of the root directory
	
	/* Constructors and destructors: */
	public:
	virtual ~Directory(void);
	
	/* Directory tree traversal methods: */
	virtual std::string getName(void) const =0; // Returns the name of this directory within its parent directory
	virtual std::string getPath(void) const =0; // Returns the path name of this directory
	virtual std::string getPath(const char* relativePath) const =0; // Returns the normalized absolute path corresponding to the given relative path; throws OpenError if path is invalid
	virtual bool hasParent(void) const =0; // Returns true if this directory has a parent directory
	virtual DirectoryPtr getParent(void) const =0; // Returns the parent directory of this directory, or 0 if the directory is the root directory
	virtual void rewind(void) =0; // Rewinds the directory such that the next call to readNextEntry returns the first entry again
	virtual bool readNextEntry(void) =0; // Reads the next directory entry; returns false if there are no more entries
	virtual const char* getEntryName(void) const =0; // Returns the name of the current directory entry
	virtual Misc::PathType getEntryType(void) const =0; // Returns the filesystem object type of the current directory entry
	
	/* Convenience methods: */
	virtual Misc::PathType getPathType(const char* relativePath) const =0; // Returns the filesystem object type of the given path relative to this directory, or starting from this directory's root directory if the path begins with a '/'
	
	/* Helper methods: */
	virtual std::string createNumberedFileName(const char* fileNameTemplate,int numDigits); // Returns a file name unique in this directory by inserting a unique number before the file name's first extension
	
	/* File and directory opening methods: */
	virtual FilePtr openFile(const char* fileName,File::AccessMode accessMode =File::ReadOnly) const =0; // Opens the file of the given directory-relative name with the given access mode
	virtual DirectoryPtr openDirectory(const char* directoryName) const =0; // Opens the subdirectory of the given directory-relative name
	};

}

#endif
