/***********************************************************************
ValuedPoint - Class for points with arbitrary data value attached.
Copyright (c) 2003-2005 Oliver Kreylos

This file is part of the Templatized Geometry Library (TGL).

The Templatized Geometry Library is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Templatized Geometry Library is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Templatized Geometry Library; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#ifndef GEOMETRY_VALUEDPOINT_INCLUDED
#define GEOMETRY_VALUEDPOINT_INCLUDED

#include <Geometry/Point.h>

namespace Geometry {

/*************************
Generic ValuedPoint class:
*************************/

template <class PointParam,class ValueParam>
class ValuedPoint:public PointParam
	{
	/* Embedded classes: */
	public:
	typedef PointParam Point; // Data type of base class
	typedef ValueParam Value; // Data type of value element
	
	/* Elements: */
	Value value; // Data value associated with point
	
	/* Constructors and destructors: */
	ValuedPoint(void) // Uninitialized valued point
		{
		}
	ValuedPoint(const Point& sPoint) // Initializes point part only
		:Point(sPoint)
		{
		}
	ValuedPoint(const Point& sPoint,const Value& sValue)
		:Point(sPoint),value(sValue)
		{
		}
	ValuedPoint& operator=(const Point& newPoint)
		{
		Point::operator=(newPoint);
		return *this;
		}
	};

/******************************************************
Specialized version of ValuedPoint class for void data:
******************************************************/

template <class PointParam>
class ValuedPoint<PointParam,void>:public PointParam
	{
	/* Embedded classes: */
	public:
	typedef PointParam Point; // Data type of base class
	typedef void Value; // Data type of value element (there is none)
	
	/* Constructors and destructors: */
	ValuedPoint(void) // Uninitialized valued point
		{
		}
	ValuedPoint(const Point& sPoint)
		:Point(sPoint)
		{
		}
	ValuedPoint& operator=(const Point& newPoint)
		{
		Point::operator=(newPoint);
		return *this;
		}
	};

}

#endif
