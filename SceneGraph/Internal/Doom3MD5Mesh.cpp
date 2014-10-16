/***********************************************************************
Doom3MD5Mesh - Class to represent animated mesh models in Doom3's MD5
mesh format.
Copyright (c) 2007-2013 Oliver Kreylos

This file is part of the Simple Scene Graph Renderer (SceneGraph).

The Simple Scene Graph Renderer is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Simple Scene Graph Renderer is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Simple Scene Graph Renderer; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <SceneGraph/Internal/Doom3MD5Mesh.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <Misc/Utility.h>
#include <Misc/ThrowStdErr.h>
#include <Math/Math.h>
#include <Math/Constants.h>
#include <Geometry/ComponentArray.h>
#include <Geometry/Matrix.h>
#include <GL/gl.h>
#define NONSTANDARD_GLVERTEX_TEMPLATES
#include <GL/GLVertex.h>
#include <GL/GLVertexArrayTemplates.h>
#include <GL/GLVertexArrayParts.h>
#include <GL/GLContextData.h>
#include <GL/GLExtensionManager.h>
#include <GL/Extensions/GLARBVertexBufferObject.h>
#include <GL/Extensions/GLARBVertexShader.h>
#include <GL/GLGeometryWrappers.h>
#include <SceneGraph/Internal/Doom3FileManager.h>
#include <SceneGraph/Internal/Doom3ValueSource.h>

namespace SceneGraph {

/***************************************
Methods of class Doom3MD5Mesh::DataItem:
***************************************/

Doom3MD5Mesh::DataItem::DataItem(int sNumMeshes)
	:hasVertexBufferObjectExtension(GLARBVertexBufferObject::isSupported()),
	 numMeshes(sNumMeshes),
	 meshVertexBufferObjectIds(new GLuint[numMeshes]),
	 meshIndexBufferObjectIds(new GLuint[numMeshes]),
	 vertexBufferVersion(0)
	{
	if(hasVertexBufferObjectExtension)
		{
		/* Initialize the vertex buffer object extension: */
		GLARBVertexBufferObject::initExtension();
		
		/* Initialize the buffer objects: */
		glGenBuffersARB(numMeshes,meshVertexBufferObjectIds);
		glGenBuffersARB(numMeshes,meshIndexBufferObjectIds);
		}
	
	if(GLARBVertexShader::isSupported())
		{
		/* Initialize the vertex shader extension: */
		GLARBVertexShader::initExtension();
		}
	}

Doom3MD5Mesh::DataItem::~DataItem(void)
	{
	if(hasVertexBufferObjectExtension)
		{
		/* Destroy the buffer objects: */
		if(meshVertexBufferObjectIds!=0)
			glDeleteBuffersARB(numMeshes,meshVertexBufferObjectIds);
		if(meshIndexBufferObjectIds!=0)
			glDeleteBuffersARB(numMeshes,meshIndexBufferObjectIds);
		}
	delete[] meshVertexBufferObjectIds;
	delete[] meshIndexBufferObjectIds;
	}

/*****************************
Methods of class Doom3MD5Mesh:
*****************************/

void Doom3MD5Mesh::poseMesh(Doom3MD5Mesh::Mesh& mesh)
	{
	const Mesh::Vertex* vPtr=mesh.vertices;
	Mesh::RenderVertex* rvPtr=mesh.posedVertices;
	for(int vertexIndex=0;vertexIndex<mesh.numVertices;++vertexIndex,++vPtr,++rvPtr)
		{
		/* Initialize the posed vertex: */
		rvPtr->normal=Vector::zero;
		for(int i=0;i<2;++i)
			rvPtr->tangents[i]=Vector::zero;
		rvPtr->position=Point::origin;
		
		/* Accumulate the posed vertex from its joint weights: */
		const Mesh::Weight* wPtr=&mesh.weights[vPtr->firstWeightIndex];
		for(int weightIndex=0;weightIndex<vPtr->numWeights;++weightIndex,++wPtr)
			{
			const Joint& j=joints[wPtr->jointIndex];
			
			/* Transform the vertex normal, tangent, and position: */
			Vector ntrans=j.transform.transform(wPtr->normal);
			Vector tStrans=j.transform.transform(wPtr->tangents[0]);
			Vector tTtrans=j.transform.transform(wPtr->tangents[1]);
			Point vtrans=j.transform.transform(wPtr->position);
			
			/* Accumulate the transformed vertex: */
			for(int i=0;i<3;++i)
				{
				rvPtr->normal[i]+=ntrans[i]*wPtr->weight;
				rvPtr->tangents[0][i]+=tStrans[i]*wPtr->weight;
				rvPtr->tangents[1][i]+=tTtrans[i]*wPtr->weight;
				rvPtr->position[i]+=vtrans[i]*wPtr->weight;
				}
			}
		}
	}

Doom3MD5Mesh::Doom3MD5Mesh(Doom3FileManager& fileManager,Doom3MaterialManager& sMaterialManager,const char* meshFileName)
	:GLObject(false),
	 materialManager(sMaterialManager),
	 numJoints(0),
	 joints(0),
	 numMeshes(0),
	 meshes(0),
	 jointTreeVersion(1),
	 posedVerticesVersion(1)
	{
	/* Check if the mesh file name has an extension: */
	const char* extPtr=0;
	for(const char* mfnPtr=meshFileName;*mfnPtr!='\0';++mfnPtr)
		if(*mfnPtr=='.')
			extPtr=mfnPtr;
	
	/* Add the .md5mesh extension to the mesh file name: */
	char fileNameBuffer[1024];
	if(extPtr==0)
		{
		snprintf(fileNameBuffer,sizeof(fileNameBuffer),"%s.md5mesh",meshFileName);
		meshFileName=fileNameBuffer;
		}
	
	/* Open the mesh file and create a tokenizer for it: */
	Doom3ValueSource source(fileManager.getFile(meshFileName),meshFileName);
	
	/* Parse the mesh file header: */
	if(!source.isString("MD5Version"))
		Misc::throwStdErr("Doom3MD5Mesh::Doom3MD5Mesh: Input file %s is not a valid MD5 mesh file",meshFileName);
	int md5Version=source.readInteger();
	if(md5Version!=10)
		Misc::throwStdErr("Doom3MD5Mesh::Doom3MD5Mesh: Cannot parse MD5 mesh files of version %d",md5Version);
	
	/* Read and otherwise ignore the commandline string: */
	if(!source.isString("commandline"))
		Misc::throwStdErr("Doom3MD5Mesh::Doom3MD5Mesh: Input file %s is not a valid MD5 mesh file",meshFileName);
	source.skipString();
	
	/* Read the number of joints and meshes: */
	if(!source.isString("numJoints"))
		Misc::throwStdErr("Doom3MD5Mesh::Doom3MD5Mesh: Input file %s is not a valid MD5 mesh file",meshFileName);
	numJoints=source.readInteger();
	if(!source.isString("numMeshes"))
		Misc::throwStdErr("Doom3MD5Mesh::Doom3MD5Mesh: Input file %s is not a valid MD5 mesh file",meshFileName);
	numMeshes=source.readInteger();
	
	/* Allocate the joint array and parse the joint tree: */
	joints=new Joint[numJoints];
	if(!source.isString("joints")||source.readChar()!='{')
		Misc::throwStdErr("Doom3MD5Mesh::Doom3MD5Mesh: Input file %s does not contain a joint list",meshFileName);
	for(int jointIndex=0;jointIndex<numJoints;++jointIndex)
		{
		if(source.peekc()=='}')
			Misc::throwStdErr("Doom3MD5Mesh::Doom3MD5Mesh: Short joint list at %s",source.where().c_str());
		Joint& j=joints[jointIndex];
		
		/* Read the joint's name: */
		std::string name=source.readString();
		j.name=new char[name.size()+1];
		memcpy(j.name,name.c_str(),name.size()+1);
		
		/* Read the joint's parent index: */
		int parentIndex=source.readInteger();
		if(parentIndex==-1)
			j.parent=0;
		else if(parentIndex<numJoints)
			j.parent=&joints[parentIndex];
		else
			Misc::throwStdErr("Doom3MD5Mesh::Doom3MD5Mesh: Parent joint index out of range at %s",source.where().c_str());
		
		/* Read the joint's origin: */
		if(source.readChar()!='(')
			Misc::throwStdErr("Doom3MD5Mesh::Doom3MD5Mesh: Malformed joint position at %s",source.where().c_str());
		Vector translation;
		for(int i=0;i<3;++i)
			translation[i]=Scalar(source.readNumber());
		if(source.readChar()!=')')
			Misc::throwStdErr("Doom3MD5Mesh::Doom3MD5Mesh: Malformed joint position at %s",source.where().c_str());
		
		/* Read the joint's orientation quaternion: */
		if(source.readChar()!='(')
			Misc::throwStdErr("Doom3MD5Mesh::Doom3MD5Mesh: Malformed joint orientation at %s",source.where().c_str());
		Scalar orientation[4];
		Scalar weightDet(1);
		for(int i=0;i<3;++i)
			{
			orientation[i]=Scalar(source.readNumber());
			weightDet-=Math::sqr(orientation[i]);
			}
		orientation[3]=weightDet>Scalar(0)?-Math::sqrt(weightDet):Scalar(0);
		if(source.readChar()!=')')
			Misc::throwStdErr("Doom3MD5Mesh::Doom3MD5Mesh: Malformed joint orientation at %s",source.where().c_str());
		
		/* Store the joint's transformation: */
		j.transform=Transform(translation,Transform::Rotation(orientation));
		}
	if(source.readChar()!='}')
		Misc::throwStdErr("Doom3MD5Mesh::Doom3MD5Mesh: Long joint list at %s",source.where().c_str());
	
	/* Allocate the mesh array and read all meshes: */
	meshes=new Mesh[numMeshes];
	for(int meshIndex=0;meshIndex<numMeshes;++meshIndex)
		{
		if(!source.isString("mesh")||source.readChar()!='{')
			Misc::throwStdErr("Doom3MD5Mesh::Doom3MD5Mesh: Missing mesh definition at %s",source.where().c_str());
		Mesh& m=meshes[meshIndex];
		
		/* Read the mesh's shader name and load the shader from the material manager: */
		if(!source.isString("shader"))
			Misc::throwStdErr("Doom3MD5Mesh::Doom3MD5Mesh: Missing shader name in mesh definition at %s",source.where().c_str());
		m.shader=materialManager.loadMaterial(source.readString().c_str());
		
		/* Read the mesh's vertices: */
		if(!source.isString("numverts"))
			Misc::throwStdErr("Doom3MD5Mesh::Doom3MD5Mesh: Missing vertex list in mesh definition at %s",source.where().c_str());
		m.numVertices=source.readInteger();
		m.vertices=new Mesh::Vertex[m.numVertices];
		for(int vertexIndex=0;vertexIndex<m.numVertices;++vertexIndex)
			{
			Mesh::Vertex& v=m.vertices[vertexIndex];
			
			/* Parse the vertex entry's header: */
			if(!source.isString("vert"))
				Misc::throwStdErr("Doom3MD5Mesh::Doom3MD5Mesh: Malformed vertex definition at %s",source.where().c_str());
			if(source.readInteger()!=vertexIndex)
				Misc::throwStdErr("Doom3MD5Mesh::Doom3MD5Mesh: Mismatching vertex index at %s",source.where().c_str());
			
			/* Read the vertex' texture coordinates: */
			if(source.readChar()!='(')
				Misc::throwStdErr("Doom3MD5Mesh::Doom3MD5Mesh: Malformed vertex texture coordinates at %s",source.where().c_str());
			for(int i=0;i<2;++i)
				v.texCoord[i]=float(source.readNumber());
			if(source.readChar()!=')')
				Misc::throwStdErr("Doom3MD5Mesh::Doom3MD5Mesh: Malformed vertex texture coordinates at %s",source.where().c_str());
			
			/* Read the first weight index and number of weights: */
			v.firstWeightIndex=source.readInteger();
			v.numWeights=source.readInteger();
			}
		
		/* Read the mesh's triangles: */
		if(!source.isString("numtris"))
			Misc::throwStdErr("Doom3MD5Mesh::Doom3MD5Mesh: Missing triangle list in mesh definition at %s",source.where().c_str());
		m.numTriangles=source.readInteger();
		m.triangleVertexIndices=new GLuint[m.numTriangles*3];
		GLuint* tPtr=m.triangleVertexIndices;
		for(int triangleIndex=0;triangleIndex<m.numTriangles;++triangleIndex,tPtr+=3)
			{
			/* Parse the triangle entry's header: */
			if(!source.isString("tri"))
				Misc::throwStdErr("Doom3MD5Mesh::Doom3MD5Mesh: Malformed triangle definition at %s",source.where().c_str());
			if(source.readInteger()!=triangleIndex)
				Misc::throwStdErr("Doom3MD5Mesh::Doom3MD5Mesh: Mismatching triangle index at %s",source.where().c_str());
			
			/* Read the triangle's vertex indices: */
			for(int i=0;i<3;++i)
				tPtr[i]=GLuint(source.readUnsignedInteger());
			Misc::swap(tPtr[1],tPtr[2]);
			}
		
		/* Read the mesh's joint weights: */
		if(!source.isString("numweights"))
			Misc::throwStdErr("Doom3MD5Mesh::Doom3MD5Mesh: Missing joint weight list in mesh definition at %s",source.where().c_str());
		m.numWeights=source.readInteger();
		m.weights=new Mesh::Weight[m.numWeights];
		for(int weightIndex=0;weightIndex<m.numWeights;++weightIndex)
			{
			Mesh::Weight& w=m.weights[weightIndex];
			
			/* Parse the weight entry's header: */
			if(!source.isString("weight"))
				Misc::throwStdErr("Doom3MD5Mesh::Doom3MD5Mesh: Malformed joint weight definition at %s",source.where().c_str());
			if(source.readInteger()!=weightIndex)
				Misc::throwStdErr("Doom3MD5Mesh::Doom3MD5Mesh: Mismatching joint weight index at %s",source.where().c_str());
			
			/* Read the weight's joint index and affine combination weight: */
			w.jointIndex=source.readInteger();
			if(w.jointIndex<0||w.jointIndex>=numJoints)
				Misc::throwStdErr("Doom3MD5Mesh::Doom3MD5Mesh: Joint index out of range in joint weight definition at %s",source.where().c_str());
			w.weight=Scalar(source.readNumber());
			
			/* Read the joint weight's vertex position: */
			if(source.readChar()!='(')
				Misc::throwStdErr("Doom3MD5Mesh::Doom3MD5Mesh: Malformed joint weight position at %s",source.where().c_str());
			for(int i=0;i<3;++i)
				w.position[i]=Scalar(source.readNumber());
			if(source.readChar()!=')')
				Misc::throwStdErr("Doom3MD5Mesh::Doom3MD5Mesh: Malformed joint weight position at %s",source.where().c_str());
			}
		
		/* Check that the joint weights for all vertices add up to one: */
		Mesh::Vertex* vPtr=m.vertices;
		for(int vertexIndex=0;vertexIndex<m.numVertices;++vertexIndex,++vPtr)
			{
			Scalar weightSum=Scalar(0);
			Mesh::Weight* wPtr=&m.weights[vPtr->firstWeightIndex];
			for(int i=0;i<vPtr->numWeights;++i,++wPtr)
				weightSum+=wPtr->weight;
			if(Math::abs(weightSum-Scalar(1))>Scalar(1.0e-6))
				Misc::throwStdErr("Doom3MD5Mesh::Doom3MD5Mesh: Vertex weights add up to %f instead of 1.0 at %s",weightSum,source.where().c_str());
			}
		
		if(source.readChar()!='}')
			Misc::throwStdErr("Doom3MD5Mesh::Doom3MD5Mesh: Malformed mesh definition at %s",source.where().c_str());
		
		/* Compute the initial posed positions for all vertices: */
		m.posedVertices=new Mesh::RenderVertex[m.numVertices];
		poseMesh(m);
		vPtr=m.vertices;
		Mesh::RenderVertex* pvPtr=m.posedVertices;
		for(int vertexIndex=0;vertexIndex<m.numVertices;++vertexIndex,++vPtr,++pvPtr)
			{
			for(int i=0;i<2;++i)
				pvPtr->texCoord[i]=vPtr->texCoord[i];
			pvPtr->normal=Vector::zero;
			for(int i=0;i<2;++i)
				pvPtr->tangents[i]=Vector::zero;
			}
		
		/* Compute normal and tangent vectors for all posed vertices in model space: */
		tPtr=m.triangleVertexIndices;
		for(int triangleIndex=0;triangleIndex<m.numTriangles;++triangleIndex,tPtr+=3)
			{
			Mesh::RenderVertex* pvs[3];
			for(int i=0;i<3;++i)
				pvs[i]=&m.posedVertices[tPtr[i]];
			Vector d1=pvs[1]->position-pvs[0]->position;
			Vector d2=pvs[2]->position-pvs[0]->position;
			
			/* Calculate the triangle's normal vector: */
			Vector triangleNormal=d1^d2;
			triangleNormal.normalize();
			
			/* Calculate the triangle's tangent vectors: */
			Geometry::Matrix<Scalar,2,2> T;
			for(int i=0;i<2;++i)
				for(int j=0;j<2;++j)
					T(i,j)=pvs[j+1]->texCoord[i]-pvs[0]->texCoord[i];
			T=Geometry::invert(T);
			Vector triangleTangents[2];
			for(int i=0;i<2;++i)
				triangleTangents[i]=d1*T(0,i)+d2*T(1,i);
			
			/* Accumulate the normal and tangent vectors in the triangle's vertices: */
			for(int i=0;i<3;++i)
				{
				/* Calculate the vertex' accumulation weight, proportional to triangle angle at vertex: */
				Vector dp=pvs[(i+1)%3]->position-pvs[i]->position;
				Vector dm=pvs[(i+2)%3]->position-pvs[i]->position;
				Scalar angle=Math::acos((dp*dm)/(Geometry::mag(dp)*Geometry::mag(dm)));
				
				/* Accumulate the vertex' normal and tangent: */
				pvs[i]->normal+=triangleNormal*angle;
				for(int j=0;j<2;++j)
					pvs[i]->tangents[j]+=triangleTangents[j]*angle;
				}
			}
		
		/* Finalize the posed vertices and store their normals and tangents in joint space with each weight: */
		vPtr=m.vertices;
		pvPtr=m.posedVertices;
		for(int vertexIndex=0;vertexIndex<m.numVertices;++vertexIndex,++vPtr,++pvPtr)
			{
			/* Finalize the posed vertex: */
			pvPtr->normal.normalize();
			for(int i=0;i<2;++i)
				{
				pvPtr->tangents[i]-=pvPtr->normal*(pvPtr->tangents[i]*pvPtr->normal);
				pvPtr->tangents[i].normalize();
				}
			
			/* Store normal and tangent with each weight for this vertex: */
			Mesh::Weight* wPtr=&m.weights[vPtr->firstWeightIndex];
			for(int weightIndex=0;weightIndex<vPtr->numWeights;++weightIndex,++wPtr)
				{
				const Joint& j=joints[wPtr->jointIndex];
				
				/* Transform the vertex normal and tangent to joint space and store them: */
				wPtr->normal=j.transform.inverseTransform(pvPtr->normal);
				for(int i=0;i<2;++i)
					wPtr->tangents[i]=j.transform.inverseTransform(pvPtr->tangents[i]);
				}
			}
		}
	
	GLObject::init();
	}

Doom3MD5Mesh::~Doom3MD5Mesh(void)
	{
	delete[] joints;
	delete[] meshes;
	}

void Doom3MD5Mesh::initContext(GLContextData& contextData) const
	{
	/* Create the data item and store it in the OpenGL context: */
	DataItem* dataItem=new DataItem(numMeshes);
	contextData.addDataItem(this,dataItem);
	
	/* Initialize the meshes: */
	if(dataItem->hasVertexBufferObjectExtension)
		{
		const Mesh* mPtr=meshes;
		for(int meshIndex=0;meshIndex<numMeshes;++meshIndex,++mPtr)
			{
			/* Allocate the vertex buffer: */
			glBindBufferARB(GL_ARRAY_BUFFER_ARB,dataItem->meshVertexBufferObjectIds[meshIndex]);
			glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
			
			/* Allocate the index buffer and upload the index data: */
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,dataItem->meshIndexBufferObjectIds[meshIndex]);
			glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,mPtr->numTriangles*3*sizeof(GLuint),mPtr->triangleVertexIndices,GL_STATIC_DRAW_ARB);
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,0);
			}
		}
	}

Doom3MD5Mesh::JointID Doom3MD5Mesh::findJoint(const char* jointName) const
	{
	int foundIndex=-1;
	for(int i=0;i<numJoints;++i)
		{
		if(strcmp(jointName,joints[i].name)==0)
			{
			foundIndex=i;
			break;
			}
		}
	if(foundIndex>=0)
		return JointID(this,foundIndex);
	else
		return JointID();
	}

Doom3MD5Mesh::JointID Doom3MD5Mesh::pickJoint(const Doom3MD5Mesh::Point& position,Doom3MD5Mesh::Scalar maxDist) const
	{
	int foundIndex=-1;
	Scalar maxDist2=Math::sqr(maxDist);
	for(int i=0;i<numJoints;++i)
		{
		Scalar dist2=Geometry::dist(position,joints[i].transform.getOrigin());
		if(maxDist2>dist2)
			{
			foundIndex=i;
			maxDist2=dist2;
			}
		}
	if(foundIndex>=0)
		return JointID(this,foundIndex);
	else
		return JointID();
	}

Doom3MD5Mesh::JointID Doom3MD5Mesh::pickJoint(const Doom3MD5Mesh::Ray& ray,Doom3MD5Mesh::Scalar cosMaxAngle) const
	{
	int foundIndex=-1;
	Scalar maxParameter=Math::Constants<Scalar>::max;
	Vector dir=ray.getDirection();
	dir.normalize();
	for(int i=0;i<numJoints;++i)
		{
		Vector dist=joints[i].transform.getOrigin()-ray.getOrigin();
		Scalar parameter=dist*dir;
		if(parameter<maxParameter)
			{
			Scalar cosAngle=parameter/Geometry::mag(dist);
			if(cosAngle>=cosMaxAngle)
				{
				foundIndex=i;
				maxParameter=parameter;
				}
			}
		}
	if(foundIndex>=0)
		return JointID(this,foundIndex);
	else
		return JointID();
	}

void Doom3MD5Mesh::setJointTransform(const Doom3MD5Mesh::JointID& jointID,const Doom3MD5Mesh::Transform& newTransform,bool cascade)
	{
	if(jointID.mesh!=this)
		Misc::throwStdErr("Doom3MD5Mesh::getJointTransform: Given joint ID not part of mesh");
	Joint& joint=joints[jointID.jointIndex];
	
	/* Calculate the difference between the joint's previous and new transformation: */
	Transform deltaTransform=newTransform;
	deltaTransform*=Geometry::invert(joint.transform);
	
	/* Set the joint's transformation: */
	joint.transform=newTransform;
	
	if(cascade)
		{
		/* Update transformations of all joints that are below the changed joint: */
		for(int i=0;i<numJoints;++i)
			{
			/* Check if this joint is below the changed joint: */
			Joint* j=&joints[i];
			do
				{
				j=j->parent;
				}
			while(j!=0&&j!=&joint);
			if(j!=0)
				{
				/* Update the joint's transformation: */
				joints[i].transform.leftMultiply(deltaTransform);
				joints[i].transform.renormalize();
				}
			}
		}
	
	/* Update the joint tree: */
	++jointTreeVersion;
	}

void Doom3MD5Mesh::updatePose(void)
	{
	/* Check if the current mesh pose is outdated: */
	if(posedVerticesVersion!=jointTreeVersion)
		{
		/* Pose all meshes: */
		Mesh* mPtr=meshes;
		for(int meshIndex=0;meshIndex<numMeshes;++meshIndex,++mPtr)
			poseMesh(*mPtr);
		
		/* Update the mesh pose: */
		posedVerticesVersion=jointTreeVersion;
		}
	}

Doom3MD5Mesh::Box Doom3MD5Mesh::calcBoundingBox(void) const
	{
	Box result=Box::empty;
	
	/* Go through all meshes: */
	const Mesh* mPtr=meshes;
	for(int meshIndex=0;meshIndex<numMeshes;++meshIndex,++mPtr)
		{
		/* Go through the mesh's posed vertices: */
		const Mesh::RenderVertex* pvPtr=mPtr->posedVertices;
		for(int vertexIndex=0;vertexIndex<mPtr->numVertices;++vertexIndex,++pvPtr)
			{
			/* Add the vertex to the bounding box: */
			result.addPoint(pvPtr->position);
			}
		}
	
	return result;
	}

void Doom3MD5Mesh::drawSkeleton(void) const
	{
	/* Draw a point for each joint's position: */
	glColor3f(1.0f,1.0f,0.0f);
	glBegin(GL_POINTS);
	for(int i=0;i<numJoints;++i)
		glVertex(joints[i].transform.getOrigin());
	glEnd();
	
	/* Draw a line segment from each joint's position to its parent's position: */
	glColor3f(1.0f,0.0f,0.0f);
	glBegin(GL_LINES);
	for(int i=0;i<numJoints;++i)
		if(joints[i].parent!=0)
			{
			glVertex(joints[i].transform.getOrigin());
			glVertex(joints[i].parent->transform.getOrigin());
			}
	glEnd();
	}

void Doom3MD5Mesh::drawSurface(GLContextData& contextData,bool useDefaultPipeline) const
	{
	/* Get the data item from the OpenGL context: */
	DataItem* dataItem=contextData.retrieveDataItem<DataItem>(this);
	
	/* Initialize the material manager: */
	Doom3MaterialManager::RenderContext mmRc=materialManager.start(contextData,useDefaultPipeline);
	GLint tangentAttributeIndexS=materialManager.getTangentAttributeIndex(mmRc,0);
	GLint tangentAttributeIndexT=materialManager.getTangentAttributeIndex(mmRc,1);
	
	/* Enable the appropriate vertex arrays: */
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	if(tangentAttributeIndexS>=0)
		glEnableVertexAttribArrayARB(tangentAttributeIndexS);
	if(tangentAttributeIndexT>=0)
		glEnableVertexAttribArrayARB(tangentAttributeIndexT);
	glEnableClientState(GL_VERTEX_ARRAY);
	
	/* Go through all meshes: */
	for(int meshIndex=0;meshIndex<numMeshes;++meshIndex)
		{
		const Mesh& m=meshes[meshIndex];
		
		/* Install the mesh's material and check whether to render this mesh: */
		if(materialManager.setMaterial(mmRc,m.shader))
			{
			const Mesh::RenderVertex* vertexPtr;
			const GLuint* indexPtr;
			if(dataItem->hasVertexBufferObjectExtension)
				{
				/* Bind the mesh's vertex buffer: */
				glBindBufferARB(GL_ARRAY_BUFFER_ARB,dataItem->meshVertexBufferObjectIds[meshIndex]);
				vertexPtr=0;
				
				/* Check if the buffered posed vertices are outdated: */
				if(dataItem->vertexBufferVersion!=posedVerticesVersion)
					{
					/* Upload the new vertex data: */
					glBufferDataARB(GL_ARRAY_BUFFER_ARB,m.numVertices*sizeof(Mesh::RenderVertex),m.posedVertices,GL_DYNAMIC_DRAW_ARB);
					}
				
				/* Bind the mesh's index buffer: */
				glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,dataItem->meshIndexBufferObjectIds[meshIndex]);
				indexPtr=0;
				}
			else
				{
				vertexPtr=m.posedVertices;
				indexPtr=m.triangleVertexIndices;
				}
			
			/* Render the mesh: */
			glTexCoordPointer(2,sizeof(Mesh::RenderVertex),vertexPtr->texCoord);
			glNormalPointer(sizeof(Mesh::RenderVertex),vertexPtr->normal.getComponents());
			if(tangentAttributeIndexS>=0)
				glVertexAttribPointerARB(tangentAttributeIndexS,3,GL_FALSE,sizeof(Mesh::RenderVertex),vertexPtr->tangents[0].getComponents());
			if(tangentAttributeIndexT>=0)
				glVertexAttribPointerARB(tangentAttributeIndexT,3,GL_FALSE,sizeof(Mesh::RenderVertex),vertexPtr->tangents[1].getComponents());
			glVertexPointer(3,sizeof(Mesh::RenderVertex),vertexPtr->position.getComponents());
			glDrawElements(GL_TRIANGLES,m.numTriangles*3,GL_UNSIGNED_INT,indexPtr);
			}
		}
	
	if(dataItem->hasVertexBufferObjectExtension)
		{
		dataItem->vertexBufferVersion=posedVerticesVersion;
		
		/* Unbind all buffers: */
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,0);
		}
	
	/* Disable the appropriate vertex arrays: */
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	if(tangentAttributeIndexS>=0)
		glDisableVertexAttribArrayARB(tangentAttributeIndexS);
	if(tangentAttributeIndexT>=0)
		glDisableVertexAttribArrayARB(tangentAttributeIndexT);
	glDisableClientState(GL_VERTEX_ARRAY);
	
	/* Shut down the material manager: */
	materialManager.finish(mmRc);
	}

void Doom3MD5Mesh::drawSurfaceWireframe(GLContextData& contextData) const
	{
	/* Get the data item from the OpenGL context: */
	DataItem* dataItem=contextData.retrieveDataItem<DataItem>(this);
	
	/* Enable the appropriate vertex arrays: */
	glEnableClientState(GL_VERTEX_ARRAY);
	
	/* Go through all meshes: */
	const Mesh* mPtr=meshes;
	for(int meshIndex=0;meshIndex<numMeshes;++meshIndex,++mPtr)
		{
		const Mesh::RenderVertex* vertexPtr;
		if(dataItem->hasVertexBufferObjectExtension)
			{
			/* Bind the mesh's vertex buffer: */
			glBindBufferARB(GL_ARRAY_BUFFER_ARB,dataItem->meshVertexBufferObjectIds[meshIndex]);
			vertexPtr=0;
			
			/* Check if the buffered posed vertices are outdated: */
			if(dataItem->vertexBufferVersion!=posedVerticesVersion)
				{
				/* Upload the new vertex data: */
				glBufferDataARB(GL_ARRAY_BUFFER_ARB,mPtr->numVertices*sizeof(Mesh::RenderVertex),mPtr->posedVertices,GL_DYNAMIC_DRAW_ARB);
				}
			
			/* Bind the mesh's index buffer: */
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,dataItem->meshIndexBufferObjectIds[meshIndex]);
			}
		else
			vertexPtr=mPtr->posedVertices;
		
		/* Render the mesh: */
		glVertexPointer(3,sizeof(Mesh::RenderVertex),vertexPtr->position.getComponents());
		glBegin(GL_LINES);
		const GLuint* tPtr=mPtr->triangleVertexIndices;
		for(int triangleIndex=0;triangleIndex<mPtr->numTriangles;++triangleIndex,tPtr+=3)
			{
			glVertex(mPtr->posedVertices[tPtr[0]].position);
			glVertex(mPtr->posedVertices[tPtr[1]].position);
			glVertex(mPtr->posedVertices[tPtr[1]].position);
			glVertex(mPtr->posedVertices[tPtr[2]].position);
			glVertex(mPtr->posedVertices[tPtr[2]].position);
			glVertex(mPtr->posedVertices[tPtr[0]].position);
			}
		glEnd();
		}
	
	if(dataItem->hasVertexBufferObjectExtension)
		{
		dataItem->vertexBufferVersion=posedVerticesVersion;
		
		/* Unbind all buffers: */
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,0);
		}
	
	/* Disable the appropriate vertex arrays: */
	glDisableClientState(GL_VERTEX_ARRAY);
	}

void Doom3MD5Mesh::drawNormals(GLContextData& contextData,Doom3MD5Mesh::Scalar scale) const
	{
	/* Go through all meshes: */
	for(int meshIndex=0;meshIndex<numMeshes;++meshIndex)
		{
		const Mesh& m=meshes[meshIndex];
		
		/* Draw coordinate frames for all the mesh's vertices: */
		glBegin(GL_LINES);
		const Mesh::RenderVertex* pvPtr=m.posedVertices;
		for(int vertexIndex=0;vertexIndex<m.numVertices;++vertexIndex,++pvPtr)
			{
			/* Render the coordinate frame: */
			glColor3f(0.0f,0.0f,1.0f);
			glVertex(pvPtr->position);
			glVertex(pvPtr->position+pvPtr->normal*scale);
			glColor3f(1.0f,0.0f,0.0f);
			glVertex(pvPtr->position);
			glVertex(pvPtr->position+pvPtr->tangents[0]*scale);
			glColor3f(0.0f,1.0f,0.0f);
			glVertex(pvPtr->position);
			glVertex(pvPtr->position+pvPtr->tangents[1]*scale);
			}
		glEnd();
		}
	}

}
