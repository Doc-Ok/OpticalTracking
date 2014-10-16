/***********************************************************************
GLPolylineTube - Class to render a polyline as a cylindrical tube.
Copyright (c) 2006-2013 Oliver Kreylos

This file is part of the OpenGL Support Library (GLSupport).

The OpenGL Support Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The OpenGL Support Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the OpenGL Support Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef GLPOLYLINETUBE_INCLUDED
#define GLPOLYLINETUBE_INCLUDED

#include <vector>
#include <Geometry/Point.h>
#include <GL/gl.h>
#include <GL/GLVertex.h>
#include <GL/GLObject.h>

/* Forward declarations: */
class GLContextData;

class GLPolylineTube:public GLObject
	{
	/* Embedded classes: */
	public:
	typedef float Scalar; // Scalar type for polyline vertices
	typedef Geometry::Point<Scalar,3> Point; // Type for points
	typedef GLVertex<void,0,void,0,Scalar,Scalar,3> Vertex; // Type for tube vertices
	
	struct DataItem:public GLObject::DataItem // Class storing the OpenGL state of a GLPolylineTube object
		{
		/* Elements: */
		public:
		GLuint vertexBufferId; // ID of the buffer object holding the tube vertices
		GLuint indexBufferId; // ID of the buffer object holding the tube vertex indices
		Vertex* vertices; // Holds tube vertices if the vertex buffer object extension is not supported
		GLuint* indices; // Holds tube vertex indices if the vertex buffer object extension is not supported
		unsigned int vertexVersion; // Version number of the vertex buffer / vertex array
		unsigned int indexVersion; // Version number of the index buffer / index array
		
		/* Constructors and destructors: */
		DataItem(void);
		virtual ~DataItem(void);
		};
	
	/* Elements: */
	private:
	std::vector<Point> vertices; // Vector of polyline vertices
	Scalar tubeRadius; // Radius of the tube around the polyline
	int numTubeSegments; // Number of quads around the tube
	unsigned int vertexVersion; // Version number of the tube vertices
	unsigned int indexVersion; // Version number of the tube indices
	
	/* Private methods: */
	void updateTubeVertices(DataItem* dataItem) const; // Updates the version of the vertices stored in the vertex buffer / vertex array
	void updateTubeIndices(DataItem* dataItem) const; // Updates the version of the indices stored in the vertex buffer / vertex array
	
	/* Constructors and destructors: */
	public:
	GLPolylineTube(Scalar sTubeRadius,size_t sNumVertices,const Point* sVertices =0); // Creates a polyline of the given number of vertices; copies vertices from given array if !=0
	private:
	GLPolylineTube(const GLPolylineTube& source); // Prohibit copy constructor
	GLPolylineTube& operator=(const GLPolylineTube& source); // Prohibit assignment operator
	public:
	virtual ~GLPolylineTube(void);
	
	/* Methods: */
	virtual void initContext(GLContextData& contextData) const;
	size_t getNumVertices(void) const // Returns the number of vertices in the polyline
		{
		return vertices.size();
		}
	const Point& getVertex(size_t vertexIndex) const // Returns one polyline vertex
		{
		return vertices[vertexIndex];
		}
	void setVertex(size_t vertexIndex,const Point& newVertex); // Sets one of the polyline vertices to a new position
	void addVertex(const Point& newVertex); // Adds a vertex to the end of the polyline
	Scalar getTubeRadius(void) const // Returns the tube radius
		{
		return tubeRadius;
		}
	void setTubeRadius(Scalar newTubeRadius); // Sets the tube radius to a new value
	int getNumTubeSegments(void) const // Returns the number of quads around the tube
		{
		return numTubeSegments;
		}
	void setNumTubeSegments(int newNumTubeSegments); // Sets the number of segments around the tube
	void glRenderAction(GLContextData& contextData) const; // Renders the tube
	};

#endif
