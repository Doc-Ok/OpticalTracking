/***********************************************************************
EarthModelNode - Class for high-level nodes that render a model of
Earth.
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

#ifndef EARTHMODELNODE_INCLUDED
#define EARTHMODELNODE_INCLUDED

#include <GL/gl.h>
#include <GL/GLObject.h>

#include "Types.h"

#include "VRMLNode.h"
#include "AttributeNode.h"

class EarthModelNode:public VRMLNode,public GLObject
	{
	/* Embedded classes: */
	private:
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		public:
		GLuint displayListIdBase; // Base ID of set of display lists for Earth model components
		unsigned int surfaceVersion; // Version number of surface display list
		unsigned int gridVersion; // Version number of longitude/latitude grid display list
		unsigned int outerCoreVersion; // Version number of outer core display list
		unsigned int innerCoreVersion; // Version number of inner core display list
		
		/* Constructors and destructors: */
		DataItem(void);
		virtual ~DataItem(void);
		};
	
	/* Elements: */
	static const double earthSurfaceRadius; // Equatorial radius of Earth's surface (WGS84 ellipsoid)
	static const double earthSurfaceFlatteningFactor; // Flattening factor of Earth's surface (WGS84 ellipsoid)
	static const double earthOuterCoreRadius; // Radius of Earth's outer core
	static const double earthInnerCoreRadius; // Radius of Earth's inner core
	double scaleFactor; // Scale factor from meters to model units
	double flatteningFactor; // Ellipsoid flattening factor to use for this earth model
	Bool surface; // Flag to render the surface
	AttributeNodePointer surfaceMaterial; // The node defining the surface's material
	AttributeNodePointer surfaceTexture; // The node defining the surface's texture
	Int32 surfaceDetail; // Surface's detail level
	Bool grid; // Flag to render the latitude/longitude grid
	Color gridColor; // Color to render the grid
	Int32 gridDetail; // Grid's detail level
	Bool outerCore; // Flag to render the outer core
	AttributeNodePointer outerCoreMaterial; // The node defining the outer core's material
	Int32 outerCoreDetail; // Outer core's detail level
	Bool innerCore; // Flag to render the inner core
	AttributeNodePointer innerCoreMaterial; // The node defining the inner core's material
	Int32 innerCoreDetail; // Inner core's detail level
	
	/* Private methods: */
	void renderSurface(void) const;
	void renderGrid(void) const;
	void renderOuterCore(void) const;
	void renderInnerCore(void) const;
	
	/* Constructors and destructors: */
	public:
	EarthModelNode(VRMLParser& parser);
	virtual ~EarthModelNode(void);
	
	/* Methods: */
	virtual void initContext(GLContextData& contextData) const;
	virtual VRMLNode::Box calcBoundingBox(void) const;
	virtual void glRenderAction(VRMLRenderState& renderState) const;
	};

#endif
