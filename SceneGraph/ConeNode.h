/***********************************************************************
ConeNode - Class for upright circular cones as renderable geometry.
Copyright (c) 2009-2010 Oliver Kreylos

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

#ifndef SCENEGRAPH_CONENODE_INCLUDED
#define SCENEGRAPH_CONENODE_INCLUDED

#include <SceneGraph/FieldTypes.h>
#include <SceneGraph/DisplayList.h>
#include <SceneGraph/GeometryNode.h>

namespace SceneGraph {

class ConeNode:public GeometryNode,public DisplayList
	{
	/* Elements: */
	
	/* Fields: */
	public:
	SFFloat height;
	SFFloat bottomRadius;
	SFInt numSegments;
	SFBool side;
	SFBool bottom;
	
	/* Protected methods from DisplayList: */
	protected:
	virtual void createList(GLContextData& contextData) const;
	
	/* Constructors and destructors: */
	public:
	ConeNode(void); // Creates a default cone
	
	/* Methods from Node: */
	static const char* getStaticClassName(void);
	virtual const char* getClassName(void) const;
	virtual EventOut* getEventOut(const char* fieldName) const;
	virtual EventIn* getEventIn(const char* fieldName);
	virtual void parseField(const char* fieldName,VRMLFile& vrmlFile);
	virtual void update(void);
	
	/* Methods from GeometryNode: */
	virtual Box calcBoundingBox(void) const;
	virtual void glRenderAction(GLRenderState& renderState) const;
	};

}

#endif
