/***********************************************************************
IntersectionTests - Namespace-global functions to test for intersections
between various geometric objects.
Copyright (c) 2012-2013 Oliver Kreylos

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

#ifndef GEOMETRY_INTERSECTIONTESTS_INCLUDED
#define GEOMETRY_INTERSECTIONTESTS_INCLUDED

#include <stddef.h>

/* Forward declarations: */
namespace Geometry {
template <class ScalarParam,int dimensionParam>
class Point;
template <class ScalarParam,int dimensionParam>
class Vector;
template <class ScalarParam,int dimensionParam>
class Box;
}

namespace Geometry {

template <class ScalarParam>
bool isPointInsideTriangle(const Point<ScalarParam,2> triangle[3],const Point<ScalarParam,2>& p); // Returns true if the given point is inside the given closed triangle; assumes triangle is right-handed
template <class ScalarParam>
bool isPointInsideTriangle(const Point<ScalarParam,2>& t0,const Point<ScalarParam,2>& t1,const Point<ScalarParam,2>& t2,const Point<ScalarParam,2>& p); // Ditto
template <class ScalarParam>
bool isPointInsideTriangle(const Point<ScalarParam,3> triangle[3],const Vector<ScalarParam,3>& triangleNormal,const Point<ScalarParam,3>& p); // Returns true if the given point, assumed to be inside the given triangle's plane, is inside the given closed triangle; assumes triangle is right-handed with respect to normal vector
template <class ScalarParam>
bool isPointInsideTriangle(const Point<ScalarParam,3>& t0,const Point<ScalarParam,3>& t1,const Point<ScalarParam,3>& t2,const Vector<ScalarParam,3>& triangleNormal,const Point<ScalarParam,3>& p); // Ditto

template <class ScalarParam,int dimensionParam>
bool doesLineIntersectBox(const Box<ScalarParam,dimensionParam>& box,const Point<ScalarParam,dimensionParam> line[2]); // Returns true if the intersection set between the closed box and the closed line segment defined by the two points is non-empty
template <class ScalarParam>
bool doesLineIntersectTriangle(const Point<ScalarParam,3> triangle[3],const Point<ScalarParam,3> line[2]); // Returns true if the intersection set between the closed triangle defined by the three vertices and the closed line segment defined by the two points is non-empty
template <class ScalarParam>
bool doesTriangleIntersectBox(const Box<ScalarParam,3>& box,const Point<ScalarParam,3> triangle[3]); // Returns true if the intersection set between the closed box and the closed triangle defined by the three vertices is non-empty
template <class ScalarParam>
bool doesTriangleIntersectBox(const Box<ScalarParam,3>& box,const Point<ScalarParam,3>* vertexArray,const size_t triangleIndices[3]); // Ditto using a triangle defined by three vertex indices into the given vertex array

}

#if defined(GEOMETRY_NONSTANDARD_TEMPLATES) && !defined(GEOMETRY_INTERSECTIONTESTS_IMPLEMENTATION)
#include <Geometry/IntersectionTests.icpp>
#endif

#endif
