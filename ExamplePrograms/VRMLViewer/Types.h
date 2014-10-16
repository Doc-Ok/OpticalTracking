/***********************************************************************
Types - Declarations of data types used in VRML node fields.
Copyright (c) 2006-2008 Oliver Kreylos

This file is part of the Virtual Reality VRML viewer (VRMLViewer).

The Virtual Reality VRML viewer is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Virtual Reality VRML viewer is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Virtual Reality VRML viewer; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef TYPES_INCLUDED
#define TYPES_INCLUDED

#include <string>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/HVector.h>
#include <Geometry/Rotation.h>
#include <Geometry/OrthogonalTransformation.h>
#include <GL/gl.h>
#include <GL/GLColor.h>
#include <GL/GLVector.h>

/* Typed used in node fields: */
typedef bool Bool; // Type for boolean values
typedef int Int32; // Type for 32-bit signed integer values
typedef float Float; // Type for 32-bit floating-point values
typedef std::string String; // Type for strings
typedef GLColor<GLubyte,4> Color; // Type for RGB color values
typedef GLVector<GLfloat,2> Vec2f; // Type for 2D vectors or points
typedef GLVector<GLfloat,3> Vec3f; // Type for 3D vectors or points
typedef Geometry::Rotation<float,3> Rotation; // Type for rotations (actually, orientations)

/* Other types: */
typedef Geometry::Point<float,3> Point; // Type for points
typedef Geometry::Vector<float,3> Vector; // Type for vectors
typedef Geometry::HVector<float,3> HVector; // Type for homogeneous vectors
typedef Geometry::OrthogonalTransformation<float,3> Transformation; // Type for transformations

#endif
