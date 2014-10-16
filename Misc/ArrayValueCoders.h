/***********************************************************************
ArrayValueCoders - Generic value coder classes for standard C-style
arrays with fixed or dynamic array sizes or array wrapper classes.
Copyright (c) 2004-2013 Oliver Kreylos

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

#ifndef MISC_ARRAYVALUECODERS_INCLUDED
#define MISC_ARRAYVALUECODERS_INCLUDED

#include <Misc/ValueCoder.h>

/* Forward declarations: */
namespace Misc {
template <class ElementParam,int sizeParam>
class FixedArray;
}

namespace Misc {

template <class ElementParam,size_t numElementsParam>
class CFixedArrayValueCoder // Value coder class for arrays with compile-time known sizes
	{
	/* Elements: */
	public:
	ElementParam* elements; // C-style array to which to decode
	
	/* Constructors and destructors: */
	CFixedArrayValueCoder(void)
		{
		}
	CFixedArrayValueCoder(ElementParam* sElements)
		:elements(sElements)
		{
		}
	
	/* Methods: */
	public:
	static std::string encode(const ElementParam* elements);
	ElementParam* decode(const char* start,const char* end,const char** decodeEnd =0) const;
	};

template <class ElementParam>
class FixedArrayValueCoder // Value coder class for arrays with a-priori known sizes
	{
	/* Elements: */
	public:
	ElementParam* elements; // C-style array to which to decode
	size_t numElements; // Expected array size
	
	/* Constructors and destructors: */
	FixedArrayValueCoder(size_t sNumElements)
		:numElements(sNumElements)
		{
		}
	FixedArrayValueCoder(ElementParam* sElements,size_t sNumElements)
		:elements(sElements),numElements(sNumElements)
		{
		}
	
	/* Methods: */
	public:
	std::string encode(const ElementParam* elements) const;
	ElementParam* decode(const char* start,const char* end,const char** decodeEnd =0) const;
	};

template <class ElementParam>
class DynamicArrayValueCoder // Value coder class for arrays with dynamic sizes up to a maximum
	{
	/* Elements: */
	public:
	ElementParam* elements; // C-style array to which to decode
	size_t numElements; // On encode: number of elements actually in array. On decode: maximum number of elements in array; upon return: number of elements in array, may be larger than maximum
	
	/* Constructors and destructors: */
	DynamicArrayValueCoder(size_t sNumElements)
		:numElements(sNumElements)
		{
		}
	DynamicArrayValueCoder(ElementParam* sElements,size_t sNumElements)
		:elements(sElements),numElements(sNumElements)
		{
		}
	
	/* Methods: */
	public:
	std::string encode(const ElementParam* elements) const;
	ElementParam* decode(const char* start,const char* end,const char** decodeEnd =0);
	};

template <class ElementParam,int sizeParam>
class ValueCoder<FixedArray<ElementParam,sizeParam> >
	{
	/* Methods: */
	public:
	static std::string encode(const FixedArray<ElementParam,sizeParam>& value);
	static FixedArray<ElementParam,sizeParam> decode(const char* start,const char* end,const char** decodeEnd =0);
	};

}

#if !defined(MISC_ARRAYVALUECODERS_IMPLEMENTATION)
#include <Misc/ArrayValueCoders.icpp>
#endif

#endif
