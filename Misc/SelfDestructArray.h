/***********************************************************************
SelfDestructArray - Class for pointers to new[]-allocated arrays that
automatically deletes the array when the pointer goes out of scope. Does
not support multiple references to the same object or anything fancy
like that (use Misc::Autopointer instead), but does not require any help
from the target class, either.
Copyright (c) 2010-2011 Oliver Kreylos

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

#ifndef MISC_SELFDESTRUCTARRAY_INCLUDED
#define MISC_SELFDESTRUCTARRAY_INCLUDED

#include <stddef.h>

namespace Misc {

template <class TargetParam>
class SelfDestructArray
	{
	/* Embedded classes: */
	public:
	typedef TargetParam Target; // Type of pointed-to class
	
	/* Elements: */
	private:
	Target* target; // Standard pointer to target array
	
	/* Constructors and destructors: */
	public:
	SelfDestructArray(void) // Creates invalid array
		:target(0)
		{
		}
	SelfDestructArray(size_t numElements) // Creates self-destructor for new-allocated array
		:target(new Target[numElements])
		{
		}
	SelfDestructArray(Target* sTarget) // Creates self-destructor for given target array
		:target(sTarget)
		{
		}
	private:
	SelfDestructArray(const SelfDestructArray& source); // Prohibit copy constructor
	SelfDestructArray& operator=(const SelfDestructArray& source); // Prohibit assignment operator
	public:
	~SelfDestructArray(void) // Destroys the target array
		{
		/* Only delete target array if it's an actual array (somewhat redundant): */
		if(target!=0)
			delete[] target;
		}
	
	/* Methods: */
	operator const Target*(void) const // Conversion to standard array pointer
		{
		return target;
		}
	operator Target*(void) // Ditto
		{
		return target;
		}
	const Target* getArray(void) const // Explicit conversion to standard array pointer
		{
		return target;
		}
	Target* getArray(void) // Ditto
		{
		return target;
		}
	Target& operator[](int index) const // Array access operator
		{
		return target[index];
		}
	void setTarget(Target* newTarget) // Sets the target array and destroys the previous target array
		{
		/* Delete the previous target array: */
		if(target!=0)
			delete[] target;
		
		/* Take ownership of the new target array: */
		target=newTarget;
		}
	Target* releaseTarget(void) // Returns a standard pointer to the target array and releases it from self-destruction
		{
		Target* result=target;
		target=0;
		return result;
		}
	};

}

#endif
