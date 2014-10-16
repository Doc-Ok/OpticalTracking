/***********************************************************************
CurveSetNode - Class for sets of curves written by curve tracing
application.
Copyright (c) 2009-2013 Oliver Kreylos

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

#ifndef SCENEGRAPH_CURVESETNODE_INCLUDED
#define SCENEGRAPH_CURVESETNODE_INCLUDED

#include <vector>
#include <GL/gl.h>
#include <GL/GLObject.h>
#include <GL/GLLineLightingShader.h>
#include <SceneGraph/Geometry.h>
#include <SceneGraph/FieldTypes.h>
#include <SceneGraph/GeometryNode.h>

/* Forward declarations: */
namespace Cluster {
class Multiplexer;
}

namespace SceneGraph {

class CurveSetNode:public GeometryNode,public GLObject
	{
	/* Elements: */
	
	protected:
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		public:
		GLuint vertexBufferObjectId; // ID of vertex buffer object containing the vertices, if supported
		unsigned int version; // Version of curve set stored in vertex buffer object
		GLLineLightingShader lineLightingShader; // Shader to illuminate curves using Phong's model
		
		/* Constructors and destructors: */
		DataItem(GLContextData& contextData);
		virtual ~DataItem(void);
		};
	
	/* Fields: */
	public:
	MFString url;
	SFColor color;
	SFFloat lineWidth;
	SFFloat pointSize;
	
	/* Derived state: */
	protected:
	Cluster::Multiplexer* multiplexer; // Multiplexer to read curve files in a cluster environment
	std::vector<GLsizei> numVertices; // Array of numbers of vertices for each curve
	std::vector<Point> vertices; // Array of vertices for all curves
	unsigned int version; // Version number of curve set
	
	/* Constructors and destructors: */
	public:
	CurveSetNode(void); // Creates an empty curve set
	
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
