/***********************************************************************
StringHashFunctions - Specialization of Misc::StandardHashFunction class
for C++ strings, and new StringHashFunction class for C strings.
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

#ifndef MISC_STRINGHASHFUNCTIONS_INCLUDED
#define MISC_STRINGHASHFUNCTIONS_INCLUDED

#include <string>

/* Forward declarations: */
namespace Misc {
template <class Source>
class StandardHashFunction;
}

namespace Misc {

template <>
class StandardHashFunction<std::string>
	{
	/* Static methods: */
	public:
	static size_t rawHash(std::string source)
		{
		size_t result=0;
		for(std::string::const_iterator sIt=source.begin();sIt!=source.end();++sIt)
			result=result*37+size_t(*sIt);
		return result;
		}
	static size_t hash(std::string source,size_t tableSize)
		{
		return rawHash(source)%tableSize;
		}
	};

class StringHashFunction
	{
	/* Static methods: */
	public:
	static size_t rawHash(const char* source)
		{
		size_t result=0;
		for(const char* sPtr=source;*sPtr!='\0';++sPtr)
			result=result*37+size_t(*sPtr);
		return result;
		}
	static size_t hash(const char* source,size_t tableSize)
		{
		return rawHash(source)%tableSize;
		}
	};

}

#endif
