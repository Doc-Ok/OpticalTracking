/***********************************************************************
ColorNode - Class for arrays of vertex colors.
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

#ifndef COLORNODE_INCLUDED
#define COLORNODE_INCLUDED

#include <vector>

#include "Types.h"

#include "VRMLNode.h"

class ColorNode:public VRMLNode
	{
	/* Embedded classes: */
	public:
	typedef std::vector<Color> ColorList; // Type for lists of colors
	
	/* Elements: */
	private:
	ColorList colors; // The color array
	
	/* Constructors and destructors: */
	public:
	ColorNode(void); // Creates empty color node, to be filled in later
	ColorNode(VRMLParser& parser); // Creates color node by parsing VRML file
	
	/* Methods: */
	virtual void glRenderAction(VRMLRenderState& renderState) const;
	ColorList& getColors(void) // Returns the array of colors
		{
		return colors;
		};
	size_t getNumColors(void) const // Returns the number of colors in the array
		{
		return colors.size();
		};
	const Color& getColor(int index) const // Returns the index-th color in the array
		{
		return colors[index];
		};
	};

#endif
