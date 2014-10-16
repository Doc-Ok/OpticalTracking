/***********************************************************************
IndexedLineSetNode - Class for sets of lines or polylines as renderable
geometry.
Copyright (c) 2009 Oliver Kreylos

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

#ifndef SCENEGRAPH_INDEXEDLINESETNODE_INCLUDED
#define SCENEGRAPH_INDEXEDLINESETNODE_INCLUDED

#include <vector>
#include <GL/gl.h>
#include <GL/GLObject.h>
#include <SceneGraph/FieldTypes.h>
#include <SceneGraph/GeometryNode.h>
#include <SceneGraph/ColorNode.h>
#include <SceneGraph/CoordinateNode.h>

namespace SceneGraph {

class IndexedLineSetNode:public GeometryNode,public GLObject
	{
	/* Embedded classes: */
	public:
	typedef SF<ColorNodePointer> SFColorNode;
	typedef SF<CoordinateNodePointer> SFCoordinateNode;
	
	/* Elements: */
	
	protected:
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		public:
		GLuint vertexBufferObjectId; // ID of vertex buffer object containing the vertices, if supported
		unsigned int version; // Version of point set stored in vertex buffer object
		
		/* Constructors and destructors: */
		DataItem(void);
		virtual ~DataItem(void);
		};
	
	/* Fields: */
	public:
	SFColorNode color;
	SFCoordinateNode coord;
	MFInt colorIndex;
	SFBool colorPerVertex;
	MFInt coordIndex;
	SFFloat lineWidth;
	
	/* Derived state: */
	protected:
	std::vector<GLsizei> numVertices; // List of numbers of vertices for each line in the line set
	size_t totalNumVertices; // Total number of vertices in the line set
	unsigned int version; // Version number of indexed line set
	
	/* Protected methods: */
	void uploadColoredLineSet(DataItem* dataItem) const;
	void uploadLineSet(DataItem* dataItem) const;
	
	/* Constructors and destructors: */
	public:
	IndexedLineSetNode(void); // Creates a default line set
	
	/* Methods from Node: */
	static const char* getStaticClassName(void);
	virtual const char* getClassName(void) const;
	virtual void parseField(const char* fieldName,VRMLFile& vrmlFile);
	virtual void update(void);
	
	/* Methods from GeometryNode: */
	virtual Box calcBoundingBox(void) const;
	virtual void glRenderAction(GLRenderState& renderState) const;
	
	/* Methods from GLObject: */
	virtual void initContext(GLContextData& contextData) const;
	};

typedef Misc::Autopointer<IndexedLineSetNode> IndexedLineSetNodePointer;

}

#endif
