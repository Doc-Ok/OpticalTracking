/***********************************************************************
GroupNode - Base class for group nodes in VRML world files.
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

#ifndef GROUPNODE_INCLUDED
#define GROUPNODE_INCLUDED

#include <vector>

#include "Types.h"

#include "VRMLNode.h"

class GroupNode:public VRMLNode
	{
	/* Embedded classes: */
	protected:
	typedef std::vector<VRMLNodePointer> NodeList; // Data type for lists of nodes
	
	/* Elements: */
	protected:
	NodeList children; // List of this node's children
	bool haveBoundingBox; // Flag whether the node has an explicit bounding box
	Box boundingBox; // Bounding box around node's children
	
	/* Protected methods: */
	void parseChildren(VRMLParser& parser); // Processes a "children" attribute
	void addChild(VRMLNodePointer newChild); // Adds a new child to the group
	void setBoundingBox(const Vec3f& bboxCenter,const Vec3f& bboxSize); // Set's the group node's explicit bounding box
	
	/* Constructors and destructors: */
	protected:
	GroupNode(void); // Creates a group node but does not parse; responsibility of child class
	public:
	GroupNode(VRMLParser& parser); // Initializes the node from the given VRML parser
	virtual ~GroupNode(void);
	
	/* Methods: */
	virtual VRMLNode::Box calcBoundingBox(void) const;
	virtual void glRenderAction(VRMLRenderState& renderState) const;
	};

#endif
