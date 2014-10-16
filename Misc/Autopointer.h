/***********************************************************************
Autopointer - Class for pointers to reference-counted objects. Uses the
destination class' ref() and unref() methods.
Copyright (c) 2007-2012 Oliver Kreylos

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

#ifndef MISC_AUTOPOINTER_INCLUDED
#define MISC_AUTOPOINTER_INCLUDED

namespace Misc {

template <class TargetParam>
class Autopointer
	{
	/* Embedded classes: */
	public:
	typedef TargetParam Target; // Type of pointed-to class
	
	/* Elements: */
	private:
	Target* target; // Standard pointer to target
	
	/* Constructors and destructors: */
	public:
	Autopointer(void) // Creates a NULL autopointer
		:target(0)
		{
		}
	Autopointer(Target* sTarget) // Creates autopointer from standard pointer
		:target(sTarget)
		{
		if(target!=0)
			target->ref();
		}
	template <class SourceTargetParam>
	Autopointer(SourceTargetParam* sTarget) // Ditto, with target type conversion
		:target(dynamic_cast<Target*>(sTarget))
		{
		if(target!=0)
			target->ref();
		}
	Autopointer& operator=(Target* sTarget) // Assignment operator from standard pointer
		{
		/* Reference the new target and unreference the old one: */
		if(sTarget!=0)
			sTarget->ref();
		if(target!=0)
			target->unref();
		target=sTarget;
		
		return *this;
		}
	template <class SourceTargetParam>
	Autopointer& operator=(SourceTargetParam* sTarget) // Ditto, with target type conversion
		{
		/* Reference the new target and unreference the old one: */
		if(sTarget!=0)
			sTarget->ref();
		if(target!=0)
			target->unref();
		target=sTarget;
		
		return *this;
		}
	Autopointer(const Autopointer& source) // Copy constructor; adds reference to target object
		:target(source.target)
		{
		if(target!=0)
			target->ref();
		}
	template <class SourceTargetParam>
	Autopointer(const Autopointer<SourceTargetParam>& source) // Ditto, with target type conversion
		:target(dynamic_cast<Target*>(source.getPointer()))
		{
		if(target!=0)
			target->ref();
		}
	Autopointer& operator=(const Autopointer& source) // Assignment operator; removes reference from previous target object and references new target object
		{
		/* Reference the new target and unreference the old one: */
		if(source.target!=0)
			source.target->ref();
		if(target!=0)
			target->unref();
		target=source.target;
		
		return *this;
		}
	template <class SourceTargetParam>
	Autopointer& operator=(const Autopointer<SourceTargetParam>& source) // Ditto, with target type conversion
		{
		/* Reference the new target and unreference the old one: */
		Target* sTarget=dynamic_cast<Target*>(source.getPointer());
		if(sTarget!=0)
			sTarget->ref();
		if(target!=0)
			target->unref();
		target=sTarget;
		
		return *this;
		}
	~Autopointer(void) // Destructor; removes reference from target object
		{
		if(target!=0)
			target->unref();
		}
	
	/* Methods: */
	friend bool operator==(const Autopointer& ptr1,const Autopointer& ptr2) // Equality operator
		{
		return ptr1.target==ptr2.target;
		}
	friend bool operator==(const Autopointer& ptr1,const Target* ptr2) // Ditto
		{
		return ptr1.target==ptr2;
		}
	friend bool operator==(const Target* ptr1,const Autopointer& ptr2) // Ditto
		{
		return ptr1==ptr2.target;
		}
	friend bool operator!=(const Autopointer& ptr1,const Autopointer& ptr2) // Inequality operator
		{
		return ptr1.target!=ptr2.target;
		}
	friend bool operator!=(const Autopointer& ptr1,const Target* ptr2) // Ditto
		{
		return ptr1.target!=ptr2;
		}
	friend bool operator!=(const Target* ptr1,const Autopointer& ptr2) // Ditto
		{
		return ptr1!=ptr2.target;
		}
	Target& operator*(void) const // Indirection operator
		{
		return *target;
		}
	Target* operator->(void) const // Arrow operator
		{
		return target;
		}
	Target* getPointer(void) const // Converts autopointer to standard pointer
		{
		return target;
		}
	};

}

#endif
