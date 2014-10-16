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

#include <Geometry/IntersectionTests.icpp>

namespace Geometry {

/***************************************************************
Force instantiation of all standard intersection test functions:
***************************************************************/

template bool isPointInsideTriangle<float>(const Point<float,2>[3],const Point<float,2>&);
template bool isPointInsideTriangle<double>(const Point<double,2>[3],const Point<double,2>&);

template bool isPointInsideTriangle<float>(const Point<float,2>&,const Point<float,2>&,const Point<float,2>&,const Point<float,2>&);
template bool isPointInsideTriangle<double>(const Point<double,2>&,const Point<double,2>&,const Point<double,2>&,const Point<double,2>&);

template bool isPointInsideTriangle<float>(const Point<float,3>[3],const Vector<float,3>&,const Point<float,3>&);
template bool isPointInsideTriangle<double>(const Point<double,3>[3],const Vector<double,3>&,const Point<double,3>&);

template bool isPointInsideTriangle<float>(const Point<float,3>&,const Point<float,3>&,const Point<float,3>&,const Vector<float,3>&,const Point<float,3>&);
template bool isPointInsideTriangle<double>(const Point<double,3>&,const Point<double,3>&,const Point<double,3>&,const Vector<double,3>&,const Point<double,3>&);

template bool doesLineIntersectBox<float,2>(const Box<float,2>&,const Point<float,2>[2]);
template bool doesLineIntersectBox<float,3>(const Box<float,3>&,const Point<float,3>[2]);
template bool doesLineIntersectBox<double,2>(const Box<double,2>&,const Point<double,2>[2]);
template bool doesLineIntersectBox<double,3>(const Box<double,3>&,const Point<double,3>[2]);

template bool doesLineIntersectTriangle<float>(const Point<float,3>[3],const Point<float,3>[2]);
template bool doesLineIntersectTriangle<double>(const Point<double,3>[3],const Point<double,3>[2]);

template bool doesTriangleIntersectBox<float>(const Box<float,3>&,const Point<float,3>[3]);
template bool doesTriangleIntersectBox<double>(const Box<double,3>&,const Point<double,3>[3]);

template bool doesTriangleIntersectBox<float>(const Box<float,3>&,const Point<float,3>*,const size_t[3]);
template bool doesTriangleIntersectBox<double>(const Box<double,3>&,const Point<double,3>*,const size_t[3]);

}
