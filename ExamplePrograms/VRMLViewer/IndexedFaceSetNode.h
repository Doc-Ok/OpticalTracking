/***********************************************************************
IndexedFaceSetNode - Class for shapes represented as sets of faces.
Copyright (c) 2006-2008 Oliver Kreylos

This file is part of the Virtual Reality VRML viewer (VRMLViewer).

The Virtual Reality VRML viewer is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Virtual Reality VRML viewer is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Virtual Reality VRML viewer; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef INDEXEDFACESETNODE_INCLUDED
#define INDEXEDFACESETNODE_INCLUDED

#include <vector>
#include <GL/gl.h>
#include <GL/GLVertex.h>

#include "Types.h"

#include "GeometryNode.h"

class IndexedFaceSetNode:public GeometryNode
	{
	/* Embedded classes: */
	private:
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		public:
		GLuint vertexBufferObjectId; // ID of the buffer holding the vertex data
		GLuint indexBufferObjectId; // ID of the buffer holding the index data
		size_t numTriangles; // Number of triangles in the index buffer
		
		/* Constructors and destructors: */
		DataItem(void);
		virtual ~DataItem(void);
		};
	
	typedef GLVertex<GLfloat,2,GLubyte,4,GLfloat,GLfloat,3> Vertex; // Type for vertices
	
	/* Elements: */
	Bool ccw; // Flag if the shape's faces are oriented counter-clockwise
	Bool solid; // Flag if the shape can be considered solid
	Bool convex; // Flag if all faces can be considered convex
	Bool colorPerVertex; // Flag if the shape uses per-vertex or per-face colors
	Bool normalPerVertex; // Flag if the shape uses per-vertex or per-face normal vectors
	Float creaseAngle; // Crease angle for automatic normal vector calculation
	VRMLNodePointer texCoord; // Pointer to the node holding vertex texture coordinates
	VRMLNodePointer color; // Pointer to the node holding vertex colors
	VRMLNodePointer normal; // Pointer to the node holding vertex normal vectors
	VRMLNodePointer coord; // Pointer to the node holding vertex coordinates
	std::vector<Int32> texCoordIndices; // Array of vertex texture coordinate indices for each face
	std::vector<Int32> colorIndices; // Array of vertex color indices for each face
	std::vector<Int32> normalIndices; // Array of vertex normal vector indices for each face
	std::vector<Int32> coordIndices; // Array of vertex coordinate indices for each face
	
	/* Constructors and destructors: */
	public:
	IndexedFaceSetNode(VRMLParser& parser);
	virtual ~IndexedFaceSetNode(void);
	
	/* Methods: */
	virtual void initContext(GLContextData& contextData) const;
	virtual VRMLNode::Box calcBoundingBox(void) const;
	virtual void glRenderAction(VRMLRenderState& renderState) const;
	};

#endif
