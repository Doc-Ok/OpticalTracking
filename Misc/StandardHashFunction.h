/***********************************************************************
StandardHashFunction - Generic class (and specializations for common
data types) to calculate hash functions.
Copyright (c) 2005 Oliver Kreylos

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

#ifndef MISC_STANDARDHASHFUNCTION_INCLUDED
#define MISC_STANDARDHASHFUNCTION_INCLUDED

namespace Misc {

/*********************
Generic hash function:
*********************/

template <class Source>
class StandardHashFunction
	{
	/* Static methods: */
	public:
	static size_t rawHash(const Source& source)
		{
		/* Standard hasher reinterprets object as bit pattern and runs simple CRC on that: */
		size_t result=0;
		size_t sourceSize=sizeof(source);
		
		/* Process complete words first: */
		const size_t* h1Ptr;
		for(h1Ptr=(const size_t*)&source;sourceSize>=sizeof(size_t);sourceSize-=sizeof(size_t),++h1Ptr)
			result+=*h1Ptr;
		
		/* Process rest of source byte-wise: */
		size_t lastBytes=0;
		const unsigned char* h2Ptr;
		for(h2Ptr=(const unsigned char*)h1Ptr;sourceSize>0;sourceSize-=sizeof(unsigned char),++h2Ptr)
			lastBytes=(lastBytes<<8)+*h2Ptr;
		result+=lastBytes;
		
		return result;
		}
	static size_t hash(const Source& source,size_t tableSize)
		{
		return rawHash(source)%tableSize;
		}
	};

/**************************************************
Specialized hash functions for standard data types:
**************************************************/

template <>
class StandardHashFunction<bool>
	{
	/* Static methods: */
	public:
	static size_t rawHash(bool source)
		{
		return source?0x80000000U:0;
		}
	static size_t hash(bool source,size_t tableSize)
		{
		return source?(tableSize>>1)+1:0;
		}
	};

template <>
class StandardHashFunction<unsigned char>
	{
	/* Static methods: */
	public:
	static size_t rawHash(unsigned char source)
		{
		return source;
		}
	static size_t hash(unsigned char source,size_t tableSize)
		{
		return source%tableSize;
		}
	};

template <>
class StandardHashFunction<signed char>
	{
	/* Static methods: */
	public:
	static size_t rawHash(signed char source)
		{
		return (size_t)((int)source+128);
		}
	static size_t hash(signed char source,size_t tableSize)
		{
		return (size_t)((int)source+128)%tableSize;
		}
	};

template <>
class StandardHashFunction<unsigned short>
	{
	/* Static methods: */
	public:
	static size_t rawHash(unsigned short source)
		{
		return source;
		}
	static size_t hash(unsigned short source,size_t tableSize)
		{
		return source%tableSize;
		}
	};

template <>
class StandardHashFunction<signed short>
	{
	/* Static methods: */
	public:
	static size_t rawHash(signed short source)
		{
		return (size_t)((int)source+32768);
		}
	static size_t hash(signed short source,size_t tableSize)
		{
		return (size_t)((int)source+32768)%tableSize;
		}
	};

template <>
class StandardHashFunction<unsigned int>
	{
	/* Static methods: */
	public:
	static size_t rawHash(unsigned int source)
		{
		return source;
		}
	static size_t hash(unsigned int source,size_t tableSize)
		{
		return source%tableSize;
		}
	};

template <>
class StandardHashFunction<signed int>
	{
	/* Static methods: */
	public:
	static size_t rawHash(signed int source)
		{
		return size_t(source);
		}
	static size_t hash(signed int source,size_t tableSize)
		{
		return size_t(source)%tableSize;
		}
	};

template <class Source>
class StandardHashFunction<const Source*>
	{
	/* Static methods: */
	public:
	static size_t rawHash(const Source* source)
		{
		return reinterpret_cast<size_t>(source);
		}
	static size_t hash(const Source* source,size_t tableSize)
		{
		return reinterpret_cast<size_t>(source)%tableSize;
		}
	};

template <class Source>
class StandardHashFunction<Source*>
	{
	/* Static methods: */
	public:
	static size_t rawHash(Source* source)
		{
		return reinterpret_cast<size_t>(source);
		}
	static size_t hash(Source* source,size_t tableSize)
		{
		return reinterpret_cast<size_t>(source)%tableSize;
		}
	};

}

#endif
