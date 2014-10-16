/***********************************************************************
FixedArray - Class for 1-dimensional arrays of compile-time fixed size.
Copyright (c) 2012 Oliver Kreylos

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

#ifndef MISC_FIXEDARRAY_INCLUDED
#define MISC_FIXEDARRAY_INCLUDED

namespace Misc {

template <class ElementParam,int sizeParam>
class FixedArray
	{
	/* Embedded classes: */
	public:
	typedef ElementParam Element; // Element type
	static const int size=sizeParam; // Array size
	
	/* Elements: */
	private:
	Element elements[size]; // Array of elements
	
	/* Constructors and destructors: */
	public:
	FixedArray(void) // No initialization
		{
		}
	FixedArray(const Element& filler) // Fills the array with a single value
		{
		for(int i=0;i<size;++i)
			elements[i]=filler;
		}
	FixedArray(const Element* array) // Copies C-style array
		{
		for(int i=0;i<size;++i)
			elements[i]=array[i];
		}
	template <class SourceElementParam>
	FixedArray(const SourceElementParam* array) // Copies C-style array with type conversion
		{
		for(int i=0;i<size;++i)
			elements[i]=Element(array[i]);
		}
	FixedArray(const FixedArray& source) // Copy constructor
		{
		for(int i=0;i<size;++i)
			elements[i]=source.elements[i];
		}
	template <class SourceElementParam>
	FixedArray(const FixedArray<SourceElementParam,sizeParam>& source) // Copy constructor with type conversion
		{
		for(int i=0;i<size;++i)
			elements[i]=Element(source[i]);
		}
	
	/* Methods: */
	const Element* getElements(void) const // Returns C-style array
		{
		return elements;
		}
	Element* getElements(void) // Ditto
		{
		return elements;
		}
	const Element& operator[](int index) const // Returns element as lvalue
		{
		return elements[index];
		}
	Element& operator[](int index) // Ditto
		{
		return elements[index];
		}
	};

}

#endif
