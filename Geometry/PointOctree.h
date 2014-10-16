/***********************************************************************
PointOctree - Class to store three--dimensional points in an octree.
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

#ifndef GEOMETRY_POINTOCTREE_INCLUDED
#define GEOMETRY_POINTOCTREE_INCLUDED

#include <Geometry/Vector.h>
#include <Geometry/Point.h>
#include <Geometry/ValuedPoint.h>

namespace Geometry {

template <class ScalarParam,class StoredPointParam>
class PointOctree
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar; // Scalar type used by points
	static const int dimension=3; // Dimension of points and octree
	typedef StoredPointParam StoredPoint; // Type of points stored in octree (typically with some associated value)
	typedef Geometry::Point<ScalarParam,3> Point; // Type for positions
	
	private:
	typedef Geometry::Vector<Scalar,3> Vector; // Type for differences of points
	
	struct Traversal
		{
		/* Elements: */
		public:
		Point center; // Center of the cube associated with a node
		Vector size; // Vector from the cube center to corner number 7
		
		/* Constructors and destructors: */
		Traversal(void)
			{
			}
		Traversal(const Point& sCenter,const Vector& sSize)
			:center(sCenter),size(sSize)
			{
			}
		
		/* Methods: */
		Traversal getChild(int childIndex) const
			{
			Vector cSize=size/Scalar(2);
			Point cCenter=center;
			for(int i=0;i<3;++i)
				{
				if(childIndex&(1<<i))
					cCenter[i]+=cSize[i];
				else
					cCenter[i]-=cSize[i];
				}
			return Traversal(cCenter,cSize);
			}
		};
	
	struct Node
		{
		/* Elements: */
		public:
		Node* children; // If node is not a leaf, pointer to array of eight child nodes; 0 otherwise
		int numPoints; // Number of points contained in this node's subtree
		StoredPoint* points; // Pointer to (sub-)array of points contained in this node's subtree
		
		/* Helper methods: */
		private:
		static int splitPoints(int direction,Scalar mid,int numPoints,StoredPoint* points); // Splits a point array
		
		/* Constructors and destructors: */
		public:
		Node(void) // Creates uninitialized node
			{
			}
		~Node(void) // Destroys a node and its subtree
			{
			if(children!=0)
				delete[] children;
			}
		
		/* Methods: */
		void initialize(const Traversal& t,int sNumPoints,StoredPoint* sPoints,int maxNumPoints,int maxDepth); // Creates subtree storing the given subarray of points
		bool isLeaf(void) const // Checks whether a node is a leaf
			{
			return children==0;
			}
		int getNumPoints(void) const // Returns the number of points in the node
			{
			return numPoints;
			}
		const StoredPoint* findClosePoint(const Point& p,const Traversal& t) const; // Finds a point in the subtree below a node
		void gatherStatistics(int& numNodes,int& numLeaves,int& maxNumPoints,int &depth,int nodeDepth) const;
		};
	
	struct QueueEntry
		{
		/* Elements: */
		public:
		Scalar minDist; // Minimum distance between node and search point
		Traversal traversal; // Bounding box of the node
		const Node* node; // Pointer to node
		
		/* Constructors and destructors: */
		QueueEntry(void)
			{
			}
		QueueEntry(const Traversal& sTraversal,const Node* sNode,const Point& point);
		
		/* Methods: */
		friend bool operator<=(const QueueEntry& qe1,const QueueEntry& qe2)
			{
			return qe1.minDist<=qe2.minDist;
			}
		};
	
	/* Elements: */
	int numPoints; // The number of points in the tree
	StoredPoint* points; // The array of points in the tree
	Traversal rootTraversal; // Traversal structure describing the tree's root
	Node* root; // The root node of the tree
	
	/* Constructors and destructors: */
	public:
	PointOctree(void) // Dummy constructor
		:points(0),root(0)
		{
		}
	PointOctree(const Point& min,const Point& max,int sNumPoints,StoredPoint* sPoints,int maxNumPoints,int maxDepth); // Creates an octree of the given size, containing the given points
	~PointOctree(void);
	
	/* Methods: */
	void clear(void); // Clears the octree
	void setPoints(const Point& min,const Point& max,int sNumPoints,StoredPoint* sPoints,int maxNumPoints,int maxDepth);
	const StoredPoint& findClosePoint(const Point& p) const // Returns a point "close" to the given point
		{
		return *root->findClosePoint(p,rootTraversal);
		}
	const StoredPoint& findClosestPoint(const Point& p) const; // Returns the closest point to the given point
	void gatherStatistics(int& numNodes,int& numLeaves,int& maxNumPoints,int& depth) const;
	};

}

#if !defined(GEOMETRY_POINTOCTREE_IMPLEMENTATION)
#include <Geometry/PointOctree.icpp>
#endif

#endif
