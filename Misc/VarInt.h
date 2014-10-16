/***********************************************************************
VarInt - Functions to read/write unsigned 32-bit integers to/from files
using a variable number of bytes: values < 192 are written as one byte,
values < 12288 as two bytes, values < 786432 as three bytes,
values < 50331648 as four bytes, and any larger values as five bytes.
Copyright (c) 2014 Oliver Kreylos

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

#ifndef MISC_VARINT_INCLUDED
#define MISC_VARINT_INCLUDED

#include <Misc/SizedTypes.h>

namespace Misc {

template <class SinkParam>
inline
void
writeVarInt(UInt32 value,
	SinkParam& sink)
	{
	for(int i=0;i<4&&value>=0xc0U;++i)
		{
		/* Write the value's least-significant six bits, with the two MSBs set to signal there's more to come: */
		sink.write(Misc::UInt8(value|0xc0U));
		value>>=6;
		}
	
	/* Write the value's remaining 8 bits: */
	sink.write(Misc::UInt8(value));
	}

template <class SourceParam>
inline
UInt32
readVarInt(
	SourceParam& source)
	{
	UInt32 value=0x0U;
	for(int shift=0;shift<30;shift+=6)
		{
		UInt32 byte=UInt32(source.template read<Misc::UInt8>());
		value&=~(0x3U<<shift);
		value|=byte<<shift;
		if(byte<0xc0U)
			break;
		}
	
	return value;
	}

}

#endif
