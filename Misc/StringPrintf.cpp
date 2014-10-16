/***********************************************************************
StringPrintf - Helper function to print to a C++ string using a printf-
style function call.
Copyright (c) 2009 Oliver Kreylos

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

#include <Misc/StringPrintf.h>

#include <stdio.h>
#include <stdarg.h>

namespace Misc {

std::string stringPrintf(const char* formatString,...)
	{
	static char printBuffer[256]; // Initial buffer for printf
	va_list ap;
	
	/* Print into the static buffer first: */
	va_start(ap,formatString);
	int result=vsnprintf(printBuffer,sizeof(printBuffer),formatString,ap);
	va_end(ap);
	
	/* Check for overflow in the print buffer: */
	if(result>=int(sizeof(printBuffer)))
		{
		/* Allocate a dynamic buffer of sufficient size: */
		char* buffer=new char[result+1];
		
		/* Print again, this time into the dynamic buffer: */
		va_start(ap,formatString);
		vsnprintf(buffer,result+1,formatString,ap);
		va_end(ap);
		
		/* Create the result, delete the dynamic buffer, return: */
		std::string result=buffer;
		delete[] buffer;
		return result;
		}
	else
		return std::string(printBuffer);
	}

}
