/***********************************************************************
Array - Class for n-dimensional arrays with index data type and
optimized access.
Copyright (c) 2003-2005 Oliver Kreylos

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

#ifndef MISC_ARRAY_INCLUDED
#define MISC_ARRAY_INCLUDED

#include <cstddef>
#include <Misc/ArrayIndex.h>

namespace Misc {

template <class ContentParam,int dimensionParam>
class ArrayBase
	{
	/* Embedded classes: */
	public:
	typedef ContentParam Content; // Content data type
	static const int dimension=dimensionParam; // The array's dimension
	typedef ArrayIndexBase<dimensionParam> IndexBase; // Compatible index base type
	typedef ArrayIndex<dimensionParam> Index; // Compatible index type
	typedef const Content* const_iterator; // Data type for constant array iterators
	typedef Content* iterator; // Data type for array iterators
	
	/* Elements: */
	protected:
	Index size; // Size of the array
	Content* elements; // Pointer to array elements
	Content* endElements; // Pointer to one behind last array element
	
	/* Protected methods: */
	ptrdiff_t calcOffset(const IndexBase& index) const // Calculates pointer offset value for given index
		{
		ptrdiff_t offset=index[0];
		for(int i=1;i<dimension;++i)
			offset=offset*size[i]+index[i];
		return offset;
		}
	
	/* Constructors and destructors: */
	public:
	ArrayBase(void) // Dummy constructor; creates uninitialized array
		:size(0),elements(0),endElements(0)
		{
		}
	ArrayBase(const IndexBase& sSize) // Creates uninitialized array of given size
		:size(sSize),
		 elements(new Content[size.calcIncrement(-1)]),
		 endElements(elements+size.calcIncrement(-1))
		{
		}
	ArrayBase(const ArrayBase& source) // Copies an existing array with all its elements
		:size(source.size),
		 elements(new Content[size.calcIncrement(-1)]),
		 endElements(elements+size.calcIncrement(-1))
		{
		/* Copy all elements: */
		const Content* sPtr=source.elements;
		for(Content* dPtr=elements;dPtr!=endElements;++dPtr,++sPtr)
			*dPtr=*sPtr;
		}
	ArrayBase& operator=(const ArrayBase& source) // Assignment operator; copies all array elements
		{
		/* Protect against self-assignment: */
		if(this!=&source)
			{
			/* Delete existing array: */
			delete[] elements;
			
			/* Copy source array size: */
			size=source.size;
			
			/* Allocate new array: */
			ptrdiff_t numElements=size.calcIncrement(-1);
			elements=new Content[numElements];
			endElements=elements+numElements;
			
			/* Copy all elements: */
			const Content* sPtr=source.elements;
			for(Content* dPtr=elements;dPtr!=endElements;++dPtr,++sPtr)
				*dPtr=*sPtr;
			}
		
		return *this;
		}
	~ArrayBase(void)
		{
		delete[] elements;
		}
	
	/* Methods: */
	const Index& getSize(void) const // Retrieve array size
		{
		return size;
		}
	int getSize(int component) const // Retrieve single array size
		{
		return size[component];
		}
	ptrdiff_t getIncrement(int component) const // Returns the pointer increment to advance to the next element in the given dimension
		{
		return size.calcIncrement(component);
		}
	ptrdiff_t getNumElements(void) const // Returns the number of elements in the array
		{
		return size.calcIncrement(-1);
		}
	const Content* getArray(void) const // Access complete array
		{
		return elements;
		}
	Content* getArray(void) // Ditto
		{
		return elements;
		}
	ptrdiff_t calcLinearIndex(const IndexBase& index) const // Returns the linear index of an element
		{
		return calcOffset(index);
		}
	Index calcIndex(ptrdiff_t linearIndex) const // Converts a linear index to a multiindex
		{
		Index result;
		for(int i=dimension-1;i>0;--i)
			{
			result[i]=int(linearIndex%size[i]);
			linearIndex/=size[i];
			}
		result[0]=int(linearIndex);
		return result;
		}
	const Content* getAddress(const IndexBase& index) const // Returns pointer to array element
		{
		return elements+calcOffset(index);
		}
	Content* getAddress(const IndexBase& index) // Ditto
		{
		return elements+calcOffset(index);
		}
	Index calcIndex(const Content* address) const // Converts an address inside the array to a multiindex
		{
		ptrdiff_t linearIndex=address-elements;
		Index result;
		for(int i=dimension-1;i>0;--i)
			{
			result[i]=int(linearIndex%size[i]);
			linearIndex/=size[i];
			}
		result[0]=int(linearIndex);
		return result;
		}
	bool isValid(const IndexBase& index) const // Checks whether the given index is valid for this array
		{
		for(int i=0;i<dimension;++i)
			if(index[i]<0||index[i]>=size[i])
				return false;
		return true;
		}
	const Content& operator[](const IndexBase& index) const // Access element
		{
		return elements[calcOffset(index)];
		}
	Content& operator[](const IndexBase& index) // Ditto
		{
		return elements[calcOffset(index)];
		}
	const Content& operator()(const IndexBase& index) const // Access element
		{
		return elements[calcOffset(index)];
		}
	Content& operator()(const IndexBase& index) // Ditto
		{
		return elements[calcOffset(index)];
		}
	const_iterator begin(void) const // Returns iterator to first array element in memory layout order
		{
		return elements;
		}
	iterator begin(void) // Ditto
		{
		return elements;
		}
	const_iterator end(void) const // Returns iterator beyond last array element in memory layout order
		{
		return endElements;
		}
	iterator end(void) // Ditto
		{
		return endElements;
		}
	Index beginIndex(void) const // Returns index of first array element in memory layout order
		{
		return Index(0);
		}
	Index endIndex(void) const // Returns index one past last array element in memory layout order
		{
		Index result(0);
		result[0]=size[0];
		return result;
		}
	IndexBase& preInc(IndexBase& index) const // Pre-increments the given array index in memory layout order
		{
		index.preInc(size);
		return index;
		}
	Index postInc(IndexBase& index) const // Post-increments the given array index in memory layout order
		{
		Index result=index;
		index.postInc(size);
		return result;
		}
	IndexBase& preDec(IndexBase& index) const // Pre-decrements the given array index in memory layout order
		{
		index.preDec(size);
		return index;
		}
	Index postDec(IndexBase& index) const // Post-decrements the given array index in memory layout order
		{
		Index result=index;
		index.postDec(size);
		return result;
		}
	ArrayBase& resize(const IndexBase& newSize) // Resizes array to given size
		{
		/* Delete existing array: */
		delete[] elements;
		
		/* Set array size: */
		size=newSize;
		
		/* Allocate new array: */
		ptrdiff_t numElements=size.calcIncrement(-1);
		elements=new Content[numElements];
		endElements=elements+numElements;
		
		return *this;
		}
	ArrayBase& ownArray(const IndexBase& newSize,Content* newElements) // Attaches to an existing array
		{
		/* Delete existing array: */
		delete[] elements;
		
		/* Set array size: */
		size=newSize;
		
		/* Attach to new array: */
		elements=newElements;
		endElements=elements+size.calcIncrement(-1);
		
		return *this;
		}
	ArrayBase& disownArray(void) // Releases the array without deleting elements
		{
		elements=0;
		endElements=0;
		
		return *this;
		}
	};

template <class ContentParam,int dimensionParam>
class Array:public ArrayBase<ContentParam,dimensionParam>
	{
	/* Embedded classes: */
	public:
	typedef ArrayBase<ContentParam,dimensionParam> BaseClass;
	
	/* Declarations of inherited types and elements: */
	protected:
	using ArrayBase<ContentParam,dimensionParam>::size;
	using ArrayBase<ContentParam,dimensionParam>::elements;
	
	/* Constructors and destructors: */
	public:
	Array(void)
		{
		}
	Array(const typename BaseClass::IndexBase& sSize)
		:BaseClass(sSize)
		{
		}
	Array(const Array& source)
		:BaseClass(source)
		{
		}
	Array& operator=(const Array& source)
		{
		BaseClass::operator=(source);
		return *this;
		}
	};

template <class ContentParam>
class Array<ContentParam,1>:public ArrayBase<ContentParam,1>
	{
	/* Embedded classes: */
	public:
	typedef ArrayBase<ContentParam,1> BaseClass;
	
	/* Declarations of inherited types and elements: */
	protected:
	using ArrayBase<ContentParam,1>::size;
	using ArrayBase<ContentParam,1>::elements;
	
	/* Constructors and destructors: */
	public:
	Array(void)
		{
		}
	Array(int size0)
		:BaseClass(typename BaseClass::Index(size0))
		{
		}
	Array(const typename BaseClass::IndexBase& sSize)
		:BaseClass(sSize)
		{
		}
	Array(const Array& source)
		:BaseClass(source)
		{
		}
	Array& operator=(const Array& source)
		{
		BaseClass::operator=(source);
		return *this;
		}
	
	/* Methods: */
	ptrdiff_t calcLinearIndex(int i0) const // Returns the linear index of an element with index components
		{
		return ptrdiff_t(i0);
		}
	ptrdiff_t calcLinearIndex(const typename BaseClass::IndexBase& index) const // Returns the linear index of an element
		{
		return ptrdiff_t(index[0]);
		}
	typename BaseClass::IndexBase calcIndex(ptrdiff_t linearIndex) const // Converts a linear index to a multiindex
		{
		return typename BaseClass::Index(int(linearIndex));
		}
	const typename BaseClass::Content* getAddress(int i0) const // Returns pointer to array element with index components
		{
		return elements+i0;
		}
	typename BaseClass::Content* getAddress(int i0) // Ditto
		{
		return elements+i0;
		}
	const typename BaseClass::Content* getAddress(const typename BaseClass::IndexBase& index) const // Returns pointer to array element
		{
		return elements+index[0];
		}
	typename BaseClass::Content* getAddress(const typename BaseClass::IndexBase& index) // Ditto
		{
		return elements+index[0];
		}
	typename BaseClass::IndexBase calcIndex(const typename BaseClass::Content* address) const // Converts an address inside the array to a multiindex
		{
		return typename BaseClass::Index(int(address-elements));
		}
	const typename BaseClass::Content& operator()(int i0) const // Access element with index components
		{
		return elements[i0];
		}
	typename BaseClass::Content& operator()(int i0) // Ditto
		{
		return elements[i0];
		}
	const typename BaseClass::Content& operator()(const typename BaseClass::IndexBase& index) const // Access element
		{
		return elements[index[0]];
		}
	typename BaseClass::Content& operator()(const typename BaseClass::IndexBase& index) // Ditto
		{
		return elements[index[0]];
		}
	const typename BaseClass::Content& operator[](const typename BaseClass::IndexBase& index) const // Access element
		{
		return elements[index[0]];
		}
	typename BaseClass::Content& operator[](const typename BaseClass::IndexBase& index) // Ditto
		{
		return elements[index[0]];
		}
	};

template <class ContentParam>
class Array<ContentParam,2>:public ArrayBase<ContentParam,2>
	{
	/* Embedded classes: */
	public:
	typedef ArrayBase<ContentParam,2> BaseClass;
	
	/* Declarations of inherited types and elements: */
	protected:
	using ArrayBase<ContentParam,2>::size;
	using ArrayBase<ContentParam,2>::elements;
	
	/* Protected methods: */
	ptrdiff_t calcOffset(const typename BaseClass::IndexBase& index) const
		{
		return ptrdiff_t(index[0])*ptrdiff_t(size[1])+ptrdiff_t(index[1]);
		}
	
	/* Constructors and destructors: */
	public:
	Array(void)
		{
		}
	Array(int size0,int size1)
		:BaseClass(typename BaseClass::Index(size0,size1))
		{
		}
	Array(const typename BaseClass::IndexBase& sSize)
		:BaseClass(sSize)
		{
		}
	Array(const Array& source)
		:BaseClass(source)
		{
		}
	Array& operator=(const Array& source)
		{
		BaseClass::operator=(source);
		return *this;
		}
	
	/* Methods: */
	ptrdiff_t calcLinearIndex(int i0,int i1) const // Returns the linear index of an element with index components
		{
		return ptrdiff_t(i0)*ptrdiff_t(size[1])+ptrdiff_t(i1);
		}
	ptrdiff_t calcLinearIndex(const typename BaseClass::IndexBase& index) const // Returns the linear index of an element
		{
		return calcOffset(index);
		}
	typename BaseClass::IndexBase calcIndex(ptrdiff_t linearIndex) const // Converts a linear index to a multiindex
		{
		return typename BaseClass::Index(int(linearIndex/size[1]),int(linearIndex%size[1]));
		}
	const typename BaseClass::Content* getAddress(int i0,int i1) const // Returns pointer to array element with index components
		{
		return elements+(ptrdiff_t(i0)*ptrdiff_t(size[1])+ptrdiff_t(i1));
		}
	typename BaseClass::Content* getAddress(int i0,int i1) // Ditto
		{
		return elements+(ptrdiff_t(i0)*ptrdiff_t(size[1])+ptrdiff_t(i1));
		}
	const typename BaseClass::Content* getAddress(const typename BaseClass::IndexBase& index) const // Returns pointer to array element
		{
		return elements+calcOffset(index);
		}
	typename BaseClass::Content* getAddress(const typename BaseClass::IndexBase& index) // Ditto
		{
		return elements+calcOffset(index);
		}
	typename BaseClass::IndexBase calcIndex(const typename BaseClass::Content* address) const // Converts an address inside the array to a multiindex
		{
		return typename BaseClass::Index(int((address-elements)/size[1]),int((address-elements)%size[1]));
		}
	const typename BaseClass::Content& operator()(int i0,int i1) const // Access element with index components
		{
		return elements[ptrdiff_t(i0)*ptrdiff_t(size[1])+ptrdiff_t(i1)];
		}
	typename BaseClass::Content& operator()(int i0,int i1) // Ditto
		{
		return elements[ptrdiff_t(i0)*ptrdiff_t(size[1])+ptrdiff_t(i1)];
		}
	const typename BaseClass::Content& operator()(const typename BaseClass::IndexBase& index) const // Access element
		{
		return elements[calcOffset(index)];
		}
	typename BaseClass::Content& operator()(const typename BaseClass::IndexBase& index) // Ditto
		{
		return elements[calcOffset(index)];
		}
	const typename BaseClass::Content& operator[](const typename BaseClass::IndexBase& index) const // Access element
		{
		return elements[calcOffset(index)];
		}
	typename BaseClass::Content& operator[](const typename BaseClass::IndexBase& index) // Ditto
		{
		return elements[calcOffset(index)];
		}
	};

template <class ContentParam>
class Array<ContentParam,3>:public ArrayBase<ContentParam,3>
	{
	/* Embedded classes: */
	public:
	typedef ArrayBase<ContentParam,3> BaseClass;
	
	/* Declarations of inherited types and elements: */
	protected:
	using ArrayBase<ContentParam,3>::size;
	using ArrayBase<ContentParam,3>::elements;
	
	/* Protected methods: */
	ptrdiff_t calcOffset(const typename BaseClass::IndexBase& index) const
		{
		return (ptrdiff_t(index[0])*ptrdiff_t(size[1])+ptrdiff_t(index[1]))*ptrdiff_t(size[2])+ptrdiff_t(index[2]);
		}
	
	/* Constructors and destructors: */
	public:
	Array(void)
		{
		}
	Array(int size0,int size1,int size2)
		:BaseClass(typename BaseClass::Index(size0,size1,size2))
		{
		}
	Array(const typename BaseClass::IndexBase& sSize)
		:BaseClass(sSize)
		{
		}
	Array(const Array& source)
		:BaseClass(source)
		{
		}
	Array& operator=(const Array& source)
		{
		BaseClass::operator=(source);
		return *this;
		}
	
	/* Methods: */
	ptrdiff_t calcLinearIndex(int i0,int i1,int i2) const // Returns the linear index of an element with index components
		{
		return (ptrdiff_t(i0)*ptrdiff_t(size[1])+ptrdiff_t(i1))*ptrdiff_t(size[2])+ptrdiff_t(i2);
		}
	ptrdiff_t calcLinearIndex(const typename BaseClass::IndexBase& index) const // Returns the linear index of an element
		{
		return calcOffset(index);
		}
	const typename BaseClass::Content* getAddress(int i0,int i1,int i2) const // Returns pointer to array element with index components
		{
		return elements+((ptrdiff_t(i0)*ptrdiff_t(size[1])+ptrdiff_t(i1))*ptrdiff_t(size[2])+ptrdiff_t(i2));
		}
	typename BaseClass::Content* getAddress(int i0,int i1,int i2) // Ditto
		{
		return elements+((ptrdiff_t(i0)*ptrdiff_t(size[1])+ptrdiff_t(i1))*ptrdiff_t(size[2])+ptrdiff_t(i2));
		}
	const typename BaseClass::Content* getAddress(const typename BaseClass::IndexBase& index) const // Returns pointer to array element
		{
		return elements+calcOffset(index);
		}
	typename BaseClass::Content* getAddress(const typename BaseClass::IndexBase& index) // Ditto
		{
		return elements+calcOffset(index);
		}
	const typename BaseClass::Content& operator()(int i0,int i1,int i2) const // Access element with index components
		{
		return elements[(ptrdiff_t(i0)*ptrdiff_t(size[1])+ptrdiff_t(i1))*ptrdiff_t(size[2])+ptrdiff_t(i2)];
		}
	typename BaseClass::Content& operator()(int i0,int i1,int i2) // Ditto
		{
		return elements[(ptrdiff_t(i0)*ptrdiff_t(size[1])+ptrdiff_t(i1))*ptrdiff_t(size[2])+ptrdiff_t(i2)];
		}
	const typename BaseClass::Content& operator()(const typename BaseClass::IndexBase& index) const // Access element
		{
		return elements[calcOffset(index)];
		}
	typename BaseClass::Content& operator()(const typename BaseClass::IndexBase& index) // Ditto
		{
		return elements[calcOffset(index)];
		}
	const typename BaseClass::Content& operator[](const typename BaseClass::IndexBase& index) const // Access element
		{
		return elements[calcOffset(index)];
		}
	typename BaseClass::Content& operator[](const typename BaseClass::IndexBase& index) // Ditto
		{
		return elements[calcOffset(index)];
		}
	};

}

#endif
