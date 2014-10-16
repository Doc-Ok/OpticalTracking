/***********************************************************************
AutoTriangleMesh - Class for triangular meshes that enforce triangle
shape constraints under mesh transformations.
Copyright (c) 2003-2005 Oliver Kreylos
***********************************************************************/

#include <assert.h>
#include <stdio.h>
#include <Math/Math.h>
#include <GL/gl.h>
#include <GL/GLColor.h>

#include "AutoTriangleMesh.h"

/*********************************
Methods of class AutoTriangleMesh:
*********************************/

void AutoTriangleMesh::triangulateAllFaces(void)
	{
	/* Validate all vertices: */
	++version;
	
	/* Find all non-triangular faces and triangulate them: */
	for(FaceIterator faceIt=BaseMesh::beginFaces();faceIt!=BaseMesh::endFaces();++faceIt)
		{
		/* Triangulate the face if it is not a triangle: */
		if(faceIt->getNumEdges()>3)
			triangulateFace(faceIt);
		}
	
	/* Re-calculate vertex normal vectors: */
	updateVertexNormals();
	}

AutoTriangleMesh::AutoTriangleMesh(const AutoTriangleMesh::BaseMesh& source)
	:BaseMesh(source)
	{
	/* Polygon mesh is already created; now triangulate it: */
	triangulateAllFaces();
	}

AutoTriangleMesh& AutoTriangleMesh::operator=(const AutoTriangleMesh::BaseMesh& source)
	{
	if(this!=&source)
		{
		/* Copy polygon mesh: */
		*this=source;
		
		/* Polygon mesh is already created; now triangulate it: */
		triangulateAllFaces();
		}
	
	return *this;
	}

AutoTriangleMesh::FaceIterator AutoTriangleMesh::addFace(int numVertices,const AutoTriangleMesh::VertexIterator vertices[],AutoTriangleMesh::EdgeHasher* edgeHasher)
	{
	/* Add the face directly if it is a triangle; otherwise, triangulate it first: */
	if(numVertices==3)
		return BaseMesh::addFace(numVertices,vertices,edgeHasher);
	else
		{
		FaceIterator faceIt;
		VertexIterator triangleVertices[3];
		triangleVertices[0]=vertices[0];
		for(int i=2;i<numVertices;++i)
			{
			triangleVertices[1]=vertices[i-1];
			triangleVertices[2]=vertices[i];
			faceIt=BaseMesh::addFace(3,triangleVertices,edgeHasher);
			}
		
		return faceIt;
		}
	}

AutoTriangleMesh::FaceIterator AutoTriangleMesh::addFace(const std::vector<AutoTriangleMesh::VertexIterator>& vertices,AutoTriangleMesh::EdgeHasher* edgeHasher)
	{
	/* Add the face directly if it is a triangle; otherwise, triangulate it first: */
	if(vertices.size()==3)
		return BaseMesh::addFace(vertices,edgeHasher);
	else
		{
		FaceIterator faceIt;
		VertexIterator triangleVertices[3];
		triangleVertices[0]=vertices[0];
		for(int i=2;i<vertices.size();++i)
			{
			triangleVertices[1]=vertices[i-1];
			triangleVertices[2]=vertices[i];
			faceIt=BaseMesh::addFace(3,triangleVertices,edgeHasher);
			}
		
		return faceIt;
		}
	}

void AutoTriangleMesh::splitEdge(const AutoTriangleMesh::EdgeIterator& edge)
	{
	/* Get triangle topology: */
	Edge* e1=&(*edge);
	Edge* e2=e1->getFaceSucc();
	Edge* e3=e1->getFacePred();
	Vertex* v1=e1->getStart();
	Vertex* v2=e2->getStart();
	Vertex* v3=e3->getStart();
	Face* f1=e1->getFace();
	
	assert(e2->getFaceSucc()==e3&&e3->getFacePred()==e2);
	assert(e2->getFace()==f1);
	assert(e3->getFace()==f1);
	assert(f1->getEdge()==e1||f1->getEdge()==e2||f1->getEdge()==e3);
	
	Edge* e4=e1->getOpposite();
	if(e4!=0)
		{
		Edge* e5=e4->getFaceSucc();
		Edge* e6=e4->getFacePred();
		Vertex* v4=e6->getStart();
		Face* f2=e4->getFace();
		
		assert(e5->getFaceSucc()==e6&&e6->getFacePred()==e5);
		assert(e4->getStart()==v2);
		assert(e5->getStart()==v1);
		assert(e5->getFace()==f2);
		assert(e6->getFace()==f2);
		assert(f2->getEdge()==e4||f2->getEdge()==e5||f2->getEdge()==e6);
		
		/* Don't increase aspect ratio of triangles when splitting: */
		Scalar e4Len2=Geometry::sqrDist(*v1,*v2);
		Scalar e5Len2=Geometry::sqrDist(*v1,*v4);
		Scalar e6Len2=Geometry::sqrDist(*v2,*v4);
		if(e4Len2<e5Len2||e4Len2<e6Len2)
			{
			/* Split longest edge in neighbouring triangle first: */
			if(e5Len2>e6Len2)
				splitEdge(e5);
			else
				splitEdge(e6);
			
			/* Re-get triangle topology: */
			e4=e1->getOpposite();
			e5=e4->getFaceSucc();
			e6=e4->getFacePred();
			v4=e6->getStart();
			f2=e4->getFace();
			}
		
		/* Create new vertex for edge midpoint: */
		Point p=Geometry::mid(*edge->getStart(),*edge->getEnd());
		Color c;
		for(int i=0;i<4;++i)
			c[i]=GLubyte(Math::floor((GLfloat(edge->getStart()->color[i])+GLfloat(edge->getEnd()->color[i]))*0.5f+0.5f));
		Vertex* nv=newVertex(p,c);
		
		/* Create two quadrilaterals: */
		Edge* ne1=BaseMesh::newEdge();
		Edge* ne2=BaseMesh::newEdge();
		nv->setEdge(ne1);
		e1->setFaceSucc(ne1);
		e1->setOpposite(ne2);
		e2->setFacePred(ne1);
		e4->setFaceSucc(ne2);
		e4->setOpposite(ne1);
		e5->setFacePred(ne2);
		ne1->set(nv,f1,e1,e2,e4);
		ne1->sharpness=0;
		ne2->set(nv,f2,e4,e5,e1);
		ne2->sharpness=0;
		f1->setEdge(e1);
		f2->setEdge(e4);
		
		/* Triangulate first quadrilateral: */
		Edge* ne3=BaseMesh::newEdge();
		Edge* ne4=BaseMesh::newEdge();
		Face* nf1=BaseMesh::newFace();
		e1->setFaceSucc(ne3);
		e3->setFacePred(ne3);
		e2->setFace(nf1);
		e2->setFaceSucc(ne4);
		ne1->setFace(nf1);
		ne1->setFacePred(ne4);
		ne3->set(nv,f1,e1,e3,ne4);
		ne3->sharpness=0;
		ne4->set(v3,nf1,e2,ne1,ne3);
		ne4->sharpness=0;
		nf1->setEdge(ne1);
		
		/* Triangulate second quadrilateral: */
		Edge* ne5=BaseMesh::newEdge();
		Edge* ne6=BaseMesh::newEdge();
		Face* nf2=BaseMesh::newFace();
		e4->setFaceSucc(ne5);
		e6->setFacePred(ne5);
		e5->setFace(nf2);
		e5->setFaceSucc(ne6);
		ne2->setFace(nf2);
		ne2->setFacePred(ne6);
		ne5->set(nv,f2,e4,e6,ne6);
		ne5->sharpness=0;
		ne6->set(v4,nf2,e5,ne2,ne5);
		ne6->sharpness=0;
		nf2->setEdge(ne2);
		
		/* Invalidate all involved vertices: */
		v1->version=version;
		v2->version=version;
		v3->version=version;
		v4->version=version;
		}
	else
		{
		/* Create new vertex for edge midpoint: */
		Point p=Geometry::mid(*edge->getStart(),*edge->getEnd());
		Color c;
		for(int i=0;i<4;++i)
			c[i]=GLubyte(Math::floor((GLfloat(edge->getStart()->color[i])+GLfloat(edge->getEnd()->color[i]))*0.5f+0.5f));
		Vertex* nv=newVertex(p,c);
		
		/* Create one quadrilateral: */
		Edge* ne=BaseMesh::newEdge();
		nv->setEdge(ne);
		e1->setFaceSucc(ne);
		e2->setFacePred(ne);
		ne->set(nv,f1,e1,e2,0);
		ne->sharpness=0;
		f1->setEdge(e1);
		
		/* Triangulate quadrilateral: */
		Edge* ne3=BaseMesh::newEdge();
		Edge* ne4=BaseMesh::newEdge();
		Face* nf1=BaseMesh::newFace();
		e1->setFaceSucc(ne3);
		e3->setFacePred(ne3);
		e2->setFace(nf1);
		e2->setFaceSucc(ne4);
		ne->setFace(nf1);
		ne->setFacePred(ne4);
		ne3->set(nv,f1,e1,e3,ne4);
		ne3->sharpness=0;
		ne4->set(v3,nf1,e2,ne,ne3);
		ne4->sharpness=0;
		nf1->setEdge(ne);
		
		/* Invalidate all involved vertices: */
		v1->version=version;
		v2->version=version;
		v3->version=version;
		}
	}

bool AutoTriangleMesh::canCollapseEdge(const AutoTriangleMesh::Edge* edge) const
	{
	/* Get triangle topology: */
	const Edge* e1=edge;
	const Edge* e2=e1->getFaceSucc();
	const Edge* e3=e1->getFacePred();
	const Edge* e4=e1->getOpposite();
	if(e4!=0) // Edge is interior edge
		{
		/* Check if both side edges of top triangle are on the boundary: */
		const Edge* e7=e2->getOpposite();
		const Edge* e8=e3->getOpposite();
		if(e7==0&&e8==0)
			return false;
		
		/* Check if third vertex of top triangle is interior and has valence<=3: */
		if(e7!=0&&e8!=0&&e7->getVertexSucc()==e8->getFaceSucc())
			return false;
		
		/* Get topology of bottom triangle: */
		const Edge* e5=e4->getFaceSucc();
		const Edge* e6=e4->getFacePred();
		
		/* Check if both side edges of bottom triangle are on the boundary: */
		const Edge* e9=e5->getOpposite();
		const Edge* e10=e6->getOpposite();
		if(e9==0&&e10==0)
			return false;
		
		/* Check if third vertex of bottom triangle is interior and has valence<=3: */
		if(e9!=0&&e10!=0&&e9->getVertexSucc()==e10->getFaceSucc())
			return false;
		
		/* Check if both edge's vertices are on the boundary: */
		const Edge* ve1;
		for(ve1=e8;ve1!=0&&ve1!=e5;ve1=ve1->getVertexSucc())
			;
		bool v1OnBoundary=ve1==0;
		const Edge* ve2;
		for(ve2=e10;ve2!=0&&ve2!=e2;ve2=ve2->getVertexSucc())
			;
		bool v2OnBoundary=ve2==0;
		if(v1OnBoundary&&v2OnBoundary)
			return false;
		
		/* Check if the two vertices' platelets share a vertex: */
		/* Currently highly inefficient at O(n^2); need to optimize! */
		if(v1OnBoundary) // v1 is on boundary, v2 is interior
			{
			/* Go counter-clockwise around v1 until boundary is hit: */
			if(e8!=0)
				{
				for(ve1=e8->getVertexSucc();ve1!=0;ve1=ve1->getVertexSucc())
					for(ve2=e10->getVertexSucc();ve2!=e2;ve2=ve2->getVertexSucc())
						if(ve1->getEnd()==ve2->getEnd())
							return false;
				}
			
			/* Go clockwise around v1 until boundary is hit: */
			if(e9!=0)
				{
				for(ve1=e9->getFaceSucc();ve1->getOpposite()!=0;ve1=ve1->getVertexPred())
					for(ve2=e10->getVertexSucc();ve2!=e2;ve2=ve2->getVertexSucc())
						if(ve1->getEnd()==ve2->getEnd())
							return false;
				}
			}
		else if(v2OnBoundary) // v2 is on boundary, v1 is interior
			{
			/* Go counter-clockwise around v2 until boundary is hit: */
			if(e10!=0)
				{
				for(ve2=e10->getVertexSucc();ve2!=0;ve2=ve2->getVertexSucc())
					for(ve1=e8->getVertexSucc();ve1!=e5;ve1=ve1->getVertexSucc())
						if(ve1->getEnd()==ve2->getEnd())
							return false;
				}
			
			/* Go clockwise around v2 until boundary is hit: */
			if(e7!=0)
				{
				for(ve2=e7->getFaceSucc();ve2->getOpposite()!=0;ve2=ve2->getVertexPred())
					for(ve1=e8->getVertexSucc();ve1!=e5;ve1=ve1->getVertexSucc())
						if(ve1->getEnd()==ve2->getEnd())
							return false;
				}
			}
		else // v1 and v2 are interior
			{
			for(ve1=e8->getVertexSucc();ve1!=e5;ve1=ve1->getVertexSucc())
				for(ve2=e10->getVertexSucc();ve2!=e2;ve2=ve2->getVertexSucc())
					if(ve1->getEnd()==ve2->getEnd())
						return false;
			}
		}
	else // Edge is boundary edge
		{
		/* Check if both side edges are on the boundary: */
		const Edge* e7=e2->getOpposite();
		const Edge* e8=e3->getOpposite();
		if(e7==0&&e8==0)
			return false;
		
		if(e7!=0&&e8!=0)
			{
			/* Check if third face vertex is interior and has valence<=3: */
			if(e7->getVertexSucc()==e8->getFaceSucc())
				return false;
		
			/* Check if the two vertices' platelets share a vertex: */
			/* Currently highly inefficient at O(n^2); need to optimize! */
			for(const Edge* ve1=e8->getFacePred();ve1!=0;ve1=ve1->getEndVertexSucc())
				for(const Edge* ve2=e7->getFaceSucc();ve2!=0;ve2=ve2->getVertexPred())
					if(ve1->getStart()==ve2->getEnd())
						return false;
			}
		}
			
	return true;
	}

bool AutoTriangleMesh::collapseEdge(const AutoTriangleMesh::EdgeIterator& edge)
	{
	/* Get triangle topology: */
	Edge* e1=&(*edge);
	Edge* e2=e1->getFaceSucc();
	Edge* e3=e1->getFacePred();
	Edge* e4=e1->getOpposite();
	if(e4!=0) // Edge is interior edge
		{
		/* Check if both side edges of top triangle are on the boundary: */
		Edge* e7=e2->getOpposite();
		Edge* e8=e3->getOpposite();
		if(e7==0&&e8==0)
			return false;
		
		/* Check if third vertex of top triangle is interior and has valence<=3: */
		if(e7!=0&&e8!=0&&e7->getVertexSucc()==e8->getFaceSucc())
			return false;
		
		/* Get topology of bottom triangle: */
		Edge* e5=e4->getFaceSucc();
		Edge* e6=e4->getFacePred();
		
		/* Check if both side edges of bottom triangle are on the boundary: */
		Edge* e9=e5->getOpposite();
		Edge* e10=e6->getOpposite();
		if(e9==0&&e10==0)
			return false;
		
		/* Check if third vertex of bottom triangle is interior and has valence<=3: */
		if(e9!=0&&e10!=0&&e9->getVertexSucc()==e10->getFaceSucc())
			return false;
		
		/* Check if both edge's vertices are on the boundary: */
		Edge* ve1;
		for(ve1=e8;ve1!=0&&ve1!=e5;ve1=ve1->getVertexSucc())
			;
		bool v1OnBoundary=ve1==0;
		Edge* ve2;
		for(ve2=e10;ve2!=0&&ve2!=e2;ve2=ve2->getVertexSucc())
			;
		bool v2OnBoundary=ve2==0;
		if(v1OnBoundary&&v2OnBoundary)
			return false;
		
		/* Check if the two vertices' platelets share a vertex: */
		/* Currently highly inefficient at O(n^2); need to optimize! */
		if(v1OnBoundary) // v1 is on boundary, v2 is interior
			{
			/* Go counter-clockwise around v1 until boundary is hit: */
			if(e8!=0)
				{
				for(ve1=e8->getVertexSucc();ve1!=0;ve1=ve1->getVertexSucc())
					for(ve2=e10->getVertexSucc();ve2!=e2;ve2=ve2->getVertexSucc())
						if(ve1->getEnd()==ve2->getEnd())
							return false;
				}
			
			/* Go clockwise around v1 until boundary is hit: */
			if(e9!=0)
				{
				for(ve1=e9->getFaceSucc();ve1->getOpposite()!=0;ve1=ve1->getVertexPred())
					for(ve2=e10->getVertexSucc();ve2!=e2;ve2=ve2->getVertexSucc())
						if(ve1->getEnd()==ve2->getEnd())
							return false;
				}
			}
		else if(v2OnBoundary) // v2 is on boundary, v1 is interior
			{
			/* Go counter-clockwise around v2 until boundary is hit: */
			if(e10!=0)
				{
				for(ve2=e10->getVertexSucc();ve2!=0;ve2=ve2->getVertexSucc())
					for(ve1=e8->getVertexSucc();ve1!=e5;ve1=ve1->getVertexSucc())
						if(ve1->getEnd()==ve2->getEnd())
							return false;
				}
			
			/* Go clockwise around v2 until boundary is hit: */
			if(e7!=0)
				{
				for(ve2=e7->getFaceSucc();ve2->getOpposite()!=0;ve2=ve2->getVertexPred())
					for(ve1=e8->getVertexSucc();ve1!=e5;ve1=ve1->getVertexSucc())
						if(ve1->getEnd()==ve2->getEnd())
							return false;
				}
			}
		else // v1 and v2 are interior
			{
			for(ve1=e8->getVertexSucc();ve1!=e5;ve1=ve1->getVertexSucc())
				for(ve2=e10->getVertexSucc();ve2!=e2;ve2=ve2->getVertexSucc())
					if(ve1->getEnd()==ve2->getEnd())
						return false;
			}
		
		Vertex* v1=e1->getStart();
		Vertex* v2=e2->getStart();
		Vertex* v3=e3->getStart();
		Vertex* v4=e6->getStart();
		Face* f1=e1->getFace();
		Face* f2=e4->getFace();
		
		/* Move v1 to edge midpoint: */
		Point p=Geometry::mid(*v1,*v2);
		Color c;
		for(int i=0;i<4;++i)
			c[i]=GLubyte(Math::floor((GLfloat(v1->color[i])+GLfloat(v2->color[i]))*0.5f+0.5f));
		v1->setPoint(p);
		v1->color=c;
		
		/* Remove both triangles from mesh: */
		if(e7!=0)
			e7->setOpposite(e8);
		if(e8!=0)
			e8->setOpposite(e7);
		if(e7!=0&&e8!=0)
			{
			if(e7->sharpness<e8->sharpness)
				e7->sharpness=e8->sharpness;
			else
				e8->sharpness=e7->sharpness;
			}
		if(e9!=0)
			e9->setOpposite(e10);
		if(e10!=0)
			e10->setOpposite(e9);
		if(e9!=0&&e10!=0)
			{
			if(e9->sharpness<e10->sharpness)
				e9->sharpness=e10->sharpness;
			else
				e10->sharpness=e9->sharpness;
			}
		if(e8!=0)
			v1->setEdge(e8);
		else
			v1->setEdge(e7->getFaceSucc());
		if(e7!=0)
			v3->setEdge(e7);
		else
			v3->setEdge(e8->getFaceSucc());
		if(e9!=0)
			v4->setEdge(e9);
		else
			v4->setEdge(e10->getFaceSucc());
		
		/* Remove v2 from mesh: */
		if(v2OnBoundary)
			{
			for(Edge* ve2=e10;ve2!=0;ve2=ve2->getVertexSucc())
				ve2->setStart(v1);
			for(Edge* ve2=e2->getVertexPred();ve2!=0;ve2=ve2->getVertexPred())
				ve2->setStart(v1);
			}
		else
			{
			for(Edge* ve2=e10;ve2!=e8;ve2=ve2->getVertexSucc())
				ve2->setStart(v1);
			}
		
		/* Delete removed objects: */
		v2->setEdge(0);
		f1->setEdge(0);
		f2->setEdge(0);
		
		deleteEdge(e1);
		deleteEdge(e2);
		deleteEdge(e3);
		deleteEdge(e4);
		deleteEdge(e5);
		deleteEdge(e6);
		deleteVertex(v2);
		deleteFace(f1);
		deleteFace(f2);
		
		/* Invalidate all involved vertices: */
		v1->version=version;
		ve1=v1->getEdge();
		do
			{
			ve1->getEnd()->version=version;
			ve1=ve1->getVertexSucc();
			}
		while(ve1!=0&&ve1!=v1->getEdge());
		if(ve1==0)
			for(ve1=v1->getEdge()->getVertexPred();ve1!=0;ve1=ve1->getVertexPred())
				ve1->getEnd()->version=version;
		}
	else // Edge is boundary edge
		{
		/* Check if both side edges are on the boundary: */
		Edge* e7=e2->getOpposite();
		Edge* e8=e3->getOpposite();
		if(e7==0&&e8==0)
			return false;
		
		if(e7!=0&&e8!=0)
			{
			/* Check if third face vertex is interior and has valence<=3: */
			if(e7->getVertexSucc()==e8->getFaceSucc())
				return false;
		
			/* Check if the two vertices' platelets share a vertex: */
			/* Currently highly inefficient at O(n^2); need to optimize! */
			for(const Edge* ve1=e8->getFacePred();ve1!=0;ve1=ve1->getEndVertexSucc())
				for(const Edge* ve2=e7->getFaceSucc();ve2!=0;ve2=ve2->getVertexPred())
					if(ve1->getStart()==ve2->getEnd())
						return false;
			}
		
		Vertex* v1=e1->getStart();
		Vertex* v2=e2->getStart();
		Vertex* v3=e3->getStart();
		Face* f1=e1->getFace();
		
		/* Move v1 to edge midpoint: */
		Point p=Geometry::mid(*v1,*v2);
		Color c;
		for(int i=0;i<4;++i)
			c[i]=GLubyte(Math::floor((GLfloat(v1->color[i])+GLfloat(v2->color[i]))*0.5f+0.5f));
		v1->setPoint(p);
		v1->color=c;
		
		/* Remove top triangle from mesh: */
		if(e7!=0)
			e7->setOpposite(e8);
		if(e8!=0)
			e8->setOpposite(e7);
		if(e7!=0&&e8!=0)
			{
			if(e7->sharpness<e8->sharpness)
				e7->sharpness=e8->sharpness;
			else
				e8->sharpness=e7->sharpness;
			}
		if(e8!=0)
			v1->setEdge(e8);
		else
			v1->setEdge(e7->getFaceSucc());
		if(e7!=0)
			v3->setEdge(e7);
		else
			v3->setEdge(e8->getFaceSucc());
		
		/* Remove v2 from mesh: */
		if(e7!=0)
			for(Edge* ve2=e7->getFaceSucc();ve2!=0;ve2=ve2->getVertexPred())
				ve2->setStart(v1);
		
		/* Delete removed objects: */
		v2->setEdge(0);
		f1->setEdge(0);
		
		deleteEdge(e1);
		deleteEdge(e2);
		deleteEdge(e3);
		deleteVertex(v2);
		deleteFace(f1);
		
		/* Invalidate all involved vertices: */
		v1->version=version;
		for(Edge* ve1=v1->getEdge();ve1!=0;ve1=ve1->getVertexSucc())
			ve1->getEnd()->version=version;
		for(Edge* ve1=v1->getEdge()->getVertexPred();ve1!=0;ve1=ve1->getVertexPred())
			ve1->getEnd()->version=version;
		}
			
	return true;
	}

void AutoTriangleMesh::limitEdgeLength(const AutoTriangleMesh::Point& center,AutoTriangleMesh::Scalar radius,AutoTriangleMesh::Scalar maxEdgeLength)
	{
	Scalar radius2=Math::sqr(radius);
	
	/* Iterate through all triangles: */
	FaceIterator faceIt=BaseMesh::beginFaces();
	while(faceIt!=BaseMesh::endFaces())
		{
		/* Check whether face overlaps area of influence and calculate face's maximum edge length: */
		bool overlaps=false;
		Edge* longestEdge=0;
		Scalar longestEdgeLength2=maxEdgeLength*maxEdgeLength;
		Edge* e=faceIt->getEdge();
		for(int i=0;i<3;++i)
			{
			overlaps=overlaps||Geometry::sqrDist(*e->getStart(),center)<=radius2;
			
			/* Calculate edge's squared length: */
			Scalar edgeLength2=Geometry::sqrDist(*e->getStart(),*e->getEnd());
			if(longestEdgeLength2<edgeLength2)
				{
				longestEdge=e;
				longestEdgeLength2=edgeLength2;
				}
			
			/* Go to next edge: */
			e=e->getFaceSucc();
			}
		
		/* Check whether the longest triangle edge is too long: */
		if(overlaps&&longestEdge!=0)
			{
			/* Split longest edge: */
			splitEdge(longestEdge);
			}
		else
			{
			/* Go to next triangle: */
			++faceIt;
			}
		}
	}

void AutoTriangleMesh::ensureEdgeLength(const AutoTriangleMesh::Point& center,AutoTriangleMesh::Scalar radius,AutoTriangleMesh::Scalar minEdgeLength)
	{
	Scalar radius2=Math::sqr(radius);
	
	/* Iterate through all triangles: */
	FaceIterator faceIt=BaseMesh::beginFaces();
	while(faceIt!=BaseMesh::endFaces())
		{
		/* Check quickly (ha!) if face overlaps area of influence: */
		bool overlaps=false;
		Edge* e=faceIt->getEdge();
		do
			{
			if(Geometry::sqrDist(*e->getStart(),center)<=radius2)
				{
				overlaps=true;
				break;
				}
			
			/* Go to next edge: */
			e=e->getFaceSucc();
			}
		while(e!=faceIt->getEdge());
		
		if(overlaps)
			{
			/* Calculate face's minimum edge length: */
			Edge* shortestEdge=0;
			Scalar shortestEdgeLength2=Math::sqr(minEdgeLength);
			Edge* e=faceIt->getEdge();
			do
				{
				/* Calculate edge's squared length: */
				#if 1
				Scalar edgeLength2=Geometry::sqrDist(*e->getStart(),*e->getEnd());
				#else
				/* Calculate normal vectors for edge's vertices: */
				float normal1[3],normal2[3];
				calcNormal(e->getStart(),normal1);
				calcNormal(e->getEnd(),normal2);
				float dist1=0.0f;
				float dist2=0.0f;
				float edgeLength2=0.0f;
				float normal1Length2=0.0f;
				float normal2Length2=0.0f;
				for(int i=0;i<3;++i)
					{
					float dist=(*e->getEnd())[i]-(*e->getStart())[i];
					normal1Length2+=normal1[i]*normal1[i];
					normal2Length2+=normal2[i]*normal2[i];
					dist1+=dist*normal1[i];
					dist2+=dist*normal2[i];
					edgeLength2+=dist*dist;
					}
				dist1=fabsf(dist1)/sqrtf(normal1Length2);
				dist2=fabsf(dist2)/sqrtf(normal2Length2);
				float edgeLength=sqrtf(edgeLength2)+5.0f*(dist1+dist2);
				edgeLength2=edgeLength*edgeLength;
				#endif
				if(shortestEdgeLength2>edgeLength2&&canCollapseEdge(EdgeIterator(e)))
					{
					shortestEdge=e;
					shortestEdgeLength2=edgeLength2;
					}
				
				/* Go to next edge: */
				e=e->getFaceSucc();
				}
			while(e!=faceIt->getEdge());
			
			/* Go to next triangle: */
			++faceIt;
			
			/* Check whether the shortest collapsible triangle edge is too short: */
			if(shortestEdge!=0)
				{
				/* Skip next face if it will be removed by edge collapse: */
				if(shortestEdge->getOpposite()!=0&&faceIt==shortestEdge->getOpposite()->getFace())
					++faceIt;
				
				/* Collapse shortest collapsible edge: */
				collapseEdge(shortestEdge);
				}
			}
		else
			{
			/* Go to the next triangle: */
			++faceIt;
			}
		}
	}
