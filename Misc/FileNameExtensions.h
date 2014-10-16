/***********************************************************************
FileNameExtensions - Helper functions to extract or test path names or
extensions from file names.
Copyright (c) 2009-2012 Oliver Kreylos

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

#ifndef MISC_FILENAMEEXTENSIONS_INCLUDED
#define MISC_FILENAMEEXTENSIONS_INCLUDED

namespace Misc {

const char* getFileName(const char* pathName); // Returns a pointer to the first character after the last directory separator in the given path name
const char* getExtension(const char* fileName); // Returns a pointer to the extension of the last component of the given file / path name including the period; returns empty string if no extension
const char* getExtension(const char* fileNameBegin,const char* fileNameEnd); // Ditto, using the end pointer as the end of the file name
bool hasExtension(const char* fileName,const char* extension); // Returns true if the extension of the last component of the given file / path name matches the given extension (period included, empty string matches against no extension)
bool hasCaseExtension(const char* fileName,const char* extension); // Ditto, with case-insensitive comparison

}

#endif
