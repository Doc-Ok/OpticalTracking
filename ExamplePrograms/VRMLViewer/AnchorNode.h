/***********************************************************************
AnchorNode - Node class for anchors linking to external VRML worlds or
other data.
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

#ifndef ANCHORNODE_INCLUDED
#define ANCHORNODE_INCLUDED

#include <vector>

#include "Types.h"
#include "GroupNode.h"

class AnchorNode:public GroupNode
	{
	/* Elements: */
	private:
	String description;
	std::vector<String> parameter;
	std::vector<String> url;
	
	/* Constructors and destructors: */
	public:
	AnchorNode(VRMLParser& parser); // Initializes the node from the given VRML parser
	};

#endif
