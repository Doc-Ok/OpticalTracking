/***********************************************************************
Curvilinear - Base class for vertex-centered curvilinear data sets
containing arbitrary value types (scalars, vectors, tensors, etc.).
Copyright (c) 2004-2010 Oliver Kreylos

This file is part of the Vrui VR Device Driver Daemon (VRDeviceDaemon).

The Vrui VR Device Driver Daemon is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Vrui VR Device Driver Daemon is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Vrui VR Device Driver Daemon; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#ifndef CURVILINEAR_INCLUDED
#define CURVILINEAR_INCLUDED

#include <Misc/Array.h>
#include <Geometry/ComponentArray.h>
#include <Geometry/Vector.h>
#include <Geometry/Point.h>
#include <Geometry/Box.h>
#include <Geometry/Matrix.h>
#include <Geometry/ValuedPoint.h>
#include <Geometry/ArrayKdTree.h>

#include <VRDeviceDaemon/VRCalibrators/ConvexInterpolator.h>

namespace Visualization {

template <class ScalarParam,int dimensionParam,class ValueParam,class InterpolatorParam =ConvexInterpolator<ValueParam,ScalarParam> >
class Curvilinear
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar; // Scalar type of data set's domain
	static const int dimension=dimensionParam; // Dimension of data set's domain
	static const int numCellVertices=(1<<dimensionParam); // Number of vertices for each cell
	typedef Geometry::Vector<Scalar,dimensionParam> Vector; // Type for vectors in data set's domain
	typedef Geometry::Point<Scalar,dimensionParam> Point; // Type for points in data set's domain
	typedef Geometry::Box<Scalar,dimensionParam> Box; // Type for axis-aligned boxes in data set's domain
	typedef ValueParam Value; // Data set's value type
	typedef InterpolatorParam Interpolator; // Convex interpolator for multilinear interpolation of data set values
	
	struct GridVertex // Structure for valued grid vertices
		{
		/* Elements: */
		public:
		Point pos; // Position of grid vertex in data set's domain
		Value value; // Grid vertex' value
		
		/* Constructors and destructors: */
		GridVertex(void) // Dummy constructor
			{
			};
		GridVertex(const Point& sPos,const Value& sValue) // Elementwise constructor
			:pos(sPos),value(sValue)
			{
			};
		};
	
	typedef Misc::Array<GridVertex,dimensionParam> Array; // Array type for data set storage
	typedef typename Array::Index Index; // Index type for data set storage
	protected:
	typedef Geometry::ValuedPoint<Point,Index> CellCenter; // Data type to associate a cell's center point and a pointer to its base vertex
	typedef Geometry::ArrayKdTree<CellCenter> CellCenterTree; // Data type for kd-trees to locate closest cell centers
	
	public:
	class Locator // Class responsible for evaluating a data set at a given position
		{
		friend class Curvilinear;
		
		/* Embedded classes: */
		protected:
		typedef Geometry::ComponentArray<Scalar,dimensionParam> CellPosition; // Type for local cell coordinates
		typedef Geometry::Matrix<Scalar,dimensionParam,dimensionParam> Matrix; // Type for Jacobian matrix of point transformation
		
		/* Elements: */
		const Curvilinear* grid; // Grid the locator is associated with
		Index cell; // Index of grid cell containing last located point
		const GridVertex* cellBase; // Pointer to base vertex of grid cell containing last located point
		CellPosition cellPos; // Local coordinates of last located point inside its cell
		Scalar epsilon,epsilon2; // Accuracy threshold of point location algorithm
		
		/* Protected methods: */
		Point transformCellPosition(const CellPosition& cellPos) const; // Transforms local coordinates in current cell to domain coordinates
		Matrix calcTransformDerivative(const CellPosition& cellPos) const; // Calculates Jacobian matrix of coordinate transformation
		
		/* Constructors and destructors: */
		public:
		Locator(void); // Creates invalid locator
		protected:
		Locator(const Curvilinear* sGrid,Scalar sEpsilon); // Creates non-localized locator associated with given grid
		
		/* Methods: */
		public:
		void setEpsilon(Scalar newEpsilon); // Sets a new accuracy threshold in local cell dimension
		bool isValid(void) const;
		bool locatePoint(const Point& position,bool traceHint =false);
		Value calcValue(void) const;
		Value calcValue(const Point& position,bool traceHint =false);
		};
	
	friend class Locator;
	
	/* Elements: */
	protected:
	Index numVertices; // Number of vertices in data set in each dimension
	Array vertices; // Array of vertices defining data set
	int vertexStrides[dimension]; // Array of pointer stride values in the vertex array
	int vertexOffsets[numCellVertices]; // Array of pointer offsets from a cell's base vertex to all cell vertices
	Scalar locatorEpsilon; // Default accuracy threshold for locators working on this data set
	CellCenterTree cellCenterTree; // Kd-tree containing cell centers
	
	/* Constructors and destructors: */
	public:
	Curvilinear(const Index& sNumVertices,const Point* sVertexPositions =0,const Value* sVertexValues =0);
	Curvilinear(const Index& sNumVertices,const GridVertex* sVertices);
	~Curvilinear(void);
	
	/* Low-level data access methods: */
	const Index& getNumVertices(void) const // Returns number of vertices in the grid
		{
		return numVertices;
		};
	const Array& getVertices(void) const // Returns the vertices defining data set
		{
		return vertices;
		};
	Array& getVertices(void) // Ditto
		{
		return vertices;
		};
	const GridVertex& getVertex(const Index& vertexIndex) const // Returns a grid vertex
		{
		return vertices(vertexIndex);
		};
	GridVertex& getVertex(const Index& vertexIndex) // Ditto
		{
		return vertices(vertexIndex);
		};
	const Point& getVertexPosition(const Index& vertexIndex) const // Returns a vertex' position
		{
		return vertices(vertexIndex).pos;
		};
	Point& getVertexPosition(const Index& vertexIndex) // Ditto
		{
		return vertices(vertexIndex).pos;
		};
	const Value& getVertexValue(const Index& vertexIndex) const // Returns a vertex' data value
		{
		return vertices(vertexIndex).value;
		};
	Value& getVertexValue(const Index& vertexIndex) // Ditto
		{
		return vertices(vertexIndex).value;
		};
	void finalizeGrid(void); // Recalculates derived grid information after grid structure change
	void setLocatorEpsilon(Scalar newLocatorEpsilon); // Sets the default accuracy threshold for locators working on this data set
	
	/* Methods implementing the data set interface: */
	Box getDomainBox(void) const; // Returns bounding box of the data set's domain
	Locator getLocator(void) const; // Returns any locator
	};

}

#ifndef CURVILINEAR_IMPLEMENTATION
#include <VRDeviceDaemon/VRCalibrators/Curvilinear.icpp>
#endif

#endif
