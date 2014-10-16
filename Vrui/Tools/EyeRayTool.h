/***********************************************************************
EyeRayTool - Class to transform the ray direction of an input device to
point along the sight line from the main viewer to the input device.
Copyright (c) 2008-2010 Oliver Kreylos

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

#ifndef VRUI_EYERAYTOOL_INCLUDED
#define VRUI_EYERAYTOOL_INCLUDED

#include <Vrui/TransformTool.h>

namespace Vrui {

class EyeRayTool;

class EyeRayToolFactory:public ToolFactory
	{
	friend class EyeRayTool;
	
	/* Constructors and destructors: */
	public:
	EyeRayToolFactory(ToolManager& toolManager);
	virtual ~EyeRayToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class EyeRayTool:public TransformTool
	{
	friend class EyeRayToolFactory;
	
	/* Elements: */
	private:
	static EyeRayToolFactory* factory; // Pointer to the factory object for this class
	
	/* Constructors and destructors: */
	public:
	EyeRayTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	virtual ~EyeRayTool(void);
	
	/* Methods from Tool: */
	virtual void initialize(void);
	virtual const ToolFactory* getFactory(void) const;
	virtual void frame(void);
	};

}

#endif
