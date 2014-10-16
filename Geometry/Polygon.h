/***********************************************************************
Polygon - Class for planar polygons in affine space.
Copyright (c) 2004-2014 Oliver Kreylos

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

#ifndef GEOMETRY_POLYGON_INCLUDED
#define GEOMETRY_POLYGON_INCLUDED

#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/Plane.h>

namespace Geometry {

/************************************************
A generic base class for polygon implementations:
************************************************/

template <class ScalarParam,int dimensionParam>
class PolygonBase
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar; // The underlying scalar type
	static const int dimension=dimensionParam; // The affine space's dimension
	typedef Geometry::Point<ScalarParam,dimensionParam> Point; // The type for points
	typedef Geometry::Vector<ScalarParam,dimensionParam> Vector; // Type for compatible vectors
	typedef Geometry::Plane<ScalarParam,dimensionParam> Plane; // Type for compatible planes
	
	/* Elements: */
	protected:
	int numVertices; // Number of vertices in the polygon
	Point* vertices; // C-style array of vertices in counter-clockwise order around the polygon
	
	/* Constructors and destructors: */
	public:
	PolygonBase(void) // Constructs empty polygon
		:numVertices(0),vertices(0)
		{
		}
	PolygonBase(int sNumVertices); // Creates a polygon with an uninitialized vertex array
	PolygonBase(int sNumVertices,const Point sVertices[]); // Creates a polygon from a C-style array of points
	PolygonBase(const PolygonBase& source); // Copy constructor
	PolygonBase& operator=(const PolygonBase& source); // Assignment operator
	~PolygonBase(void); // Destroys the polygon
	
	/* Methods: */
	int getNumVertices(void) const // Returns number of vertices in the polygon
		{
		return numVertices;
		}
	const Point& getVertex(int vertexIndex) const // Returns one vertex
		{
		return vertices[vertexIndex];
		}
	Point& getVertex(int vertexIndex) // Ditto
		{
		return vertices[vertexIndex];
		}
	void setVertices(int newNumVertices,const Point newVertices[]); // Assigns a new vertex list to the polygon
	Point calcCentroid(void) const; // Returns the polygon's centroid
	template <class TransformationParam>
	void doTransform(const TransformationParam& t) // Transforms the polygon by the given transformation
		{
		/* Transform all vertices: */
		for(int i=0;i<numVertices;++i)
			vertices[i]=t.transform(vertices[i]);
		}
	void doClip(const Plane& plane); // Clips a convex polygon against the given plane; retains part of the polygon behind the plane
	void doSplit(const Plane& plane,PolygonBase& front); // Ditto, but copies part of the polygon in front of the plane to the given front polygon
	};

/*************************
The generic polygon class:
*************************/

template <class ScalarParam,int dimensionParam>
class Polygon:public PolygonBase<ScalarParam,dimensionParam>
	{
	/* Embedded classes: */
	public:
	typedef PolygonBase<ScalarParam,dimensionParam> Base;
	typedef typename Base::Scalar Scalar;
	using Base::dimension;
	typedef typename Base::Point Point;
	typedef typename Base::Vector Vector;
	typedef typename Base::Plane Plane;
	
	private:
	using Base::numVertices;
	using Base::vertices;
	
	/* Constructors and destructors: */
	public:
	Polygon(void) // Constructs empty polygon
		{
		}
	Polygon(int sNumVertices) // Creates a polygon with an uninitialized vertex array
		:Base(sNumVertices)
		{
		}
	Polygon(int sNumVertices,const Point sVertices[]) // Creates a polygon from a C-style array of points
		:Base(sNumVertices,sVertices)
		{
		}
	Polygon(const Polygon& source) // Copy constructor
		:Base(source)
		{
		}
	Polygon& operator=(const Polygon& source) // Assignment operator
		{
		Base::operator=(source);
		return *this;
		}
	
	/* Methods: */
	Vector calcNormal(void) const; // Returns the polygon's (non-normalized) normal vector
	Plane calcPlane(void) const; // Returns the polygon's plane
	template <class TransformationParam>
	Polygon& transform(const TransformationParam& t) // Transforms the polygon by the given transformation
		{
		Base::doTransform(t);
		return *this;
		}
	Polygon& clip(const Plane& plane) // Clips a convex polygon against the given plane; retains part of the polygon behind the plane
		{
		Base::doClip(plane);
		return *this;
		}
	Polygon& split(const Plane& plane,Polygon& front) // Ditto, but copies part of the polygon in front of the plane to the front polygon
		{
		Base::doSplit(plane,front);
		return *this;
		}
	int* calcProjectionAxes(const Vector& polygonNormal,int projectionAxes[2]) const; // Calculates projection axes for later point-in-polygon tests
	bool isInside(const Point& p,const int projectionAxes[2]) const; // Returns true if the polygon contains the point; point is assumed to lie in polygon's plane
	bool isInside(const Point& p,const Vector& polygonNormal) const // Ditto, with given polygon normal
		{
		int projectionAxes[2];
		return isInside(p,calcProjectionAxes(polygonNormal,projectionAxes));
		}
	bool isInside(const Point& p) const // Ditto, with no additional information
		{
		int projectionAxes[2];
		return isInside(p,calcProjectionAxes(calcNormal(),projectionAxes));
		}
	};

/*******************************************
A specialized two-dimensional polygon class:
*******************************************/

template <class ScalarParam>
class Polygon<ScalarParam,2>:public PolygonBase<ScalarParam,2>
	{
	/* Embedded classes: */
	public:
	typedef PolygonBase<ScalarParam,2> Base;
	typedef typename Base::Scalar Scalar;
	using Base::dimension;
	typedef typename Base::Point Point;
	typedef typename Base::Vector Vector;
	typedef typename Base::Plane Plane;
	
	private:
	using Base::numVertices;
	using Base::vertices;
	
	/* Constructors and destructors: */
	public:
	Polygon(void) // Constructs empty polygon
		{
		}
	Polygon(int sNumVertices) // Creates a polygon with an uninitialized vertex array
		:Base(sNumVertices)
		{
		}
	Polygon(int sNumVertices,const Point sVertices[]) // Creates a polygon from a C-style array of points
		:Base(sNumVertices,sVertices)
		{
		}
	Polygon(const Polygon& source) // Copy constructor
		:Base(source)
		{
		}
	Polygon& operator=(const Polygon& source) // Assignment operator
		{
		Base::operator=(source);
		return *this;
		}
	
	/* Methods: */
	template <class TransformationParam>
	Polygon& transform(const TransformationParam& t) // Transforms the polygon by the given transformation
		{
		Base::doTransform(t);
		return *this;
		}
	Polygon& clip(const Plane& plane) // Clips a convex polygon against the given plane; retains part of the polygon behind the plane
		{
		Base::doClip(plane);
		return *this;
		}
	Polygon& split(const Plane& plane,Polygon& front) // Ditto, but copies part of the polygon in front of the plane to the front polygon
		{
		Base::doSplit(plane,front);
		return *this;
		}
	bool isInside(const Point& p) const; // Returns true if the polygon contains the point
	};

}

#if defined(GEOMETRY_NONSTANDARD_TEMPLATES) && !defined(GEOMETRY_POLYGON_IMPLEMENTATION)
#include <Geometry/Polygon.icpp>
#endif

#endif
