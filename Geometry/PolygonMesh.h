/***********************************************************************
PolygonMesh - Base class for polygon meshes represented by a split-edge
data structure.
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

#ifndef GEOMETRY_POLYGONMESH_INCLUDED
#define GEOMETRY_POLYGONMESH_INCLUDED

#include <utility>
#include <Geometry/Vector.h>
#include <Geometry/Point.h>
#include <Geometry/Box.h>

namespace Geometry {

/**************************************************************************
"Private" classes (should be inside PolygonMesh, but compiler won't do it):
**************************************************************************/

/* Forward declarations of "private" classes: */
template <class ScalarParam,int dimensionParam,class VertexDataParam,class EdgeDataParam,class FaceDataParam>
class PolygonMeshVertex;
template <class ScalarParam,int dimensionParam,class VertexDataParam,class EdgeDataParam,class FaceDataParam>
class PolygonMeshEdge;
template <class ScalarParam,int dimensionParam,class VertexDataParam,class EdgeDataParam,class FaceDataParam>
class PolygonMeshFace;

template <class ScalarParam,int dimensionParam,class VertexDataParam,class EdgeDataParam,class FaceDataParam>
class PolygonMeshBaseVertex
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar;
	static const int dimension=dimensionParam;
	typedef Geometry::Point<ScalarParam,dimensionParam> Point;
	typedef VertexDataParam VertexData;
	typedef EdgeDataParam EdgeData;
	typedef FaceDataParam FaceData;
	typedef PolygonMeshVertex<ScalarParam,dimensionParam,VertexDataParam,EdgeDataParam,FaceDataParam> Vertex;
	typedef PolygonMeshEdge<ScalarParam,dimensionParam,VertexDataParam,EdgeDataParam,FaceDataParam> Edge;
	
	/* Elements: */
	Point position; // Position of vertex
	Edge* edge; // Pointer to any edge starting at vertex
	
	/* Methods: */
	const Point& getPosition(void) const // Returns vertex position
		{
		return position;
		}
	Point& getPosition(void) // Ditto
		{
		return position;
		}
	const Edge* getEdge(void) const // Returns any edge starting at vertex
		{
		return edge;
		}
	Edge* getEdge(void) // Ditto
		{
		return edge;
		}
	bool isOnBoundary(void) const; // Returns true if vertex is on mesh's boundary
	int calcNumEdges(void) const; // Calculates number of edges starting at vertex
	};

template <class ScalarParam,int dimensionParam,class VertexDataParam,class EdgeDataParam,class FaceDataParam>
class PolygonMeshVertex:public PolygonMeshBaseVertex<ScalarParam,dimensionParam,VertexDataParam,EdgeDataParam,FaceDataParam>,public VertexDataParam
	{
	/* Embedded classes: */
	public:
	typedef VertexDataParam Data;
	};

template <class ScalarParam,int dimensionParam,class EdgeDataParam,class FaceDataParam>
class PolygonMeshVertex<ScalarParam,dimensionParam,void,EdgeDataParam,FaceDataParam>:public PolygonMeshBaseVertex<ScalarParam,dimensionParam,void,EdgeDataParam,FaceDataParam>
	{
	};

template <class ScalarParam,int dimensionParam,class VertexDataParam,class EdgeDataParam,class FaceDataParam>
class PolygonMeshBaseEdge
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar;
	static const int dimension=dimensionParam;
	typedef VertexDataParam VertexData;
	typedef EdgeDataParam EdgeData;
	typedef FaceDataParam FaceData;
	typedef PolygonMeshVertex<ScalarParam,dimensionParam,VertexDataParam,EdgeDataParam,FaceDataParam> Vertex;
	typedef PolygonMeshEdge<ScalarParam,dimensionParam,VertexDataParam,EdgeDataParam,FaceDataParam> Edge;
	typedef PolygonMeshFace<ScalarParam,dimensionParam,VertexDataParam,EdgeDataParam,FaceDataParam> Face;
	
	/* Elements: */
	Vertex* start; // Pointer to start vertex of edge
	Edge* faceSucc; // Pointer to next edge counter-clockwise around face
	Edge* opposite; // Pointer to opposite part of same edge
	Face* face; // Pointer to face edge belongs to
	
	/* Methods: */
	const Vertex* getStart(void) const // Returns start vertex of edge
		{
		return start;
		}
	Vertex* getStart(void) // Ditto
		{
		return start;
		}
	const Vertex* getEnd(void) const // Returns end vertex of edge
		{
		return faceSucc->start;
		}
	Vertex* getEnd(void) // Ditto
		{
		return faceSucc->start;
		}
	const Edge* getFaceSucc(void) const // Returns next edge counter-clockwise around face
		{
		return faceSucc;
		}
	Edge* getFaceSucc(void) // Ditto
		{
		return faceSucc;
		}
	const Edge* getFacePred(void) const; // Returns next edge clockwise around face
	Edge* getFacePred(void); // Ditto
	const Edge* getOpposite(void) const // Returns opposite part of same edge
		{
		return opposite;
		}
	Edge* getOpposite(void) // Ditto
		{
		return opposite;
		}
	const Edge* getVertexPred(void) const // Returns next edge clockwise around start vertex
		{
		return opposite!=0?opposite->faceSucc:0;
		}
	Edge* getVertexPred(void) // Ditto
		{
		return opposite!=0?opposite->faceSucc:0;
		}
	const Edge* getVertexSucc(void) const // Returns next edge counter-clockwise around start vertex
		{
		return getFacePred()->opposite;
		}
	Edge* getVertexSucc(void) // Ditto
		{
		return getFacePred()->opposite;
		}
	const Face* getFace(void) const // Returns face edge belongs to
		{
		return face;
		}
	Face* getFace(void) // Ditto
		{
		return face;
		}
	bool isOnBoundary(void) const; // Returns true if edge is on mesh's boundary
	};

template <class ScalarParam,int dimensionParam,class VertexDataParam,class EdgeDataParam,class FaceDataParam>
class PolygonMeshEdge:public PolygonMeshBaseEdge<ScalarParam,dimensionParam,VertexDataParam,EdgeDataParam,FaceDataParam>,public EdgeDataParam
	{
	/* Embedded classes: */
	public:
	typedef EdgeDataParam Data;
	};

template <class ScalarParam,int dimensionParam,class VertexDataParam,class FaceDataParam>
class PolygonMeshEdge<ScalarParam,dimensionParam,VertexDataParam,void,FaceDataParam>:public PolygonMeshBaseEdge<ScalarParam,dimensionParam,VertexDataParam,void,FaceDataParam>
	{
	};

template <class ScalarParam,int dimensionParam,class VertexDataParam,class EdgeDataParam,class FaceDataParam>
class PolygonMeshBaseFace
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar;
	static const int dimension=dimensionParam;
	typedef Geometry::Vector<ScalarParam,dimensionParam> Vector;
	typedef Geometry::Point<ScalarParam,dimensionParam> Point;
	typedef VertexDataParam VertexData;
	typedef EdgeDataParam EdgeData;
	typedef FaceDataParam FaceData;
	typedef PolygonMeshVertex<ScalarParam,dimensionParam,VertexDataParam,EdgeDataParam,FaceDataParam> Vertex;
	typedef PolygonMeshEdge<ScalarParam,dimensionParam,VertexDataParam,EdgeDataParam,FaceDataParam> Edge;
	typedef PolygonMeshFace<ScalarParam,dimensionParam,VertexDataParam,EdgeDataParam,FaceDataParam> Face;
	typedef std::pair<Vector,Scalar> PlaneEquation;
	
	/* Elements: */
	Edge* edge; // Pointer to any edge belonging to face
	
	/* Methods: */
	const Edge* getEdge(void) const // Returns any edge belonging to face
		{
		return edge;
		}
	Edge* getEdge(void) // Ditto
		{
		return edge;
		}
	int calcNumVertices(void) const; // Returns the number of vertices in face
	Point calcCentroid(void) const; // Returns the centroid of face
	Vector calcNormal(void) const; // Returns normal vector of face
	PlaneEquation calcPlaneEquation(void) const; // Returns normal vector and origin offset of face plane
	};

template <class ScalarParam,int dimensionParam,class VertexDataParam,class EdgeDataParam,class FaceDataParam>
class PolygonMeshFace:public PolygonMeshBaseFace<ScalarParam,dimensionParam,VertexDataParam,EdgeDataParam,FaceDataParam>,public FaceDataParam
	{
	/* Embedded classes: */
	public:
	typedef FaceDataParam Data;
	};

template <class ScalarParam,int dimensionParam,class VertexDataParam,class EdgeDataParam>
class PolygonMeshFace<ScalarParam,dimensionParam,VertexDataParam,EdgeDataParam,void>:public PolygonMeshBaseFace<ScalarParam,dimensionParam,VertexDataParam,EdgeDataParam,void>
	{
	};

template <class ScalarParam,int dimensionParam,class VertexDataParam,class EdgeDataParam,class FaceDataParam>
class PolygonMesh
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar;
	static const int dimension=dimensionParam;
	typedef Geometry::Point<ScalarParam,dimensionParam> Point;
	typedef Geometry::Box<ScalarParam,dimensionParam> Box;
	typedef VertexDataParam VertexData;
	typedef EdgeDataParam EdgeData;
	typedef FaceDataParam FaceData;
	typedef PolygonMeshVertex<ScalarParam,dimensionParam,VertexDataParam,EdgeDataParam,FaceDataParam> Vertex;
	typedef PolygonMeshEdge<ScalarParam,dimensionParam,VertexDataParam,EdgeDataParam,FaceDataParam> Edge;
	typedef PolygonMeshFace<ScalarParam,dimensionParam,VertexDataParam,EdgeDataParam,FaceDataParam> Face;
	
	/* Elements: */
	int numVertices; // Number of vertices in mesh
	Vertex* vertices; // Array of vertices
	int numEdges; // Number of edges in mesh
	Edge* edges; // Array of edges
	int numFaces; // Number of faces in mesh
	Face* faces; // Array of faces
	
	/* Constructors and destructors: */
	PolygonMesh(void) // Constructs "empty" polygon mesh
		:numVertices(0),vertices(0),numEdges(0),edges(0),numFaces(0),faces(0)
		{
		}
	PolygonMesh(const Point sVertices[],const int faceVertexIndices[]) // Constructs mesh from array of points and array of face vertex indices (each face terminated by -1, face list terminated by -1)
		:numVertices(0),vertices(0),numEdges(0),edges(0),numFaces(0),faces(0)
		{
		set(sVertices,faceVertexIndices);
		}
	~PolygonMesh(void); // Destroys mesh
	
	/* Methods: */
	void set(const Point sVertices[],const int faceVertexIndices[]); // Creates mesh from array of points and array of face vertex indices (each face terminated by -1, face list terminated by -1)
	int getNumVertices(void) const // Returns number of vertices
		{
		return numVertices;
		}
	const Vertex& getVertex(int index) const // Returns one vertex
		{
		return vertices[index];
		}
	Vertex& getVertex(int index) // Ditto
		{
		return vertices[index];
		}
	int getNumEdges(void) const // Returns number of edges
		{
		return numEdges;
		}
	const Edge& getEdge(int index) const // Returns one edge
		{
		return edges[index];
		}
	Edge& getEdge(int index) // Ditto
		{
		return edges[index];
		}
	int getNumFaces(void) const // Returns number of faces
		{
		return numFaces;
		}
	const Face& getFace(int index) const // Returns one face
		{
		return faces[index];
		}
	Face& getFace(int index) // Ditto
		{
		return faces[index];
		}
	Box calcBoundingBox(void) const; // Returns bounding box of mesh
	};

}

#if !defined(GEOMETRY_POLYGONMESH_IMPLEMENTATION)
#include <Geometry/PolygonMesh.icpp>
#endif

#endif
