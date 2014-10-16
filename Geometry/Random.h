/***********************************************************************
Random - Functions to create random points or vectors according to
several probability distributions.
Copyright (c) 2007-2010 Oliver Kreylos

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

#ifndef GEOMETRY_RANDOM_INCLUDED
#define GEOMETRY_RANDOM_INCLUDED

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

template <class ScalarParam,int dimensionParam>
Point<ScalarParam,dimensionParam>
randPointUniformCO(
	const Point<ScalarParam,dimensionParam>& min,
	const Point<ScalarParam,dimensionParam>& max); // Returns a point uniformly distributed between the min (inclusive) and max (exclusive)

template <class ScalarParam,int dimensionParam>
Point<ScalarParam,dimensionParam>
randPointUniformCC(
	const Point<ScalarParam,dimensionParam>& min,
	const Point<ScalarParam,dimensionParam>& max); // Returns a point uniformly distributed between the min and max (both inclusive)

template <class ScalarParam,int dimensionParam>
Point<ScalarParam,dimensionParam>
randPointUniformCO(
	const Box<ScalarParam,dimensionParam>& box); // Returns a point uniformly distributed between the box's min (inclusive) and max (exclusive)

template <class ScalarParam,int dimensionParam>
Point<ScalarParam,dimensionParam>
randPointUniformCC(
	const Box<ScalarParam,dimensionParam>& box); // Returns a point uniformly distributed between the box's min min and max (both inclusive)

template <class ScalarParam,int dimensionParam>
Vector<ScalarParam,dimensionParam>
randVectorUniform(
	ScalarParam length); // Returns a vector with the given length and uniformly distributed direction

template <class ScalarParam,int dimensionParam>
Vector<ScalarParam,dimensionParam>
randUnitVectorUniform(
	void); // Returns a unit vector with uniformly distributed direction

template <class ScalarParam,int dimensionParam>
Vector<ScalarParam,dimensionParam>
randVectorNormal(
	ScalarParam stddev); // Returns a vector with normally distributed length and uniformly distributed direction

}

#if defined(GEOMETRY_NONSTANDARD_TEMPLATES) && !defined(GEOMETRY_RANDOM_IMPLEMENTATION)
#include <Geometry/Random.icpp>
#endif

#endif
