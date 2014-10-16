/***********************************************************************
OffsetTool - Class to offset the position of an input device by a fixed
transformation to extend the user's arm.
Copyright (c) 2006-2010 Oliver Kreylos

This file is part of the Virtual Reality User Interface Library (Vrui).

The Virtual Reality User Interface Library is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The Virtual Reality User Interface Library is distributed in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Virtual Reality User Interface Library; if not, write to the
Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#ifndef VRUI_OFFSETTOOL_INCLUDED
#define VRUI_OFFSETTOOL_INCLUDED

#include <Geometry/OrthonormalTransformation.h>
#include <Vrui/TransformTool.h>

namespace Vrui {

class OffsetTool;

class OffsetToolFactory:public ToolFactory
	{
	friend class OffsetTool;
	
	/* Elements: */
	private:
	ONTransform offset; // Offset transformation from source input device to transformed input device
	
	/* Constructors and destructors: */
	public:
	OffsetToolFactory(ToolManager& toolManager);
	virtual ~OffsetToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class OffsetTool:public TransformTool
	{
	friend class OffsetToolFactory;
	
	/* Elements: */
	private:
	static OffsetToolFactory* factory; // Pointer to the factory object for this class
	
	/* Constructors and destructors: */
	public:
	OffsetTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	virtual ~OffsetTool(void);
	
	/* Methods from Tool: */
	virtual const ToolFactory* getFactory(void) const;
	virtual void frame(void);
	};

}

#endif
