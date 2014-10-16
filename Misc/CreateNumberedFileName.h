/***********************************************************************
CreateNumberedFileName - Helper function to create file names with
monotonically increasing numbers based on a given base file name.
Zero-padded numbers with the given number of digits are inserted
immediately before the base file name's extension, and the number is
guaranteed to be higher than any other matchingly numbered file in the
same directory.
Copyright (c) 2008-2009 Oliver Kreylos

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

#ifndef MISC_CREATENUMBEREDFILENAME_INCLUDED
#define MISC_CREATENUMBEREDFILENAME_INCLUDED

#include <string>

namespace Misc {

char* createNumberedFileName(const char* baseFileName,int numDigits,char* fileNameBuffer);
std::string createNumberedFileName(const std::string& baseFileName,int numDigits);

}

#endif
