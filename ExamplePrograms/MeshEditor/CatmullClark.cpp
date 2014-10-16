/***************************
* CatmullClark - Functions *
* to perform Catmull-Clark *
* subdivision on polygon   *
* meshes                   *
* (c)2001 Oliver Kreylos   *
***************************/

#include <utility>
#include <Misc/HashTable.h>
#include <GL/gl.h>
#include <GL/GLColor.h>

#include "CatmullClark.h"

/* Catmull-Clark using pointers to associate face and edge points: */

PolygonMesh& subdivideCatmullClark(PolygonMesh& mesh)
	{
	PolygonMesh::FaceIterator fIt;
	PolygonMesh::FaceEdgeIterator feIt;
	
	/* Calculate all face points and associate them with the original faces: */
	PolygonMesh::Vertex* facePoints=0;
	for(fIt=mesh.beginFaces();fIt!=mesh.endFaces();++fIt)
		{
		/* Average all the face's vertices to calculate the face point: */
		PolygonMesh::VertexCombiner centroidC;
		for(PolygonMesh::FaceEdgeIterator feIt=fIt.beginEdges();feIt!=fIt.endEdges();++feIt)
			centroidC.addVertex(feIt->getStart());
		
		/* Associate the face point with the face: */
		facePoints=mesh.createVertex(centroidC.getPoint(),centroidC.getColor(),facePoints);
		facePoints->setEdge(fIt->getEdge());
		fIt->facePoint=facePoints;
		}
	
	/* Calculate all edge midpoints and associate them with the original edges: */
	PolygonMesh::Vertex* edgePoints=0;
	for(fIt=mesh.beginFaces();fIt!=mesh.endFaces();++fIt)
		{
		for(feIt=fIt.beginEdges();feIt!=fIt.endEdges();++feIt)
			{
			if(feIt.isUpperHalf()) // Only process "dominant" half edges
				{
				/* Calculate the edge midpoint: */
				PolygonMesh::Point midPoint=Geometry::mid(*feIt->getStart(),*feIt->getEnd());
				PolygonMesh::Color midPointColor;
				for(int i=0;i<4;++i)
					midPointColor[i]=GLubyte(Math::floor((float(feIt->getStart()->color[i])+float(feIt->getEnd()->color[i]))*0.5f+0.5f));
				
				/* Associate the edge midpoint with both half edges: */
				edgePoints=mesh.createVertex(midPoint,midPointColor,edgePoints);
				edgePoints->setEdge(&(*feIt));
				feIt->edgePoint=feIt->getOpposite()->edgePoint=edgePoints;
				}
			}
		}
	
	/* Now adjust all vertices to be the new vertex points: */
	for(PolygonMesh::VertexIterator vIt=mesh.beginVertices();vIt!=mesh.endVertices();++vIt)
		{
		PolygonMesh::VertexCombiner vertexPointC;
		int numEdges=0;
		int numSharpEdges=0;
		PolygonMesh::EdgeIterator sharpEdges[2];
		for(PolygonMesh::VertexEdgeIterator veIt=vIt.beginEdges();veIt!=vIt.endEdges();++veIt,++numEdges)
			{
			/* Add the edge's midpoint and the next face's face point to the vertex point: */
			vertexPointC.addVertex(veIt->getFace()->facePoint);
			vertexPointC.addVertex(veIt->edgePoint,PolygonMesh::Scalar(2));
			if(veIt->sharpness!=0)
				{
				if(numSharpEdges<2)
					sharpEdges[numSharpEdges]=veIt;
				++numSharpEdges;
				}
			}
		
		if(numSharpEdges<2)
			{
			/* Add the original vertex to the vertex point and normalize: */
			vertexPointC.addVertex(&(*vIt),PolygonMesh::Scalar(numEdges*(numEdges-3)));
			vIt->setPoint(vertexPointC.getPoint());
			vIt->color=vertexPointC.getColor();
			}
		else if(numSharpEdges==2)
			{
			/* Forget what we calculated, use the crease vertex rule: */
			vertexPointC.reset();
			vertexPointC.addVertex(&(*vIt),PolygonMesh::Scalar(2));
			vertexPointC.addVertex(sharpEdges[0]->edgePoint);
			vertexPointC.addVertex(sharpEdges[1]->edgePoint);
			vIt->setPoint(vertexPointC.getPoint());
			vIt->color=vertexPointC.getColor();
			}
		}
	
	/* Now adjust all edge midpoints to be the new edge points: */
	for(PolygonMesh::Vertex* epIt=edgePoints;epIt!=0;epIt=epIt->getSucc())
		{
		PolygonMesh::Edge* edge=epIt->getEdge();
		if(edge->sharpness==0)
			{
			PolygonMesh::VertexCombiner edgePointC;
			edgePointC.addVertex(epIt,2.0f);
			edgePointC.addVertex(edge->getFace()->facePoint);
			edgePointC.addVertex(edge->getOpposite()->getFace()->facePoint);
			epIt->setPoint(edgePointC.getPoint());
			epIt->color=edgePointC.getColor();
			}
		else if(edge->sharpness>0)
			{
			--edge->sharpness;
			--edge->getOpposite()->sharpness;
			}
		}
	
	/* Now insert all edge points into the mesh: */
	for(PolygonMesh::Vertex* epIt=edgePoints;epIt!=0;)
		{
		PolygonMesh::Vertex* nextEpIt=epIt->getSucc();
		
		/* Insert the edge point into the mesh by splitting the edge it is associated with: */
		mesh.splitEdge(epIt->getEdge(),epIt);
		
		/* Go to the next edge point: */
		epIt=nextEpIt;
		}
	
	/* Now insert all face points into the mesh, splitting all faces into quad fans: */
	for(PolygonMesh::Vertex* fpIt=facePoints;fpIt!=0;)
		{
		PolygonMesh::Vertex* nextFpIt=fpIt->getSucc();
		
		/* Insert the face point into the mesh by splitting the face it is associated with into a quad fan: */
		mesh.splitFaceCatmullClark(fpIt->getEdge()->getFace(),fpIt);
		
		/* Go to the next face point: */
		fpIt=nextFpIt;
		}
	
	return mesh;
	}
