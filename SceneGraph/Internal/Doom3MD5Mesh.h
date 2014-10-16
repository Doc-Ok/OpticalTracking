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

#ifndef SCENEGRAPH_INTERNAL_DOOM3MD5MESH_INCLUDED
#define SCENEGRAPH_INTERNAL_DOOM3MD5MESH_INCLUDED

#include <Misc/ThrowStdErr.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/Ray.h>
#include <Geometry/Box.h>
#include <Geometry/OrthonormalTransformation.h>
#include <GL/gl.h>
#include <GL/GLObject.h>
#include <SceneGraph/Internal/Doom3MaterialManager.h>

/* Forward declarations: */
template <class TexCoordScalarParam,GLsizei numTexCoordComponentsParam,
          class ColorScalarParam,GLsizei numColorComponentsParam,
          class NormalScalarParam,
          class PositionScalarParam,GLsizei numPositionComponentsParam>
struct GLVertex;
namespace SceneGraph {
class Doom3FileManager;
class Doom3MD5Anim;
}

namespace SceneGraph {

class Doom3MD5Mesh:public GLObject
	{
	friend class Doom3MD5Anim;
	
	/* Embedded classes: */
	public:
	typedef float Scalar;
	typedef Geometry::Point<Scalar,3> Point;
	typedef Geometry::Vector<Scalar,3> Vector;
	typedef Geometry::Ray<Scalar,3> Ray;
	typedef Geometry::Box<Scalar,3> Box;
	typedef Geometry::OrthonormalTransformation<Scalar,3> Transform; // Type for joint transformations
	
	private:
	struct Joint // Structure to represent individual joints in the mesh's skeleton
		{
		/* Elements: */
		public:
		char* name; // Joint's name as read from the model file
		Joint* parent; // Pointer to joint's parent (NULL if root joint)
		Transform transform; // Joint's transformation relative to its parent
		
		/* Constructors and destructors: */
		Joint(void)
			:name(0),parent(0)
			{
			};
		~Joint(void)
			{
			delete[] name;
			};
		};
	
	public:
	class JointID // Class to identify joints in a model
		{
		friend class Doom3MD5Mesh;
		
		/* Elements: */
		private:
		const Doom3MD5Mesh* mesh; // Pointer to the mesh containing the joint
		int jointIndex; // Index of the identified joint in the mesh
		
		/* Constructors and destructors: */
		public:
		JointID(void) // Creates an invalid joint ID
			:mesh(0),jointIndex(-1)
			{
			};
		private:
		JointID(const Doom3MD5Mesh* sMesh,int sJointIndex) // Creates an ID for the given joint
			:mesh(sMesh),jointIndex(sJointIndex)
			{
			};
		
		/* Methods: */
		public:
		bool isValid(void) const // Returns true if the joint is valid
			{
			return mesh!=0;
			};
		};
	
	private:
	struct Mesh // Structure to represent surface meshes attached to the skeleton
		{
		/* Embedded classes: */
		public:
		struct Vertex // Structure for mesh vertices
			{
			/* Elements: */
			public:
			float texCoord[2]; // Vertex texture coordinates
			int firstWeightIndex; // Index of first joint weight for this vertex
			int numWeights; // Number of joint weights for this vertex
			};
		
		struct Weight // Structure for joint weights
			{
			/* Elements: */
			public:
			int jointIndex; // Index of joint this weight is associated with
			Scalar weight; // Affine combination weight for this joint weight
			Vector normal; // Vertex normal vector in joint's coordinate system
			Vector tangents[2]; // Vertex tangent vectors for s and t in joint's coordinate system
			Point position; // Vertex position in joint's coordinate system
			};
		
		struct RenderVertex // Structure containing vertex data required for rendering
			{
			/* Elements: */
			public:
			float texCoord[2]; // Texture coordinates
			Vector normal; // Normal vector
			Vector tangents[2]; // Tangent vectors for s and t
			Point position; // Vertex position
			};
		
		/* Elements: */
		Doom3MaterialManager::MaterialID shader; // Material ID of the shader for this mesh
		int numVertices; // Number of vertices in this mesh
		Vertex* vertices; // Array of vertices in this mesh
		int numTriangles; // Number of triangles in this mesh
		GLuint* triangleVertexIndices; // Array of three vertex indices for each triangle
		int numWeights; // Number of joint weights in this mesh
		Weight* weights; // Array of joint weights in this mesh
		RenderVertex* posedVertices; // Array of vertices in the current pose
		
		/* Constructors and destructors: */
		Mesh(void)
			:numVertices(0),vertices(0),
			 numTriangles(0),triangleVertexIndices(0),
			 numWeights(0),weights(0),
			 posedVertices(0)
			{
			};
		~Mesh(void)
			{
			delete[] vertices;
			delete[] triangleVertexIndices;
			delete[] weights;
			delete[] posedVertices;
			};
		};
	
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		public:
		bool hasVertexBufferObjectExtension; // Flag if the local OpenGL supports ARB vertex buffer objects
		int numMeshes; // Number of meshes in the skeleton
		GLuint* meshVertexBufferObjectIds; // Array of vertex buffer objects for the skeleton's meshes
		GLuint* meshIndexBufferObjectIds; // Array of triangle index buffer objects for the skeleton's meshes
		unsigned int vertexBufferVersion; // Version number of the vertex buffer
		
		/* Constructors and destructors: */
		DataItem(int sNumMeshes);
		virtual ~DataItem(void);
		};
	
	/* Elements: */
	private:
	Doom3MaterialManager& materialManager;
	int numJoints; // Total number of joints in the skeleton
	Joint* joints; // Array containing the skeleton's joint tree
	int numMeshes; // Total number of meshes associated with the skeleton
	Mesh* meshes; // Array containing the meshes associated with the skeleton
	unsigned int jointTreeVersion; // Version number of the joint tree settings
	unsigned int posedVerticesVersion; // Version number of the posed mesh vertices
	
	/* Private methods: */
	void poseMesh(Mesh& mesh); // Poses the given mesh according to the current joint transformations
	
	/* Constructors and destructors: */
	public:
	Doom3MD5Mesh(Doom3FileManager& fileManager,Doom3MaterialManager& sMaterialManager,const char* meshFileName); // Creates a mesh by parsing a mesh file in Doom 3's MD5 format
	virtual ~Doom3MD5Mesh(void); // Destroys a mesh
	
	/* Methods from GLObject: */
	virtual void initContext(GLContextData& contextData) const; // Initializes the mesh's OpenGL state
	
	/* New methods: */
	JointID findJoint(const char* jointName) const; // Returns an ID of the joint of the given name
	JointID pickJoint(const Point& position,Scalar maxDist) const; // Returns an ID of the joint touched by the given position
	JointID pickJoint(const Ray& ray,Scalar cosMaxAngle) const; // Returns an ID of the first joint intersected by a cone along the given ray
	const char* getJointName(const JointID& jointID) const // Returns the name of the given joint
		{
		if(jointID.mesh!=this)
			Misc::throwStdErr("Doom3MD5Mesh::getJointTransform: Given joint ID not part of mesh");
		return joints[jointID.jointIndex].name;
		};
	const Transform& getJointTransform(const JointID& jointID) const // Returns the transformation of the given joint
		{
		if(jointID.mesh!=this)
			Misc::throwStdErr("Doom3MD5Mesh::getJointTransform: Given joint ID not part of mesh");
		return joints[jointID.jointIndex].transform;
		};
	void setJointTransform(const JointID& jointID,const Transform& newTransform,bool cascade =true); // Sets the transformation of the given joint; applies transformation to children if cascade is true
	void updatePose(void); // Updates the mesh's pose according to the most recent joint angles
	Box calcBoundingBox(void) const; // Returns a bounding box of the mesh surface as currently posed
	void drawSkeleton(void) const; // Draws the mesh's skeleton as a tree of line segments
	void drawSurface(GLContextData& contextData,bool useDefaultPipeline) const; // Draws the mesh as a shaded surface
	void drawSurfaceWireframe(GLContextData& contextData) const; // Draws the mesh as a wireframe model
	void drawNormals(GLContextData& contextData,Scalar scale) const; // Draws the mesh's normal and tangent vectors
	};

}

#endif
