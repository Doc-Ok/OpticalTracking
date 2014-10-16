/***********************************************************************
Endianness - Helper functions to deal with endianness conversion of
basic data types (extensible via template specialization mechanism).
Copyright (c) 2001-2005 Oliver Kreylos

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

#ifndef MISC_ENDIANNESS_INCLUDED
#define MISC_ENDIANNESS_INCLUDED

#include <stddef.h>
#ifdef __APPLE__
#include <machine/endian.h>
#define __BIG_ENDIAN __DARWIN_BIG_ENDIAN
#define __LITTLE_ENDIAN __DARWIN_LITTLE_ENDIAN
#define __BYTE_ORDER __DARWIN_BYTE_ORDER
#endif
#ifdef __linux__
#include <endian.h>
#endif

namespace Misc {

enum Endianness // Enumerated type to select endianness types
	{
	HostEndianness, // Data has same endianness as host, i.e., never swap
	LittleEndian, // Data has little endianness
	BigEndian // Data has big endianness
	};

/****************************************************************
Helper class to allow partial specialization of endianness
swapper:
****************************************************************/

template <class ValueParam>
class EndiannessSwapper
	{
	/* Methods: */
	public:
	static void swap(ValueParam& value)
		{
		/* Treat value as array of bytes: */
		unsigned char* bytes=reinterpret_cast<unsigned char*>(&value);
		
		/* Swap value byte by byte: */
		size_t i1,i2;
		for(i1=0,i2=sizeof(ValueParam)-1;i1<i2;++i1,--i2)
			{
			unsigned char temp=bytes[i1];
			bytes[i1]=bytes[i2];
			bytes[i2]=temp;
			}
		}
	static void swap(ValueParam* values,size_t numValues)
		{
		for(size_t i=0;i<numValues;++i)
			{
			/* Treat value as array of bytes: */
			unsigned char* bytes=reinterpret_cast<unsigned char*>(values+i);
			
			/* Swap value byte by byte: */
			size_t i1,i2;
			for(i1=0,i2=sizeof(ValueParam)-1;i1<i2;++i1,--i2)
				{
				unsigned char temp=bytes[i1];
				bytes[i1]=bytes[i2];
				bytes[i2]=temp;
				}
			}
		}
	};

/****************************************************************
Generic function to swap endianness of basic data types in-place:
****************************************************************/

template <class ValueParam>
inline
void
swapEndianness(
	ValueParam& value)
	{
	EndiannessSwapper<ValueParam>::swap(value);
	}

template <>
inline
void
swapEndianness(
	char&)
	{
	/* Dummy function - no need to swap bytes! */
	}

template <>
inline
void
swapEndianness(
	unsigned char&)
	{
	/* Dummy function - no need to swap bytes! */
	}

template <>
inline
void
swapEndianness(
	signed char&)
	{
	/* Dummy function - no need to swap bytes! */
	}

/**************************************************************************
Generic function to swap endianness of arrays of basic data types in-place:
**************************************************************************/

template <class ValueParam>
inline
void
swapEndianness(
	ValueParam* values,
	size_t numValues)
	{
	EndiannessSwapper<ValueParam>::swap(values,numValues);
	}

template <>
inline
void
swapEndianness(
	char*,
	size_t)
	{
	/* Dummy function - no need to swap bytes! */
	}

template <>
inline
void
swapEndianness(
	unsigned char*,
	size_t)
	{
	/* Dummy function - no need to swap bytes! */
	}

template <>
inline
void
swapEndianness(
	signed char*,
	size_t)
	{
	/* Dummy function - no need to swap bytes! */
	}

}

#endif
