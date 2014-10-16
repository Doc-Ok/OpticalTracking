/***********************************************************************
ArcInfoExportFileNode - Class to represent an ARC/INFO export file as a
collection of line sets, point sets, and face sets.
Copyright (c) 2009-2011 Oliver Kreylos

This file is part of the Simple Scene Graph Renderer (SceneGraph).

The Simple Scene Graph Renderer is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Simple Scene Graph Renderer is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Simple Scene Graph Renderer; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef SCENEGRAPH_ARCINFOEXPORTFILENODE_INCLUDED
#define SCENEGRAPH_ARCINFOEXPORTFILENODE_INCLUDED

#include <SceneGraph/FieldTypes.h>
#include <SceneGraph/GroupNode.h>

/* Forward declarations: */
namespace Cluster {
class Multiplexer;
}

namespace SceneGraph {

class ArcInfoExportFileNode:public GroupNode
	{
	/* Elements: */
	
	/* Fields: */
	public:
	MFString url;
	
	/* Derived state: */
	protected:
	Cluster::Multiplexer* multiplexer; // Multiplexer to read ARC/INFO export files in a cluster environment
	
	/* Constructors and destructors: */
	public:
	ArcInfoExportFileNode(void); // Creates an uninitialized ARC/INFO export file node
	
	/* Methods from Node: */
	static const char* getStaticClassName(void);
	virtual const char* getClassName(void) const;
	virtual EventOut* getEventOut(const char* fieldName) const;
	virtual EventIn* getEventIn(const char* fieldName);
	virtual void parseField(const char* fieldName,VRMLFile& vrmlFile);
	virtual void update(void);
	};

}

#endif
