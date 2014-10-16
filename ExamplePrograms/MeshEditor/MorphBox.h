/***********************************************************************
MorphBox - Data structure to embed polygon meshes into upright boxes
that can be subsequently deformed to morph the embedded mesh.
Copyright (c) 2004 Oliver Kreylos
***********************************************************************/

#ifndef MORPHBOX_INCLUDED
#define MORPHBOX_INCLUDED

#include <vector>
#include <Geometry/OrthogonalTransformation.h>

#include "AutoTriangleMesh.h"

/* Forward declarations: */
class GLContextData;

class MorphBox
	{
	/* Embedded classes: */
	public:
	typedef AutoTriangleMesh Mesh; // Data type for meshes
	typedef Mesh::Scalar Scalar;
	typedef Mesh::Point Point;
	typedef Mesh::Vector Vector;
	typedef Geometry::OrthogonalTransformation<Scalar,3> OGTransform;
	private:
	typedef Mesh::Vertex MVertex;
	typedef Mesh::VertexIterator MVertexIterator;
	
	struct MorphVertex // Data structure for morphed vertices
		{
		/* Elements: */
		public:
		MVertex* v; // Pointer to the morphed vertex
		Scalar boxCoords[3]; // Box coordinates of the morphed vertex
		};
	
	/* Elements: */
	Mesh* mesh; // Mesh this morph box is manipulating
	Point boxVertices[8]; // Array of current positions of box vertices
	std::vector<MorphVertex> morphedVertices; // List of morphed mesh vertices
	int numDraggedVertices; // Number of dragged box vertices
	int draggedVertexIndices[8]; // Indices of dragged box vertices
	Point draggedVertices[8]; // Initial positions of dragged box vertices
	
	/* Constructors and destructors: */
	public:
	MorphBox(Mesh* sMesh,const Point& origin,const Scalar size[3]); // Creates an upright morph box
	
	/* Methods: */
	bool pickBox(Scalar vertexDist,Scalar edgeDist,Scalar faceDist,const Point& pickPoint); // Picks the morph box using a point; returns true if pick successful
	void startDragBox(const OGTransform& startTransformation); // Starts dragging the morph box
	void dragBox(const OGTransform& currentTransformation); // Drags the morph box and applies morph to morphed mesh vertices
	void stopDragBox(void); // Stops dragging the morph box
	void glRenderAction(const GLContextData& contextData) const;
	};

#endif
