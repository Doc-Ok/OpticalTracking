/***********************************************************************
AutoTriangleMesh - Class for triangular meshes that enforce triangle
shape constraints under mesh transformations.
Copyright (c) 2003-2005 Oliver Kreylos
***********************************************************************/

#ifndef AUTOTRIANGLEMESH_INCLUDED
#define AUTOTRIANGLEMESH_INCLUDED

#include "PolygonMesh.h"

class AutoTriangleMesh:public PolygonMesh
	{
	/* Embedded classes: */
	public:
	typedef PolygonMesh BaseMesh; // Base class type
	
	/* Protected methods: */
	protected:
	void triangulateAllFaces(void); // Converts polygon mesh to triangle mesh
	void createVertexIndices(void); // Assigns indices and version numbers to all vertices
	bool canCollapseEdge(const Edge* edge) const; // Tests if an edge can be collapsed
	
	/* Constructors and destructors: */
	public:
	AutoTriangleMesh(void) // Creates empty mesh
		{
		};
	AutoTriangleMesh(const BaseMesh& source); // Copies a polygon mesh and converts it into an automatic triangle mesh
	AutoTriangleMesh(const AutoTriangleMesh& source) // Copies an automatic triangle mesh
		:BaseMesh(source)
		{
		/* Create vertex indices and reset vertex versions: */
		createVertexIndices();
		};
	AutoTriangleMesh& operator=(const BaseMesh& source); // Assigns a polygon mesh and triangulates it
	
	/* Methods: */
	FaceIterator addFace(int numVertices,const VertexIterator vertices[],EdgeHasher* edgeHasher);
	FaceIterator addFace(const std::vector<VertexIterator>& vertices,EdgeHasher* edgeHasher);
	void splitEdge(const EdgeIterator& edge); // Splits an edge at its midpoint
	bool canCollapseEdge(const ConstEdgeIterator& edge) const // Tests if an edge can be collapsed
		{
		return canCollapseEdge(&(*edge));
		};
	bool canCollapseEdge(const EdgeIterator& edge) const // Ditto
		{
		return canCollapseEdge(&(*edge));
		};
	bool collapseEdge(const EdgeIterator& edge); // Collapses an edge to its midpoint; returns false if edge is not collapsible
	void limitEdgeLength(const Point& center,Scalar radius,Scalar maxEdgeLength);
	void ensureEdgeLength(const Point& center,Scalar radius,Scalar minEdgeLength);
	};

#endif
