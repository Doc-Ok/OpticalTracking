/***********************************************************************
StandardDirectory - Pair of classes to access cluster-transparent
standard filesystem directories.
Copyright (c) 2011-2014 Oliver Kreylos

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

#ifndef CLUSTER_STANDARDDIRECTORY_INCLUDED
#define CLUSTER_STANDARDDIRECTORY_INCLUDED

#include <dirent.h>
#include <string>
#include <IO/Directory.h>
#include <Cluster/MulticastPipe.h>

namespace Cluster {

class StandardDirectory:public IO::Directory // Common base class for master- and slave-side directories
	{
	/* Elements: */
	protected:
	mutable MulticastPipe pipe; // Multicast pipe to forward directory data to the slave nodes
	std::string pathName; // Path name of this directory
	Misc::PathType entryType; // Path type of current directory entry
	
	/* Constructors and destructors: */
	public:
	StandardDirectory(Multiplexer* sMultiplexer,const char* sPathName); // Opens the directory of the given path name
	protected:
	StandardDirectory(Multiplexer* sMultiplexer,const char* sPathName,int); // Ditto, but assumes that path is absolute and normalized
	public:
	
	/* Methods from IO::Directory: */
	virtual std::string getName(void) const;
	virtual std::string getPath(void) const;
	virtual std::string getPath(const char* relativePath) const;
	virtual bool hasParent(void) const;
	virtual IO::DirectoryPtr getParent(void) const;
	virtual Misc::PathType getEntryType(void) const;
	virtual IO::FilePtr openFile(const char* fileName,IO::File::AccessMode accessMode =IO::File::ReadOnly) const;
	virtual IO::DirectoryPtr openDirectory(const char* directoryName) const;
	};

class StandardDirectoryMaster:public StandardDirectory
	{
	friend class StandardDirectory;
	
	/* Elements: */
	private:
	DIR* directory; // Pointer to the directory access structure
	struct dirent* entry; // Pointer to the current directory entry (or 0 at end-of-directory)
	
	/* Constructors and destructors: */
	public:
	StandardDirectoryMaster(Multiplexer* sMultiplexer,const char* sPathName); // Opens the directory of the given path name
	private:
	StandardDirectoryMaster(Multiplexer* sMultiplexer,const char* sPathName,int); // Ditto, but assumes that path is absolute and normalized
	public:
	virtual ~StandardDirectoryMaster(void);
	
	/* Methods from IO::Directory: */
	virtual void rewind(void);
	virtual bool readNextEntry(void);
	virtual const char* getEntryName(void) const;
	virtual Misc::PathType getPathType(const char* relativePath) const;
	};

class StandardDirectorySlave:public StandardDirectory
	{
	friend class StandardDirectory;
	
	/* Elements: */
	private:
	std::string entryName; // Name of current directory entry
	
	/* Constructors and destructors: */
	public:
	StandardDirectorySlave(Multiplexer* sMultiplexer,const char* sPathName); // Opens the directory of the given path name
	private:
	StandardDirectorySlave(Multiplexer* sMultiplexer,const char* sPathName,int); // Ditto, but assumes that path is absolute and normalized
	public:
	virtual ~StandardDirectorySlave(void);
	
	/* Methods from IO::Directory: */
	virtual void rewind(void);
	virtual bool readNextEntry(void);
	virtual const char* getEntryName(void) const;
	virtual Misc::PathType getPathType(const char* relativePath) const;
	};

}

#endif
