/***********************************************************************
ValueCoder - Generic class to encode/decode values into/from human-
readable strings.
Copyright (c) 2004-2010 Oliver Kreylos

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

#ifndef MISC_VALUECODER_INCLUDED
#define MISC_VALUECODER_INCLUDED

#include <stdexcept>
#include <string>

namespace Misc {

/*****************************************
Exception class to report decoding errors:
*****************************************/

class DecodingError:public std::runtime_error
	{
	/* Constructors and destructors: */
	public:
	DecodingError(const std::string& what_arg)
		:std::runtime_error(what_arg)
		{
		}
	};

/*************************
Generic value coder class:
*************************/

template <class ValueParam>
class ValueCoder
	{
	/* Dummy class; only specializations can be used */
	
	#if 0
	/* Methods that must be provided by specialized classes: */
	static std::string encode(const ValueParam& value); // Encodes value into ASCII string; returns string
	static ValueParam decode(const char* start,const char* end,const char** decodeEnd =0); // Decodes value from string between start and end; sets decodeEnd one behind last decoded character
	#endif
	};

/****************
Helper functions:
****************/

const char* skipWhitespace(const char* start,const char* end);
const char* checkSeparator(char separator,const char* start,const char* end);

}

#endif
