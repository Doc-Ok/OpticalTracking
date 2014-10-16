/***********************************************************************
BillboardNode - Class for group nodes that transform their children to
always face the viewer.
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

#ifndef SCENEGRAPH_BILLBOARDNODE_INCLUDED
#define SCENEGRAPH_BILLBOARDNODE_INCLUDED

#include <Geometry/Vector.h>
#include <SceneGraph/FieldTypes.h>
#include <SceneGraph/GroupNode.h>

namespace SceneGraph {

class BillboardNode:public GroupNode
	{
	/* Elements: */
	
	/* Fields: */
	public:
	SFVector axisOfRotation;
	
	/* Derived elements: */
	protected:
	Scalar aor2; // Squared length of rotation axis
	Vector orthoZAxis; // Billboard's Z axis orthonormalized with regard to the axis of rotation
	Vector rotationNormal; // Vector normal to the plane spanned by the rotation axis and the orthonormalized Z axis
	
	/* Constructors and destructors: */
	public:
	BillboardNode(void); // Creates an empty billboard node
	
	/* Methods from Node: */
	static const char* getStaticClassName(void);
	virtual const char* getClassName(void) const;
	virtual EventOut* getEventOut(const char* fieldName) const;
	virtual EventIn* getEventIn(const char* fieldName);
	virtual void parseField(const char* fieldName,VRMLFile& vrmlFile);
	virtual void update(void);
	
	/* Methods from GraphNode: */
	virtual void glRenderAction(GLRenderState& renderState) const;
	};

}

#endif
