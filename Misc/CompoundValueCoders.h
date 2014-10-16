/***********************************************************************
CompoundValueCoders - Generic value coder classes for vectors and lists
of other data types.
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

#ifndef MISC_COMPOUNDVALUECODERS_INCLUDED
#define MISC_COMPOUNDVALUECODERS_INCLUDED

#include <utility>
#include <list>
#include <vector>
#include <Misc/ValueCoder.h>

namespace Misc {

/**************************
Generic ValueCoder classes:
**************************/

template <class FirstParam,class SecondParam>
class ValueCoder<std::pair<FirstParam,SecondParam> >
	{
	/* Methods: */
	public:
	static std::string encode(const std::pair<FirstParam,SecondParam>& value);
	static std::pair<FirstParam,SecondParam> decode(const char* start,const char* end,const char** decodeEnd =0);
	};

template <class ValueParam>
class ValueCoder<std::list<ValueParam> >
	{
	/* Methods: */
	public:
	static std::string encode(const std::list<ValueParam>& value);
	static std::list<ValueParam> decode(const char* start,const char* end,const char** decodeEnd =0);
	};

template <class ValueParam>
class ValueCoder<std::vector<ValueParam> >
	{
	/* Methods: */
	public:
	static std::string encode(const std::vector<ValueParam>& value);
	static std::vector<ValueParam> decode(const char* start,const char* end,const char** decodeEnd =0);
	};

}

#if !defined(MISC_COMPOUNDVALUECODERS_IMPLEMENTATION)
#include <Misc/CompoundValueCoders.icpp>
#endif

#endif
