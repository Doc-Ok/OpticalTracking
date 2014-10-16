/***********************************************************************
MorphBox - Data structure to embed polygon meshes into upright boxes
that can be subsequently deformed to morph the embedded mesh.
Copyright (c) 2004 Oliver Kreylos
***********************************************************************/

#include <Math/Math.h>
#include <Geometry/AffineCombiner.h>
#include <Geometry/Vector.h>
#include <Geometry/Matrix.h>
#include <GL/gl.h>
#include <GL/GLGeometryWrappers.h>

#include "MorphBox.h"

/*************************
Methods of class MorphBox:
*************************/

MorphBox::MorphBox(MorphBox::Mesh* sMesh,const MorphBox::Point& origin,const MorphBox::Scalar size[3])
	:mesh(sMesh),
	 numDraggedVertices(0)
	{
	/* Create the initial box vertices: */
	for(int i=0;i<8;++i)
		for(int j=0;j<3;++j)
			{
			boxVertices[i][j]=origin[j];
			if(i&(1<<j))
				boxVertices[i][j]+=size[j];
			}
	
	/* Find all mesh vertices inside the box and calculate their box coordinates: */
	for(MVertexIterator vIt=mesh->beginVertices();vIt!=mesh->endVertices();++vIt)
		{
		/* Calculate the vertex' box coordinates: */
		Scalar bc[3];
		bool inside=true;
		for(int i=0;i<3;++i)
			{
			bc[i]=((*vIt)[i]-origin[i])/size[i];
			if(bc[i]<Scalar(0)||bc[i]>Scalar(1))
				inside=false;
			}
		
		/* Store the vertex if it is inside the box: */
		if(inside)
			{
			MorphVertex mv;
			mv.v=&(*vIt);
			for(int i=0;i<3;++i)
				mv.boxCoords[i]=bc[i];
			morphedVertices.push_back(mv);
			}
		}
	}

bool MorphBox::pickBox(MorphBox::Scalar vertexDist,MorphBox::Scalar edgeDist,MorphBox::Scalar faceDist,const MorphBox::Point& pickPoint)
	{
	/* Compare all vertices to the pick point: */
	Scalar minDist2=Math::sqr(vertexDist);
	int pickedVertex=-1;
	for(int i=0;i<8;++i)
		{
		Scalar dist2=Geometry::sqrDist(pickPoint,boxVertices[i]);
		if(minDist2>dist2)
			{
			minDist2=dist2;
			pickedVertex=i;
			}
		}
	if(pickedVertex>=0)
		{
		numDraggedVertices=1;
		draggedVertexIndices[0]=pickedVertex;
		return true;
		}
	
	/* Compare all edges to the pick point: */
	static const int ei[12][2]={{0,1},{2,3},{4,5},{6,7},{0,2},{1,3},{4,6},{5,7},{0,4},{1,5},{2,6},{3,7}};
	minDist2=Math::sqr(edgeDist);
	int pickedEdge=-1;
	for(int i=0;i<12;++i)
		{
		Vector d=boxVertices[ei[i][1]]-boxVertices[ei[i][0]];
		Scalar d2=Geometry::sqr(d);
		Vector o=pickPoint-boxVertices[ei[i][0]];
		Scalar offset=o*d;
		if(offset>=Scalar(0)&&offset<=d2)
			{
			o-=d*(offset/d2);
			Scalar dist2=Geometry::sqr(o);
			if(minDist2>dist2)
				{
				minDist2=dist2;
				pickedEdge=i;
				}
			}
		}
	if(pickedEdge>=0)
		{
		numDraggedVertices=2;
		for(int i=0;i<2;++i)
			draggedVertexIndices[i]=ei[pickedEdge][i];
		return true;
		}
	
	/* Compare all faces to the pick point: */
	static const int fi[6][4]={{0,4,6,2},{1,3,7,5},{0,1,5,4},{2,6,7,3},{0,2,3,1},{4,5,7,6}};
	minDist2=Math::sqr(faceDist);
	int pickedFace=-1;
	for(int i=0;i<6;++i)
		{
		/* Calculate the face's centroid: */
		Point::AffineCombiner centroidCombiner;
		for(int j=0;j<4;++j)
			centroidCombiner.addPoint(boxVertices[fi[i][j]]);
		Point centroid=centroidCombiner.getPoint();
		Vector p=pickPoint-centroid;
		
		/* Check the picked point against each triangle composing the face: */
		for(int j=0;j<4;++j)
			{
			Vector d1=boxVertices[fi[i][j]]-centroid;
			Vector d2=boxVertices[fi[i][(j+1)%4]]-centroid;
			Vector d3=Geometry::cross(d1,d2);
			Geometry::Matrix<Scalar,3,3> m;
			for(int mi=0;mi<3;++mi)
				{
				m(mi,0)=d1[i];
				m(mi,1)=d2[i];
				m(mi,2)=d3[i];
				}
			Geometry::ComponentArray<Scalar,3> pc=p/m;
			if(pc[0]>=Scalar(0)&&pc[1]>=Scalar(0)&&pc[0]+pc[1]<=Scalar(1))
				{
				Scalar dist2=Math::sqr(pc[2])/Geometry::sqr(d3);
				if(minDist2>dist2)
					{
					minDist2=dist2;
					pickedFace=i;
					}
				}
			}
		}
	if(pickedFace>=0)
		{
		numDraggedVertices=4;
		for(int i=0;i<4;++i)
			draggedVertexIndices[i]=fi[pickedFace][i];
		return true;
		}
	
	return false;
	}

void MorphBox::startDragBox(const MorphBox::OGTransform& startTransformation)
	{
	/* Calculate initial position of dragged vertices: */
	for(int i=0;i<numDraggedVertices;++i)
		draggedVertices[i]=startTransformation.inverseTransform(boxVertices[draggedVertexIndices[i]]);
	}

void MorphBox::dragBox(const MorphBox::OGTransform& currentTransformation)
	{
	/* Transform box vertices: */
	for(int i=0;i<numDraggedVertices;++i)
		boxVertices[draggedVertexIndices[i]]=currentTransformation.transform(draggedVertices[i]);
	
	/* Calculate the new positions of all morphed vertices based on their box coordinates and the current box shape: */
	for(std::vector<MorphVertex>::iterator mvIt=morphedVertices.begin();mvIt!=morphedVertices.end();++mvIt)
		{
		/* Perform trilinear interpolation on the current box shape: */
		Scalar x1=mvIt->boxCoords[0];
		Point p01=Geometry::affineCombination(boxVertices[0],boxVertices[1],x1);
		Point p23=Geometry::affineCombination(boxVertices[2],boxVertices[3],x1);
		Point p45=Geometry::affineCombination(boxVertices[4],boxVertices[5],x1);
		Point p67=Geometry::affineCombination(boxVertices[6],boxVertices[7],x1);
		Scalar y1=mvIt->boxCoords[1];
		Point p0123=Geometry::affineCombination(p01,p23,y1);
		Point p4567=Geometry::affineCombination(p45,p67,y1);
		Scalar z1=mvIt->boxCoords[2];
		Point p=Geometry::affineCombination(p0123,p4567,z1);
		
		/* Set the morphed vertex' position: */
		for(int i=0;i<3;++i)
			(*mvIt->v)[i]=p[i];
		}
	}

void MorphBox::stopDragBox(void)
	{
	numDraggedVertices=0;
	}

void MorphBox::glRenderAction(const GLContextData& contextData) const
	{
	glPushAttrib(GL_ENABLE_BIT|GL_LINE_BIT);
	glDisable(GL_LIGHTING);
	glLineWidth(2.0f);
	glColor3f(1.0f,0.0f,0.0f);
	glBegin(GL_LINE_STRIP);
	glVertex(boxVertices[0]);
	glVertex(boxVertices[1]);
	glVertex(boxVertices[3]);
	glVertex(boxVertices[2]);
	glVertex(boxVertices[0]);
	glVertex(boxVertices[4]);
	glVertex(boxVertices[5]);
	glVertex(boxVertices[7]);
	glVertex(boxVertices[6]);
	glVertex(boxVertices[4]);
	glEnd();
	glBegin(GL_LINES);
	glVertex(boxVertices[1]);
	glVertex(boxVertices[5]);
	glVertex(boxVertices[3]);
	glVertex(boxVertices[7]);
	glVertex(boxVertices[2]);
	glVertex(boxVertices[6]);
	glEnd();
	glPopAttrib();
	}
