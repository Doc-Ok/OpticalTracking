/***********************************************************************
CompoundMarshallers - Generic marshaller classes for vectors and lists
of other data types.
Copyright (c) 2010 Oliver Kreylos

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

#ifndef MISC_COMPOUNDMARSHALLERS_INCLUDED
#define MISC_COMPOUNDMARSHALLERS_INCLUDED

#include <utility>
#include <list>
#include <vector>
#include <Misc/Marshaller.h>

namespace Misc {

template <class FirstParam,class SecondParam>
class Marshaller<std::pair<FirstParam,SecondParam> >
	{
	static size_t getSize(const std::pair<FirstParam,SecondParam>& value)
		{
		return Marshaller<FirstParam>::getSize(value.first)+Marshaller<SecondParam>::getSize(value.second);
		}
	template <class DataSinkParam>
	static void write(const std::pair<FirstParam,SecondParam>& value,DataSinkParam& sink)
		{
		Marshaller<FirstParam>::write(value.first,sink);
		Marshaller<SecondParam>::write(value.second,sink);
		}
	template <class DataSourceParam>
	static std::pair<FirstParam,SecondParam> read(DataSourceParam& source)
		{
		std::pair<FirstParam,SecondParam> result;
		result.first=Marshaller<FirstParam>::read(source);
		result.second=Marshaller<SecondParam>::read(source);
		return result;
		}
	};

template <class ValueParam>
class Marshaller<std::list<ValueParam> >
	{
	static size_t getSize(const std::list<ValueParam>& value)
		{
		size_t result=sizeof(unsigned int);
		for(typename std::list<ValueParam>::const_iterator vIt=value.begin();vIt!=value.end();++vIt)
			result+=Marshaller<ValueParam>::getSize(*vIt);
		return result;
		}
	template <class DataSinkParam>
	static void write(const std::list<ValueParam>& value,DataSinkParam& sink)
		{
		sink.template write<unsigned int>((unsigned int)value.size());
		for(typename std::list<ValueParam>::const_iterator vIt=value.begin();vIt!=value.end();++vIt)
			Marshaller<ValueParam>::write(*vIt,sink);
		}
	template <class DataSourceParam>
	static std::list<ValueParam> read(DataSourceParam& source)
		{
		std::list<ValueParam> result;
		unsigned int size=source.template read<unsigned int>();
		for(unsigned int i=0;i<size;++i)
			result.push_back(Marshaller<ValueParam>::read(source));
		return result;
		}
	};

template <class ValueParam>
class Marshaller<std::vector<ValueParam> >
	{
	static size_t getSize(const std::vector<ValueParam>& value)
		{
		size_t result=sizeof(unsigned int);
		for(typename std::vector<ValueParam>::const_iterator vIt=value.begin();vIt!=value.end();++vIt)
			result+=Marshaller<ValueParam>::getSize(*vIt);
		return result;
		}
	template <class DataSinkParam>
	static void write(const std::vector<ValueParam>& value,DataSinkParam& sink)
		{
		sink.template write<unsigned int>((unsigned int)value.size());
		for(typename std::vector<ValueParam>::const_iterator vIt=value.begin();vIt!=value.end();++vIt)
			Marshaller<ValueParam>::write(*vIt,sink);
		}
	template <class DataSourceParam>
	static std::vector<ValueParam> read(DataSourceParam& source)
		{
		std::vector<ValueParam> result;
		unsigned int size=source.template read<unsigned int>();
		for(unsigned int i=0;i<size;++i)
			result.push_back(Marshaller<ValueParam>::read(source));
		return result;
		}
	};

}

#endif
