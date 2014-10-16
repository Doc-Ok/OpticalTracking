/***********************************************************************
Doom3Model - Class to represent static models using Doom3 materials and
shaders.
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

#ifndef SCENEGRAPH_INTERNAL_DOOM3MODEL_INCLUDED
#define SCENEGRAPH_INTERNAL_DOOM3MODEL_INCLUDED

#include <string>
#include <vector>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/Ray.h>
#include <Geometry/Box.h>
#include <GL/gl.h>
#include <GL/GLObject.h>
#include <SceneGraph/Internal/Doom3MaterialManager.h>

namespace SceneGraph {

class Doom3Model:public GLObject
	{
	/* Embedded classes: */
	public:
	typedef float Scalar;
	typedef Geometry::Point<Scalar,3> Point;
	typedef Geometry::Point<Scalar,2> TexturePoint;
	typedef Geometry::Vector<Scalar,3> Vector;
	typedef Geometry::Ray<Scalar,3> Ray;
	typedef Geometry::Box<Scalar,3> Box;
	
	struct Vertex // Structure containing vertex data
		{
		/* Elements: */
		public:
		TexturePoint texCoord; // Texture coordinates
		Vector normal; // Normal vector
		Vector tangents[2]; // Tangent vectors in texture's s and t directions, respectively
		Point position; // Vertex position
		};
	
	struct Surface // Structure to represent triangulated surfaces sharing the same material
		{
		/* Elements: */
		public:
		Doom3MaterialManager::MaterialID material; // Material to render this surface
		int firstVertex; // Index of first vertex in the surface
		int numVertices; // Number of vertices in the surface
		int firstVertexIndex; // Index of first vertex index in the surface
		int numVertexIndices; // Number of vertex indices in the surface
		};
	
	private:
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		public:
		bool hasVertexBufferExtension; // Flag if the local OpenGL supports vertex buffer objects
		GLuint vertexBufferId; // ID of buffer object for vertex data
		GLuint indexBufferId; // ID of buffer object for index data
		
		/* Constructors and destructors: */
		DataItem(void);
		virtual ~DataItem(void);
		};
	
	/* Elements: */
	Doom3MaterialManager& materialManager; // Reference to the material manager
	std::string name; // Name of the model
	public:
	std::vector<Vertex> vertices; // List of all vertices in the model
	std::vector<GLuint> vertexIndices; // List of all vertex indices in the model
	std::vector<Surface> surfaces; // List of all surfaces in the model
	Box boundingBox; // The model's bounding box
	
	/* Private methods: */
	void calcNormalVectors(void); // Calculates normal vectors for all model vertices
	void calcTangentVectors(void); // Calculates tangent vectors for all model vertices
	
	/* Constructors and destructors: */
	Doom3Model(Doom3MaterialManager& sMaterialManager,std::string sName); // Creates an empty model of the given name
	
	/* Methods: */
	virtual void initContext(GLContextData& contextData) const;
	const std::string& getName(void) const // Returns the model's name
		{
		return name;
		};
	const Box& getBoundingBox(void) const // Returns the model's bounding box
		{
		return boundingBox;
		};
	void finalizeVertices(bool calcNormals,bool calcTangents); // Calculates appropriate normal and tangent vectors for all vertices if the respective parameters are true
	void glRenderAction(GLContextData& contextData,SceneGraph::Doom3MaterialManager::RenderContext& mmRc) const; // Renders the model
	void drawNormals(GLContextData& contextData,Scalar scale) const; // Renders the model's normal and tangent vectors
	};

}

#endif
