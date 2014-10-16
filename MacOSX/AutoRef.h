/***********************************************************************
AutoRef - Helper class to manage Mac OSX Core Foundation object
references. Templatized by the actual reference type to guarantee type
safety.
Copyright (c) 2009-2010 Oliver Kreylos

This file is part of the Mac OSX Support Library (MacOSX).

The Mac OSX Support Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Mac OSX Support Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Mac OSX Support Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef MACOSX_AUTOREF_INCLUDED
#define MACOSX_AUTOREF_INCLUDED

#ifdef __APPLE__

#include <CoreFoundation/CoreFoundation.h>

namespace MacOSX {

template <class RefParam>
class AutoRef
	{
	/* Embedded classes: */
	public:
	typedef RefParam Ref; // Underlying Core Foundation reference type
	
	/* Elements: */
	private:
	Ref ref; // Core Foundation object reference
	
	/* Constructors and destructors: */
	public:
	AutoRef(void)
		:ref(0)
		{
		}
	AutoRef(Ref sRef)
		:ref(sRef)
		{
		}
	AutoRef(const AutoRef& source)
		:ref(source.ref)
		{
		if(ref!=0)
			CFRetain(ref);
		}
	AutoRef& operator=(Ref sourceRef)
		{
		if(ref!=sourceRef&&ref!=0)
			CFRelease(ref);
		ref=sourceRef;
		return *this;
		}
	AutoRef& operator=(const AutoRef& source)
		{
		if(this!=&source)
			{
			if(ref!=0)
				CFRelease(ref);
			ref=source.ref;
			if(ref!=0)
				CFRetain(ref);
			}
		return *this;
		}
	~AutoRef(void)
		{
		if(ref!=0)
			CFRelease(ref);
		}
	
	/* Methods: */
	operator Ref(void) const // Implicit conversion to plain Core Foundation reference type
		{
		return ref;
		}
	};

}

#endif

#endif
