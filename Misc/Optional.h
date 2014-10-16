/***********************************************************************
Optional - Class for optional configuration file values that remember
whether they were explicitly specified or not. Relies on the value class
having a default constructor.
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

#ifndef MISC_OPTIONAL_INCLUDED
#define MISC_OPTIONAL_INCLUDED

#include <Misc/ConfigurationFile.h>

namespace Misc {

template <class ValueParam>
class Optional
	{
	/* Embedded classes: */
	public:
	typedef ValueParam Value; // Value type
	
	/* Elements: */
	private:
	bool specified; // Flag whether the value was explicitly specified
	ValueParam value; // The explicity specified value
	
	/* Constructors and destructors: */
	public:
	Optional(void) // Creates an unspecified value
		:specified(false)
		{
		}
	Optional(const Value& sValue) // Creates a specified value
		:specified(true),value(sValue)
		{
		}
	
	/* Methods: */
	bool isSpecified(void) const // Returns true if a value was specified
		{
		return specified;
		}
	const Value& getValue(void) const // Returns the specified value
		{
		return value;
		}
	Value& getValue(void) // Ditto
		{
		return value;
		}
	Optional& retrieve(const Misc::ConfigurationFileSection& configFileSection,const char* tag) // Reads the optional value from a configuration file
		{
		if(configFileSection.hasTag(tag))
			{
			specified=true;
			value=configFileSection.retrieveValue<Value>(tag);
			}
		else
			specified=false;
		return *this;
		}
	void store(Misc::ConfigurationFileSection& configFileSection,const char* tag) const // Writes the optional value to a configuration file
		{
		if(specified)
			configFileSection.storeValue<Value>(tag,value);
		}
	};

}

#endif
