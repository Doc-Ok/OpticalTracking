/***********************************************************************
PointSetNode - Class for shapes represented as sets of points.
Copyright (c) 2008 Oliver Kreylos

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

#ifndef POINTSETNODE_INCLUDED
#define POINTSETNODE_INCLUDED

#include <GL/gl.h>
#include <GL/GLVertex.h>

#include "Types.h"

#include "GeometryNode.h"

class PointSetNode:public GeometryNode
	{
	/* Embedded classes: */
	private:
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		public:
		GLuint vertexBufferObjectId; // ID of the buffer holding the vertex data
		
		/* Constructors and destructors: */
		DataItem(void);
		virtual ~DataItem(void);
		};
	
	typedef GLVertex<GLvoid,0,GLubyte,4,GLvoid,GLfloat,3> Vertex; // Type for vertices
	
	/* Elements: */
	Float pointSize; // Rendered point size in pixels
	VRMLNodePointer color; // Pointer to the node holding vertex colors
	VRMLNodePointer coord; // Pointer to the node holding vertex coordinates
	
	/* Constructors and destructors: */
	public:
	PointSetNode(VRMLParser& parser);
	virtual ~PointSetNode(void);
	
	/* Methods: */
	virtual void initContext(GLContextData& contextData) const;
	virtual VRMLNode::Box calcBoundingBox(void) const;
	virtual void glRenderAction(VRMLRenderState& renderState) const;
	};

#endif
