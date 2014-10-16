/***********************************************************************
PointTwoNTree - Abstract class for n-dimensional spatial trees
(quadtrees, octrees, etc.) with dynamic point insertion/removal.
Copyright (c) 2004-2010 Oliver Kreylos

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

#ifndef GEOMETRY_POINTTWONTREE_INCLUDED
#define GEOMETRY_POINTTWONTREE_INCLUDED

#include <stdexcept>
#include <Misc/PoolAllocator.h>
#include <Math/Math.h>
#include <Geometry/Point.h>
#include <Geometry/ClosePointSet.h>

namespace Geometry {

template <class StoredPointParam>
class PointTwoNTree
	{
	/* Embedded classes: */
	public:
	typedef StoredPointParam StoredPoint; // Type of points stored in 2^n-tree (typically with some associated value)
	typedef typename StoredPoint::Point Point; // Type for positions
	typedef typename Point::Scalar Scalar; // Scalar type used by points
	static const int dimension=Point::dimension; // Dimension of points and kd-tree
	typedef Geometry::ClosePointSet<StoredPoint> ClosePointSet; // Type for nearest neighbours query results
	
	struct PointNotFoundError:public std::runtime_error // Error class if a point is not found in the tree
		{
		/* Elements: */
		public:
		StoredPoint point; // The point that was not found
		
		/* Constructors and destructors: */
		PointNotFoundError(const StoredPoint& sPoint);
		};
	
	struct NoClosePointFoundError:public std::runtime_error // Error class if no close points were found in the tree
		{
		/* Elements: */
		public:
		Point queryPoint; // The query point that has no close points
		
		/* Constructors and destructors: */
		NoClosePointFoundError(const Point& sQueryPoint);
		};
	
	private:
	struct StoredPointListItem // Structure for items in lists of stored points
		{
		/* Elements: */
		public:
		StoredPoint point; // Point stored in list item
		StoredPointListItem* succ; // Pointer to next list item
		
		/* Constructors and destructors: */
		StoredPointListItem(const StoredPoint& sPoint)
			:point(sPoint),
			 succ(0)
			{
			}
		};
	
	struct NodeBlock;
	
	struct Node
		{
		/* Embedded classes: */
		public:
		static const int numChildren=(1<<dimension); // Number of children of internal nodes
		
		/* Elements: */
		bool leaf; // Flag if the node is a leaf
		unsigned short numListItems; // Number of list items if the node is a leaf
		union // Anonymous union holding the interior/leaf node pointers
			{
			NodeBlock* children; // Pointer to array of numNodeChildren child nodes, ==0 if leaf node
			StoredPointListItem* firstItem; // Pointer to first point stored in leaf node
			};
		
		/* Constructors and destructors: */
		Node(void) // Constructs an empty leaf node
			:leaf(true),numListItems(0),
			 firstItem(0)
			{
			}
		
		/* Methods: */
		template <class TraversalFunctionParam>
		void traverseTree(const Point& nodeCenter,Scalar nodeSize,int nodeDepth,TraversalFunctionParam& traversalFunction) const // Traverses tree in prefix order and calls traversal function for each node
			{
			/* Call traversal function: */
			traversalFunction(nodeCenter,nodeSize,nodeDepth);
			
			if(!leaf)
				{
				/* Traverse all children: */
				Scalar childNodeSize=Math::div2(nodeSize);
				for(int i=0;i<numChildren;++i)
					{
					/* Calculate the child node's center: */
					Point childNodeCenter=nodeCenter;
					for(int j=0;j<dimension;++j)
						if(i&(1<<j))
							childNodeCenter[j]+=childNodeSize;
						else
							childNodeCenter[j]-=childNodeSize;
					
					/* Traverse the child node's subtree: */
					children->children[i].traverseTree(childNodeCenter,childNodeSize,nodeDepth+1,traversalFunction);
					}
				}
			}
		const StoredPointListItem* findClosePoint(const Point& nodeCenter,Scalar nodeSize,const Point& queryPoint) const;
		const StoredPointListItem* findClosestPoint(const Point& nodeCenter,Scalar nodeSize,const Point& queryPoint,Scalar& minDist2) const;
		void findClosestPoints(const Point& nodeCenter,Scalar nodeSize,const Point& queryPoint,ClosePointSet& closestPoints) const;
		};
	
	struct NodeBlock // Structure to hold a block of numChildren nodes
		{
		/* Elements: */
		public:
		Node children[Node::numChildren];
		};
	
	/* Elements: */
	Misc::PoolAllocator<StoredPointListItem> listItemAllocator; // Allocator for stored point list items
	Misc::PoolAllocator<NodeBlock> nodeBlockAllocator; // Allocator for child nodes
	Point rootCenter; // Center point of the root node
	Scalar rootSize; // Size of the root node
	Node root; // The tree's root node
	unsigned short splitThreshold; // Number of points at which a node is split
	unsigned short mergeThreshold; // Number of points at which a node's subtree collapses
	
	/* Private methods: */
	void deleteSubtree(Node* node); // Deletes subtree underneath a node
	
	/* Constructors and destructors: */
	public:
	PointTwoNTree(const Point& sRootCenter,Scalar sRootSize); // Creates an empty tree with the given (initial) root node size
	
	/* Methods: */
	void setSplitThreshold(int newSplitThreshold); // Sets a new split threshold; does not affect the tree directly
	void setMergeThreshold(int newMergeThreshold); // Sets a new merge threshold; does not affect the tree directly
	const StoredPoint& insertPoint(const StoredPoint& newPoint); // Inserts a new point into the tree
	void removePoint(const StoredPoint& removePoint); // Removes a point from the tree; throws exception if point does not exist
	template <class TraversalFunctionParam>
	void traverseTree(TraversalFunctionParam& traversalFunction) const // Traverses tree in prefix order and calls traversal function for each node
		{
		root.traverseTree(rootCenter,rootSize,0,traversalFunction);
		}
	const StoredPoint& findClosePoint(const Point& queryPoint) const; // Returns a stored point close to the query point; throws exception if no point is found
	const StoredPoint& findClosestPoint(const Point& queryPoint) const; // Returns the stored point closest to the query point
	ClosePointSet& findClosestPoints(const Point& queryPoint,ClosePointSet& closestPoints) const; // Returns a set of closest points
	};

}

#if !defined(GEOMETRY_POINTTWONTREE_IMPLEMENTATION)
#include <Geometry/PointTwoNTree.icpp>
#endif

#endif
