/***********************************************************************
BallPivoting - Function to triangulate a set of points lying on a two-
manifold using the pivoting ball algorithm.
Copyright (c) 2005 Oliver Kreylos
***********************************************************************/

#ifndef BALLPIVOTING_INCLUDED
#define BALLPIVOTING_INCLUDED

#include <Geometry/Point.h>
#include <Geometry/Vector.h>

#include "AutoTriangleMesh.h"

struct ShootBallResult
	{
	/* Elements: */
	Geometry::Point<double,3> ballCenter1;
	Geometry::Point<double,3> ballCenter2;
	Geometry::Point<double,3> ballCenter3;
	Geometry::Point<double,3> ballCenter;
	double ballRadius;
	int numVertices;
	AutoTriangleMesh::VertexIterator vertices[3];
	bool valid; // Flag if the selected face can be added to the mesh
	
	/* Constructors and destructors: */
	ShootBallResult(void) // Creates invalid result
		:numVertices(0),valid(false)
		{
		};
	};

ShootBallResult shootBall(AutoTriangleMesh& mesh,const Geometry::Point<double,3>& ballStart,const Geometry::Vector<double,3>& ballDirection,double ballRadius);

AutoTriangleMesh& triangulateVertices(AutoTriangleMesh& mesh,double ballRadius);

struct BPState;

BPState* startBallPivoting(AutoTriangleMesh& mesh,double ballRadius);
BPState* startBallPivoting(AutoTriangleMesh& mesh,const ShootBallResult& sbr);
void renderState(const BPState* bpState);
bool pivotOnce(BPState* bpState,int numEdges); // Returns true if triangulation is finished
void finishBallPivoting(BPState* bpState);

#endif
