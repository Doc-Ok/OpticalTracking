/***********************************************************************
GeometryValueCoders - Value coder classes for templatized geometry
objects.
Copyright (c) 2003-2013 Oliver Kreylos

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

#include <Geometry/GeometryValueCoders.icpp>

namespace Misc {

/******************************************************
Force instantiation of all standard ValueCoder classes:
******************************************************/

template class ValueCoder<Geometry::ComponentArray<int,2> >;
template class ValueCoder<Geometry::Vector<int,2> >;
template class ValueCoder<Geometry::Point<int,2> >;
template class ValueCoder<Geometry::HVector<int,2> >;
template class ValueCoder<Geometry::Box<int,2> >;
template class ValueCoder<Geometry::Matrix<int,2,2> >;
template class ValueCoder<Geometry::Matrix<int,2,3> >;

template class ValueCoder<Geometry::ComponentArray<int,3> >;
template class ValueCoder<Geometry::Vector<int,3> >;
template class ValueCoder<Geometry::Point<int,3> >;
template class ValueCoder<Geometry::HVector<int,3> >;
template class ValueCoder<Geometry::Box<int,3> >;
template class ValueCoder<Geometry::Matrix<int,3,3> >;
template class ValueCoder<Geometry::Matrix<int,3,4> >;

template class ValueCoder<Geometry::ComponentArray<int,4> >;
template class ValueCoder<Geometry::Matrix<int,4,4> >;

template class ValueCoder<Geometry::ComponentArray<float,2> >;
template class ValueCoder<Geometry::Vector<float,2> >;
template class ValueCoder<Geometry::Point<float,2> >;
template class ValueCoder<Geometry::HVector<float,2> >;
template class ValueCoder<Geometry::Box<float,2> >;
template class ValueCoder<Geometry::Plane<float,2> >;
template class ValueCoder<Geometry::Matrix<float,2,2> >;
template class ValueCoder<Geometry::Matrix<float,2,3> >;
template class ValueCoder<Geometry::Rotation<float,2> >;
template class ValueCoder<Geometry::OrthonormalTransformation<float,2> >;
template class ValueCoder<Geometry::OrthogonalTransformation<float,2> >;
template class ValueCoder<Geometry::AffineTransformation<float,2> >;
template class ValueCoder<Geometry::ProjectiveTransformation<float,2> >;

template class ValueCoder<Geometry::ComponentArray<float,3> >;
template class ValueCoder<Geometry::Vector<float,3> >;
template class ValueCoder<Geometry::Point<float,3> >;
template class ValueCoder<Geometry::HVector<float,3> >;
template class ValueCoder<Geometry::Box<float,3> >;
template class ValueCoder<Geometry::Plane<float,3> >;
template class ValueCoder<Geometry::Matrix<float,3,3> >;
template class ValueCoder<Geometry::Matrix<float,3,4> >;
template class ValueCoder<Geometry::Rotation<float,3> >;
template class ValueCoder<Geometry::OrthonormalTransformation<float,3> >;
template class ValueCoder<Geometry::OrthogonalTransformation<float,3> >;
template class ValueCoder<Geometry::AffineTransformation<float,3> >;
template class ValueCoder<Geometry::ProjectiveTransformation<float,3> >;

template class ValueCoder<Geometry::ComponentArray<float,4> >;
template class ValueCoder<Geometry::Matrix<float,4,4> >;

template class ValueCoder<Geometry::ComponentArray<double,2> >;
template class ValueCoder<Geometry::Vector<double,2> >;
template class ValueCoder<Geometry::Point<double,2> >;
template class ValueCoder<Geometry::HVector<double,2> >;
template class ValueCoder<Geometry::Box<double,2> >;
template class ValueCoder<Geometry::Plane<double,2> >;
template class ValueCoder<Geometry::Matrix<double,2,2> >;
template class ValueCoder<Geometry::Matrix<double,2,3> >;
template class ValueCoder<Geometry::Rotation<double,2> >;
template class ValueCoder<Geometry::OrthonormalTransformation<double,2> >;
template class ValueCoder<Geometry::OrthogonalTransformation<double,2> >;
template class ValueCoder<Geometry::AffineTransformation<double,2> >;
template class ValueCoder<Geometry::ProjectiveTransformation<double,2> >;

template class ValueCoder<Geometry::ComponentArray<double,3> >;
template class ValueCoder<Geometry::Vector<double,3> >;
template class ValueCoder<Geometry::Point<double,3> >;
template class ValueCoder<Geometry::HVector<double,3> >;
template class ValueCoder<Geometry::Box<double,3> >;
template class ValueCoder<Geometry::Plane<double,3> >;
template class ValueCoder<Geometry::Matrix<double,3,3> >;
template class ValueCoder<Geometry::Matrix<double,3,4> >;
template class ValueCoder<Geometry::Rotation<double,3> >;
template class ValueCoder<Geometry::OrthonormalTransformation<double,3> >;
template class ValueCoder<Geometry::OrthogonalTransformation<double,3> >;
template class ValueCoder<Geometry::AffineTransformation<double,3> >;
template class ValueCoder<Geometry::ProjectiveTransformation<double,3> >;

template class ValueCoder<Geometry::ComponentArray<double,4> >;
template class ValueCoder<Geometry::Matrix<double,4,4> >;

}
