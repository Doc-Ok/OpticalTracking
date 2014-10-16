/***********************************************************************
Geometry - Geometry data types used with the scene graph renderer.
Copyright (c) 2009 Oliver Kreylos

This file is part of the Simple Scene Graph Renderer (SceneGraph).

The Simple Scene Graph Renderer is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Simple Scene Graph Renderer is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Simple Scene Graph Renderer; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef SCENEGRAPH_GEOMETRY_INCLUDED
#define SCENEGRAPH_GEOMETRY_INCLUDED

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
class Ray;
template <class ScalarParam,int dimensionParam>
class Box;
}

/******************************************
Declarations of scene graph geometry types:
******************************************/

namespace SceneGraph {

typedef float Scalar; // Scalar type of scene graph renderer's affine space
typedef Geometry::ComponentArray<Scalar,3> Size; // 3D size
typedef Geometry::Point<Scalar,3> Point; // Affine point
typedef Geometry::Vector<Scalar,3> Vector; // Affine vector
typedef Geometry::Rotation<Scalar,3> Rotation; // Affine rotation
typedef Geometry::OrthonormalTransformation<Scalar,3> ONTransform; // Rigid body transformation (translation+rotation)
typedef Geometry::OrthogonalTransformation<Scalar,3> OGTransform; // Rigid body transformation with uniform scaling
typedef Geometry::Ray<Scalar,3> Ray; // Affine ray (half-line)
typedef Geometry::Box<Scalar,3> Box; // Axis-aligned box
typedef Geometry::Point<Scalar,2> TexCoord; // 2D Texture coordinate

}

#endif
