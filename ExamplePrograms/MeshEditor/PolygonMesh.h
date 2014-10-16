/***********************************************************************
PolygonMesh - Class providing the infrastructure for algorithms working
on meshes of convex polygons
Copyright (c) 2001-2005 Oliver Kreylos
***********************************************************************/

#ifndef POLYGONMESH_INCLUDED
#define POLYGONMESH_INCLUDED

#include <vector>
#include <Misc/PoolAllocator.h>
#include <Misc/HashTable.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <GL/gl.h>
#include <GL/GLColor.h>

class PolygonMesh
	{
	/* Embedded classes: */
	public:
	typedef float Scalar;
	static const int dimension=3;
	typedef Geometry::Point<Scalar,dimension> Point;
	typedef Geometry::Vector<Scalar,dimension> Vector;
	typedef GLColor<GLubyte,4> Color;
	
	/* Forward declarations: */
	class Vertex;
	class Edge;
	class Face;
	class EdgeIterator;
	class ConstEdgeIterator;
	class VertexIterator;
	class ConstVertexIterator;
	class VertexEdgeIterator;
	class ConstVertexEdgeIterator;
	class FaceIterator;
	class ConstFaceIterator;
	class FaceEdgeIterator;
	class ConstFaceEdgeIterator;
	
	class Vertex:public Point // Basic data structure for representing mesh vertices
		{
		friend class VertexIterator;
		friend class ConstVertexIterator;
		friend class PolygonMesh;
		
		/* Elements: */
		public:
		unsigned int version; // If this is equal to mesh's version number, vertex was updated
		Color color; // Vertex color
		Vector normal; // Vertex normal vector
		private:
		Edge* edge; // Pointer to one half-edge starting at the vertex
		/* Pointers to form a double-linked list: */
		Vertex* pred;
		Vertex* succ;
		
		/* Constructors and destructors: */
		Vertex(const Point& sPoint,const Color& sColor,Vertex* sSucc) // Constructs vertex from a point and a color
			:Point(sPoint),color(sColor),
			 edge(0),pred(0),succ(sSucc)
			{
			};
		
		/* Methods: */
		public:
		void setPoint(const Point& sPoint)
			{
			Point::operator=(sPoint);
			};
		const Edge* getEdge(void) const
			{
			return edge;
			};
		Edge* getEdge(void)
			{
			return edge;
			};
		void setEdge(Edge* sEdge)
			{
			edge=sEdge;
			};
		int getNumEdges(void) const;
		Vertex* getSucc(void)
			{
			return succ;
			};
		bool isInterior(void) const; // Checks if the vertex is completely surrounded by faces
		void checkVertex(void) const;
		};
	
	class VertexCombiner // Class to calculate affine combinations of mesh vertices
		{
		/* Elements: */
		private:
		float pointSum[3]; // Non-normalized point components
		float colorSum[4]; // Non-normalized color components
		float weightSum; // Sum of affine weights used in combination
		
		/* Constructors and destructors: */
		public:
		VertexCombiner(void)
			:weightSum(0.0f)
			{
			for(int i=0;i<3;++i)
				pointSum[i]=0.0f;
			for(int i=0;i<4;++i)
				colorSum[i]=0.0f;
			};
		
		/* Methods: */
		Point getPoint(void) const // Returns the created point
			{
			Point result;
			for(int i=0;i<3;++i)
				result[i]=pointSum[i]/weightSum;
			return result;
			};
		Color getColor(void) const // Returns the created color
			{
			Color result;
			for(int i=0;i<4;++i)
				result[i]=GLubyte(Math::floor(colorSum[i]/weightSum+0.5f));
			return result;
			};
		VertexCombiner& reset(void) // Resets the combiner to "empty"
			{
			for(int i=0;i<3;++i)
				pointSum[i]=0.0f;
			for(int i=0;i<4;++i)
				colorSum[i]=0.0f;
			weightSum=0.0f;
			return *this;
			};
		VertexCombiner& addVertex(const Vertex* vPtr) // Adds vertex with affine weight 1
			{
			for(int i=0;i<3;++i)
				pointSum[i]+=(*vPtr)[i];
			for(int i=0;i<4;++i)
				colorSum[i]+=float(vPtr->color[i]);
			weightSum+=1.0f;
			return *this;
			};
		VertexCombiner& addVertex(const Vertex* vPtr,float weight) // Adds vertex with given affine weight
			{
			for(int i=0;i<3;++i)
				pointSum[i]+=(*vPtr)[i]*weight;
			for(int i=0;i<4;++i)
				colorSum[i]+=float(vPtr->color[i])*weight;
			weightSum+=weight;
			return *this;
			};
		};
	
	class Edge // Basic data structure for representing vertices, edges and polygons and their adjacency
		{
		/* Elements: */
		private:
		Vertex* start; // Pointer to start vertex of half-edge
		Face* face; // Pointer to face
		Edge* facePred; // Pointer to next half-edge in clockwise order around a face
		Edge* faceSucc; // Pointer to next half-edge in counter-clockwise order around a face
		Edge* opposite; // Pointer to opposite half-edge in adjacent polygon
		public:
		int sharpness; // Sharpness coefficient of edge for Catmull-Clark subdivision
		Vertex* edgePoint; // Pointer to edge point for Catmull-Clark subdivision
		
		/* Access methods: */
		public:
		const Vertex* getStart(void) const
			{
			return start;
			};
		Vertex* getStart(void)
			{
			return start;
			};
		const Vertex* getEnd(void) const // Returns end point of half-edge
			{
			return faceSucc->start;
			};
		Vertex* getEnd(void) // Ditto
			{
			return faceSucc->start;
			};
		const Face* getFace(void) const
			{
			return face;
			};
		Face* getFace(void)
			{
			return face;
			};
		const Edge* getFacePred(void) const
			{
			return facePred;
			};
		Edge* getFacePred(void)
			{
			return facePred;
			};
		const Edge* getFaceSucc(void) const
			{
			return faceSucc;
			};
		Edge* getFaceSucc(void)
			{
			return faceSucc;
			};
		const Edge* getVertexPred(void) const // Returns next half-edge around vertex in clockwise order, 0 if doesn't exist
			{
			return opposite!=0?opposite->faceSucc:0;
			};
		Edge* getVertexPred(void) // Ditto
			{
			return opposite!=0?opposite->faceSucc:0;
			};
		const Edge* getVertexSucc(void) const // Returns next half-edge around vertex in counter-clockwise order, 0 if doesn't exist
			{
			return facePred->opposite;
			};
		Edge* getVertexSucc(void) // Ditto
			{
			return facePred->opposite;
			};
		const Edge* getEndVertexPred(void) const // Returns next half-edge around edge end vertex in clockwise order, 0 if doesn't exist
			{
			return faceSucc->opposite;
			};
		Edge* getEndVertexPred(void) // Ditto
			{
			return faceSucc->opposite;
			};
		const Edge* getEndVertexSucc(void) const // Returns next half-edge around edge end vertex in counter-clockwise order, 0 if doesn't exist
			{
			return opposite!=0?opposite->facePred:0;
			};
		Edge* getEndVertexSucc(void) // Ditto
			{
			return opposite!=0?opposite->facePred:0;
			};
		const Edge* getOpposite(void) const
			{
			return opposite;
			};
		Edge* getOpposite(void)
			{
			return opposite;
			};
		void set(Vertex* sStart,Face* sFace,Edge* sFacePred,Edge* sFaceSucc,Edge* sOpposite)
			{
			start=sStart;
			face=sFace;
			facePred=sFacePred;
			faceSucc=sFaceSucc;
			opposite=sOpposite;
			};
		void setStart(Vertex* sStart)
			{
			start=sStart;
			};
		void setFace(Face* sFace)
			{
			face=sFace;
			};
		void setFacePred(Edge* sFacePred)
			{
			facePred=sFacePred;
			};
		void setFaceSucc(Edge* sFaceSucc)
			{
			faceSucc=sFaceSucc;
			};
		void setOpposite(Edge* sOpposite)
			{
			opposite=sOpposite;
			};
		};
	
	class VertexPair // Helper structure for creating polygon meshes
		{
		friend class PolygonMesh;
		
		/* Elements: */
		private:
		const Vertex* vertices[2]; // Pointers to the two vertices (ordered by size_t order)
		
		/* Constructors and destructors: */
		public:
		VertexPair(void)
			{
			};
		VertexPair(const Vertex* sVertex1,const Vertex* sVertex2)
			{
			if(reinterpret_cast<size_t>(sVertex1)<reinterpret_cast<size_t>(sVertex2))
				{
				vertices[0]=sVertex1;
				vertices[1]=sVertex2;
				}
			else
				{
				vertices[0]=sVertex2;
				vertices[1]=sVertex1;
				}
			};
		VertexPair(const Edge& edge)
			{
			const Vertex* v1=edge.getStart();
			const Vertex* v2=edge.getEnd();
			if(reinterpret_cast<size_t>(v1)<reinterpret_cast<size_t>(v2))
				{
				vertices[0]=v1;
				vertices[1]=v2;
				}
			else
				{
				vertices[0]=v2;
				vertices[1]=v1;
				}
			};
		
		/* Methods: */
		friend bool operator==(const VertexPair& p1,const VertexPair& p2)
			{
			return p1.vertices[0]==p2.vertices[0]&&p1.vertices[1]==p2.vertices[1];
			};
		friend bool operator!=(const VertexPair& p1,const VertexPair& p2)
			{
			return p1.vertices[0]!=p2.vertices[0]||p1.vertices[1]!=p2.vertices[1];
			};
		static size_t hash(const VertexPair& p,size_t tableSize)
			{
			size_t val1=reinterpret_cast<size_t>(p.vertices[0]);
			size_t val2=reinterpret_cast<size_t>(p.vertices[1]);
			return (val1*17+val2*31)%tableSize;
			};
		};
	
	class Face // Basic data structure for representing faces
		{
		friend class FaceIterator;
		friend class ConstFaceIterator;
		friend class PolygonMesh;
		
		/* Elements: */
		private:
		Edge* edge; // Pointer to any one of the polygon's edges
		/* Pointers to form a double-linked list: */
		Face* pred;
		Face* succ;
		public:
		Vertex* facePoint; // Pointer to face point for Catmull-Clark subdivision
		mutable bool visited; // Flag to mark faces as visited during triangle strip generation
		
		/* Constructors and destructors: */
		Face(Face* sSucc)
			:edge(0),pred(0),succ(sSucc)
			{
			};
		
		/* Methods: */
		public:
		const Edge* getEdge(void) const
			{
			return edge;
			};
		Edge* getEdge(void)
			{
			return edge;
			};
		void setEdge(Edge* sEdge)
			{
			edge=sEdge;
			};
		int getNumEdges(void) const;
		void checkFace(void) const;
		};
	
	class EdgeIterator // Iterator for edges
		{
		friend class PolygonMesh;
		friend class ConstEdgeIterator;
		
		/* Elements: */
		private:
		Edge* loopStart; // First edge in loop around vertex/face
		Edge* edge; // Edge pointed to
		
		/* Constructors and destructors: */
		public:
		EdgeIterator(void) // Constructs invalid operator
			:loopStart(0),edge(0)
			{
			};
		EdgeIterator(Edge* sEdge)
			:loopStart(sEdge),edge(sEdge)
			{
			};
		
		/* Methods: */
		friend bool operator==(const EdgeIterator& it1,const EdgeIterator& it2)
			{
			return it1.edge==it2.edge;
			};
		friend bool operator!=(const EdgeIterator& it1,const EdgeIterator& it2)
			{
			return it1.edge!=it2.edge;
			};
		Edge& operator*(void) const
			{
			return *edge;
			};
		Edge* operator->(void) const
			{
			return edge;
			};
		bool isUpperHalf(void) const // Checks if an edge iterator is the "dominant" part of a pair of half edges
			{
			return reinterpret_cast<size_t>(edge)>reinterpret_cast<size_t>(edge->getOpposite());
			};
		VertexPair getVertexPair(void) const
			{
			return VertexPair(*edge);
			};
		EdgeIterator& advanceFace(void)
			{
			if((edge=edge->getFaceSucc())==loopStart) // Did we walk around a face once?
				edge=0; // Mark iterator as invalid
			return *this;
			};
		EdgeIterator& advanceVertex(void)
			{
			if((edge=edge->getVertexSucc())==loopStart) // Did we walk around a vertex once?
				edge=0; // Mark iterator as invalid
			return *this;
			};
		};
	
	class ConstEdgeIterator // Iterator for edges
		{
		friend class PolygonMesh;
		
		/* Elements: */
		private:
		const Edge* loopStart; // First edge in loop around vertex/face
		const Edge* edge; // Edge pointed to
		
		/* Constructors and destructors: */
		public:
		ConstEdgeIterator(void) // Constructs invalid operator
			:loopStart(0),edge(0)
			{
			};
		ConstEdgeIterator(const Edge* sEdge)
			:loopStart(sEdge),edge(sEdge)
			{
			};
		ConstEdgeIterator(const EdgeIterator& eIt)
			:loopStart(eIt.loopStart),edge(eIt.edge)
			{
			};
		
		/* Methods: */
		friend bool operator==(const ConstEdgeIterator& it1,const ConstEdgeIterator& it2)
			{
			return it1.edge==it2.edge;
			};
		friend bool operator!=(const ConstEdgeIterator& it1,const ConstEdgeIterator& it2)
			{
			return it1.edge!=it2.edge;
			};
		const Edge& operator*(void) const
			{
			return *edge;
			};
		const Edge* operator->(void) const
			{
			return edge;
			};
		bool isUpperHalf(void) const // Checks if an edge iterator is the "dominant" part of a pair of half edges
			{
			return reinterpret_cast<size_t>(edge)>reinterpret_cast<size_t>(edge->getOpposite());
			};
		VertexPair getVertexPair(void) const
			{
			return VertexPair(*edge);
			};
		ConstEdgeIterator& advanceFace(void)
			{
			if((edge=edge->getFaceSucc())==loopStart) // Did we walk around a face once?
				edge=0; // Mark iterator as invalid
			return *this;
			};
		ConstEdgeIterator& advanceVertex(void)
			{
			if((edge=edge->getVertexSucc())==loopStart) // Did we walk around a vertex once?
				edge=0; // Mark iterator as invalid
			return *this;
			};
		};
	
	class VertexIterator // Iterator for vertices
		{
		friend class PolygonMesh;
		friend class ConstVertexIterator;
		
		/* Elements: */
		private:
		Vertex* vertex; // Vertex pointed to
		
		/* Constructors and destructors: */
		public:
		VertexIterator(void) // Constructs invalid iterator
			:vertex(0)
			{
			};
		VertexIterator(Vertex* sVertex)
			:vertex(sVertex)
			{
			};
		
		/* Methods: */
		friend bool operator==(const VertexIterator& it1,const VertexIterator& it2)
			{
			return it1.vertex==it2.vertex;
			};
		friend bool operator!=(const VertexIterator& it1,const VertexIterator& it2)
			{
			return it1.vertex!=it2.vertex;
			};
		Vertex& operator*(void) const
			{
			return *vertex;
			};
		Vertex* operator->(void) const
			{
			return vertex;
			};
		static size_t hash(const VertexIterator& vertexIt,size_t tableSize)
			{
			return reinterpret_cast<size_t>(vertexIt.vertex)%tableSize;
			};
		VertexEdgeIterator beginEdges(void) const
			{
			return VertexEdgeIterator(vertex);
			};
		VertexEdgeIterator endEdges(void) const
			{
			return VertexEdgeIterator();
			};
		Point& getPoint(void) const
			{
			return *vertex;
			};
		VertexIterator& operator++(void)
			{
			vertex=vertex->succ;
			return *this;
			};
		VertexIterator operator++(int)
			{
			VertexIterator result(*this);
			vertex=vertex->succ;
			return result;
			};
		};
	
	class ConstVertexIterator // Iterator for constant vertices
		{
		friend class PolygonMesh;
		
		/* Elements: */
		private:
		const Vertex* vertex; // Vertex pointed to
		
		/* Constructors and destructors: */
		public:
		ConstVertexIterator(void) // Constructs invalid iterator
			:vertex(0)
			{
			};
		ConstVertexIterator(const Vertex* sVertex)
			:vertex(sVertex)
			{
			};
		ConstVertexIterator(const VertexIterator& it)
			:vertex(it.vertex)
			{
			};
		
		/* Methods: */
		friend bool operator==(const ConstVertexIterator& it1,const ConstVertexIterator& it2)
			{
			return it1.vertex==it2.vertex;
			};
		friend bool operator!=(const ConstVertexIterator& it1,const ConstVertexIterator& it2)
			{
			return it1.vertex!=it2.vertex;
			};
		const Vertex& operator*(void) const
			{
			return *vertex;
			};
		const Vertex* operator->(void) const
			{
			return vertex;
			};
		static size_t hash(const ConstVertexIterator& vertexIt,size_t tableSize)
			{
			return reinterpret_cast<size_t>(vertexIt.vertex)%tableSize;
			};
		ConstVertexEdgeIterator beginEdges(void) const
			{
			return ConstVertexEdgeIterator(vertex);
			};
		ConstVertexEdgeIterator endEdges(void) const
			{
			return ConstVertexEdgeIterator();
			};
		const Point& getPoint(void) const
			{
			return *vertex;
			};
		ConstVertexIterator& operator++(void)
			{
			vertex=vertex->succ;
			return *this;
			};
		ConstVertexIterator operator++(int)
			{
			ConstVertexIterator result(*this);
			vertex=vertex->succ;
			return result;
			};
		};
	
	class VertexEdgeIterator:public EdgeIterator // Iterator for edges around a vertex
		{
		friend class PolygonMesh;
		friend class ConstVertexEdgeIterator;
		
		/* Constructors and destructors: */
		public:
		VertexEdgeIterator(void) // Constructs invalid iterator
			{
			};
		VertexEdgeIterator(Vertex* vertex) // Constructs an iterator for a vertex
			:EdgeIterator(vertex->getEdge())
			{
			};
		
		/* Methods: */
		VertexEdgeIterator& operator++(void)
			{
			EdgeIterator::advanceVertex();
			return *this;
			};
		VertexEdgeIterator operator++(int)
			{
			VertexEdgeIterator result(*this);
			EdgeIterator::advanceVertex();
			return result;
			};
		};
	
	class ConstVertexEdgeIterator:public ConstEdgeIterator // Iterator for edges around a vertex
		{
		friend class PolygonMesh;
		
		/* Constructors and destructors: */
		public:
		ConstVertexEdgeIterator(void) // Constructs invalid iterator
			{
			};
		ConstVertexEdgeIterator(const Vertex* vertex) // Constructs an iterator for a vertex
			:ConstEdgeIterator(vertex->getEdge())
			{
			};
		ConstVertexEdgeIterator(const VertexEdgeIterator& it)
			:ConstEdgeIterator(it)
			{
			};
		
		/* Methods: */
		ConstVertexEdgeIterator& operator++(void)
			{
			ConstEdgeIterator::advanceVertex();
			return *this;
			};
		ConstVertexEdgeIterator operator++(int)
			{
			ConstVertexEdgeIterator result(*this);
			ConstEdgeIterator::advanceVertex();
			return result;
			};
		};
	
	class FaceIterator // Iterator for faces
		{
		friend class PolygonMesh;
		friend class ConstFaceIterator;
		
		/* Elements: */
		private:
		Face* face; // Face pointed to
		
		/* Constructors and destructors: */
		public:
		FaceIterator(void) // Constructs invalid iterator
			:face(0)
			{
			};
		FaceIterator(Face* sFace)
			:face(sFace)
			{
			};
		
		/* Methods: */
		friend bool operator==(const FaceIterator& it1,const FaceIterator& it2)
			{
			return it1.face==it2.face;
			};
		friend bool operator!=(const FaceIterator& it1,const FaceIterator& it2)
			{
			return it1.face!=it2.face;
			};
		Face& operator*(void) const
			{
			return *face;
			};
		Face* operator->(void) const
			{
			return face;
			};
		static size_t hash(const FaceIterator& faceIt,size_t tableSize)
			{
			return reinterpret_cast<size_t>(faceIt.face)%tableSize;
			};
		FaceEdgeIterator beginEdges(void) const
			{
			return FaceEdgeIterator(face);
			};
		FaceEdgeIterator endEdges(void) const
			{
			return FaceEdgeIterator();
			};
		FaceIterator& operator++(void)
			{
			face=face->succ;
			return *this;
			};
		FaceIterator operator++(int)
			{
			FaceIterator result(*this);
			face=face->succ;
			return result;
			};
		};
	
	class ConstFaceIterator // Iterator for constant faces
		{
		friend class PolygonMesh;
		
		/* Elements: */
		private:
		const Face* face; // Face pointed to
		
		/* Constructors and destructors: */
		public:
		ConstFaceIterator(void) // Constructs invalid iterator
			:face(0)
			{
			};
		ConstFaceIterator(const Face* sFace)
			:face(sFace)
			{
			};
		ConstFaceIterator(const FaceIterator& it)
			:face(it.face)
			{
			};
		
		/* Methods: */
		friend bool operator==(const ConstFaceIterator& it1,const ConstFaceIterator& it2)
			{
			return it1.face==it2.face;
			};
		friend bool operator!=(const ConstFaceIterator& it1,const ConstFaceIterator& it2)
			{
			return it1.face!=it2.face;
			};
		const Face& operator*(void) const
			{
			return *face;
			};
		const Face* operator->(void) const
			{
			return face;
			};
		static size_t hash(const ConstFaceIterator& faceIt,size_t tableSize)
			{
			return reinterpret_cast<size_t>(faceIt.face)%tableSize;
			};
		ConstFaceEdgeIterator beginEdges(void) const
			{
			return ConstFaceEdgeIterator(face);
			};
		ConstFaceEdgeIterator endEdges(void) const
			{
			return ConstFaceEdgeIterator();
			};
		ConstFaceIterator& operator++(void)
			{
			face=face->succ;
			return *this;
			};
		ConstFaceIterator operator++(int)
			{
			ConstFaceIterator result(*this);
			face=face->succ;
			return result;
			};
		};
	
	class FaceEdgeIterator:public EdgeIterator // Class to enumerate the edges around a face
		{
		friend class PolygonMesh;
		friend class ConstFaceEdgeIterator;
		
		/* Constructors and destructors: */
		public:
		FaceEdgeIterator(void) // Constructs invalid iterator
			{
			};
		FaceEdgeIterator(Face* face) // Constructs an iterator for a face
			:EdgeIterator(face->getEdge())
			{
			};
		
		/* Methods: */
		FaceEdgeIterator& operator++(void)
			{
			EdgeIterator::advanceFace();
			return *this;
			};
		FaceEdgeIterator operator++(int)
			{
			FaceEdgeIterator result(*this);
			EdgeIterator::advanceFace();
			return result;
			};
		};
	
	class ConstFaceEdgeIterator:public ConstEdgeIterator // Class to enumerate the edges around a face
		{
		friend class PolygonMesh;
		
		/* Constructors and destructors: */
		public:
		ConstFaceEdgeIterator(void) // Constructs invalid iterator
			{
			};
		ConstFaceEdgeIterator(const Face* face) // Constructs an iterator for a face
			:ConstEdgeIterator(face->getEdge())
			{
			};
		ConstFaceEdgeIterator(const FaceEdgeIterator& it)
			:ConstEdgeIterator(it)
			{
			};
		
		/* Methods: */
		ConstFaceEdgeIterator& operator++(void)
			{
			ConstEdgeIterator::advanceFace();
			return *this;
			};
		ConstFaceEdgeIterator operator++(int)
			{
			ConstFaceEdgeIterator result(*this);
			ConstEdgeIterator::advanceFace();
			return result;
			};
		};
	
	typedef Misc::HashTable<VertexPair,Edge*,VertexPair> EdgeHasher;
	
	class CollapsedEdge // Class representing a collapsed edge, with enough information to undo the collapse
		{
		friend class PolygonMesh;
		
		/* Elements: */
		private:
		Edge* edges[2]; // Two half edges starting at the collapsed edge vertex that need to be pried apart when undoing
		Vector offsets[2]; // Offset vectors to add to the collapsed edge vertex to reconstruct the original vertices
		
		/* Constructors and destructors: */
		CollapsedEdge(...);
		};
	
	/* Elements: */
	protected:
	/* Allocators for vertices, edges and faces: */
	Misc::PoolAllocator<Vertex> vertexAllocator;
	Misc::PoolAllocator<Edge> edgeAllocator;
	Misc::PoolAllocator<Face> faceAllocator;
	
	unsigned int version; // Version number of the entire mesh
	int numVertices; // Number of vertices in the mesh
	Vertex* vertices; // Pointer to head of vertex list
	Vertex* lastVertex; // Pointer to tail of vertex list
	int numEdges; // Number of edges in the mesh
	int numFaces; // Number of faces in the mesh
	Face* faces; // Pointer to head of face list
	Face* lastFace; // Pointer to tail of face list
	
	/* Protected methods: */
	Vertex* newVertex(const Point& p,const Color& c); // Creates a new vertex
	void deleteVertex(Vertex* vertex); // Deletes a vertex
	Edge* newEdge(void); // Creates a new edge
	void deleteEdge(Edge* edge); // Deletes an edge
	Face* newFace(void); // Creates a new face
	void deleteFace(Face* face); // Deletes a face
	
	/* Constructors and destructors: */
	public:
	PolygonMesh(void) // Creates empty mesh
		:version(0),
		 numVertices(0),vertices(0),lastVertex(0),
		 numEdges(0),
		 numFaces(0),faces(0),lastFace(0)
		{
		};
	PolygonMesh(const PolygonMesh& source); // Copies a polygon mesh
	~PolygonMesh(void);
	
	/* Methods: */
	VertexIterator addVertex(const Point& pos,const Color& color)
		{
		return VertexIterator(newVertex(pos,color));
		};
	EdgeHasher* startAddingFaces(void);
	FaceIterator addFace(int numVertices,const VertexIterator vertices[],EdgeHasher* edgeHasher);
	FaceIterator addFace(const std::vector<VertexIterator>& vertices,EdgeHasher* edgeHasher);
	void setEdgeSharpness(VertexIterator v1,VertexIterator v2,int sharpness,EdgeHasher* edgeHasher);
	void finishAddingFaces(EdgeHasher* edgeHasher);
	int getNumVertices(void) const
		{
		return numVertices;
		};
	VertexIterator beginVertices(void)
		{
		return VertexIterator(vertices);
		};
	ConstVertexIterator beginVertices(void) const
		{
		return ConstVertexIterator(vertices);
		};
	VertexIterator endVertices(void)
		{
		return VertexIterator(0);
		};
	ConstVertexIterator endVertices(void) const
		{
		return ConstVertexIterator(0);
		};
	Vertex* createVertex(const Point& p,const Color& c,Vertex* succ =0) // Creates a new vertex without adding it to the main vertex list
		{
		return new(vertexAllocator.allocate()) Vertex(p,c,succ);
		};
	int getNumEdges(void) const
		{
		return numEdges;
		};
	int getNumFaces(void) const
		{
		return numFaces;
		};
	FaceIterator beginFaces(void)
		{
		return FaceIterator(faces);
		};
	ConstFaceIterator beginFaces(void) const
		{
		return ConstFaceIterator(faces);
		};
	FaceIterator endFaces(void)
		{
		return FaceIterator(0);
		};
	ConstFaceIterator endFaces(void) const
		{
		return ConstFaceIterator(0);
		};
	void updateVertexNormals(void); // Recalculates the normal vectors of all updated vertices
	void validateVertices(void) // Marks all vertices as up-to-date
		{
		++version;
		};
	void invalidateVertex(const VertexIterator& vertexIt) // Invalidates a vertex
		{
		vertexIt->version=version;
		};
	void removeSingularVertex(const VertexIterator& vertexIt); // Removes a singular vertex (a vertex without edges)
	void removeVertex(const VertexIterator& vertexIt); // Removes a vertex by turning its platelet into a hole
	FaceIterator vertexToFace(const VertexIterator& vertexIt); // Converts a vertex into a face by splitting triangles off all adjacent faces
	VertexIterator splitEdge(const EdgeIterator& edgeIt,Vertex* edgePoint); // Splits an existing edge using an existing unlinked vertex
	VertexIterator splitEdge(const EdgeIterator& edgeIt,const Point& p,const Color& c) // Splits an existing edge by creating a new vertex
		{
		return splitEdge(edgeIt,createVertex(p,c));
		};
	void rotateEdge(const EdgeIterator& edgeIt); // Rotates an edge in counter-clockwise direction
	FaceIterator removeEdge(const EdgeIterator& edgeIt); // Joins two faces by removing an edge
	int getEdgeSharpness(const EdgeIterator& edgeIt) // Returns the sharpness of an edge
		{
		return edgeIt->sharpness;
		};
	void setEdgeSharpness(const EdgeIterator& edgeIt,int newSharpness) // Sets the sharpness of an edge
		{
		edgeIt->sharpness=newSharpness;
		if(edgeIt->getOpposite()!=0)
			edgeIt->getOpposite()->sharpness=newSharpness;
		};
	void removeFace(const FaceIterator& faceIt); // Removes a face by replacing it with a hole
	void triangulateFace(const FaceIterator& faceIt); // Splits a face into triangles by inserting new edges
	EdgeIterator splitFace(const VertexIterator& vIt1,const VertexIterator& vIt2); // Splits a face by connecting two points by a new edge
	VertexIterator splitFace(const FaceIterator& faceIt,Vertex* facePoint); // Splits an existing face into a triangle fan around the existing unlinked vertex
	template <class InputPointType>
	VertexIterator splitFace(const FaceIterator& faceIt,const InputPointType& p) // Splits an existing face into a triangle fan around the new vertex
		{
		return splitFace(faceIt,createVertex(p));
		};
	VertexIterator splitFaceCatmullClark(const FaceIterator& faceIt,Vertex* v); // Ditto, uses existing unlinked vertex
	template <class InputPointType>
	VertexIterator splitFaceCatmullClark(const FaceIterator& faceIt,const InputPointType& p) // Splits an existing face into a quad fan around the new vertex
		{
		return splitFaceCatmullClark(faceIt,createVertex(p));
		};
	FaceIterator splitFaceDooSabin(const FaceIterator& faceIt); // Insets a face as in Doo-Sabin subdivision
	void checkMesh(void) const; // Checks the mesh for sanity
	};

#endif
