/***********************************************************************
GLFrustum - Class describing an OpenGL view frustum in model coordinates
to perform software-based frustum culling and LOD calculation.
Copyright (c) 2007-2013 Oliver Kreylos

This file is part of the OpenGL Wrapper Library for the Templatized
Geometry Library (GLGeometry).

The OpenGL Wrapper Library for the Templatized Geometry Library is free
software; you can redistribute it and/or modify it under the terms of
the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any
later version.

The OpenGL Wrapper Library for the Templatized Geometry Library is
distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License along
with the OpenGL Wrapper Library for the Templatized Geometry Library; if
not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite
330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef GLFRUSTUM_INCLUDED
#define GLFRUSTUM_INCLUDED

#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/HVector.h>
#include <Geometry/Plane.h>
#include <Geometry/Box.h>

template <class ScalarParam>
class GLFrustum
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar; // Scalar type for points and planes
	typedef Geometry::Point<Scalar,3> Point; // Type for points
	typedef Geometry::Vector<Scalar,3> Vector; // Type for vectors
	typedef Geometry::HVector<Scalar,3> HVector; // Type for homogeneous vectors
	typedef Geometry::Plane<Scalar,3> Plane; // Type for planes
	typedef Geometry::Box<Scalar,3> Box; // Type for axis-aligned boxes
	
	/* Elements: */
	private:
	Point frustumVertices[8]; // Position of the eight corner vertices of the frustum in model coordinates
	Plane frustumPlanes[6]; // Normalized plane equations of the six faces of the view frustum in model coordinates, with normals facing inwards
	Plane screenPlane; // Plane equation of the screen
	HVector eye; // Eye position in (homogeneous) model coordinates
	Scalar eyeScreenDist; // Inverse distance from screen to eye
	Scalar pixelSize; // Approximate inverse pixel size in model coordinates
	
	/* Methods: */
	public:
	void setFrustumVertex(int vertexIndex,const Point& newVertex) // Sets a frustum vertex
		{
		frustumVertices[vertexIndex]=newVertex;
		}
	void setFrustumPlane(int planeIndex,const Plane& newPlane) // Sets a frustum plane
		{
		frustumPlanes[planeIndex]=newPlane;
		}
	void setScreenEye(const Plane& newScreenPlane,const HVector& newEye); // Sets the screen plane equation and the eye point
	void setPixelSize(Scalar newPixelSize) // Sets the inverse pixel size
		{
		pixelSize=newPixelSize;
		}
	void setFromGL(void); // Extracts the view frustum from the current OpenGL state
	const Point& getFrustumVertex(int vertexIndex) const // Returns one of the frustum corner vertices
		{
		return frustumVertices[vertexIndex];
		}
	const Plane& getFrustumPlane(int planeIndex) const // Returns one of the frustum face planes
		{
		return frustumPlanes[planeIndex];
		}
	const Plane& getScreenPlane(void) const // Returns the screen's plane equation in model coordinates
		{
		return screenPlane;
		}
	const HVector& getEye(void) const // Returns the eye position in (homogeneous) model coordinates
		{
		return eye;
		}
	Scalar getEyeScreenDistance(void) const // Returns the inverse distance from the eye to the screen
		{
		return eyeScreenDist;
		}
	Scalar getPixelSize(void) const // Returns the inverse size of a pixel in model coordinates
		{
		return pixelSize;
		}
	Scalar calcProjectedRadius(const Point& sphereCenter,Scalar sphereRadius) const // Returns the (approximate) projected radius of the given sphere in pixels
		{
		return (sphereRadius*pixelSize)/(Scalar(1)-eyeScreenDist*screenPlane.calcDistance(sphereCenter));
		}
	bool doesBoxIntersect(const Box& box) const; // Returns false if the given box is guaranteed not to intersect the frustum
	bool doesSphereIntersect(const Point& center,Scalar radius) const; // Returns false if the given sphere is guaranteed not to intersect the frustum
	};

#if defined(GLGEOMETRY_NONSTANDARD_TEMPLATES) && !defined(GLFRUSTUM_IMPLEMENTATION)
#include <GL/GLFrustum.icpp>
#endif

#endif
