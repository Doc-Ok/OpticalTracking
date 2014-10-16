/***********************************************************************
Interval - Class for closed intervals of arbitrary scalar types.
Copyright (c) 2003-2010 Oliver Kreylos

This file is part of the Templatized Math Library (Math).

The Templatized Math Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Templatized Math Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Templatized Math Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef MATH_INTERVAL_INCLUDED
#define MATH_INTERVAL_INCLUDED

namespace Math {

/* Forward declarations for friend functions: */
template <class ScalarParam>
class Interval;
template <class ScalarParam>
bool operator==(const Interval<ScalarParam>&,const Interval<ScalarParam>&);
template <class ScalarParam>
bool operator!=(const Interval<ScalarParam>&,const Interval<ScalarParam>&);
template <class ScalarParam>
Interval<ScalarParam> intersect(const Interval<ScalarParam>&,const Interval<ScalarParam>&);
template <class ScalarParam>
Interval<ScalarParam> add(const Interval<ScalarParam>&,ScalarParam);
template <class ScalarParam>
Interval<ScalarParam> operator+(ScalarParam,const Interval<ScalarParam>&);
template <class ScalarParam>
Interval<ScalarParam> operator+(const Interval<ScalarParam>&,const Interval<ScalarParam>&);
template <class ScalarParam>
Interval<ScalarParam> operator-(const Interval<ScalarParam>&,ScalarParam);
template <class ScalarParam>
Interval<ScalarParam> operator-(ScalarParam,const Interval<ScalarParam>&);
template <class ScalarParam>
Interval<ScalarParam> operator-(const Interval<ScalarParam>&,const Interval<ScalarParam>&);

template <class ScalarParam>
class Interval
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar; // Interval's scalar type
	typedef ScalarParam Size; // Interval's size type
	
	/* Elements: */
	private:
	Scalar min,max; // Lower and upper interval limits
	
	/* Constructors and destructors: */
	public:
	static const Interval empty; // The interval containing no values
	static const Interval full; // The interval containing every possible value
	Interval(void) // Constructs uninitialized interval
		{
		}
	Interval(Scalar value) // Creates singleton interval; embedding operator for scalar type
		:min(value),max(value)
		{
		}
	Interval(Scalar sMin,Scalar sMax) // Elementwise constructor
		:min(sMin),max(sMax)
		{
		}
	template <class SourceScalarParam>
	Interval(const Interval<SourceScalarParam>& source) // Copy constructor with type conversion
		:min(Scalar(source.min)),max(Scalar(source.max))
		{
		}
	template <class SourceScalarParam>
	Interval& operator=(const Interval<SourceScalarParam>& source) // Assignment operator with type conversion
		{
		/* Operation is idempotent; no need to check for aliasing: */
		min=Scalar(source.min);
		max=Scalar(source.max);
		return *this;
		}
	
	/* Methods: */
	friend bool operator==<>(const Interval& i1,const Interval& i2); // Equality operator
	friend bool operator!=<>(const Interval& i1,const Interval& i2); // Inequality operator
	bool isNull(void) const // Returns true if the interval contains no values at all
		{
		return min>max;
		}
	bool isEmpty(void) const // Returns true if the interval has no interior (i.e., contains exactly one value)
		{
		return min==max;
		}
	bool isFull(void) const // Returns true if the interval contains all values
		{
		return min==full.min&&max==full.max;
		}
	Scalar getMin(void) const // Returns interval's minimum value
		{
		return min;
		}
	Scalar getMax(void) const // Returns interval's maximum value
		{
		return max;
		}
	Size getSize(void) const // Returns the interval's width
		{
		return max-min;
		}
	bool contains(Scalar s) const // Checks if interval contains a value
		{
		return min<=s&&s<=max; // Intervals are closed
		}
	bool contains(const Interval& other) const // Checks if an interval contains another interval
		{
		return min<=other.min&&other.max<=max;
		}
	bool intersects(const Interval& other) const // Checks if interval intersects another interval
		{
		return min<=other.max&&other.min<=max;
		}
	bool overlaps(const Interval& other) const // Checks if a box intersects another box with a non-zero volume intersection
		{
		return min<other.max&&other.min<max;
		}
	Interval& addValue(Scalar s) // Changes the interval to contain the given value
		{
		/* Adjust interval borders: */
		if(min>s)
			min=s;
		if(max<s)
			max=s;
		
		/* Return changed interval: */
		return *this;
		}
	Interval& addInterval(const Interval& other) // Changes the interval to contain the given interval
		{
		/* Adjust interval borders: */
		if(min>other.min)
			min=other.min;
		if(max<other.max)
			max=other.max;
		
		/* Return changed interval: */
		return *this;
		}
	Interval& intersectInterval(const Interval& other); // Changes the interval to the intersection with the given interval
	friend Interval intersect<>(const Interval& i1,const Interval& i2); // Intersects two intervals
	
	/* Methods for interval arithmetic: */
	Interval operator+(void) const // Unary plus operator; returns copy of interval
		{
		return *this;
		}
	Interval operator-(void) const // Negation operator
		{
		return Interval(-max,-min);
		}
	Interval& operator+=(Scalar s) // Addition assignment with scalar
		{
		min+=s;
		max+=s;
		
		return *this;
		}
	Interval& operator+=(const Interval& other) // Addition assignment
		{
		min+=other.min;
		max+=other.max;
		
		return *this;
		}
	friend Interval add<>(const Interval& i,Scalar s); // Adds an interval and a scalar
	//friend Interval operator+<>(Scalar s,const Interval& i); // Adds a scalar and an interval
	//friend Interval operator+<>(const Interval& i1,const Interval& i2); // Adds two intervals
	Interval& operator-=(Scalar s) // Subtraction assignment with scalar
		{
		min-=s;
		max-=s;
		
		return *this;
		}
	Interval& operator-=(const Interval& other) // Subtraction assignment
		{
		min-=other.max;
		max-=other.min;
		
		return *this;
		}
	//friend Interval operator-<>(const Interval& i,Scalar s); // Subtracts a scalar from an interval
	//friend Interval operator-<>(Scalar s,const Interval& i); // Subtracts an interval from a scalar
	//friend Interval operator-<>(const Interval& i1,const Interval& i2); // Subtracts two intervals
	Interval& operator*=(Scalar s); // Multiplication assignment with scalar
	};

/**********************************
Friend functions of class Interval:
**********************************/

template <class ScalarParam>
inline bool operator==(const Interval<ScalarParam>& i1,const Interval<ScalarParam>& i2)
	{
	return i1.min==i2.min&&i1.max==i2.max;
	}

template <class ScalarParam>
inline bool operator!=(const Interval<ScalarParam>& i1,const Interval<ScalarParam>& i2)
	{
	return i1.min!=i2.min||i1.max!=i2.max;
	}

template <class ScalarParam>
inline Interval<ScalarParam> add(const Interval<ScalarParam>& i,ScalarParam s)
	{
	return Interval<ScalarParam>(i.min+s,i.max+s);
	}

template <class ScalarParam>
inline Interval<ScalarParam> operator+(ScalarParam s,const Interval<ScalarParam>& i)
	{
	return Interval<ScalarParam>(s+i.min,s+i.max);
	}

template <class ScalarParam>
inline Interval<ScalarParam> operator+(const Interval<ScalarParam>& i1,const Interval<ScalarParam>& i2)
	{
	return Interval<ScalarParam>(i1.min+i2.min,i1.max+i2.max);
	}

template <class ScalarParam>
inline Interval<ScalarParam> operator-(const Interval<ScalarParam>& i,ScalarParam s)
	{
	return Interval<ScalarParam>(i.min-s,i.max-s);
	}

template <class ScalarParam>
inline Interval<ScalarParam> operator-(ScalarParam s,const Interval<ScalarParam>& i)
	{
	return Interval<ScalarParam>(s-i.max,s-i.min);
	}

template <class ScalarParam>
inline Interval<ScalarParam> operator-(const Interval<ScalarParam>& i1,const Interval<ScalarParam>& i2)
	{
	return Interval<ScalarParam>(i1.min-i2.max,i1.max-i2.min);
	}

}

#if defined(MATH_NONSTANDARD_TEMPLATES) && !defined(MATH_INTERVAL_IMPLEMENTATION)
#include <Math/Interval.icpp>
#endif

#endif
