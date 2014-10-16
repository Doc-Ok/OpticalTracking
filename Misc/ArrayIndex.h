/***********************************************************************
ArrayIndex - Class for n-dimensional multiindices, used primarily for
indexing n-dimensional arrays.
Copyright (c) 2004-2005 Oliver Kreylos

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

#ifndef MISC_ARRAYINDEX_INCLUDED
#define MISC_ARRAYINDEX_INCLUDED

#include <cstddef>

/*****************************************
Base class for n-dimensional multiindices:
*****************************************/

namespace Misc {

template <int dimensionParam>
class ArrayIndexBase
	{
	/* Embedded classes: */
	public:
	static const int dimension=dimensionParam; // The index's dimension
	
	/* Elements: */
	protected:
	int index[dimension]; // Array of index components

	/* Constructors and destructors: */
	public:
	ArrayIndexBase(void) // Dummy constructor
		{
		}
	explicit ArrayIndexBase(int sIndices) // Copies given value into each index component
		{
		for(int i=0;i<dimension;++i)
			index[i]=sIndices;
		}
	ArrayIndexBase(const int sIndex[dimension]) // Elementwise constructor
		{
		for(int i=0;i<dimension;++i)
			index[i]=sIndex[i];
		}
	ArrayIndexBase(const ArrayIndexBase& source) // Copy constructor
		{
		for(int i=0;i<dimension;++i)
			index[i]=source.index[i];
		}
	ArrayIndexBase& operator=(const ArrayIndexBase& source) // Assignment operator
		{
		for(int i=0;i<dimension;++i)
			index[i]=source.index[i];
		return *this;
		}
	
	/* Methods: */
	const int* getComponents(void) const // Returns pointer to array of index components
		{
		return index;
		}
	int* getComponents(void) // Ditto
		{
		return index;
		}
	friend bool operator==(const ArrayIndexBase& index1,const ArrayIndexBase& index2) // Compares two indices for equality
		{
		for(int i=0;i<dimension;++i)
			if(index1[i]!=index2[i])
				return false;
		return true;
		}
	friend bool operator!=(const ArrayIndexBase& index1,const ArrayIndexBase& index2) // Compares two indices for inequality
		{
		for(int i=0;i<dimension;++i)
			if(index1[i]!=index2[i])
				return true;
		return false;
		}
	friend bool operator<(const ArrayIndexBase& index1,const ArrayIndexBase& index2) // Compares two indices according to lexicographical order
		{
		for(int i=0;i<dimension;++i)
			{
			if(index1[i]<index2[i])
				return true;
			else if(index1[i]>index2[i])
				return false;
			}
		return false;
		}
	int operator[](int component) const // Returns index component
		{
		return index[component];
		}
	int& operator[](int component) // Returns index component as modifiable L-value
		{
		return index[component];
		}
	ptrdiff_t calcIncrement(int component) const // Calculates pointer increment value for given index component
		{
		ptrdiff_t result=1;
		for(int i=dimension-1;i>component;--i)
			result*=ptrdiff_t(index[i]);
		return result;
		}
	ptrdiff_t calcOffset(const ArrayIndexBase& multiIndex) const // Converts multi-index to linear index using this index as bounds
		{
		ptrdiff_t offset=multiIndex[0];
		for(int i=1;i<dimension;++i)
			offset=offset*ptrdiff_t(index[i])+ptrdiff_t(multiIndex[i]);
		return offset;
		}
	ArrayIndexBase calcIndex(ptrdiff_t linearIndex) const // Converts linear index to multi-index using this index as bounds
		{
		ArrayIndexBase result;
		for(int i=dimension-1;i>0;--i)
			{
			result[i]=int(linearIndex%ptrdiff_t(index[i]));
			linearIndex/=ptrdiff_t(index[i]);
			}
		result[0]=int(linearIndex);
		return result;
		}
	ArrayIndexBase& preInc(int component) // Pre-increments one index component
		{
		++index[component];
		return *this;
		}
	ArrayIndexBase postInc(int component) // Post-increments one index component
		{
		ArrayIndexBase result=*this;
		++result.index[component];
		return result;
		}
	ArrayIndexBase& preDec(int component) // Pre-decrements one index component
		{
		--index[component];
		return *this;
		}
	ArrayIndexBase postDec(int component) // Post-decrements one index component
		{
		ArrayIndexBase result=*this;
		--result.index[component];
		return result;
		}
	ArrayIndexBase& preSet(int component,int newIndex) // Pre-sets one component to a new value
		{
		index[component]=newIndex;
		return *this;
		}
	ArrayIndexBase postSet(int component,int newIndex) // Post-sets one component to a new value
		{
		ArrayIndexBase result=*this;
		result.index[component]=newIndex;
		return result;
		}
	bool isInBounds(const ArrayIndexBase& bounds) const // Returns true if the index is inside the given bounds
		{
		for(int i=0;i<dimension;++i)
			if(index[i]>=bounds.index[i])
				return false;
		return true;
		}
	ArrayIndexBase& preInc(const ArrayIndexBase& bounds) // Pre-increments index inside given bounds
		{
		/* Increment one index component with wrap-around: */
		int incDim;
		for(incDim=dimension-1;incDim>0&&index[incDim]>=bounds.index[incDim]-1;--incDim)
			index[incDim]=0;
		++index[incDim];
		
		/* Return modified index: */
		return *this;
		}
	ArrayIndexBase postInc(const ArrayIndexBase& bounds) // Post-increments index inside given bounds
		{
		/* Save current index value: */
		ArrayIndexBase result(index);
		
		/* Increment one index component with wrap-around: */
		int incDim;
		for(incDim=dimension-1;incDim>0&&index[incDim]>=bounds.index[incDim]-1;--incDim)
			index[incDim]=0;
		++index[incDim];
		
		/* Return saved index: */
		return result;
		}
	ArrayIndexBase& preDec(const ArrayIndexBase& bounds) // Pre-decrements index inside given bounds
		{
		/* Decrement one index component with wrap-around: */
		int decDim;
		for(decDim=dimension-1;decDim>0&&index[decDim]<=0;--decDim)
			index[decDim]=bounds.index[decDim]-1;
		--index[decDim];
		
		/* Return modified index: */
		return *this;
		}
	ArrayIndexBase postDec(const ArrayIndexBase& bounds) // Post-decrements index inside given bounds
		{
		/* Save current index value: */
		ArrayIndexBase result(index);
		
		/* Decrement one index component with wrap-around: */
		int decDim;
		for(decDim=dimension-1;decDim>0&&index[decDim]<=0;--decDim)
			index[decDim]=bounds.index[decDim]-1;
		--index[decDim];
		
		/* Return saved index: */
		return result;
		}
	ArrayIndexBase& preInc(const ArrayIndexBase& minBound,const ArrayIndexBase& maxBound) // Pre-increments index inside given bounds
		{
		/* Increment one index component with wrap-around: */
		int incDim;
		for(incDim=dimension-1;incDim>0&&index[incDim]>=maxBound.index[incDim]-1;--incDim)
			index[incDim]=minBound.index[incDim];
		++index[incDim];
		
		/* Return modified index: */
		return *this;
		}
	ArrayIndexBase& preDec(const ArrayIndexBase& minBound,const ArrayIndexBase& maxBound) // Pre-decrements index inside given bounds
		{
		/* Decrement one index component with wrap-around: */
		int decDim;
		for(decDim=dimension-1;decDim>0&&index[decDim]<=minBound.index[decDim];--decDim)
			index[decDim]=maxBound.index[decDim]-1;
		--index[decDim];
		
		/* Return modified index: */
		return *this;
		}
	ArrayIndexBase postInc(const ArrayIndexBase& minBound,const ArrayIndexBase& maxBound) // Post-increments index inside given bounds
		{
		/* Save current index value: */
		ArrayIndexBase result(index);
		
		/* Increment one index component with wrap-around: */
		int incDim;
		for(incDim=dimension-1;incDim>0&&index[incDim]>=maxBound.index[incDim]-1;--incDim)
			index[incDim]=minBound.index[incDim];
		++index[incDim];
		
		/* Return saved index: */
		return result;
		}
	ArrayIndexBase& postDec(const ArrayIndexBase& minBound,const ArrayIndexBase& maxBound) // Post-decrements index inside given bounds
		{
		/* Save current index value: */
		ArrayIndexBase result(index);
		
		/* Decrement one index component with wrap-around: */
		int decDim;
		for(decDim=dimension-1;decDim>0&&index[decDim]<=minBound.index[decDim];--decDim)
			index[decDim]=maxBound.index[decDim]-1;
		--index[decDim];
		
		/* Return saved index: */
		return result;
		}
	};

/********************************************
Generic class for n-dimensional multiindices:
********************************************/

template <int dimensionParam>
class ArrayIndex:public ArrayIndexBase<dimensionParam>
	{
	/* Constructors and destructors: */
	public:
	ArrayIndex(void)
		{
		}
	explicit ArrayIndex(int sIndices)
		:ArrayIndexBase<dimensionParam>(sIndices)
		{
		}
	ArrayIndex(const int sIndex[dimensionParam])
		:ArrayIndexBase<dimensionParam>(sIndex)
		{
		}
	ArrayIndex(const ArrayIndexBase<dimensionParam>& source)
		:ArrayIndexBase<dimensionParam>(source)
		{
		}
	ArrayIndex& operator=(const ArrayIndexBase<dimensionParam>& source)
		{
		ArrayIndexBase<dimensionParam>::operator=(source);
		return *this;
		}
	};

/* Operators on n-dimensional multiindices: */

template <int dimensionParam>
inline ArrayIndex<dimensionParam> operator+(const ArrayIndexBase<dimensionParam>& i1,const ArrayIndexBase<dimensionParam>& i2)
	{
	ArrayIndex<dimensionParam> result;
	for(int i=0;i<dimensionParam;++i)
		result[i]=i1[i]+i2[i];
	return result;
	}

template <int dimensionParam>
inline ArrayIndex<dimensionParam> operator-(const ArrayIndexBase<dimensionParam>& i1,const ArrayIndexBase<dimensionParam>& i2)
	{
	ArrayIndex<dimensionParam> result;
	for(int i=0;i<dimensionParam;++i)
		result[i]=i1[i]-i2[i];
	return result;
	}

/*********************************************
Specialization for 1-dimensional multiindices:
*********************************************/

template <>
class ArrayIndex<1>:public ArrayIndexBase<1>
	{
	/* Constructors and destructors: */
	public:
	ArrayIndex(void)
		{
		}
	ArrayIndex(int i0)
		{
		index[0]=i0;
		}
	ArrayIndex(const int sIndex[1])
		:ArrayIndexBase<1>(sIndex)
		{
		}
	ArrayIndex(const ArrayIndexBase<1>& source)
		:ArrayIndexBase<1>(source)
		{
		}
	ArrayIndex& operator=(const ArrayIndexBase<1>& source)
		{
		ArrayIndexBase<1>::operator=(source);
		return *this;
		}
	
	/* Methods: */
	operator int(void) const // Converts one-dimensional multiindex to integer
		{
		return index[0];
		}
	};

/* Operators on 1-dimensional multiindices: */

template <>
inline ArrayIndex<1> operator+(const ArrayIndexBase<1>& i1,const ArrayIndexBase<1>& i2)
	{
	return ArrayIndex<1>(i1[0]+i2[0]);
	}

template <>
inline ArrayIndex<1> operator-(const ArrayIndexBase<1>& i1,const ArrayIndexBase<1>& i2)
	{
	return ArrayIndex<1>(i1[0]-i2[0]);
	}

/*********************************************
Specialization for 2-dimensional multiindices:
*********************************************/

template <>
class ArrayIndex<2>:public ArrayIndexBase<2>
	{
	/* Constructors and destructors: */
	public:
	ArrayIndex(void)
		{
		}
	explicit ArrayIndex(int sIndices)
		:ArrayIndexBase<2>(sIndices)
		{
		}
	ArrayIndex(int i0,int i1)
		{
		index[0]=i0;
		index[1]=i1;
		}
	ArrayIndex(const int sIndex[2])
		:ArrayIndexBase<2>(sIndex)
		{
		}
	ArrayIndex(const ArrayIndexBase<2>& source)
		:ArrayIndexBase<2>(source)
		{
		}
	ArrayIndex& operator=(const ArrayIndexBase<2>& source)
		{
		ArrayIndexBase<2>::operator=(source);
		return *this;
		}
	};

/* Operators on 2-dimensional multiindices: */

template <>
inline ArrayIndex<2> operator+(const ArrayIndexBase<2>& i1,const ArrayIndexBase<2>& i2)
	{
	return ArrayIndex<2>(i1[0]+i2[0],i1[1]+i2[1]);
	}

template <>
inline ArrayIndex<2> operator-(const ArrayIndexBase<2>& i1,const ArrayIndexBase<2>& i2)
	{
	return ArrayIndex<2>(i1[0]-i2[0],i1[1]-i2[1]);
	}

/*********************************************
Specialization for 3-dimensional multiindices:
*********************************************/

template <>
class ArrayIndex<3>:public ArrayIndexBase<3>
	{
	/* Constructors and destructors: */
	public:
	ArrayIndex(void)
		{
		}
	explicit ArrayIndex(int sIndices)
		:ArrayIndexBase<3>(sIndices)
		{
		}
	ArrayIndex(int i0,int i1,int i2)
		{
		index[0]=i0;
		index[1]=i1;
		index[2]=i2;
		}
	ArrayIndex(const int sIndex[3])
		:ArrayIndexBase<3>(sIndex)
		{
		}
	ArrayIndex(const ArrayIndexBase<3>& source)
		:ArrayIndexBase<3>(source)
		{
		}
	ArrayIndex& operator=(const ArrayIndexBase<3>& source)
		{
		ArrayIndexBase<3>::operator=(source);
		return *this;
		}
	};

/* Operators on 3-dimensional multiindices: */

template <>
inline ArrayIndex<3> operator+(const ArrayIndexBase<3>& i1,const ArrayIndexBase<3>& i2)
	{
	return ArrayIndex<3>(i1[0]+i2[0],i1[1]+i2[1],i1[2]+i2[2]);
	}

template <>
inline ArrayIndex<3> operator-(const ArrayIndexBase<3>& i1,const ArrayIndexBase<3>& i2)
	{
	return ArrayIndex<3>(i1[0]-i2[0],i1[1]-i2[1],i1[2]-i2[2]);
	}

/*********************************************
Specialization for 4-dimensional multiindices:
*********************************************/

template <>
class ArrayIndex<4>:public ArrayIndexBase<4>
	{
	/* Constructors and destructors: */
	public:
	ArrayIndex(void)
		{
		}
	explicit ArrayIndex(int sIndices)
		:ArrayIndexBase<4>(sIndices)
		{
		}
	ArrayIndex(int i0,int i1,int i2,int i3)
		{
		index[0]=i0;
		index[1]=i1;
		index[2]=i2;
		index[3]=i3;
		}
	ArrayIndex(const int sIndex[4])
		:ArrayIndexBase<4>(sIndex)
		{
		}
	ArrayIndex(const ArrayIndexBase<4>& source)
		:ArrayIndexBase<4>(source)
		{
		}
	ArrayIndex& operator=(const ArrayIndexBase<4>& source)
		{
		ArrayIndexBase<4>::operator=(source);
		return *this;
		}
	};

/* Operators on 4-dimensional multiindices: */

template <>
inline ArrayIndex<4> operator+(const ArrayIndexBase<4>& i1,const ArrayIndexBase<4>& i2)
	{
	return ArrayIndex<4>(i1[0]+i2[0],i1[1]+i2[1],i1[2]+i2[2],i1[3]+i2[3]);
	}

template <>
inline ArrayIndex<4> operator-(const ArrayIndexBase<4>& i1,const ArrayIndexBase<4>& i2)
	{
	return ArrayIndex<4>(i1[0]-i2[0],i1[1]-i2[1],i1[2]-i2[2],i1[3]-i2[3]);
	}

}

#endif
