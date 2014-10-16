/***********************************************************************
SizedTypes - Type declarations for signed and unsigned integers and IEEE
floating-point numbers of fixed specified sizes to guarantee matching
types across network connections.
Copyright (c) 2011 Oliver Kreylos

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

#ifndef MISC_SIZEDTYPES_INCLUDED
#define MISC_SIZEDTYPES_INCLUDED

#include <stdint.h>

namespace Misc {

typedef int8_t SInt8; // 8-bit signed integers
typedef int16_t SInt16; // 16-bit signed integers
typedef int32_t SInt32; // 32-bit signed integers
typedef int64_t SInt64; // 64-bit signed integers

typedef uint8_t UInt8; // 8-bit unsigned integers
typedef uint16_t UInt16; // 16-bit unsigned integers
typedef uint32_t UInt32; // 32-bit unsigned integers
typedef uint64_t UInt64; // 64-bit unsigned integers

typedef float Float32; // 32-bit IEEE floating-point numbers
typedef double Float64; // 64-bit IEEE floating-point numbers

}

#endif
