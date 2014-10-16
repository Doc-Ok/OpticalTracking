/***********************************************************************
PointKdTree - Class to store k-dimensional points in a kd-tree.
Copyright (c) 2003-2010 Oliver Kreylos

This file is part of the Templatized Geometry Library (TGL).

The Templatized Geometry Library is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Templatized Geometry Library is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Templatized Geometry Library; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#ifndef GEOMETRY_POINTKDTREE_INCLUDED
#define GEOMETRY_POINTKDTREE_INCLUDED

#include <Misc/PoolAllocator.h>
#include <Geometry/Point.h>
#include <Geometry/ClosePointSet.h>

namespace Geometry {

template <class ScalarParam,int dimensionParam,class StoredPointParam>
class PointKdTree
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar; // Scalar type used by points
	static const int dimension=dimensionParam; // Dimension of points and kd-tree
	typedef StoredPointParam StoredPoint; // Type of points stored in kd-tree (typically with some associated value)
	typedef Geometry::Point<ScalarParam,dimensionParam> Point; // Type for positions
	typedef Geometry::ClosePointSet<StoredPointParam> ClosePointSet; // Type for nearest neighbours query results
	
	struct TreeStats // Structure to report tree statistics
		{
		/* Elements: */
		public:
		int numNodes; // Total number of nodes (internal and exterior)
		int depth; // Depth of tree
		};
	
	private:
	struct Traversal // Structure to traverse kd-trees
		{
		/* Elements: */
		Point min,max; // Minimum and maximum extents of traversal box
		int splitDimension; // Next split dimension
		};
	
	struct Node
		{
		/* Elements: */
		public:
		static Misc::PoolAllocator<Node> nodeAllocator; // Memory allocator for nodes
		StoredPoint point; // Point stored in node
		Node* left; // Pointer to left child node
		Node* right; // Pointer to right child node
		
		/* Constructors and destructors: */
		Node(const StoredPoint& sPoint,Node* sLeft =0,Node* sRight =0) // Elementwise
			:point(sPoint),left(sLeft),right(sRight)
			{
			}
		Node(int numPoints,StoredPoint points[],int splitDimension,Scalar heap[]); // Creates sub-kd-tree for array of points; shuffles point array
		~Node(void) // Destroys node's subtree
			{
			delete left;
			delete right;
			}
		
		/* Methods: */
		void* operator new(size_t size)
			{
			return nodeAllocator.allocate();
			}
		void operator delete(void* pointer)
			{
			nodeAllocator.free(pointer);
			}
		
		void insertPoint(const StoredPoint& newPoint,int splitDimension); // Inserts a new point below this node
		TreeStats getTreeStatistics(void) const; // Returns tree statistics
		template <class TraversalFunctionParam>
		void traverseTree(TraversalFunctionParam& traversalFunction) const // Traverses subtree in prefix order and calls traversal function for each node
			{
			traversalFunction(point);
			if(left!=0)
				left->traverseTree(traversalFunction);
			if(right!=0)
				right->traverseTree(traversalFunction);
			}
		};
	
	struct QueueEntry
		{
		/* Elements: */
		public:
		const Node* node; // Current node
		Traversal traversal; // Box of current node
		Scalar minDist; // Minimum distance between node's box and query point
		
		/* Constructors and destructors: */
		QueueEntry(void)
			{
			}
		QueueEntry(const Node* sNode,const Traversal& sTraversal,const Point& queryPosition)
			:node(sNode),traversal(sTraversal),minDist(0)
			{
			/* Find the space region the point is in and calculate the minimum distance: */
			// Scalar d;
			for(int i=0;i<dimension;++i)
				{
				#if 1
				if(queryPosition[i]<traversal.min[i])
					{
					Scalar d=queryPosition[i]-traversal.min[i];
					minDist+=d*d;
					}
				else if(queryPosition[i]>traversal.max[i])
					{
					Scalar d=queryPosition[i]-traversal.max[i];
					minDist+=d*d;
					}
				#else
				if((d=traversal.min[i]-queryPosition[i])>Scalar(0))
					minDist+=d*d;
				else if((d=queryPosition[i]-traversal.max[i])>Scalar(0))
					minDist+=d*d;
				#endif
				}
			}
		
		/* Methods: */
		friend bool operator<=(const QueueEntry& qe1,const QueueEntry& qe2)
			{
			return qe1.minDist<=qe2.minDist;
			}
		};
	
	/* Elements: */
	Node* root; // Pointer to root node
	
	/* Constructors and destructors: */
	public:
	PointKdTree(void) // Creates an empty kd-tree
		:root(0)
		{
		}
	PointKdTree(int numPoints,StoredPoint points[]) // Creates balanced kd-tree from point array; shuffles point array in the process
		:root(0)
		{
		Scalar* heap=new Scalar[numPoints];
		root=new Node(numPoints,points,0,heap);
		delete[] heap;
		}
	~PointKdTree(void)
		{
		delete root;
		}
	
	/* Methods: */
	void setPoints(int numPoints,StoredPoint points[]) // Creates balanced kd-tree from point array; shuffles point array in the process
		{
		delete root;
		Scalar* heap=new Scalar[numPoints];
		root=new Node(numPoints,points,0,heap);
		delete[] heap;
		}
	void insertPoint(const StoredPoint& newPoint) // Inserts a new point into the kd-tree
		{
		if(root!=0)
			root->insertPoint(newPoint,0);
		else
			root=new Node(newPoint);
		}
	TreeStats getTreeStatistics(void) const; // Returns tree statistics
	template <class TraversalFunctionParam>
	void traverseTree(TraversalFunctionParam& traversalFunction) const // Traverses tree in prefix order and calls traversal function for each node
		{
		root->traverseTree(traversalFunction);
		}
	const StoredPoint& findClosePoint(const Point& queryPosition) const; // Returns a stored point that is close to the query position
	const StoredPoint& findClosestPoint(const Point& queryPosition) const; // Returns the stored point closest to the query position
	ClosePointSet& findClosestPoints(const Point& queryPosition,ClosePointSet& closestPoints) const; // Returns a set of closest points
	};

}

#if !defined(GEOMETRY_POINTKDTREE_IMPLEMENTATION)
#include <Geometry/PointKdTree.icpp>
#endif

#endif
