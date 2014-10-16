/***********************************************************************
PolygonMesh - Class providing the infrastructure for algorithms working
on meshes of convex polygons
Copyright (c) 2001-2005 Oliver Kreylos
***********************************************************************/

#include <assert.h>
#include <new>
#include <vector>
#include <iostream>
#include <Misc/ThrowStdErr.h>
#include <Misc/HashTable.h>
#include <Math/Math.h>
#include <GL/gl.h>

#include "PolygonMesh.h"

/************************************
Methods of class PolygonMesh::Vertex:
************************************/

int PolygonMesh::Vertex::getNumEdges(void) const
	{
	int result=0;
	const Edge* ePtr=edge;
	do
		{
		++result;
		ePtr=ePtr->getVertexSucc();
		}
	while(ePtr!=edge);

	return result;
	}

bool PolygonMesh::Vertex::isInterior(void) const
	{
	if(edge==0)
		return false;
	
	const Edge* ePtr=edge;
	do
		{
		ePtr=ePtr->getVertexSucc();
		}
	while(ePtr!=edge&&ePtr!=0);
	
	return ePtr!=0;
	}

void PolygonMesh::Vertex::checkVertex(void) const
	{
	assert(edge!=0);
	
	const Edge* ePtr=edge;
	do
		{
		assert(ePtr->getStart()==this);
		assert(ePtr->getFacePred()->getFaceSucc()==ePtr);
		assert(ePtr->getOpposite()!=0);
		assert(ePtr->getOpposite()->getOpposite()==ePtr);
		assert(ePtr->sharpness==ePtr->getOpposite()->sharpness);
		assert(ePtr->getVertexSucc()->getVertexPred()==ePtr);
		assert(ePtr->getVertexPred()->getVertexSucc()==ePtr);
		
		ePtr=ePtr->getVertexSucc();
		}
	while(ePtr!=edge);
	}

/**********************************
Methods of class PolygonMesh::Face:
**********************************/

int PolygonMesh::Face::getNumEdges(void) const
	{
	int result=0;
	const Edge* ePtr=edge;
	do
		{
		++result;
		ePtr=ePtr->getFaceSucc();
		}
	while(ePtr!=edge);

	return result;
	}

void PolygonMesh::Face::checkFace(void) const
	{
	assert(edge!=0);
	
	const Edge* ePtr=edge;
	do
		{
		assert(ePtr->getFace()==this);
		assert(ePtr->getFaceSucc()->getFacePred()==ePtr);
		assert(ePtr->getFacePred()->getFaceSucc()==ePtr);
		
		ePtr=ePtr->getFaceSucc();
		}
	while(ePtr!=edge);
	}

/****************************
Methods of class PolygonMesh:
****************************/

PolygonMesh::Vertex* PolygonMesh::newVertex(const PolygonMesh::Point& p,const PolygonMesh::Color& c)
	{
	/* Create a new vertex and link it to the vertex list: */
	++numVertices;
	Vertex* newVertex=new(vertexAllocator.allocate()) Vertex(p,c,0);
	newVertex->version=version;
	newVertex->pred=lastVertex;
	if(lastVertex!=0)
		lastVertex->succ=newVertex;
	else
		vertices=newVertex;
	lastVertex=newVertex;
	return newVertex;
	}

void PolygonMesh::deleteVertex(PolygonMesh::Vertex* vertex)
	{
	/* Unlink the vertex from the vertex list: */
	if(vertex->pred!=0)
		vertex->pred->succ=vertex->succ;
	else
		vertices=vertex->succ;
	if(vertex->succ!=0)
		vertex->succ->pred=vertex->pred;
	else
		lastVertex=vertex->pred;
	
	/* Delete the vertex: */
	vertex->~Vertex();
	vertexAllocator.free(vertex);
	--numVertices;
	}

PolygonMesh::Edge* PolygonMesh::newEdge(void)
	{
	/* Create a new edge: */
	++numEdges;
	return new(edgeAllocator.allocate()) Edge;
	}

void PolygonMesh::deleteEdge(PolygonMesh::Edge* edge)
	{
	/* Delete the edge: */
	edge->~Edge();
	edgeAllocator.free(edge);
	--numEdges;
	}

PolygonMesh::Face* PolygonMesh::newFace(void)
	{
	/* Create a new face and link it to the face list: */
	++numFaces;
	Face* newFace=new(faceAllocator.allocate()) Face(0);
	newFace->pred=lastFace;
	if(lastFace!=0)
		lastFace->succ=newFace;
	else
		faces=newFace;
	lastFace=newFace;
	return newFace;
	}

void PolygonMesh::deleteFace(PolygonMesh::Face* face)
	{
	/* Unlink the face from the face list: */
	if(face->pred!=0)
		face->pred->succ=face->succ;
	else
		faces=face->succ;
	if(face->succ!=0)
		face->succ->pred=face->pred;
	else
		lastFace=face->pred;
	
	/* Delete the face: */
	face->~Face();
	faceAllocator.free(face);
	--numFaces;
	}

PolygonMesh::PolygonMesh(const PolygonMesh& source)
	:version(0),
	 numVertices(0),vertices(0),lastVertex(0),
	 numEdges(0),
	 numFaces(0),faces(0),lastFace(0)
	{
	/* Copy all vertices and associate the copies with their originals: */
	Misc::HashTable<const Vertex*,Vertex*> vertexMap((source.getNumVertices()*3)/2);
	for(const Vertex* vPtr=source.vertices;vPtr!=0;vPtr=vPtr->succ)
		vertexMap.setEntry(Misc::HashTable<const Vertex*,Vertex*>::Entry(vPtr,newVertex(*vPtr,vPtr->color)));
	
	/* Count the number of edges in the source mesh to estimate the needed hash table size: */
	const Face* fPtr;
	int numEdges=0;
	for(fPtr=source.faces;fPtr!=0;fPtr=fPtr->succ)
		{
		const Edge* firstSourceEdge=fPtr->getEdge();
		const Edge* fePtr=firstSourceEdge;
		do
			{
			++numEdges;
			fePtr=fePtr->getFaceSucc();
			}
		while(fePtr!=firstSourceEdge);
		}
	EdgeHasher companions(numEdges);
	
	/* Copy faces one at a time: */
	for(fPtr=source.faces;fPtr!=0;fPtr=fPtr->succ)
		{
		Face* face=newFace();
		
		/* Copy all edges of the face (don't connect them to other faces yet): */
		const Edge* firstSourceEdge=fPtr->getEdge();
		const Edge* fePtr=firstSourceEdge;
		Edge* firstEdge;
		Edge* lastEdge=0;
		do
			{
			/* Create a new edge: */
			Edge* edge=newEdge();
			Vertex* vPtr=vertexMap.getEntry(fePtr->getStart()).getDest();
			edge->set(vPtr,face,lastEdge,0,0);
			edge->sharpness=fePtr->sharpness;
			vPtr->setEdge(edge);
			if(lastEdge!=0)
				lastEdge->setFaceSucc(edge);
			else
				firstEdge=edge;
			
			lastEdge=edge;
			fePtr=fePtr->getFaceSucc();
			}
		while(fePtr!=firstSourceEdge);
		lastEdge->setFaceSucc(firstEdge);
		firstEdge->setFacePred(lastEdge);
		face->setEdge(firstEdge);
		
		/* Now go around the edge loop again to connect the face to its neighbours: */
		Edge* edge=firstEdge;
		do
			{
			VertexPair vp(*edge);
			EdgeHasher::Iterator companionsIt=companions.findEntry(vp);
			if(companionsIt!=companions.end())
				{
				/* Connect the edge to its companion: */
				edge->setOpposite(companionsIt->getDest());
				companionsIt->getDest()->setOpposite(edge);
				companions.removeEntry(companionsIt);
				}
			else
				{
				/* Add the edge to the companion table: */
				companions.setEntry(EdgeHasher::Entry(vp,edge));
				}
			
			edge=edge->getFaceSucc();
			}
		while(edge!=firstEdge);
		}
	
	/* Calculate all vertex normal vectors: */
	updateVertexNormals();
	}

PolygonMesh::~PolygonMesh(void)
	{
	/* Delete all faces and their associated half-edges: */
	Face* fPtr=faces;
	while(fPtr!=0)
		{
		/* Delete all the face's half-edges: */
		Edge* ePtr=fPtr->edge;
		do
			{
			Edge* next=ePtr->getFaceSucc();
			deleteEdge(ePtr);
			ePtr=next;
			}
		while(ePtr!=fPtr->edge);
		
		/* Delete the face: */
		Face* next=fPtr->succ;
		fPtr->~Face();
		faceAllocator.free(fPtr);
		
		fPtr=next;
		}
	
	/* Delete all vertices: */
	Vertex* vPtr=vertices;
	while(vPtr!=0)
		{
		Vertex* next=vPtr->succ;
		vPtr->~Vertex();
		vertexAllocator.free(vPtr);
		
		vPtr=next;
		}
	}

PolygonMesh::EdgeHasher* PolygonMesh::startAddingFaces(void)
	{
	return new EdgeHasher(101);
	}

PolygonMesh::FaceIterator PolygonMesh::addFace(int numVertices,const PolygonMesh::VertexIterator vertices[],PolygonMesh::EdgeHasher* edgeHasher)
	{
	/* Check whether the given face conforms with the mesh: */
	for(int i=0;i<numVertices;++i)
		{
		/* Look for the current edge in the edge hash table: */
		Vertex* v1=vertices[i].vertex;
		Vertex* v2=vertices[(i+1)%numVertices].vertex;
		EdgeHasher::Iterator ehIt=edgeHasher->findEntry(VertexPair(v1,v2));
		if(!ehIt.isFinished())
			{
			Edge* edge=ehIt->getDest();
			
			/* Test if the companion edge already has an opposite: */
			if(edge->getOpposite()!=0)
				return FaceIterator(0);
				// Misc::throwStdErr("PolygonMesh::addFace: Given face would create non-manifold mesh");
			
			/* Test if the two edges are properly oriented: */
			if(edge->getStart()!=v2||edge->getEnd()!=v1)
				return FaceIterator(0);
				// Misc::throwStdErr("PolygonMesh::addFace: Given face is oriented wrongly");
			}
		}
	
	/* Create the new face without connecting it to neighbours yet: */
	Face* face=newFace();
	Edge* firstEdge;
	Edge* lastEdge=0;
	for(int i=0;i<numVertices;++i)
		{
		Edge* edge=newEdge();
		vertices[i]->setEdge(edge);
		edge->set(vertices[i].vertex,face,lastEdge,0,0);
		edge->sharpness=0;
		if(lastEdge!=0)
			lastEdge->setFaceSucc(edge);
		else
			firstEdge=edge;
		lastEdge=edge;
		}
	lastEdge->setFaceSucc(firstEdge);
	firstEdge->setFacePred(lastEdge);
	face->setEdge(firstEdge);
	
	/* Walk around the face again and connect it to its neighbours: */
	lastEdge=firstEdge;
	do
		{
		VertexPair vp(*lastEdge);
		EdgeHasher::Iterator ehIt=edgeHasher->findEntry(vp);
		if(!ehIt.isFinished())
			{
			/* Connect the edge to its companion: */
			Edge* companion=ehIt->getDest();
			assert(companion->getOpposite()==0);
			assert(companion->getEnd()==lastEdge->getStart());
			lastEdge->setOpposite(companion);
			companion->setOpposite(lastEdge);
			// edgeHasher->removeEntry(ehIt);
			}
		else
			{
			/* Add the edge to the companion table: */
			edgeHasher->setEntry(EdgeHasher::Entry(vp,lastEdge));
			}
		
		lastEdge=lastEdge->getFaceSucc();
		}
	while(lastEdge!=firstEdge);
	
	return FaceIterator(face);
	}

PolygonMesh::FaceIterator PolygonMesh::addFace(const std::vector<PolygonMesh::VertexIterator>& vertices,PolygonMesh::EdgeHasher* edgeHasher)
	{
	int numVertices=vertices.size();
	/* Check whether the given face conforms with the mesh: */
	for(int i=0;i<numVertices;++i)
		{
		/* Look for the current edge in the edge hash table: */
		Vertex* v1=vertices[i].vertex;
		Vertex* v2=vertices[(i+1)%numVertices].vertex;
		EdgeHasher::Iterator ehIt=edgeHasher->findEntry(VertexPair(v1,v2));
		if(!ehIt.isFinished())
			{
			Edge* edge=ehIt->getDest();
			
			/* Test if the companion edge already has an opposite: */
			if(edge->getOpposite()!=0)
				return FaceIterator(0);
				// Misc::throwStdErr("PolygonMesh::addFace: Given face would create non-manifold mesh");
			
			/* Test if the two edges are properly oriented: */
			if(edge->getStart()!=v2||edge->getEnd()!=v1)
				return FaceIterator(0);
				// Misc::throwStdErr("PolygonMesh::addFace: Given face is oriented wrongly");
			}
		}
	
	/* Create the new face without connecting it to neighbours yet: */
	Face* face=newFace();
	Edge* firstEdge;
	Edge* lastEdge=0;
	for(int i=0;i<numVertices;++i)
		{
		Edge* edge=newEdge();
		vertices[i]->setEdge(edge);
		edge->set(vertices[i].vertex,face,lastEdge,0,0);
		edge->sharpness=0;
		if(lastEdge!=0)
			lastEdge->setFaceSucc(edge);
		else
			firstEdge=edge;
		lastEdge=edge;
		}
	lastEdge->setFaceSucc(firstEdge);
	firstEdge->setFacePred(lastEdge);
	face->setEdge(firstEdge);
	
	/* Walk around the face again and connect it to its neighbours: */
	lastEdge=firstEdge;
	do
		{
		VertexPair vp(*lastEdge);
		EdgeHasher::Iterator ehIt=edgeHasher->findEntry(vp);
		if(!ehIt.isFinished())
			{
			/* Connect the edge to its companion: */
			Edge* companion=ehIt->getDest();
			assert(companion->getOpposite()==0);
			assert(companion->getEnd()==lastEdge->getStart());
			lastEdge->setOpposite(companion);
			companion->setOpposite(lastEdge);
			// edgeHasher->removeEntry(ehIt);
			}
		else
			{
			/* Add the edge to the companion table: */
			edgeHasher->setEntry(EdgeHasher::Entry(vp,lastEdge));
			}
		
		lastEdge=lastEdge->getFaceSucc();
		}
	while(lastEdge!=firstEdge);
	
	return FaceIterator(face);
	}

void PolygonMesh::setEdgeSharpness(PolygonMesh::VertexIterator v1,PolygonMesh::VertexIterator v2,int sharpness,PolygonMesh::EdgeHasher* edgeHasher)
	{
	/* Find the edge in the mesh: */
	EdgeHasher::Iterator ehIt=edgeHasher->findEntry(VertexPair(v1.vertex,v2.vertex));
	if(!ehIt.isFinished())
		{
		Edge* edge=ehIt->getDest();
		edge->sharpness=sharpness;
		if(edge->getOpposite()!=0)
			edge->getOpposite()->sharpness=sharpness;
		}
	else
		Misc::throwStdErr("PolygonMesh::setEdgeSharpness: Given edge does not exist in mesh");
	}

void PolygonMesh::finishAddingFaces(PolygonMesh::EdgeHasher* edgeHasher)
	{
	delete edgeHasher;
	updateVertexNormals();
	}

void PolygonMesh::updateVertexNormals(void)
	{
	/* Update the normals of all changed vertices: */
	for(Vertex* vPtr=vertices;vPtr!=0;vPtr=vPtr->succ)
		if(vPtr->version==version&&vPtr->getEdge()!=0)
			{
			/* Calculate the vertex' normal vector: */
			vPtr->normal=Vector::zero;
			
			/* Iterate through vertex' platelet: */
			const Edge* ve=vPtr->getEdge();
			do
				{
				const Edge* ve2=ve->getFacePred();
				vPtr->normal+=Geometry::cross(*ve->getEnd()-*vPtr,*ve2->getStart()-*vPtr);
				
				/* Go to next edge around vertex: */
				ve=ve2->getOpposite();
				}
			while(ve!=0&&ve!=vPtr->getEdge());
			
			if(ve==0) // The vertex' platelet is open
				{
				ve=vPtr->getEdge()->getOpposite();
				while(ve!=0)
					{
					const Edge* ve2=ve->getFaceSucc();
					vPtr->normal+=Geometry::cross(*ve2->getEnd()-*vPtr,*ve->getStart()-*vPtr);

					/* Go to next edge around vertex: */
					ve=ve2->getOpposite();
					}
				}
			}
	}

void PolygonMesh::removeSingularVertex(const PolygonMesh::VertexIterator& vertexIt)
	{
	/* Check if the vertex is singular: */
	if(vertexIt->getEdge()!=0)
		return;
	
	/* Remove the vertex: */
	deleteVertex(vertexIt.vertex);
	}

void PolygonMesh::removeVertex(const PolygonMesh::VertexIterator& vertexIt)
	{
	/* Store all faces from the vertex' platelet: */
	std::vector<Face*> faces;
	Edge* edge=vertexIt->getEdge();
	do
		{
		/* Store the current face: */
		faces.push_back(edge->getFace());
		
		/* Go to the next face: */
		edge=edge->getVertexSucc();
		}
	while(edge!=0&&edge!=vertexIt->getEdge());
	if(edge==0)
		for(edge=vertexIt->getEdge()->getVertexPred();edge!=0;edge=edge->getVertexPred())
			faces.push_back(edge->getFace());
	
	/* Remove all faces: */
	for(std::vector<Face*>::const_iterator fIt=faces.begin();fIt!=faces.end();++fIt)
		removeFace(*fIt);
	
	/* Delete the vertex: */
	deleteVertex(vertexIt.vertex);
	}

#if 0

/******************************************************************
Do not use this function - put in but abandoned and probably broken
******************************************************************/

PolygonMesh::FaceIterator PolygonMesh::removeVertex(const PolygonMesh::VertexIterator& vertexIt)
	{
	if(vertexIt->isInterior())
		{
		/* Combine all surrounding faces into a single face: */
		Face* vertexFace=newFace();
		Edge* lastEdge=0;
		Edge* ePtr=vertexIt->getEdge();
		ePtr->getOpposite()->setOpposite(0);
		while(ePtr!=0)
			{
			/* Re-arrange all face pointers: */
			Edge* vePtr=ePtr->getFaceSucc();
			while(vePtr!=ePtr->getFacePred())
				{
				vePtr->setFace(vertexFace);
				vePtr=vePtr->getFaceSucc();
				}
			
			/* Fix up the vertex: */
			ePtr->getEnd()->setEdge(ePtr->getFaceSucc());
			
			/* Delete the outgoing and incoming edges: */
			Edge* nextEptr=ePtr->getVertexSucc();
			ePtr->getFaceSucc()->setFacePred(lastEdge);
			if(lastEdge!=0)
				lastEdge->setFacePred(ePtr->getFaceSucc());
			else
				vertexFace->setEdge(ePtr->getFaceSucc());
			lastEdge=ePtr->getFacePred()->getFacePred();
			deleteFace(ePtr->getFace());
			deleteEdge(ePtr->getFacePred());
			deleteEdge(ePtr);
			
			/* Go to the next face: */
			ePtr=nextEptr;
			}
		
		/* Close the face loop: */
		lastEdge->setFaceSucc(vertexFace->getEdge());
		vertexFace->getEdge()->setFacePred(lastEdge);
		
		deleteVertex(vertexIt.vertex);
		return vertexFace;
		}
	else if(vertexIt->getEdge()!=0)
		{
		/* Go backwards until border edge is hit: */
		Edge* ePtr;
		for(ePtr=vertexIt->getEdge();ePtr->getOpposite()!=0;ePtr=ePtr->getVertexPred())
			;
		
		/* Remove all surrounding faces: */
		while(ePtr!=0)
			{
			Edge* nextEptr=ePtr->getVertexSucc();
			deleteFace(ePtr->getFace());
			Edge* fePtr=ePtr;
			do
				{
				Edge* nextFePtr=fePtr->getFaceSucc();
				
				/* Fix up the vertex: */
				if(nextFePtr!=ePtr)
					{
					if(nextFePtr->getVertexPred()!=0)
						nextFePtr->getStart()->setEdge(nextFePtr->getVertexPred());
					else
						nextFePtr->getStart()->setEdge(fePtr->getOpposite()->getFacePred());
					}
				
				/* Remove the edge: */
				if(fePtr->getOpposite()!=0)
					fePtr->getOpposite()->setOpposite(0);
				deleteEdge(fePtr);
				}
			while(fePtr!=ePtr);
			
			ePtr=nextEptr;
			}
		
		deleteVertex(vertexIt.vertex);
		return FaceIterator(0);
		}
	else // Dangling vertex
		{
		deleteVertex(vertexIt.vertex);
		return FaceIterator(0);
		}
	}

#endif

PolygonMesh::FaceIterator PolygonMesh::vertexToFace(const PolygonMesh::VertexIterator& vertexIt)
	{
	/* Remove solitary vertices: */
	if(vertexIt->getEdge()==0)
		{
		deleteVertex(vertexIt.vertex);
		return FaceIterator(0);
		}
	
	/* Walk around the vertex and flip its edges: */
	Face* vertexFace=newFace();
	Edge* lastEdge=0;
	Edge* ePtr=vertexIt->getEdge();
	do
		{
		Edge* nextEdge=ePtr->getFacePred()->getOpposite();
		
		/* Remove the edge from its current face and add it to the vertex face: */
		Edge* pred=ePtr->getFacePred();
		Edge* succ=ePtr->getFaceSucc();
		
		/* Test for the dangerous special case of a triangle: */
		if(succ->getFaceSucc()==pred)
			{
			/* Remove the triangle completely: */
			deleteFace(succ->getFace());
			deleteEdge(ePtr);
			deleteEdge(pred);
			
			/* Put the outside edge into the new face loop: */
			succ->set(succ->getStart(),vertexFace,lastEdge,0,succ->getOpposite());
			ePtr=succ;
			}
		else
			{
			pred->setFaceSucc(succ);
			succ->setFacePred(pred);
			ePtr->set(succ->getStart(),vertexFace,lastEdge,0,pred);
			pred->setOpposite(ePtr);
			ePtr->sharpness=pred->sharpness=0;
			pred->getFace()->setEdge(pred);

			#ifndef NDEBUG
			pred->getFace()->checkFace();
			#endif
			}
		
		if(lastEdge!=0)
			{
			lastEdge->setFaceSucc(ePtr);
			
			#ifndef NDEBUG
			ePtr->getStart()->checkVertex();
			#endif
			}
		else
			vertexFace->setEdge(ePtr);
		lastEdge=ePtr;
		
		ePtr=nextEdge;
		}
	while(ePtr!=vertexIt->getEdge());
	lastEdge->setFaceSucc(vertexFace->getEdge());
	vertexFace->getEdge()->setFacePred(lastEdge);
	
	#ifndef NDEBUG
	ePtr->getStart()->checkVertex();
	vertexFace->checkFace();
	#endif
	
	/* Delete the vertex and return the new face: */
	vertexIt->setEdge(0);
	deleteVertex(vertexIt.vertex);
	
	return FaceIterator(vertexFace);
	}

PolygonMesh::VertexIterator PolygonMesh::splitEdge(const PolygonMesh::EdgeIterator& edgeIt,PolygonMesh::Vertex* edgePoint)
	{
	/* Link vertex to mesh: */
	edgePoint->pred=lastVertex;
	edgePoint->succ=0;
	if(lastVertex!=0)
		lastVertex->succ=edgePoint;
	else
		vertices=edgePoint;
	lastVertex=edgePoint;
	
	Edge* edge1=edgeIt.edge;
	Vertex* vertex1=edge1->getStart();
	Edge* edge2=edge1->getOpposite();
	Vertex* vertex2=edge2->getStart();
	Edge* edge3=newEdge();
	Edge* edge4=newEdge();
	
	/* Split edge1 and edge2: */
	edgePoint->setEdge(edge3);
	edge3->set(edgePoint,edge1->getFace(),edge1,edge1->getFaceSucc(),edge2);
	edge3->sharpness=edge1->sharpness;
	edge4->set(edgePoint,edge2->getFace(),edge2,edge2->getFaceSucc(),edge1);
	edge4->sharpness=edge2->sharpness;
	edge1->setFaceSucc(edge3);
	edge1->setOpposite(edge4);
	edge2->setFaceSucc(edge4);
	edge2->setOpposite(edge3);
	edge3->getFaceSucc()->setFacePred(edge3);
	edge4->getFaceSucc()->setFacePred(edge4);
	
	#ifndef NDEBUG
	vertex1->checkVertex();
	vertex2->checkVertex();
	edgePoint->checkVertex();
	edge1->getFace()->checkFace();
	edge2->getFace()->checkFace();
	#endif
	
	return VertexIterator(edgePoint);
	}

void PolygonMesh::rotateEdge(const PolygonMesh::EdgeIterator& edgeIt)
	{
	/* Collect environment of the edge to rotate: */
	Edge* edge1=edgeIt.edge;
	Vertex* vertex1=edge1->getStart();
	Face* face1=edge1->getFace();
	Edge* edge3=edge1->getFacePred();
	Edge* edge4=edge1->getFaceSucc();
	Edge* edge2=edge1->getOpposite();
	Vertex* vertex2=edge2->getStart();
	Face* face2=edge2->getFace();
	Edge* edge5=edge2->getFacePred();
	Edge* edge6=edge2->getFaceSucc();
	
	/* Rotate the edge: */
	vertex1->setEdge(edge6);
	vertex2->setEdge(edge4);
	face1->setEdge(edge1);
	face2->setEdge(edge2);
	edge1->set(edge6->getEnd(),face1,edge6,edge4->getFaceSucc(),edge2);
	edge2->set(edge4->getEnd(),face2,edge4,edge6->getFaceSucc(),edge1);
	edge3->setFaceSucc(edge6);
	edge4->set(vertex2,face2,edge5,edge2,edge4->getOpposite());
	edge5->setFaceSucc(edge4);
	edge6->set(vertex1,face1,edge3,edge1,edge6->getOpposite());
	}

PolygonMesh::FaceIterator PolygonMesh::removeEdge(const PolygonMesh::EdgeIterator& edgeIt)
	{
	Edge* edge2=edgeIt->getOpposite();
	if(edge2!=0)
		{
		/* Have all edges of the second face point to the first instead: */
		Face* newFace=edgeIt->getFace();
		for(Edge* ePtr=edge2->getFaceSucc();ePtr!=edge2;ePtr=ePtr->getFaceSucc())
			ePtr->setFace(newFace);
		
		/* Fix up the edge's start vertex: */
		edgeIt->getFacePred()->setFaceSucc(edge2->getFaceSucc());
		edge2->getFaceSucc()->setFacePred(edgeIt->getFacePred());
		edgeIt->getStart()->setEdge(edge2->getFaceSucc());
		
		/* Fix up the edge's end vertex: */
		edgeIt->getFaceSucc()->setFacePred(edge2->getFacePred());
		edge2->getFacePred()->setFaceSucc(edgeIt->getFaceSucc());
		edge2->getStart()->setEdge(edgeIt->getFaceSucc());
		
		/* Remove the edge and the second face: */
		newFace->setEdge(edgeIt->getFaceSucc());
		deleteFace(edge2->getFace());
		deleteEdge(edgeIt.edge);
		deleteEdge(edge2);
		
		return FaceIterator(newFace);
		}
	else
		{
		/* Fix up all vertices around the face: */
		Edge* ePtr=edgeIt.edge;
		do
			{
			/* Fix up the edge's start vertex: */
			if(ePtr->getVertexSucc()!=0)
				ePtr->getStart()->setEdge(ePtr->getVertexSucc());
			else
				ePtr->getStart()->setEdge(ePtr->getVertexPred());
			
			/* Go to the next edge: */
			ePtr=ePtr->getFaceSucc();
			}
		while(ePtr!=edgeIt.edge);
		
		/* Delete the face: */
		deleteFace(ePtr->getFace());
		
		/* Delete all edges: */
		ePtr=edgeIt.edge;
		ePtr->getFacePred()->setFaceSucc(0);
		while(ePtr!=0)
			{
			Edge* next=ePtr->getFaceSucc();
			deleteEdge(ePtr);
			ePtr=next;
			}
		
		return FaceIterator(0);
		}
	}

void PolygonMesh::removeFace(const PolygonMesh::FaceIterator& fIt)
	{
	Edge* fe;
	
	/* Fix the edge pointers of the face's vertices: */
	fe=fIt->getEdge();
	do
		{
		if(fe->getOpposite()!=0)
			fe->getStart()->setEdge(fe->getVertexPred());
		else
			fe->getStart()->setEdge(fe->getVertexSucc());
		
		fe=fe->getFaceSucc();
		}
	while(fe!=fIt->getEdge());
	
	/* Unlink the face's edges from their opposites: */
	do
		{
		if(fe->getOpposite()!=0)
			fe->getOpposite()->setOpposite(0);
		
		fe=fe->getFaceSucc();
		}
	while(fe!=fIt->getEdge());
	
	/* Delete the face's edges: */
	do
		{
		Edge* fe2=fe->getFaceSucc();
		
		deleteEdge(fe);
		
		fe=fe2;
		}
	while(fe!=fIt->getEdge());
	
	/* Delete the face: */
	deleteFace(&(*fIt));
	}

void PolygonMesh::triangulateFace(const PolygonMesh::FaceIterator& fIt)
	{
	/* Set up an initial triangle at the face's base point: */
	Face* f=&(*fIt);
	Edge* e1=f->getEdge();
	Vertex* v0=e1->getStart();
	v0->version=version;
	Edge* e2=e1->getFaceSucc();
	Vertex* v1=e2->getStart();
	v1->version=version;
	Edge* e3=e2->getFaceSucc();
	Vertex* v2=e3->getStart();
	v2->version=version;
	Edge* lastEdge=e1->getFacePred();
	
	/* Walk around face: */
	while(e3!=lastEdge)
		{
		/* Chop triangle (v0,v1,v2) off face: */
		Edge* ne1=newEdge();
		Edge* ne2=newEdge();
		Face* nf=newFace();
		nf->setEdge(e1);
		e1->setFace(nf);
		e1->setFacePred(ne1);
		e2->setFace(nf);
		e2->setFaceSucc(ne1);
		ne1->set(v2,nf,e2,e1,ne2);
		ne1->sharpness=0;
		f->setEdge(ne2);
		
		/* Reconnect old face: */
		ne2->set(v0,f,lastEdge,e3,ne1);
		ne2->sharpness=0;
		e3->setFacePred(ne2);
		lastEdge->setFaceSucc(ne2);
		
		/* Move to next triangle: */
		e1=ne2;
		v1=v2;
		e2=e3;
		e3=e3->getFaceSucc();
		v2=e3->getStart();
		v2->version=version;
		}
	}

PolygonMesh::EdgeIterator PolygonMesh::splitFace(const PolygonMesh::VertexIterator& vIt1,const PolygonMesh::VertexIterator& vIt2)
	{
	/* Find the face sharing both vertices: */
	
	}

PolygonMesh::VertexIterator PolygonMesh::splitFace(const PolygonMesh::FaceIterator& faceIt,PolygonMesh::Vertex* facePoint)
	{
	/* Link the face point to the mesh: */
	facePoint->pred=lastVertex;
	facePoint->succ=0;
	if(lastVertex!=0)
		lastVertex->succ=facePoint;
	else
		vertices=facePoint;
	lastVertex=facePoint;

	/* Create a fan of triangles around the face point: */
	Edge* firstOuterEdge=faceIt->getEdge();
	deleteFace(faceIt.face);
	Edge* outerEdge=firstOuterEdge;
	Edge* firstInnerEdge;
	Edge* lastInnerEdge=0;
	do
		{
		Edge* nextOuterEdge=outerEdge->getFaceSucc();
		
		/* Create a new triangle: */
		Face* triangle=newFace();
		Edge* innerEdge1=newEdge();
		Edge* innerEdge2=newEdge();
		facePoint->setEdge(innerEdge1);
		innerEdge1->set(facePoint,triangle,innerEdge2,outerEdge,lastInnerEdge);
		innerEdge1->sharpness=0;
		if(lastInnerEdge!=0)
			lastInnerEdge->setOpposite(innerEdge1);
		else
			firstInnerEdge=innerEdge1;
		innerEdge2->set(outerEdge->getEnd(),triangle,outerEdge,innerEdge1,0);
		innerEdge2->sharpness=0;
		outerEdge->setFace(triangle);
		outerEdge->setFacePred(innerEdge1);
		outerEdge->setFaceSucc(innerEdge2);
		triangle->setEdge(outerEdge);
		
		#ifndef NDEBUG
		triangle->checkFace();
		#endif
		
		lastInnerEdge=innerEdge2;
		outerEdge=nextOuterEdge;
		}
	while(outerEdge!=firstOuterEdge);
	
	/* Close the fan by connecting the first and last inner edges: */
	lastInnerEdge->setOpposite(firstInnerEdge);
	firstInnerEdge->setOpposite(lastInnerEdge);
	
	#ifndef NDEBUG
	facePoint->checkVertex();
	#endif
	
	return VertexIterator(facePoint);
	}

PolygonMesh::VertexIterator PolygonMesh::splitFaceCatmullClark(const PolygonMesh::FaceIterator& faceIt,PolygonMesh::Vertex* facePoint)
	{
	assert(faceIt->getNumEdges()%2==0);
	
	/* Link the face point to the mesh: */
	facePoint->pred=lastVertex;
	facePoint->succ=0;
	if(lastVertex!=0)
		lastVertex->succ=facePoint;
	else
		vertices=facePoint;
	lastVertex=facePoint;
	
	/* Create a fan of quadrilaterals around the face point: */
	Edge* firstOuterEdge=faceIt->getEdge()->getFaceSucc();
	deleteFace(faceIt.face);
	Edge* outerEdge=firstOuterEdge;
	Edge* firstInnerEdge;
	Edge* lastInnerEdge=0;
	do
		{
		Edge* nextOuterEdge=outerEdge->getFaceSucc()->getFaceSucc();
		
		/* Create a new quadrilateral: */
		Face* quad=newFace();
		Edge* innerEdge1=newEdge();
		Edge* innerEdge2=newEdge();
		facePoint->setEdge(innerEdge1);
		innerEdge1->set(facePoint,quad,innerEdge2,outerEdge,lastInnerEdge);
		innerEdge1->sharpness=0;
		if(lastInnerEdge!=0)
			lastInnerEdge->setOpposite(innerEdge1);
		else
			firstInnerEdge=innerEdge1;
		outerEdge->setFace(quad);
		outerEdge->setFacePred(innerEdge1);
		outerEdge=outerEdge->getFaceSucc();
		innerEdge2->set(outerEdge->getEnd(),quad,outerEdge,innerEdge1,0);
		innerEdge2->sharpness=0;
		outerEdge->setFace(quad);
		outerEdge->setFaceSucc(innerEdge2);
		quad->setEdge(innerEdge1);
		
		#ifndef NDEBUG
		quad->checkFace();
		#endif
		
		lastInnerEdge=innerEdge2;
		outerEdge=nextOuterEdge;
		}
	while(outerEdge!=firstOuterEdge);
	
	/* Close the fan by connecting the first and last inner edges: */
	lastInnerEdge->setOpposite(firstInnerEdge);
	firstInnerEdge->setOpposite(lastInnerEdge);
	
	#ifndef NDEBUG
	facePoint->checkVertex();
	#endif
	
	return VertexIterator(facePoint);
	}

PolygonMesh::FaceIterator PolygonMesh::splitFaceDooSabin(const PolygonMesh::FaceIterator& faceIt)
	{
	/* Calculate the face's centroid: */
	VertexCombiner centroidC;
	int numVertices=0;
	for(FaceEdgeIterator feIt=faceIt.beginEdges();feIt!=faceIt.endEdges();++feIt,++numVertices)
		centroidC.addVertex(feIt->getStart());
	Point centroid=centroidC.getPoint();
	Color centroidColor=centroidC.getColor();
	
	/* Walk around the face again and create the inner face: */
	Face* innerFace=newFace();
	Edge* lastInnerEdge=0;
	Edge* outerEdge=faceIt->getEdge();
	for(int i=0;i<numVertices;++i,outerEdge=outerEdge->getFaceSucc())
		{
		/* Create a new vertex and a new edge: */
		Point newPoint=Geometry::mid(centroid,*outerEdge->getStart());
		Color newColor;
		for(int i=0;i<4;++i)
			newColor[i]=GLubyte(Math::floor((centroidColor[i]+GLfloat(outerEdge->getStart()->color[i]))*0.5f+0.5f));
		Vertex* newV=newVertex(newPoint,newColor);
		Edge* newE=newEdge();
		newV->setEdge(newE);
		newE->set(newV,innerFace,lastInnerEdge,0,0);
		newE->sharpness=0;
		if(lastInnerEdge!=0)
			lastInnerEdge->setFaceSucc(newE);
		else
			innerFace->setEdge(newE);
		lastInnerEdge=newE;
		}
	lastInnerEdge->setFaceSucc(innerFace->getEdge());
	innerFace->getEdge()->setFacePred(lastInnerEdge);
	
	/* Walk around the face again and create one quad face for each edge: */
	Edge* innerEdge=innerFace->getEdge();
	outerEdge=faceIt->getEdge();
	Edge* lastCrossEdge=0;
	Edge* firstCrossEdge;
	for(int i=0;i<numVertices;++i,innerEdge=innerEdge->getFaceSucc())
		{
		Edge* nextOuterEdge=outerEdge->getFaceSucc();
		
		/* Create a new face and three new edges: */
		Face* quad=newFace();
		quad->setEdge(outerEdge);
		Edge* e1=newEdge();
		Edge* e2=newEdge();
		Edge* e3=newEdge();
		e1->set(innerEdge->getEnd(),quad,e3,e2,innerEdge);
		e1->sharpness=0;
		innerEdge->setOpposite(e1);
		e2->set(innerEdge->getStart(),quad,e1,outerEdge,lastCrossEdge);
		e2->sharpness=0;
		if(lastCrossEdge!=0)
			lastCrossEdge->setOpposite(e2);
		else
			firstCrossEdge=e2;
		e3->set(outerEdge->getEnd(),quad,outerEdge,e1,0);
		e3->sharpness=0;
		lastCrossEdge=e3;
		outerEdge->set(outerEdge->getStart(),quad,e2,e3,outerEdge->getOpposite());
		
		outerEdge=nextOuterEdge;
		}
	lastCrossEdge->setOpposite(firstCrossEdge);
	firstCrossEdge->setOpposite(lastCrossEdge);
	
	/* Delete the old face and return the inner one: */
	deleteFace(faceIt.face);
	
	return FaceIterator(innerFace);
	}

void PolygonMesh::checkMesh(void) const
	{
	/* Check all vertices: */
	for(const Vertex* vPtr=vertices;vPtr!=0;vPtr=vPtr->succ)
		vPtr->checkVertex();
	
	/* Check all faces: */
	for(const Face* fPtr=faces;fPtr!=0;fPtr=fPtr->succ)
		fPtr->checkFace();
	}
