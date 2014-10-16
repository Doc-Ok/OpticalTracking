/***********************************************************************
BallPivoting - Function to triangulate a set of points lying on a two-
manifold using the pivoting ball algorithm.
Copyright (c) 2005 Oliver Kreylos
***********************************************************************/

#include <utility>
#include <vector>
#include <deque>
#include <queue>
#include <Math/Math.h>
#include <Math/Constants.h>
#include <Geometry/ComponentArray.h>
#include <Geometry/Matrix.h>
#include <Geometry/ValuedPoint.h>
#include <Geometry/ArrayKdTree.h>
#include <GL/gl.h>
#include <GL/GLGeometryWrappers.h>

#include "BallPivoting.h"

typedef Geometry::Point<double,3> Point;
typedef Geometry::Vector<double,3> Vector;
typedef AutoTriangleMesh::Vertex Vertex;
typedef Geometry::ValuedPoint<Point,Vertex*> VertexPoint;
typedef Geometry::ArrayKdTree<VertexPoint> VertexTree;
typedef AutoTriangleMesh::Edge Edge;
typedef AutoTriangleMesh::VertexPair VertexPair;
typedef AutoTriangleMesh::EdgeHasher EdgeHasher;
typedef AutoTriangleMesh::Face Face;
typedef Geometry::ComponentArray<double,3> CA;
typedef Geometry::Matrix<double,3,3> Matrix;

ShootBallResult shootBall(AutoTriangleMesh& mesh,const Point& ballStart,const Vector& ballDirection,double ballRadius)
	{
	typedef AutoTriangleMesh::VertexIterator VIt;
	
	ShootBallResult result;
	result.ballCenter=ballStart;
	result.ballRadius=ballRadius;
	
	/* Find the first vertex hit by the moving ball: */
	double dirLen2=Geometry::sqr(ballDirection);
	double firstLambda=Math::Constants<double>::max;
	VIt firstVertex=mesh.endVertices();
	for(VIt vIt=mesh.beginVertices();vIt!=mesh.endVertices();++vIt)
		if(!vIt->isInterior())
			{
			/* Calculate motion parameter of vertex: */
			Vector dist=ballStart-Point(*vIt);
			double ph=(dist*ballDirection)/dirLen2;
			double det=Math::sqr(ph)-(Geometry::sqr(dist)-Math::sqr(ballRadius))/dirLen2;
			if(det>=0.0)
				{
				double lambda=-ph-Math::sqrt(det);
				if(lambda>=0.0&&firstLambda>lambda)
					{
					firstLambda=lambda;
					firstVertex=vIt;
					}
				}
			}
	if(firstVertex==mesh.endVertices())
		return result;
	
	/* Rotate the ball until it hits the next vertex: */
	Point ballCenter=ballStart+ballDirection*firstLambda;
	result.ballCenter1=ballCenter;
	result.ballCenter=ballCenter;
	result.vertices[0]=firstVertex;
	result.numVertices=1;
	Point firstPoint=Point(*firstVertex);
	Vector rotate1X=ballCenter-firstPoint;
	rotate1X.normalize();
	Vector rotationAxis=Geometry::cross(rotate1X,ballDirection);
	if(Geometry::sqr(rotationAxis)==0.0)
		return result;
	rotationAxis.normalize();
	Vector rotate1Y=Geometry::cross(rotationAxis,rotate1X);
	rotate1Y.normalize();
	double cosSecondAlpha=-3.0;
	VIt secondVertex=mesh.endVertices();
	Point secondBallCenter;
	for(VIt vIt=mesh.beginVertices();vIt!=mesh.endVertices();++vIt)
		if(vIt!=firstVertex&&!vIt->isInterior())
			{
			Point p(*vIt);
			Vector bisectorNormal=p-firstPoint;
			if(Geometry::sqr(bisectorNormal)<=Math::sqr(ballRadius*2.0))
				{
				Point bisectorMid=Geometry::mid(firstPoint,p);
				Vector dir1=bisectorNormal-rotationAxis*(bisectorNormal*rotationAxis);
				double lambda1=-((firstPoint-bisectorMid)*bisectorNormal)/(dir1*bisectorNormal);
				Vector dist2=dir1*lambda1;
				Vector dir2=Geometry::cross(bisectorNormal,rotationAxis);
				double dir2Len2=Geometry::sqr(dir2);
				double ph2=(dist2*dir2)/dir2Len2;
				double det2=Math::sqr(ph2)-(Geometry::sqr(dist2)-Math::sqr(ballRadius))/dir2Len2;
				if(det2>=0.0)
					{
					double lambda2=-ph2+Math::sqrt(det2);
					Vector dist3=dist2+dir2*lambda2;
					double cosAlpha=(dist3*rotate1X)/dist3.mag();
					if(dist3*rotate1Y<0.0)
						cosAlpha=-2.0-cosAlpha;
					if(cosSecondAlpha<cosAlpha)
						{
						cosSecondAlpha=cosAlpha;
						secondVertex=vIt;
						secondBallCenter=firstPoint+dist3;
						}
					}
				}
			}
	if(secondVertex==mesh.endVertices())
		return result;
	
	result.ballCenter2=secondBallCenter;
	result.ballCenter=secondBallCenter;
	result.vertices[1]=secondVertex;
	result.numVertices=2;
	Point secondPoint=Point(*secondVertex);
	Point pivotPoint=Geometry::mid(firstPoint,secondPoint);
	Vector pivotAxis=secondPoint-firstPoint;
	pivotAxis.normalize();
	Vector pivot2X=secondBallCenter-pivotPoint;
	pivot2X.normalize();
	Point triangle[3];
	triangle[1]=firstPoint;
	triangle[2]=secondPoint;
	double cosThirdAlpha=-1.0;
	VIt thirdVertex=mesh.endVertices();
	Point thirdBallCenter;
	for(VIt vIt=mesh.beginVertices();vIt!=mesh.endVertices();++vIt)
		if(vIt!=firstVertex&&vIt!=secondVertex&&!vIt->isInterior())
			{
			/* Set the third point of the potential face: */
			triangle[0]=Point(*vIt);
			
			/* Calculate the plane equation of the potential face: */
			Vector faceNormal=Geometry::cross(triangle[1]-triangle[0],triangle[2]-triangle[0]);
			double faceOffset=(triangle[0]*faceNormal+triangle[1]*faceNormal+triangle[2]*faceNormal)/3.0;
			Matrix ata=Matrix::zero;
			CA atb(0.0);
			for(int i=0;i<3;++i)
				{
				for(int j=0;j<3;++j)
					ata(i,j)+=faceNormal[i]*faceNormal[j];
				atb[i]+=faceNormal[i]*faceOffset;
				}
			
			/* Calculate the plane equations of the three edge bisectors: */
			for(int e=0;e<3;++e)
				{
				Vector bisectorNormal=triangle[(e+1)%3]-triangle[e];
				double bisectorOffset=Math::div2(triangle[e]*bisectorNormal+triangle[(e+1)%3]*bisectorNormal);
				for(int i=0;i<3;++i)
					{
					for(int j=0;j<3;++j)
						ata(i,j)+=bisectorNormal[i]*bisectorNormal[j];
					atb[i]+=bisectorNormal[i]*bisectorOffset;
					}
				}
			
			/* Calculate the circumcircle center of the potential face: */
			Point faceCenter(atb/ata);

			/* Calculate the ball center of the potential face: */
			double faceRadius2=0.0;
			for(int i=0;i<3;++i)
				faceRadius2+=Geometry::sqrDist(faceCenter,triangle[i]);
			faceRadius2/=3.0;
			double discriminant=(Math::sqr(ballRadius)-faceRadius2)/Geometry::sqr(faceNormal);
			if(discriminant>=0.0)
				{
				Vector offset=faceNormal*Math::sqrt(discriminant);
				
				/* Check the first ball center: */
				Point bc1=faceCenter+offset;
				double cosAngle1=((bc1-pivotPoint)*pivot2X)/Geometry::dist(bc1,pivotPoint);
				if(cosThirdAlpha<cosAngle1)
					{
					cosThirdAlpha=cosAngle1;
					thirdVertex=vIt;
					thirdBallCenter=bc1;
					}
				
				/* Check the second ball center: */
				Point bc2=faceCenter-offset;
				double cosAngle2=((bc2-pivotPoint)*pivot2X)/Geometry::dist(bc2,pivotPoint);
				if(cosAngle2>cosThirdAlpha)
					{
					cosThirdAlpha=cosAngle2;
					thirdVertex=vIt;
					thirdBallCenter=bc2;
					}
				}
			}
	if(thirdVertex==mesh.endVertices())
		return result;
	
	result.ballCenter=thirdBallCenter;
	result.numVertices=3;
	Point thirdPoint=Point(*thirdVertex);
	Vector faceNormal=Geometry::cross(secondPoint-firstPoint,thirdPoint-firstPoint);
	double faceOffset=(firstPoint*faceNormal+secondPoint*faceNormal+thirdPoint*faceNormal)/3.0;
	if(thirdBallCenter*faceNormal>=faceOffset)
		{
		result.vertices[0]=firstVertex;
		result.vertices[1]=secondVertex;
		result.vertices[2]=thirdVertex;
		}
	else
		{
		result.vertices[0]=firstVertex;
		result.vertices[1]=thirdVertex;
		result.vertices[2]=secondVertex;
		}
	
	/* Check if the face is valid: */
	result.valid=true;
	for(int i=0;i<3;++i)
		if(result.vertices[i]->isInterior())
			result.valid=false;
	
	return result;
	}

struct PivotRequest // Structure to store a pivot request over a boundary edge
	{
	/* Elements: */
	public:
	Point ballCenter; // Current center point of the pivoting ball
	Vector faceNormal; // Normal vector of the face the pivoting ball is resting on
	Edge* edge; // Pointer to the pivot edge
	
	/* Constructors and destructors: */
	PivotRequest(const Point& sBallCenter,const Vector& sFaceNormal,Edge* sEdge)
		:ballCenter(sBallCenter),faceNormal(sFaceNormal),
		 edge(sEdge)
		{
		};
	};

class FindNextVertexFunctor
	{
	/* Elements: */
	private:
	Point ballCenter;
	double ballRadius;
	Vector lastFaceNormal;
	Point triangle[3];
	Point pivot;
	double maxPivotDistance;
	Vector pivotX,pivotY;
	double minCosPivotAngle;
	Vertex* minVertex;
	Point minBallCenter;
	Vector minFaceNormal;
	
	/* Constructors and destructors: */
	public:
	FindNextVertexFunctor(const PivotRequest& pr,double sBallRadius)
		:ballCenter(pr.ballCenter),ballRadius(sBallRadius),
		 lastFaceNormal(pr.faceNormal),
		 minCosPivotAngle(-3.0),
		 minVertex(0)
		{
		triangle[1]=*pr.edge->getEnd();
		triangle[2]=*pr.edge->getStart();
		pivot=Geometry::mid(triangle[1],triangle[2]);
		maxPivotDistance=Math::sqrt(Math::sqr(ballRadius)-Geometry::sqrDist(triangle[1],triangle[2])*0.25)+ballRadius;
		Vector pivotNormal=triangle[1]-triangle[2];
		pivotX=ballCenter-pivot;
		pivotX.normalize();
		pivotY=Geometry::cross(pivotNormal,pivotX);
		pivotY.normalize();
		};
	
	/* Methods: */
	const Point& getQueryPosition(void) const
		{
		return pivot;
		};
	bool operator()(const VertexPoint& vp,int splitDimension)
		{
		/* Trivially reject if the node (and its farther subtree) is too far away: */
		if(Math::abs(vp[splitDimension]-pivot[splitDimension])>maxPivotDistance)
			return false;
		
		/* Trivially reject the node if it is too far away: */
		if(Geometry::sqrDist(vp,pivot)>=Math::sqr(maxPivotDistance))
			return true;
		
		/* Collect the third triangle point: */
		triangle[0]=vp;
		
		/* Calculate the plane equation of the potential face: */
		Vector faceNormal=Geometry::cross(triangle[1]-triangle[0],triangle[2]-triangle[0]);
		if(faceNormal*lastFaceNormal<0.0)
			return true;
		
		double faceOffset=(triangle[0]*faceNormal+triangle[1]*faceNormal+triangle[2]*faceNormal)/3.0;
		Matrix ata=Matrix::zero;
		CA atb(0.0);
		for(int i=0;i<3;++i)
			{
			for(int j=0;j<3;++j)
				ata(i,j)+=faceNormal[i]*faceNormal[j];
			atb[i]+=faceNormal[i]*faceOffset;
			}
		
		/* Calculate the plane equations of the three edge bisectors: */
		for(int e=0;e<3;++e)
			{
			Vector bisectorNormal=triangle[(e+1)%3]-triangle[e];
			double bisectorOffset=Math::div2(triangle[e]*bisectorNormal+triangle[(e+1)%3]*bisectorNormal);
			for(int i=0;i<3;++i)
				{
				for(int j=0;j<3;++j)
					ata(i,j)+=bisectorNormal[i]*bisectorNormal[j];
				atb[i]+=bisectorNormal[i]*bisectorOffset;
				}
			}
		
		/* Calculate the circumcircle center of the potential face: */
		Point faceCenter(atb/ata);
		
		/* Calculate the ball center of the potential face: */
		double faceRadius2=0.0;
		for(int i=0;i<3;++i)
			faceRadius2+=Geometry::sqrDist(faceCenter,triangle[i]);
		faceRadius2/=3.0;
		double discriminant=(Math::sqr(ballRadius)-faceRadius2)/Geometry::sqr(faceNormal);
		if(discriminant>=0.0)
			{
			double offset=Math::sqrt(discriminant);
			Point nextBallCenter=faceCenter+faceNormal*offset;
			
			/* Calculate the pivoting angle: */
			Vector v=nextBallCenter-pivot;
			double cosPivotAngle=(v*pivotX)/v.mag();
			if(v*pivotY<0.0)
				cosPivotAngle=-2.0-cosPivotAngle;
			if(minCosPivotAngle<cosPivotAngle)
				{
				minCosPivotAngle=cosPivotAngle;
				minVertex=vp.value;
				minBallCenter=nextBallCenter;
				minFaceNormal=faceNormal;
				}
			}
		
		return true;
		};
	Vertex* getNextVertex(void) const
		{
		return minVertex;
		};
	const Point& getNextBallCenter(void) const
		{
		return minBallCenter;
		};
	const Vector& getNextFaceNormal(void) const
		{
		return minFaceNormal;
		};
	};

typedef std::deque<PivotRequest> PivotQueue;

struct BPState
	{
	/* Elements: */
	public:
	AutoTriangleMesh& mesh;
	double ballRadius;
	VertexTree tree;
	EdgeHasher* edgeHasher;
	PivotQueue pivotQueue;
	
	/* Constructors and destructors: */
	BPState(AutoTriangleMesh& sMesh,double sBallRadius)
		:mesh(sMesh),ballRadius(sBallRadius),
		 edgeHasher(0)
		{
		};
	};

BPState* startBallPivoting(AutoTriangleMesh& mesh,const ShootBallResult& sbr)
	{
	typedef AutoTriangleMesh::VertexIterator VIt;
	typedef AutoTriangleMesh::FaceIterator FIt;
	typedef AutoTriangleMesh::FaceEdgeIterator FEIt;
	
	BPState* bpState=new BPState(mesh,sbr.ballRadius);
	
	/* Create kd-tree of all vertices in the given mesh: */
	VertexPoint* vertices=new VertexPoint[bpState->mesh.getNumVertices()];
	int numVertices=0;
	for(VIt vIt=bpState->mesh.beginVertices();vIt!=bpState->mesh.endVertices();++vIt)
		if(!vIt->isInterior())
			{
			vertices[numVertices]=VertexPoint(*vIt,&(*vIt));
			++numVertices;
			}
	bpState->tree.donatePoints(numVertices,vertices);
	
	/* Put all edges into a hash table: */
	bpState->edgeHasher=bpState->mesh.startAddingFaces();
	for(FIt fIt=bpState->mesh.beginFaces();fIt!=bpState->mesh.endFaces();++fIt)
		for(FEIt feIt=fIt.beginEdges();feIt!=fIt.endEdges();++feIt)
			bpState->edgeHasher->setEntry(EdgeHasher::Entry(feIt.getVertexPair(),&(*feIt)));
	
	/* Create the face contained in the shoot ball result: */
	if(sbr.numVertices==3)
		{
		FIt newFace=bpState->mesh.addFace(3,sbr.vertices,bpState->edgeHasher);
		if(newFace!=bpState->mesh.endFaces())
			{
			for(int i=0;i<3;++i)
				bpState->mesh.invalidateVertex(sbr.vertices[i]);
			
			/* Calculate the new face's normal vector: */
			Vector faceNormal=Geometry::cross((*sbr.vertices[1])-(*sbr.vertices[0]),(*sbr.vertices[2])-(*sbr.vertices[0]));
			
			/* Push the new triangle's boundary edges onto the queue: */
			for(FEIt feIt=newFace.beginEdges();feIt!=newFace.endEdges();++feIt)
				if(feIt->getOpposite()==0)
					bpState->pivotQueue.push_back(PivotRequest(sbr.ballCenter,faceNormal,&*feIt));
			}
		}
	
	return bpState;
	}

void renderState(const BPState* bpState)
	{
	glPointSize(3.0f);
	glLineWidth(1.0f);
	glColor3f(1.0f,0.0f,0.0f);
	for(PivotQueue::const_iterator pqIt=bpState->pivotQueue.begin();pqIt!=bpState->pivotQueue.end();++pqIt)
		if(pqIt->edge->getOpposite()==0)
			{
			glBegin(GL_POINTS);
			glVertex(pqIt->ballCenter);
			glEnd();
			glBegin(GL_LINES);
			glVertex(*pqIt->edge->getStart());
			glVertex(*pqIt->edge->getEnd());
			glEnd();
			glColor3f(1.0f,1.0f,0.0f);
			}
	}

bool pivotOnce(BPState* bpState,int numEdges)
	{
	typedef AutoTriangleMesh::VertexIterator VIt;
	typedef AutoTriangleMesh::FaceIterator FIt;
	typedef AutoTriangleMesh::FaceEdgeIterator FEIt;
	
	for(int i=0;i<numEdges;++i)
	{
	/* Process the next edge from the queue: */
	if(!bpState->pivotQueue.empty())
		{
		/* Get the next pivot request from the queue: */
		Edge* edge=bpState->pivotQueue.front().edge;
		
		if(edge->getOpposite()==0)
			{
			/* Find the next pivot vertex by checking all nearby vertices in the tree: */
			FindNextVertexFunctor findNextVertex(bpState->pivotQueue.front(),bpState->ballRadius);
			bpState->pivotQueue.pop_front();
			bpState->tree.traverseTreeDirected(findNextVertex);
			
			/* Create a new triangle if a boundary vertex was found: */
			Vertex* v=findNextVertex.getNextVertex();
			if(v!=0&&!v->isInterior())
				{
				/* Create the new triangle: */
				VIt vs[3];
				vs[0]=v;
				vs[1]=edge->getEnd();
				vs[2]=edge->getStart();
				FIt newFace=bpState->mesh.addFace(3,vs,bpState->edgeHasher);
				if(newFace!=bpState->mesh.endFaces())
					{
					bpState->mesh.invalidateVertex(v);
					bpState->mesh.invalidateVertex(edge->getStart());
					bpState->mesh.invalidateVertex(edge->getEnd());
					
					/* Push the new triangle's boundary edges onto the queue: */
					for(FEIt feIt=newFace.beginEdges();feIt!=newFace.endEdges();++feIt)
						if(feIt->getOpposite()==0)
							bpState->pivotQueue.push_back(PivotRequest(findNextVertex.getNextBallCenter(),findNextVertex.getNextFaceNormal(),&*feIt));
					
					#if 0
					/* Remove non-boundary triangles from the mesh: */
					std::vector<FIt> interiorFaces;
					if(edge->getStart()->isInterior())
						{
						for(Edge* ve=edge;ve!=edge->getOpposite()->getFaceSucc();ve=ve->getVertexSucc())
							if(ve->getEnd()->isInterior()&&ve->getFaceSucc()->getEnd()->isInterior())
								interiorFaces.push_back(ve->getFace());
						}
					if(edge->getEnd()->isInterior())
						{
						for(Edge* ve=edge->getOpposite();ve!=edge->getFaceSucc();ve=ve->getVertexSucc())
							if(ve->getEnd()->isInterior()&&ve->getFaceSucc()->getEnd()->isInterior())
								interiorFaces.push_back(ve->getFace());
							
						}
					for(std::vector<FIt>::const_iterator fIt=interiorFaces.begin();fIt!=interiorFaces.end();++fIt)
						bpState->mesh.removeFace(*fIt);
					#endif
					}
				}
			}
		else
			bpState->pivotQueue.pop_front();
		}
	}
	
	bpState->mesh.updateVertexNormals();
	bpState->mesh.validateVertices();
	
	return bpState->pivotQueue.empty();
	}

void finishBallPivoting(BPState* bpState)
	{
	bpState->mesh.finishAddingFaces(bpState->edgeHasher);
	delete bpState;
	}

/******************************************
Everything below here is not used right now
******************************************/

std::pair<Point,bool> calcBallCenter(const Vertex* edgeStart,const Vertex* edgeEnd,const Vertex* oppositeVertex,double ballRadius)
	{
	/* Gather the three vertices of the given triangle: */
	Point vs[3];
	vs[0]=*oppositeVertex;
	vs[1]=*edgeStart;
	vs[2]=*edgeEnd;
	
	Vector faceNormal=Geometry::cross(vs[1]-vs[0],vs[2]-vs[0]);
	double faceOffset=(vs[0]*faceNormal+vs[1]*faceNormal+vs[2]*faceNormal)/3.0;
	Matrix ata=Matrix::zero;
	CA atb(0.0);
	for(int i=0;i<3;++i)
		{
		for(int j=0;j<3;++j)
			ata(i,j)+=faceNormal[i]*faceNormal[j];
		atb[i]+=faceNormal[i]*faceOffset;
		}
	for(int i=0;i<3;++i)
		{
		Vector bisectorNormal=vs[(i+1)%3]-vs[i];
		double bisectorOffset=Math::div2(vs[i]*bisectorNormal+vs[(i+1)%3]*bisectorNormal);
		for(int i=0;i<3;++i)
			{
			for(int j=0;j<3;++j)
				ata(i,j)+=bisectorNormal[i]*bisectorNormal[j];
			atb[i]+=bisectorNormal[i]*bisectorOffset;
			}
		}
	
	Point faceCenter(atb/ata);
	double faceRadius2=0.0;
	for(int i=0;i<3;++i)
		faceRadius2+=Geometry::sqrDist(faceCenter,vs[i]);
	faceRadius2/=3.0;
	double discriminant=(Math::sqr(ballRadius)-faceRadius2)/Geometry::sqr(faceNormal);
	if(discriminant<0.0)
		return std::pair<Point,bool>(faceCenter,false);
	else
		{
		double offset=Math::sqrt(discriminant);
		Point ballCenter=faceCenter+faceNormal*offset;
		return std::pair<Point,bool>(ballCenter,true);
		}
	}

AutoTriangleMesh& triangulateVertices(AutoTriangleMesh& mesh,double ballRadius)
	{
	typedef AutoTriangleMesh::VertexIterator VIt;
	typedef AutoTriangleMesh::FaceIterator FIt;
	typedef AutoTriangleMesh::FaceEdgeIterator FEIt;
	typedef std::queue<PivotRequest> PivotQueue;
	
	/* Create kd-tree of all vertices in the given mesh: */
	int numVertices=mesh.getNumVertices();
	VertexPoint* vertices=new VertexPoint[numVertices];
	int i=0;
	for(VIt vIt=mesh.beginVertices();vIt!=mesh.endVertices();++vIt,++i)
		vertices[i]=VertexPoint(*vIt,&(*vIt));
	VertexTree tree;
	tree.donatePoints(numVertices,vertices);
	
	/* Put all boundary edges into a hash table and expansion queue: */
	EdgeHasher* edgeHasher=mesh.startAddingFaces();
	PivotQueue pivotQueue;
	
	for(FIt fIt=mesh.beginFaces();fIt!=mesh.endFaces();++fIt)
		{
		bool haveFaceData=false;
		Vector faceNormal;
		std::pair<Point,bool> ballCenterResult;
		for(FEIt feIt=fIt.beginEdges();feIt!=fIt.endEdges();++feIt)
			if(feIt->getOpposite()==0)
				{
				Edge* edge=&(*feIt);
				edgeHasher->setEntry(EdgeHasher::Entry(feIt.getVertexPair(),edge));
				if(!haveFaceData)
					{
					faceNormal=Geometry::cross((*edge->getEnd())-(*edge->getStart()),(*edge->getFacePred()->getStart())-(*edge->getStart()));
					ballCenterResult=calcBallCenter(edge->getStart(),edge->getEnd(),edge->getFacePred()->getStart(),ballRadius);
					}
				if(ballCenterResult.second)
					pivotQueue.push(PivotRequest(ballCenterResult.first,faceNormal,edge));
				}
		}
	
	/* Process edges from the queue: */
	while(!pivotQueue.empty())
		{
		/* Get the next pivot request from the queue: */
		Edge* edge=pivotQueue.front().edge;
		
		if(edge->getOpposite()==0)
			{
			/* Find the next pivot vertex by checking all nearby vertices in the tree: */
			FindNextVertexFunctor findNextVertex(pivotQueue.front(),ballRadius);
			pivotQueue.pop();
			tree.traverseTreeDirected(findNextVertex);
			
			/* Create a new triangle if a boundary vertex was found: */
			Vertex* v=findNextVertex.getNextVertex();
			if(v!=0&&!v->isInterior())
				{
				/* Create the new triangle: */
				VIt vs[3];
				vs[0]=v;
				vs[1]=edge->getEnd();
				vs[2]=edge->getStart();
				FIt newFace=mesh.addFace(3,vs,edgeHasher);
				if(newFace!=mesh.endFaces())
					{
					mesh.invalidateVertex(v);
					mesh.invalidateVertex(edge->getStart());
					mesh.invalidateVertex(edge->getEnd());
					
					/* Push the new triangle's boundary edges onto the queue: */
					for(FEIt feIt=newFace.beginEdges();feIt!=newFace.endEdges();++feIt)
						if(feIt->getOpposite()==0)
							pivotQueue.push(PivotRequest(findNextVertex.getNextBallCenter(),findNextVertex.getNextFaceNormal(),&*feIt));
					}
				}
			}
		else
			pivotQueue.pop();
		}
	
	/* Clean up and return the modified mesh: */
	mesh.finishAddingFaces(edgeHasher);
	return mesh;
	}

BPState* startBallPivoting(AutoTriangleMesh& mesh,double ballRadius)
	{
	typedef AutoTriangleMesh::VertexIterator VIt;
	typedef AutoTriangleMesh::FaceIterator FIt;
	typedef AutoTriangleMesh::FaceEdgeIterator FEIt;
	
	BPState* bpState=new BPState(mesh,ballRadius);
	
	/* Create kd-tree of all vertices in the given mesh: */
	VertexPoint* vertices=new VertexPoint[bpState->mesh.getNumVertices()];
	int numVertices=0;
	for(VIt vIt=bpState->mesh.beginVertices();vIt!=bpState->mesh.endVertices();++vIt)
		if(!vIt->isInterior())
			{
			vertices[numVertices]=VertexPoint(*vIt,&(*vIt));
			++numVertices;
			}
	bpState->tree.donatePoints(numVertices,vertices);
	
	/* Put all boundary edges into a hash table and expansion queue: */
	bpState->edgeHasher=bpState->mesh.startAddingFaces();
	for(FIt fIt=bpState->mesh.beginFaces();fIt!=bpState->mesh.endFaces();++fIt)
		{
		bool haveFaceData=false;
		Vector faceNormal;
		std::pair<Point,bool> ballCenterResult;
		for(FEIt feIt=fIt.beginEdges();feIt!=fIt.endEdges();++feIt)
			{
			Edge* edge=&(*feIt);
			bpState->edgeHasher->setEntry(EdgeHasher::Entry(feIt.getVertexPair(),edge));
			if(feIt->getOpposite()==0)
				{
				if(!haveFaceData)
					{
					faceNormal=Geometry::cross((*edge->getEnd())-(*edge->getStart()),(*edge->getFacePred()->getStart())-(*edge->getStart()));
					ballCenterResult=calcBallCenter(edge->getStart(),edge->getEnd(),edge->getFacePred()->getStart(),bpState->ballRadius);
					}
				if(ballCenterResult.second)
					bpState->pivotQueue.push_back(PivotRequest(ballCenterResult.first,faceNormal,edge));
				}
			}
		}
	
	return bpState;
	}
