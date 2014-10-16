/***********************************************************************
Marshaller - Generic class to read or write values of arbitrary types
from or to binary data sources or sinks.
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

#ifndef MISC_MARSHALLER_INCLUDED
#define MISC_MARSHALLER_INCLUDED

namespace Misc {

/************************
Generic marshaller class:
************************/

template <class ValueParam>
class Marshaller
	{
	/* Dummy class; only specializations can be used */
	
	#if 0
	/* Methods that must be provided by specialized classes: */
	static size_t getSize(const ValueParam& value); // Returns the size of the given value when written to a binary sink in bytes
	template <class DataSinkParam>
	static void write(const ValueParam& value,DataSinkParam& sink); // Writes the given value to given sink
	template <class DataSourceParam>
	static ValueParam read(DataSourceParam& source); // Reads value from given source
	#endif
	};

}

#endif
