/***********************************************************************
SelfDestructPointer - Class for pointers to new-allocated objects that
automatically deletes the object when the pointer goes out of scope.
Does not support multiple references to the same object or anything
fancy like that (use Misc::Autopointer instead), but does not require
any help from the target class, either.
Copyright (c) 2008-2011 Oliver Kreylos

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

#ifndef MISC_SELFDESTRUCTPOINTER_INCLUDED
#define MISC_SELFDESTRUCTPOINTER_INCLUDED

namespace Misc {

template <class TargetParam>
class SelfDestructPointer
	{
	/* Embedded classes: */
	public:
	typedef TargetParam Target; // Type of pointed-to class
	
	/* Elements: */
	private:
	Target* target; // Standard pointer to target
	
	/* Constructors and destructors: */
	public:
	SelfDestructPointer(void) // Creates invalid pointer
		:target(0)
		{
		}
	SelfDestructPointer(Target* sTarget) // Creates self-destructor for given target
		:target(sTarget)
		{
		}
	private:
	SelfDestructPointer(const SelfDestructPointer& source); // Prohibit copy constructor
	SelfDestructPointer& operator=(const SelfDestructPointer& source); // Prohibit assignment operator
	public:
	~SelfDestructPointer(void) // Destroys the pointer target
		{
		/* Only delete target if it's an actual object (somewhat redundant): */
		if(target!=0)
			delete target;
		}
	
	/* Methods: */
	bool isValid(void) const // Returns true if the pointer points to a valid object
		{
		return target!=0;
		}
	Target& operator*(void) const // Indirection operator
		{
		return *target;
		}
	Target* operator->(void) const // Arrow operator
		{
		return target;
		}
	Target* getTarget(void) const // Returns standard pointer to target
		{
		return target;
		}
	void setTarget(Target* newTarget) // Sets the pointer's target and destroys previous target
		{
		/* Delete the previous target: */
		if(target!=0)
			delete target;
		
		/* Take ownership of the new target: */
		target=newTarget;
		}
	Target* releaseTarget(void) // Returns a standard pointer to the target and releases it from self-destruction
		{
		Target* result=target;
		target=0;
		return result;
		}
	};

}

#endif
