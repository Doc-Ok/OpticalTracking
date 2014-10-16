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

#include <Misc/Directory.h>

namespace Misc {

/**************************
Methods of class Directory:
**************************/

Directory::Directory(const char* sDirectoryName)
	:
	#ifndef _DIRENT_HAVE_D_TYPE
	 directoryName(sDirectoryName),
	#endif
	 directory(opendir(sDirectoryName)),entry(0)
	{
	/* Check for failure: */
	if(directory==0)
		throw OpenError(sDirectoryName);
	
	/* Read the first directory entry: */
	entry=readdir(directory);
	}

Directory::~Directory(void)
	{
	if(directory!=0)
		closedir(directory);
	}

bool Directory::readNextEntry()
	{
	/* Read the next entry: */
	entry=readdir(directory);
	
	/* Check for eod: */
	return entry!=0;
	}

PathType Directory::getEntryType(void) const
	{
	#ifdef _DIRENT_HAVE_D_TYPE
	
	switch(entry->d_type)
		{
		case DT_REG:
			return PATHTYPE_FILE;
		
		case DT_DIR:
			return PATHTYPE_DIRECTORY;
		
		case DT_CHR:
			return PATHTYPE_CHARACTER_DEVICE;
		
		case DT_BLK:
			return PATHTYPE_BLOCK_DEVICE;
		
		case DT_FIFO:
			return PATHTYPE_NAMED_PIPE;
		
		case DT_LNK:
			return PATHTYPE_SYMBOLIC_LINK;
		
		case DT_SOCK:
			return PATHTYPE_SOCKET;
		
		default:
			return PATHTYPE_UNKNOWN;
		}
	
	#else
	
	/* Assemble the relative path name of the directory entry: */
	std::string entryPath=directoryName;
	entryPath.push_back('/');
	entryPath.append(entry->d_name);
	
	/* Return the path's file type: */
	return getPathType(entryPath.c_str());
	
	#endif
	}

}
