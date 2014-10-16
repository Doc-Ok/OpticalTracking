/***********************************************************************
OutputOperators - Basic iostream output operators for templatized
geometry objects.
Copyright (c) 2009-2010 Oliver Kreylos

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

#ifndef GEOMETRY_OUTPUTOPERATORS_INCLUDED
#define GEOMETRY_OUTPUTOPERATORS_INCLUDED

#include <iostream>

/* Forward declarations: */
namespace Geometry {
template <class ScalarParam,int dimensionParam>
class ComponentArray;
template <class ScalarParam,int dimensionParam>
class Box;
template <class ScalarParam,int dimensionParam>
class Plane;
template <class ScalarParam,int numRowsParam,int numColumnsParam>
class Matrix;
template <class ScalarParam,int dimensionParam>
class Rotation;
template <class ScalarParam,int dimensionParam>
class TranslationTransformation;
template <class ScalarParam,int dimensionParam>
class RotationTransformation;
template <class ScalarParam,int dimensionParam>
class OrthonormalTransformation;
template <class ScalarParam,int dimensionParam>
class OrthogonalTransformation;
template <class ScalarParam,int dimensionParam>
class AffineTransformation;
template <class ScalarParam,int dimensionParam>
class ProjectiveTransformation;
}

namespace std {

template <class ScalarParam,int dimensionParam>
ostream& operator<<(ostream& os,const Geometry::ComponentArray<ScalarParam,dimensionParam>& ca);
template <class ScalarParam,int dimensionParam>
ostream& operator<<(ostream& os,const Geometry::Box<ScalarParam,dimensionParam>& b);
template <class ScalarParam,int dimensionParam>
ostream& operator<<(ostream& os,const Geometry::Plane<ScalarParam,dimensionParam>& p);
template <class ScalarParam,int numRowsParam,int numColumnsParam>
ostream& operator<<(ostream& os,const Geometry::Matrix<ScalarParam,numRowsParam,numColumnsParam>& m);
template <class ScalarParam,int dimensionParam>
ostream& operator<<(ostream& os,const Geometry::Rotation<ScalarParam,dimensionParam>& r);
template <class ScalarParam,int dimensionParam>
ostream& operator<<(ostream& os,const Geometry::TranslationTransformation<ScalarParam,dimensionParam>& t);
template <class ScalarParam,int dimensionParam>
ostream& operator<<(ostream& os,const Geometry::RotationTransformation<ScalarParam,dimensionParam>& t);
template <class ScalarParam,int dimensionParam>
ostream& operator<<(ostream& os,const Geometry::OrthonormalTransformation<ScalarParam,dimensionParam>& t);
template <class ScalarParam,int dimensionParam>
ostream& operator<<(ostream& os,const Geometry::OrthogonalTransformation<ScalarParam,dimensionParam>& t);
template <class ScalarParam,int dimensionParam>
ostream& operator<<(ostream& os,const Geometry::AffineTransformation<ScalarParam,dimensionParam>& t);
template <class ScalarParam,int dimensionParam>
ostream& operator<<(ostream& os,const Geometry::ProjectiveTransformation<ScalarParam,dimensionParam>& t);

}

#if defined(GEOMETRY_NONSTANDARD_TEMPLATES) && !defined(GEOMETRY_OUTPUTOPERATORS_IMPLEMENTATION)
#include <Geometry/OutputOperators.icpp>
#endif

#endif
