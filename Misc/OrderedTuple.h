/***********************************************************************
OrderedTuple - Class for ordered tuples; intended to be used as hash
table keys. The element type must be a signed or unsigned integer
type.
Copyright (c) 2008-2009 Oliver Kreylos

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

#ifndef MISC_ORDEREDTUPLE_INCLUDED
#define MISC_ORDEREDTUPLE_INCLUDED

#include <stddef.h>

namespace Misc {

template <class ElementParam,int dimensionParam>
class OrderedTuple
	{
	/* Embedded classes: */
	public:
	typedef ElementParam Element; // Type of tuple elements
	static const int dimension=dimensionParam; // Dimension of the tuple
	
	/* Elements: */
	private:
	Element elements[dimension]; // The tuple's elements
	
	/* Constructors and destructors: */
	public:
	OrderedTuple(void) // Dummy constructor
		{
		}
	OrderedTuple(const Element sElements[dimensionParam]) // Construction from C-style array
		{
		/* Copy the source array: */
		for(int i=0;i<dimension;++i)
			elements[i]=sElements[i];
		}
	
	/* Methods: */
	const Element* getElements(void) const // Returns element array
		{
		return elements;
		}
	Element operator[](int index) const // Returns element
		{
		return elements[index];
		}
	void set(int index,Element newElement) // Sets an element to a new value
		{
		elements[index]=newElement;
		}
	static size_t hash(const OrderedTuple& source,size_t tableSize) // Calculates a hash function for the given tuple and table size
		{
		size_t result=size_t(source.elements[0]);
		for(int i=1;i<dimension;++i)
			result=result*10000003+size_t(source.elements[i]);
		return result%tableSize;
		}
	};

/**********************************************
Specialized versions of the OrderedTuple class:
**********************************************/

template <class ElementParam>
class OrderedTuple<ElementParam,2>
	{
	/* Embedded classes: */
	public:
	typedef ElementParam Element; // Type of tuple elements
	static const int dimension=2; // Dimension of the tuple
	
	/* Elements: */
	private:
	Element elements[dimension]; // The tuple's elements
	
	/* Constructors and destructors: */
	public:
	OrderedTuple(void) // Dummy constructor
		{
		}
	OrderedTuple(Element sElement0,Element sElement1) // Construction from two elements
		{
		elements[0]=sElement0;
		elements[1]=sElement1;
		}
	OrderedTuple(const Element sElements[2]) // Construction from C-style array
		{
		elements[0]=sElements[0];
		elements[1]=sElements[1];
		}
	
	/* Methods: */
	const Element* getElements(void) const // Returns element array
		{
		return elements;
		}
	Element operator[](int index) const // Returns element
		{
		return elements[index];
		}
	void set(int index,Element newElement) // Sets an element to a new value
		{
		elements[index]=newElement;
		}
	static size_t hash(const OrderedTuple& source,size_t tableSize) // Calculates a hash function for the given tuple and table size
		{
		return (size_t(source.elements[0])*10000003+size_t(source.elements[1]))%tableSize;
		}
	};

template <class ElementParam>
class OrderedTuple<ElementParam,3>
	{
	/* Embedded classes: */
	public:
	typedef ElementParam Element; // Type of tuple elements
	static const int dimension=3; // Dimension of the tuple
	
	/* Elements: */
	private:
	Element elements[dimension]; // The tuple's elements
	
	/* Constructors and destructors: */
	public:
	OrderedTuple(void) // Dummy constructor
		{
		}
	OrderedTuple(Element sElement0,Element sElement1,Element sElement2) // Construction from three elements
		{
		elements[0]=sElement0;
		elements[1]=sElement1;
		elements[2]=sElement2;
		}
	OrderedTuple(const Element sElements[3]) // Construction from C-style array
		{
		elements[0]=sElements[0];
		elements[1]=sElements[1];
		elements[2]=sElements[2];
		}
	
	/* Methods: */
	const Element* getElements(void) const // Returns element array
		{
		return elements;
		}
	Element operator[](int index) const // Returns element
		{
		return elements[index];
		}
	void set(int index,Element newElement) // Sets an element to a new value
		{
		elements[index]=newElement;
		}
	static size_t hash(const OrderedTuple& source,size_t tableSize) // Calculates a hash function for the given tuple and table size
		{
		return ((size_t(source.elements[0])*10000003+size_t(source.elements[1]))*10000003+size_t(source.elements[2]))%tableSize;
		}
	};

/********************************
Operations on class OrderedTuple:
********************************/

template <class ElementParam,int dimensionParam>
inline bool operator==(const OrderedTuple<ElementParam,dimensionParam>& ot1,const OrderedTuple<ElementParam,dimensionParam>& ot2) // Equality operator
	{
	bool result=true;
	for(int i=0;i<dimensionParam&&result;++i)
		result=ot1[i]==ot2[i];
	return result;
	}

template <class ElementParam,int dimensionParam>
inline bool operator!=(const OrderedTuple<ElementParam,dimensionParam>& ot1,const OrderedTuple<ElementParam,dimensionParam>& ot2) // Equality operator
	{
	bool result=false;
	for(int i=0;i<dimensionParam&&!result;++i)
		result=ot1[i]!=ot2[i];
	return result;
	}

template <class ElementParam>
inline bool operator==(const OrderedTuple<ElementParam,2>& ot1,const OrderedTuple<ElementParam,2>& ot2) // Equality operator
	{
	return ot1[0]==ot2[0]&&ot1[1]==ot2[1];
	}

template <class ElementParam>
inline bool operator!=(const OrderedTuple<ElementParam,2>& ot1,const OrderedTuple<ElementParam,2>& ot2) // Equality operator
	{
	return ot1[0]!=ot2[0]||ot1[1]!=ot2[1];
	}

template <class ElementParam>
inline bool operator==(const OrderedTuple<ElementParam,3>& ot1,const OrderedTuple<ElementParam,3>& ot2) // Equality operator
	{
	return ot1[0]==ot2[0]&&ot1[1]==ot2[1]&&ot1[2]==ot2[2];
	}

template <class ElementParam>
inline bool operator!=(const OrderedTuple<ElementParam,3>& ot1,const OrderedTuple<ElementParam,3>& ot2) // Equality operator
	{
	return ot1[0]!=ot2[0]||ot1[1]!=ot2[1]||ot1[2]!=ot2[2];
	}

}

#endif
