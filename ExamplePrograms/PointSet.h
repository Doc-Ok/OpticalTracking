/***********************************************************************
PointSet - Class to represent and render sets of scattered 3D points.
Copyright (c) 2005-2007 Oliver Kreylos

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef POINTSET_INCLUDED
#define POINTSET_INCLUDED

#include <vector>
#include <Geometry/Point.h>
#include <Geometry/ValuedPoint.h>
#include <Geometry/Box.h>
#include <GL/gl.h>
#include <GL/GLColor.h>
#include <GL/GLVertex.h>
#include <GL/GLObject.h>

class PointSet:public GLObject
	{
	/* Embedded classes: */
	public:
	typedef Geometry::Point<float,3> Point; // Type for affine points
	typedef Geometry::Box<float,3> Box; // Type for axis-aligned boxes
	typedef GLColor<GLubyte,4> Color; // Type for RGB colors with (unused) opacity component
	private:
	typedef Geometry::ValuedPoint<Point,Color> SourcePoint; // Type for representation of source points
	typedef GLVertex<void,0,GLubyte,4,void,GLfloat,3> Vertex; // Vertex type for points (position and RGB color)
	
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		public:
		GLuint vertexBufferObjectId; // ID of vertex buffer object that contains the point set (0 if extension not supported)
		
		/* Constructors and destructors: */
		public:
		DataItem(void); // Creates a data item
		virtual ~DataItem(void); // Destroys a data item
		};
	
	/* Elements: */
	std::vector<SourcePoint> points; // Array of source points
	
	/* Constructors and destructors: */
	public:
	PointSet(const char* pointFileName,double scaleFactor,const float colorMask[3]); // Creates a point set by reading a file; applies scale factor to Cartesian coordinates
	
	/* Methods: */
	virtual void initContext(GLContextData& contextData) const;
	Box calcBoundingBox(void) const; // Returns the point set's bounding box
	void glRenderAction(GLContextData& contextData) const; // Renders point set into the current OpenGL context
	};

#endif
