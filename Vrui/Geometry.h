/***********************************************************************
Geometry - Geometry data types used with the Vrui library.
Copyright (c) 2005 Oliver Kreylos

This file is part of the Virtual Reality User Interface Library (Vrui).

The Virtual Reality User Interface Library is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The Virtual Reality User Interface Library is distributed in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Virtual Reality User Interface Library; if not, write to the
Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#ifndef VRUI_GEOMETRY_INCLUDED
#define VRUI_GEOMETRY_INCLUDED

/**************************************
Forward declarations of geometry types:
**************************************/

namespace Geometry {
template <class ScalarParam,int dimensionParam>
class ComponentArray;
template <class ScalarParam,int dimensionParam>
class Point;
template <class ScalarParam,int dimensionParam>
class Vector;
template <class ScalarParam,int dimensionParam>
class Rotation;
template <class ScalarParam,int dimensionParam>
class OrthonormalTransformation;
template <class ScalarParam,int dimensionParam>
class OrthogonalTransformation;
template <class ScalarParam,int dimensionParam>
class AffineTransformation;
template <class ScalarParam,int dimensionParam>
class ProjectiveTransformation;
template <class ScalarParam,int dimensionParam>
class Ray;
template <class ScalarParam,int dimensionParam>
class Plane;
}

/***********************************
Declarations of Vrui geometry types:
***********************************/

namespace Vrui {

/* Basic (non-usage specific) geometry data types: */
typedef double Scalar; // Scalar type of Vrui's affine space
typedef Geometry::ComponentArray<Scalar,3> Size; // Size of an object along the primary axes
typedef Geometry::Point<Scalar,3> Point; // Affine point
typedef Geometry::Vector<Scalar,3> Vector; // Affine vector
typedef Geometry::Rotation<Scalar,3> Rotation; // Affine rotation
typedef Geometry::OrthonormalTransformation<Scalar,3> ONTransform; // Rigid body transformation (translation+rotation)
typedef Geometry::OrthogonalTransformation<Scalar,3> OGTransform; // Rigid body transformation with uniform scaling
typedef Geometry::AffineTransformation<Scalar,3> ATransform; // General affine transformation
typedef Geometry::ProjectiveTransformation<Scalar,3> PTransform; // General projective transformation
typedef Geometry::Ray<Scalar,3> Ray; // Affine ray (half-line)
typedef Geometry::Plane<Scalar,3> Plane; // Plane (half-space)

/* Usage specific geometry data types: */
typedef ONTransform TrackerState; // Type for raw (untransformed) tracker states
typedef OGTransform NavTransform; // Type for navigation transformations
typedef OGTransform NavTrackerState; // Type for user (transformed) tracker states

}

#endif
