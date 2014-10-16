/***********************************************************************
QuadSetNode - Class for sets of quadrilaterals as renderable
geometry.
Copyright (c) 2011-2013 Oliver Kreylos

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

#ifndef SCENEGRAPH_QUADSETNODE_INCLUDED
#define SCENEGRAPH_QUADSETNODE_INCLUDED

#include <vector>
#include <GL/gl.h>
#include <GL/GLObject.h>
#include <SceneGraph/FieldTypes.h>
#include <SceneGraph/GeometryNode.h>
#include <SceneGraph/CoordinateNode.h>

namespace SceneGraph {

class QuadSetNode:public GeometryNode,public GLObject
	{
	/* Embedded classes: */
	public:
	typedef SF<CoordinateNodePointer> SFCoordinateNode;
	
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		public:
		GLuint vertexBufferObjectId; // ID of vertex buffer holding quad vertices
		GLuint indexBufferObjectId; // ID of index buffer holding quad / quad strip vertex indices
		unsigned int version; // Version of face set stored in vertex buffer object
		
		/* Constructors and destructors: */
		DataItem(void);
		virtual ~DataItem(void);
		};
	
	/* Elements: */
	
	/* Fields: */
	public:
	SFCoordinateNode coord;
	SFBool ccw;
	SFBool solid;
	SFInt subdivideX;
	SFInt subdivideY;
	
	/* Derived state: */
	private:
	bool inited; // Flag whether GLObject::init() has already been called
	unsigned int numQuads; // Number of quads defined in the quad set
	unsigned int version; // Version of quad se defined by fields
	
	/* Private methods: */
	void uploadQuads(DataItem* dataItem) const; // Uploads the quad set state defined by the fields to the graphics card
	
	/* Constructors and destructors: */
	public:
	QuadSetNode(void); // Creates a default quad set
	
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

}

#endif
