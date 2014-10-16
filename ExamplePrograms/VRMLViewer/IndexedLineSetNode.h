/***********************************************************************
IndexedLineSetNode - Class for shapes represented as sets of polylines.
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

#ifndef INDEXEDLINESETNODE_INCLUDED
#define INDEXEDLINESETNODE_INCLUDED

#include <vector>
#include <GL/gl.h>
#include <GL/GLVertex.h>

#include "Types.h"

#include "GeometryNode.h"

class IndexedLineSetNode:public GeometryNode
	{
	/* Embedded classes: */
	private:
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		public:
		GLuint vertexBufferObjectId; // ID of the buffer holding the vertex data
		GLuint indexBufferObjectId; // ID of the buffer holding the index data
		std::vector<GLsizei> numLineStripVertices; // Array of numbers of vertices for each polyline
		
		/* Constructors and destructors: */
		DataItem(void);
		virtual ~DataItem(void);
		};
	
	typedef GLVertex<void,0,GLubyte,4,void,GLfloat,3> Vertex; // Type for vertices
	
	/* Elements: */
	Bool colorPerVertex; // Flag if the shape uses per-vertex or per-line colors
	VRMLNodePointer color; // Pointer to the node holding vertex colors
	VRMLNodePointer coord; // Pointer to the node holding vertex coordinates
	std::vector<Int32> colorIndices; // Array of vertex color indices for each line
	std::vector<Int32> coordIndices; // Array of vertex coordinate indices for each line
	
	/* Constructors and destructors: */
	public:
	IndexedLineSetNode(VRMLParser& parser);
	virtual ~IndexedLineSetNode(void);
	
	/* Methods: */
	virtual void initContext(GLContextData& contextData) const;
	virtual VRMLNode::Box calcBoundingBox(void) const;
	virtual void glRenderAction(VRMLRenderState& renderState) const;
	};

#endif
