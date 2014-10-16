/***********************************************************************
NormalNode - Class for arrays of vertex normal vectors.
Copyright (c) 2006-2008 Oliver Kreylos

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

#ifndef NORMALNODE_INCLUDED
#define NORMALNODE_INCLUDED

#include <vector>
#include <Geometry/Vector.h>

#include "VRMLNode.h"

class NormalNode:public VRMLNode
	{
	/* Embedded classes: */
	public:
	typedef Geometry::Vector<float,3> Vector; // Type for vectors
	typedef std::vector<Vector> VectorList; // Type for lists of vectors
	
	/* Elements: */
	private:
	VectorList vectors; // The vector array
	
	/* Constructors and destructors: */
	public:
	NormalNode(void); // Creates empty normal node, to be filled in later
	NormalNode(VRMLParser& parser); // Creates normal node by parsing VRML file
	
	/* Methods: */
	virtual void glRenderAction(VRMLRenderState& renderState) const;
	VectorList& getVectors(void) // Returns the array of vectors
		{
		return vectors;
		};
	size_t getNumVectors(void) const // Returns the number of vectors in the array
		{
		return vectors.size();
		};
	const Vector& getVector(int index) const // Returns the index-th vector in the array
		{
		return vectors[index];
		};
	};

#endif
