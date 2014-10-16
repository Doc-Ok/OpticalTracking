/***********************************************************************
FileLocator - Class to find files from an ordered list of search paths.
Copyright (c) 2007-2011 Oliver Kreylos
Based on code written by Braden Pellett.

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

#ifndef MISC_FILELOCATOR_INCLUDED
#define MISC_FILELOCATOR_INCLUDED

#include <string>
#include <vector>
#include <stdexcept>

namespace Misc {

class FileLocator
	{
	/* Embedded classes: */
	public:
	class FileNotFound:public std::runtime_error // Exception class to signal that a requested file was not found in any search path
		{
		/* Elements: */
		public:
		char fileName[128]; // The requested file name
		
		/* Constructors and destructors: */
		FileNotFound(const char* sFileName);
		};
	
	/* Elements: */
	private:
	std::vector<std::string> pathList; // List of search paths, in order of search
	
	/* Constructors and destructors: */
	public:
	FileLocator(void); // Creates a file locator with an empty search path list
	
	/* Methods: */
	void addCurrentDirectory(void); // Adds the current working directory to the search path list
	void addPath(const char* newPath); // Adds a new path to the search path list
	void addPath(const char* newPathBegin,const char* newPathEnd); // Ditto
	void addPath(std::string newPath) // Ditto
		{
		addPath(newPath.c_str());
		}
	void addPathFromFile(const char* fileName); // Adds the path leading to the given file name to the search path list
	void addPathList(const char* newPathList); // Adds a list of colon-separated paths to the search path list
	void addPathList(std::string newPathList) // Ditto
		{
		addPathList(newPathList.c_str());
		}
	void addPathFromApplication(const char* executablePath); // Assumes that the executable names an "application package" and adds the application's resource paths to the search path list
	void addPathFromApplication(std::string executablePath) // Ditto
		{
		addPathFromApplication(executablePath.c_str());
		}
	const std::vector<std::string>& getPathList(void) const // Returns the path list
		{
		return pathList;
		}
	std::string locateFile(const char* fileName); // Returns the path of the first file matching the given name from the search path list
	};

}

#endif
