/***********************************************************************
EditableGrid - Data structure to represent 3D grids with editable data
values and interactive isosurface extraction.
Copyright (c) 2006-2013 Oliver Kreylos

This file is part of the Virtual Clay Editing Package.

The Virtual Clay Editing Package is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The Virtual Clay Editing Package is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Virtual Clay Editing Package; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#ifndef EDITABLEGRID_INCLUDED
#define EDITABLEGRID_INCLUDED

#include <vector>
#include <Misc/ArrayIndex.h>
#include <Misc/Array.h>
#include <Geometry/ComponentArray.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <GL/gl.h>
#include <GL/GLVertex.h>
#include <GL/GLObject.h>

/* Forward declarations: */
namespace IO {
class File;
}
class GLContextData;

class EditableGrid:public GLObject
	{
	/* Embedded classes: */
	public:
	typedef Misc::ArrayIndex<3> Index; // Type for array indices
	typedef Geometry::ComponentArray<float,3> Size; // Type for sizes
	typedef Geometry::Point<float,3> Point; // Type for points in data set's domain
	private:
	typedef float VertexValue; // Data type for vertex values
	typedef Geometry::Vector<float,3> VertexGradient; // Data type for vertex gradients
	
	struct Vertex // Structure to represent the state of a grid vertex
		{
		/* Elements: */
		public:
		VertexValue value; // Vertex value
		VertexGradient gradient; // Vertex gradient
		};
	
	typedef Misc::Array<Vertex,3> VertexArray;
	
	struct Cell // Structure to represent the state of a grid cell
		{
		/* Elements: */
		public:
		int numTriangles; // Number of triangles in this cell's isosurface fragment
		unsigned int triangleOffset; // Offset of cell's isosurface fragment in fragment storage
		};
	
	typedef Misc::Array<Cell,3> CellArray;
	
	typedef GLVertex<void,0,void,0,float,float,3> IsosurfaceVertex; // Data type for isosurface vertices
	
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		public:
		GLuint isoFragmentVertexBufferObjectIds[5]; // Vertex buffer IDs for isosurface fragments
		unsigned int isoFragmentVersions[5]; // Version numbers for cached isosurface fragments
		
		/* Constructors and destructors: */
		DataItem(void);
		virtual ~DataItem(void);
		};
	
	/* Elements: */
	private:
	
	/* Hard-coded case tables for Marching Cubes isosurface extraction: */
	static const int edgeVertexIndices[12][2]; // Indices of edge vertices
	static const int edgeMasks[256]; // Bit masks of affected cell edges for each cell case
	static const int fragmentNumTriangles[256]; // Numbers of triangles in a cell's isosurface for each cell case
	static const int triangleEdgeIndices[256][16]; // Indices of edges defining a cell's isosurface fragment for each cell case
	
	/* Grid definition: */
	Index numVertices; // Number of grid vertices
	Index numCells; // Number of grid cells
	Size cellSize; // Size of grid cell in each dimension
	VertexArray vertices; // Data values for each grid vertex
	CellArray cells; // States for each grid cell
	ptrdiff_t vertexStrides[3];
	ptrdiff_t cellVertexOffsets[8];
	float gradientScale[3]; // Scale factors to compute gradients from vertex values
	
	/* Current isosurface storage: */
	std::vector<IsosurfaceVertex> isoFragments[5]; // Storage for isosurface fragments
	std::vector<unsigned int> isoFragmentOwners[5]; // Storage for cells owning each isosurface fragment
	unsigned int isoFragmentVersions[5]; // Version numbers for current isosurface fragments
	
	/* Constructors and destructors: */
	public:
	EditableGrid(const Index& sNumVertices,const Size& sCellSize); // Creates an empty editable grid
	virtual ~EditableGrid(void); // Destroys an editable grid
	
	/* Methods: */
	const Index& getNumVertices(void) const
		{
		return numVertices;
		};
	int getNumVertices(int dimension) const
		{
		return numVertices[dimension];
		};
	const Size& getCellSize(void) const
		{
		return cellSize;
		};
	float getCellSize(int dimension) const
		{
		return cellSize[dimension];
		};
	float getValue(const Index& vertexIndex) const // Returns vertex value for given vertex index
		{
		return vertices(vertexIndex).value;
		};
	void setValue(const Index& vertexIndex,float newValue) // Changes vertex value for given vertex index; does not update impacted data structures
		{
		vertices(vertexIndex).value=newValue;
		};
	float getValue(const Point& p) const; // Returns interpolated value at arbitrary location inside domain
	void invalidateVertices(const Index& min,const Index& max); // Marks vertices inside given index range [min, max) as invalid
	virtual void initContext(GLContextData& contextData) const;
	void glRenderAction(GLContextData& contextData) const; // Renders the current isosurface
	void exportSurface(IO::File& file) const; // Saves the current isosurface as a mesh file
	};

#endif
