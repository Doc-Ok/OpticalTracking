/***********************************************************************
GeodeticToCartesianPointTransformNode - Point transformation class to
convert geodetic coordinates (longitude/latitude/altitude on a reference
ellipsoid) to Cartesian coordinates.
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

#ifndef SCENEGRAPH_GEODETICTOCARTESIANPOINTTRANSFORMNODE_INCLUDED
#define SCENEGRAPH_GEODETICTOCARTESIANPOINTTRANSFORMNODE_INCLUDED

#include <Geometry/Vector.h>
#include <SceneGraph/FieldTypes.h>
#include <SceneGraph/PointTransformNode.h>
#include <SceneGraph/ReferenceEllipsoidNode.h>

namespace SceneGraph {

class GeodeticToCartesianPointTransformNode:public PointTransformNode
	{
	/* Embedded classes: */
	public:
	typedef SF<ReferenceEllipsoidNodePointer> SFReferenceEllipsoidNode;
	
	/* Elements: */
	
	/* Fields: */
	public:
	SFReferenceEllipsoidNode referenceEllipsoid;
	SFString longitude;
	SFString latitude;
	SFString elevation;
	SFBool degrees;
	SFBool colatitude;
	SFTScalar elevationScale;
	SFBool moveToOrigin;
	SFTPoint originPoint;
	
	/* Derived state: */
	protected:
	const ReferenceEllipsoidNode::Geoid* re;
	int componentIndices[3]; // Indices of (longitude, latitude, elevation) components in input points
	TScalar componentScales[3],componentOffsets[3]; // Scale and offset values to transform input points to long, lat in radians and elevation
	TVector offset; // Offset vector to be applied to Cartesian coordinates
	bool flipNormals; // Flag whether to flip normal vectors after transformation
	
	/* Constructors and destructors: */
	public:
	GeodeticToCartesianPointTransformNode(void); // Creates a default node
	
	/* Methods from Node: */
	static const char* getStaticClassName(void);
	virtual const char* getClassName(void) const;
	virtual void parseField(const char* fieldName,VRMLFile& vrmlFile);
	virtual void update(void);
	
	/* Methods from PointTransformNode: */
	virtual TPoint transformPoint(const TPoint& point) const;
	virtual TPoint inverseTransformPoint(const TPoint& point) const;
	virtual TBox calcBoundingBox(const std::vector<Point>& points) const;
	virtual TBox transformBox(const TBox& box) const;
	virtual TVector transformNormal(const TPoint& basePoint,const TVector& normal) const;
	};

}

#endif
