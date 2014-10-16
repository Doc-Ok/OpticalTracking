/***********************************************************************
Node - Base class for nodes, i.e., shared elements of rendering or other
state.
Copyright (c) 2009 Oliver Kreylos

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

#include <SceneGraph/Node.h>

#include <Misc/StringPrintf.h>
#include <SceneGraph/VRMLFile.h>

namespace SceneGraph {

/*********************************
Methods of class Node::FieldError:
*********************************/

Node::FieldError::FieldError(std::string errorString)
	:std::runtime_error(errorString)
	{
	}

/*********************
Methods of class Node:
*********************/

Node::~Node(void)
	{
	}

EventOut* Node::getEventOut(const char* fieldName) const
	{
	throw FieldError(Misc::stringPrintf("No eventOut %s in node class %s",fieldName,getClassName()));
	}

EventIn* Node::getEventIn(const char* fieldName)
	{
	throw FieldError(Misc::stringPrintf("No eventIn %s in node class %s",fieldName,getClassName()));
	}

void Node::parseField(const char* fieldName,VRMLFile& vrmlFile)
	{
	throw VRMLFile::ParseError(vrmlFile,Misc::stringPrintf("Unknown field \"%s\"",fieldName));
	}

void Node::update(void)
	{
	}

}
