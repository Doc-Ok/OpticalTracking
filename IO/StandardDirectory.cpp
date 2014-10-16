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

#include <IO/StandardDirectory.h>

#include <Misc/GetCurrentDirectory.h>
#include <IO/OpenFile.h>

namespace IO {

/**********************************
Methods of class StandardDirectory:
**********************************/

StandardDirectory::StandardDirectory(const char* sPathName)
	:directory(0),
	 entry(0)
	{
	/* Prepend the current directory path to the path name if the given path name is relative: */
	if(sPathName[0]!='/')
		{
		pathName=Misc::getCurrentDirectory();
		pathName.push_back('/');
		pathName.append(sPathName);
		}
	else
		pathName=sPathName;
	
	/* Normalize the path name and open the directory: */
	normalizePath(pathName,1);
	directory=opendir(pathName.c_str());
	
	/* Check for failure: */
	if(directory==0)
		throw OpenError(pathName.c_str());
	}

StandardDirectory::StandardDirectory(const char* sPathName,int)
	:pathName(sPathName),
	 directory(opendir(pathName.c_str())),
	 entry(0)
	{
	/* Check for failure: */
	if(directory==0)
		throw OpenError(pathName.c_str());
	}

StandardDirectory::~StandardDirectory(void)
	{
	/* Close the directory: */
	if(directory!=0)
		closedir(directory);
	}

std::string StandardDirectory::getName(void) const
	{
	return std::string(getLastComponent(pathName,1),pathName.end());
	}

std::string StandardDirectory::getPath(void) const
	{
	return pathName;
	}

std::string StandardDirectory::getPath(const char* relativePath) const
	{
	/* Check if the given path is absolute: */
	if(relativePath[0]=='/')
		{
		/* Normalize the given absolute path name: */
		std::string result=relativePath;
		normalizePath(result,1);
		
		return result;
		}
	else
		{
		/* Assemble and normalize the absolute path name: */
		std::string result=pathName;
		if(result.length()>1)
			result.push_back('/');
		result.append(relativePath);
		normalizePath(result,1);
		
		return result;
		}
	}

bool StandardDirectory::hasParent(void) const
	{
	return pathName.length()>1;
	}

DirectoryPtr StandardDirectory::getParent(void) const
	{
	/* Check for the special case of the root directory: */
	if(pathName.length()==1)
		return 0;
	else
		{
		/* Find the last component in the absolute path name: */
		std::string::const_iterator lastCompIt=getLastComponent(pathName,1);
		
		/* Strip off the last slash unless it's the prefix: */
		if(lastCompIt-pathName.begin()>1)
			--lastCompIt;
		
		/* Open and return the directory corresponding to the path name prefix before the last slash: */
		return new StandardDirectory(std::string(pathName.begin(),lastCompIt).c_str(),0);
		}
	}

void StandardDirectory::rewind(void)
	{
	rewinddir(directory);
	entry=0;
	}

bool StandardDirectory::readNextEntry(void)
	{
	/* Read the next entry: */
	entry=readdir(directory);
	
	return entry!=0;
	}

const char* StandardDirectory::getEntryName(void) const
	{
	return entry->d_name;
	}

Misc::PathType StandardDirectory::getEntryType(void) const
	{
	#ifdef _DIRENT_HAVE_D_TYPE
	
	/* Convert the OS-level entry type to a Misc::PathType: */
	switch(entry->d_type)
		{
		case DT_REG:
			return Misc::PATHTYPE_FILE;
		
		case DT_DIR:
			return Misc::PATHTYPE_DIRECTORY;
		
		case DT_CHR:
			return Misc::PATHTYPE_CHARACTER_DEVICE;
		
		case DT_BLK:
			return Misc::PATHTYPE_BLOCK_DEVICE;
		
		case DT_FIFO:
			return Misc::PATHTYPE_NAMED_PIPE;
		
		case DT_LNK:
			return Misc::PATHTYPE_SYMBOLIC_LINK;
		
		case DT_SOCK:
			return Misc::PATHTYPE_SOCKET;
		
		default:
			return Misc::PATHTYPE_UNKNOWN;
		}
	
	#else
	
	/* Assemble the relative path name of the directory entry: */
	std::string entryPath=pathName;
	if(entryPath.length()>1)
		entryPath.push_back('/');
	entryPath.append(entry->d_name);
	
	/* Return the path's file type: */
	return Misc::getPathType(entryPath.c_str());
	
	#endif
	}

Misc::PathType StandardDirectory::getPathType(const char* relativePath) const
	{
	/* Check if the given path is absolute: */
	if(relativePath[0]=='/')
		{
		/* Use the given absolute path directly: */
		return Misc::getPathType(relativePath);
		}
	else
		{
		/* Assemble the absolute path name: */
		std::string absolutePath=pathName;
		if(absolutePath.length()>1)
			absolutePath.push_back('/');
		absolutePath.append(relativePath);
		
		/* Use the assembled absolute path: */
		return Misc::getPathType(absolutePath.c_str());
		}
	}

FilePtr StandardDirectory::openFile(const char* fileName,File::AccessMode accessMode) const
	{
	/* Assemble the absolute path name of the given file: */
	std::string filePath;
	if(fileName[0]=='/')
		{
		/* Use the provided absolute file path: */
		filePath=fileName;
		}
	else
		{
		/* Create a relative file path: */
		filePath=pathName;
		if(filePath.length()>1)
			filePath.push_back('/');
		filePath.append(fileName);
		}
	
	/* Open and return the file: */
	return IO::openFile(filePath.c_str(),accessMode);
	}

DirectoryPtr StandardDirectory::openDirectory(const char* directoryName) const
	{
	/* Assemble the absolute path name of the given directory: */
	std::string directoryPath;
	if(directoryName[0]=='/')
		{
		/* Use the provided absolute path: */
		directoryPath=directoryName;
		}
	else
		{
		/* Create a relative path: */
		directoryPath=pathName;
		if(directoryPath.length()>1)
			directoryPath.push_back('/');
		directoryPath.append(directoryName);
		}
	
	/* Open and return the new directory: */
	return new StandardDirectory(directoryPath.c_str());
	}

}
