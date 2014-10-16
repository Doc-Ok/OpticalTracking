/***********************************************************************
GeodeticToCartesianTransformNode - Special transformation node class to
transform from a local frame on a reference ellipsoid given in geodetic
coordinates to Cartesian coordinates.
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

#ifndef SCENEGRAPH_GEODETICTOCARTESIANTRANSFORMNODE_INCLUDED
#define SCENEGRAPH_GEODETICTOCARTESIANTRANSFORMNODE_INCLUDED

#include <Geometry/OrthogonalTransformation.h>
#include <SceneGraph/Geometry.h>
#include <SceneGraph/FieldTypes.h>
#include <SceneGraph/GroupNode.h>
#include <SceneGraph/ReferenceEllipsoidNode.h>

namespace SceneGraph {

class GeodeticToCartesianTransformNode:public GroupNode
	{
	/* Embedded classes: */
	public:
	typedef SF<ReferenceEllipsoidNodePointer> SFReferenceEllipsoidNode;
	
	/* Elements: */
	
	/* Fields: */
	public:
	SFReferenceEllipsoidNode referenceEllipsoid;
	SFBool longitudeFirst;
	SFBool degrees;
	SFBool colatitude;
	SFPoint geodetic;
	SFBool translateOnly;
	
	/* Derived state: */
	protected:
	OGTransform transform; // The current transformation
	
	/* Constructors and destructors: */
	public:
	GeodeticToCartesianTransformNode(void); // Creates a default node
	
	/* Methods from Node: */
	static const char* getStaticClassName(void);
	virtual const char* getClassName(void) const;
	virtual void parseField(const char* fieldName,VRMLFile& vrmlFile);
	virtual void update(void);
	
	/* Methods from GraphNode: */
	virtual Box calcBoundingBox(void) const;
	virtual void glRenderAction(GLRenderState& renderState) const;
	
	/* New methods: */
	const OGTransform& getTransform(void) const // Returns the current derived transformation
		{
		return transform;
		}
	};

}

#endif
