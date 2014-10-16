/***********************************************************************
PolygonMesh - Class to represent meshes of planar convex polygons. Meant
as a temporary helper structure to convert polygon soup into an
efficiently rendered representation, and compute a full set of vertex
attributes.
Copyright (c) 2009-2013 Oliver Kreylos

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

#ifndef SCENEGRAPH_POLYGONMESH_INCLUDED
#define SCENEGRAPH_POLYGONMESH_INCLUDED

#include <vector>
#include <Misc/OrderedTuple.h>
#include <Misc/UnorderedTuple.h>
#include <Misc/HashTable.h>

/* Forward declarations: */
namespace SceneGraph {
template <class MeshVertexParam>
class TexCoordCalculator;
}

namespace SceneGraph {

template <class MeshVertexParam>
class PolygonMesh
	{
	/* Embedded classes: */
	public:
	typedef MeshVertexParam MeshVertex; // Type for mesh vertices
	typedef typename MeshVertex::Scalar Scalar; // Scalar type for points and vectors
	typedef typename MeshVertex::Point Point; // Type for points
	typedef typename MeshVertex::Vector Vector; // Type for vectors
	typedef typename MeshVertex::TPoint TPoint; // Type for points in texture space
	typedef unsigned int Card; // Cardinal integer type
	static const Card invalidIndex; // Invalid Cardinal integer
	typedef Misc::OrderedTuple<Card,2> DirectedEdge; // Type to represent a directed edge as a pair of vertex indices
	typedef Misc::UnorderedTuple<Card,2> UndirectedEdge; // Type to represent an undirected edge as a pair of vertex indices
	typedef Misc::OrderedTuple<Card,2> FaceVertex; // Type to represent a (face index, vertex index) pair to store per-face vertex attributes
	
	private:
	struct Face // Structure for polygon mesh faces
		{
		/* Elements: */
		public:
		Card numVertices; // Number of vertices in this face
		Card firstVertexIndex; // Index of face's first vertex in face vertex index list
		Card surfaceIndex; // Index of the "surface" (collection of connected faces) to which this face belongs
		unsigned int smoothingGroupMask; // Bit mask of smoothing groups to which this face belongs
		bool convex; // Flag if the face is convex
		Vector normal; // Face normal vector
		};
	
	struct FaceEdge // Structure for half-edges around polygon mesh faces
		{
		/* Elements: */
		Card faceIndex; // Index of face to which this half-edge belongs
		Card previousVertexIndex; // Index of face vertex preceding half-edge in face order
		};
	
	typedef Misc::HashTable<DirectedEdge,FaceEdge,DirectedEdge> FaceEdgeHasher; // Hash table type to map directed edges to face edges
	typedef Misc::HashTable<FaceVertex,TPoint,FaceVertex> FaceVertexTexCoordHasher; // Hash table type to map face vertices to per-face vertex texture coordinates
	typedef Misc::HashTable<UndirectedEdge,void,UndirectedEdge> CreaseEdgeHasher; // Hash table type to represent sets of crease edges
	typedef Misc::HashTable<FaceVertex,Vector,FaceVertex> FaceVertexNormalHasher; // Hash table type to map face vertices to per-face vertex normals
	
	/* Elements: */
	private:
	std::vector<MeshVertex> vertices; // List of mesh vertices
	std::vector<Card> vertexEdges; // List of one outgoing directed edge for each mesh vertex
	std::vector<bool> vertexMultiSurfaceFlags; // List of flags indicating per-face texture coordinates for each mesh vertex
	std::vector<bool> vertexCreaseFlags; // List of crease flags indicating per-face normal vectors for each mesh vertex
	std::vector<Card> faceVertexIndices; // List of vertex indices for each mesh face
	std::vector<Face> faces; // List of mesh faces
	Card numSurfaces; // Number of different surfaces present in the mesh
	FaceEdgeHasher faceEdges; // Hash table of face edges
	FaceVertexTexCoordHasher vertexTexCoords; // Hash table storing per-face vertex texture coordinates for multi-surface vertices
	CreaseEdgeHasher creaseEdges; // Hash table of crease edges
	FaceVertexNormalHasher vertexNormals; // Hash table storing per-face normal vectors for crease vertices
	
	/* Temporary state while adding a face: */
	bool addingFace; // Flag that we're currently adding a face
	Card newNumVertices; // Current number of vertices in new face
	Card newFirstVertexIndex; // Index of new face's first vertex index
	
	/* Private methods: */
	void connectFace(Card faceIndex); // Inserts a face into the mesh's connectivity
	void triangulateFace(Card faceIndex,std::vector<Card>& triangleVertexIndices) const; // Triangulates the given convex or non-convex face; puts triangle vertex index triples into vector
	const TPoint& getVertexTexCoord(Card faceIndex,Card vertexIndex) const; // Returns texture coordinates of a vertex for a given face
	const Vector& getVertexNormal(Card faceIndex,Card vertexIndex) const; // Returns normal vector of a vertex for a given face
	Card getTriangleVertexIndex(Card faceIndex,Card vertexIndex) const;
	
	/* Constructors and destructors: */
	public:
	PolygonMesh(void); // Creates empty polygon mesh
	~PolygonMesh(void); // Destroys polygon mesh
	
	/* New Methods: */
	Card getNumVertices(void) const // Returns the current number of vertices in the mesh
		{
		return vertices.size();
		}
	const MeshVertex& getVertex(Card vertexIndex) const // Returns a vertex
		{
		return vertices[vertexIndex];
		}
	MeshVertex& getVertex(Card vertexIndex) // Ditto
		{
		return vertices[vertexIndex];
		}
	Card addVertex(const MeshVertex& newVertex); // Adds a new vertex to the mesh and returns its index
	Card getNumFaces(void) const // Returns the current number of faces in the mesh
		{
		return faces.size();
		}
	void startFace(void); // Starts adding a new face incrementally
	void addFaceVertex(Card vertexIndex); // Adds a new vertex to the new face
	Card finishFace(void); // Finishes adding a new face to the given surface and returns its index
	Card addFace(Card newNumVertices,const Card newVertexIndices[]); // Adds new face from array of vertex indices to given surface
	Card addFace(const std::vector<Card>& newVertexIndices); // Ditto
	void setFaceSurface(Card faceIndex,Card surfaceIndex); // Sets a face's surface index to the given value
	void setFaceSmoothingGroupMask(Card faceIndex,unsigned int newSmoothingGroupMask); // Sets a face's smoothing group mask
	void setFaceNormal(Card faceIndex,const Vector& newNormal); // Sets a face's normal vector
	void setFaceVertexNormal(Card faceIndex,Card vertexIndex,const Vector& newNormal); // Sets a vertex' per-face normal vector
	void calcVertexTexCoords(const std::vector<const TexCoordCalculator<MeshVertex>*>& texCoordCalculators); // Calculates texture coordinates for all vertices using an array of per-surface texture coordinate calculators
	void addCreaseEdge(Card vertexIndex0,Card vertexIndex1); // Marks the edge connecting the two given vertices as a crease edge
	void findSmoothingGroupCreaseEdges(void); // Marks all edges as crease edges whose faces do not belong to the same smoothing group
	void findCreaseEdges(Scalar creaseAngle); // Marks all edges as crease edges whose faces form an angle of more than creaseAngle (given in radians)
	void findCreaseEdges(Card surfaceIndex,Scalar creaseAngle); // Marks all edges as crease edges whose faces belong to the given surface and form an angle of more than creaseAngle (given in radians)
	void findCreaseEdges(const std::vector<Scalar>& creaseAngles); // Marks all edges according to surface-specific crease angles
	void findSurfaceCreaseEdges(void); // Marks all edges between faces belonging to different surfaces as crease edges
	void calcVertexNormals(void); // Calculates vertex normals for all vertices, respecting previously marked crease edges
	void triangulate(std::vector<MeshVertex>& vertices) const; // Triangulates all faces of the polygon mesh and adds them to the given triangle set
	void triangulate(std::vector<MeshVertex>& vertices,std::vector<Card>& indices) const; // Triangulates all faces of the polygon mesh and adds them to the given indexed triangle set
	void triangulateSurface(Card surfaceIndex,std::vector<MeshVertex>& vertices) const; // Triangulates all faces of the polygon mesh belonging to the given surface and adds them to the given triangle set
	void triangulateSurface(Card surfaceIndex,std::vector<MeshVertex>& vertices,std::vector<Card>& indices) const; // Triangulates all faces of the polygon mesh belonging to the given surface and adds them to the given indexed triangle set
	};

}

#ifndef SCENEGRAPH_POLYGONMESH_IMPLEMENTATION
#include <SceneGraph/PolygonMesh.icpp>
#endif

#endif
