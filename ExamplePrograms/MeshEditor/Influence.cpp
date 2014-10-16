/***********************************************************************
Influence - Class to encapsulate influence shapes and modification
actions.
***********************************************************************/

#include <vector>
#include <Geometry/AffineCombiner.h>
#include <GL/gl.h>
#include <GL/GLTransformationWrappers.h>

#include "AutoTriangleMesh.h"
#include "SphereRenderer.h"

#include "Influence.h"

/**************************
Methods of class Influence:
**************************/

Influence::Influence(const SphereRenderer* sSphereRenderer,double sRadius)
	:sphereRenderer(sSphereRenderer),
	 transformation(ONTransform::identity),
	 linearVelocity(Vector::zero),angularVelocity(Vector::zero),
	 radius(sRadius),radius2(Math::sqr(radius)),
	 action(EXPLODE),pressure(0.8),density(0.8)
	{
	}

void Influence::setPositionOrientation(const Influence::ONTransform& newTransformation)
	{
	/* Calculate velocities, i.e., distances to old position and orientation: */
	linearVelocity=newTransformation.getTranslation()-transformation.getTranslation();
	angularVelocity=(newTransformation.getRotation()*Geometry::invert(transformation.getRotation())).getScaledAxis();
	
	/* Set new position and orientation: */
	transformation=newTransformation;
	}

void Influence::glRenderAction(GLContextData& contextData) const
	{
	/* Translate coordinate system to influence's position and orientation: */
	glPushMatrix();
	glMultMatrix(transformation);
	glScaled(radius,radius,radius);
	
	/* Render the influence: */
	sphereRenderer->glRenderAction(contextData);
	
	/* Go back to the original coordinate system: */
	glPopMatrix();
	}

void Influence::actOnMesh(Influence::Mesh& mesh) const
	{
	/* Validate mesh's vertices: */
	mesh.validateVertices();
	
	/* Limit mesh's triangle edge lengths inside region of influence: */
	Mesh::Point center(transformation.getOrigin());
	mesh.limitEdgeLength(center,radius,radius*0.1);
	mesh.ensureEdgeLength(center,radius,radius*0.03);
	
	/* Perform influence's action: */
	switch(action)
		{
		case EXPLODE:
			for(Mesh::VertexIterator vIt=mesh.beginVertices();vIt!=mesh.endVertices();++vIt)
				{
				Mesh::Vector r=*vIt-center;
				double dist2=Geometry::sqr(r);
				if(dist2>0.0&&dist2<=radius2)
					{
					double dist=Math::sqrt(dist2);
					double factor=((radius-dist)*pressureFunction(dist/radius))/dist;
					*vIt+=r*Mesh::Scalar(factor);
					mesh.invalidateVertex(vIt);
					}
				}
			break;
		
		case DRAG:
			for(Mesh::VertexIterator vIt=mesh.beginVertices();vIt!=mesh.endVertices();++vIt)
				{
				double dist2=Geometry::sqrDist(*vIt,center);
				if(dist2<=radius2)
					{
					/* Move this vertex: */
					double factor=pressureFunction(Math::sqrt(dist2/radius2));
					Vector r=*vIt-center;
					Vector displacement=linearVelocity+Geometry::cross(angularVelocity,r);
					*vIt+=displacement*factor;
					mesh.invalidateVertex(vIt);
					}
				}
			break;
		
		case WHITTLE:
			{
			std::vector<VertexMotion> verts;
			
			/* Apply fairing operation to vertices: */
			for(Mesh::VertexIterator vIt=mesh.beginVertices();vIt!=mesh.endVertices();++vIt)
				if(vIt->getEdge()!=0)
					{
					double dist2=Geometry::sqrDist(*vIt,center);
					if(dist2<=radius2)
						{
						/* Calculate average position for this vertex: */
						Mesh::Point::AffineCombiner centroidC;
						const Mesh::Edge* e=vIt->getEdge();
						do
							{
							#if 1
							centroidC.addPoint(*e->getEnd());
							#else
							centroidC.addPoint(*e->getEnd(),Geometry::dist(*e->getEnd(),*vIt));
							#endif
							e=e->getVertexSucc();
							}
						while(e!=0&&e!=vIt->getEdge());
						if(e==0)
							{
							for(e=vIt->getEdge()->getVertexPred();e!=0;e=e->getVertexPred())
								{
								#if 1
								centroidC.addPoint(*e->getEnd());
								#else
								centroidC.addPoint(*e->getEnd(),Geometry::dist(*e->getEnd(),*vIt));
								#endif
								}
							}
						Mesh::Point centroid=centroidC.getPoint();
						
						/* Move this vertex towards the average: */
						double factor=pressureFunction(Math::sqrt(dist2/radius2));
						VertexMotion vm(vIt);
						for(int i=0;i<3;++i)
							vm.vec[i]=(centroid[i]-(*vIt)[i])*factor;
						verts.push_back(vm);
						mesh.invalidateVertex(vIt);
						}
					}
			
			for(std::vector<VertexMotion>::const_iterator vIt=verts.begin();vIt!=verts.end();++vIt)
				{
				for(int i=0;i<3;++i)
					(*vIt->vIt)[i]+=vIt->vec[i];
				}
			break;
			}
		}
	
	/* Recalculate normal vectors: */
	mesh.updateVertexNormals();
	}
