/***********************************************************************
DisplayList - Base class for objects that use OpenGL display lists for
rendering.
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

#include <SceneGraph/DisplayList.h>

#include <GL/gl.h>
#include <GL/GLContextData.h>

namespace SceneGraph {

/**************************************
Methods of class DisplayList::DataItem:
**************************************/

DisplayList::DataItem::DataItem(void)
	:displayListId(glGenLists(1)),
	 version(0)
	{
	}

DisplayList::DataItem::~DataItem(void)
	{
	glDeleteLists(displayListId,1);
	}

/****************************
Methods of class DisplayList:
****************************/

void DisplayList::initContext(GLContextData& contextData) const
	{
	/* Create a data item and store it in the OpenGL context: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	}

void DisplayList::update(void)
	{
	/* Bump up the geometry version number: */
	++version;
	}

void DisplayList::glRenderAction(GLContextData& contextData) const
	{
	/* Retrieve the data item: */
	DataItem* dataItem=contextData.retrieveDataItem<DataItem>(this);
	
	/* Check if the display list's contents need to be updated: */
	if(dataItem->version!=version)
		{
		/* Upload the new geometry into the display list: */
		glNewList(dataItem->displayListId,GL_COMPILE_AND_EXECUTE);
		createList(contextData);
		glEndList();
		
		/* Mark the display list as up-to-date: */
		dataItem->version=version;
		}
	else
		{
		/* Render the geometry stored in the display list: */
		glCallList(dataItem->displayListId);
		}
	}

}
