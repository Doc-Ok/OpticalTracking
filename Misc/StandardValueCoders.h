/***********************************************************************
StandardValueCoders - Specialized ValueCoder classes for standard data
types.
Copyright (c) 2004-2012 Oliver Kreylos

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

#ifndef MISC_STANDARDVALUECODERS_INCLUDED
#define MISC_STANDARDVALUECODERS_INCLUDED

#include <Misc/ValueCoder.h>

namespace Misc {

/******************************
Specialized ValueCoder classes:
******************************/

template <>
class ValueCoder<std::string>
	{
	/* Methods: */
	public:
	static std::string encode(const std::string& value);
	static std::string decode(const char* start,const char* end,const char** decodeEnd =0);
	};

template <>
class ValueCoder<bool>
	{
	/* Methods: */
	public:
	static std::string encode(const bool& value);
	static bool decode(const char* start,const char* end,const char** decodeEnd =0);
	};

template <>
class ValueCoder<short int>
	{
	/* Methods: */
	public:
	static std::string encode(const short int& value);
	static short int decode(const char* start,const char* end,const char** decodeEnd =0);
	};

template <>
class ValueCoder<unsigned short int>
	{
	/* Methods: */
	public:
	static std::string encode(const unsigned short int& value);
	static unsigned short int decode(const char* start,const char* end,const char** decodeEnd =0);
	};

template <>
class ValueCoder<int>
	{
	/* Methods: */
	public:
	static std::string encode(const int& value);
	static int decode(const char* start,const char* end,const char** decodeEnd =0);
	};

template <>
class ValueCoder<unsigned int>
	{
	/* Methods: */
	public:
	static std::string encode(const unsigned int& value);
	static unsigned int decode(const char* start,const char* end,const char** decodeEnd =0);
	};

template <>
class ValueCoder<float>
	{
	/* Methods: */
	public:
	static std::string encode(const float& value);
	static float decode(const char* start,const char* end,const char** decodeEnd =0);
	};

template <>
class ValueCoder<double>
	{
	/* Methods: */
	public:
	static std::string encode(const double& value);
	static double decode(const char* start,const char* end,const char** decodeEnd =0);
	};

}

#endif
