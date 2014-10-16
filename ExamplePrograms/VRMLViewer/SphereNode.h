/***********************************************************************
SphereNode - Node class for sphere shapes.
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

#ifndef SPHERENODE_INCLUDED
#define SPHERENODE_INCLUDED

#include <GL/gl.h>

#include "Types.h"

#include "GeometryNode.h"

class SphereNode:public GeometryNode
	{
	/* Embedded classes: */
	private:
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		public:
		GLuint displayListId; // ID of display list containing the sphere geometry
		
		/* Constructors and destructors: */
		DataItem(void);
		virtual ~DataItem(void);
		};
	
	/* Elements: */
	Float radius; // Sphere's radius
	
	/* Constructors and destructors: */
	public:
	SphereNode(VRMLParser& parser);
	virtual ~SphereNode(void);
	
	/* Methods: */
	virtual void initContext(GLContextData& contextData) const;
	virtual VRMLNode::Box calcBoundingBox(void) const;
	virtual void glRenderAction(VRMLRenderState& renderState) const;
	};

#endif
