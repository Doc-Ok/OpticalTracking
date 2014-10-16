/***********************************************************************
SceneGraphViewer - Vislet class to render a scene graph loaded from one
or more VRML 2.0 files.
Copyright (c) 2009-2013 Oliver Kreylos

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

#ifndef VRUI_VISLETS_SCENEGRAPHVIEWER_INCLUDED
#define VRUI_VISLETS_SCENEGRAPHVIEWER_INCLUDED

#include <SceneGraph/GroupNode.h>
#include <Vrui/Vislet.h>

/* Forward declarations: */
namespace Vrui {
class VisletManager;
}

namespace Vrui {

namespace Vislets {

class SceneGraphViewer;

class SceneGraphViewerFactory:public Vrui::VisletFactory
	{
	friend class SceneGraphViewer;
	
	/* Constructors and destructors: */
	public:
	SceneGraphViewerFactory(Vrui::VisletManager& visletManager);
	virtual ~SceneGraphViewerFactory(void);
	
	/* Methods: */
	virtual Vislet* createVislet(int numVisletArguments,const char* const visletArguments[]) const;
	virtual void destroyVislet(Vislet* vislet) const;
	};

class SceneGraphViewer:public Vrui::Vislet
	{
	friend class SceneGraphViewerFactory;
	
	/* Elements: */
	static SceneGraphViewerFactory* factory; // Pointer to the factory object for this class
	
	SceneGraph::GroupNodePointer root; // The scene graph root node
	bool navigational; // Flag whether to render the scene graph in navigational or physical coordinates
	
	/* Constructors and destructors: */
	public:
	SceneGraphViewer(int numArguments,const char* const arguments[]);
	virtual ~SceneGraphViewer(void);
	
	/* Methods: */
	public:
	virtual VisletFactory* getFactory(void) const;
	virtual void disable(void);
	virtual void enable(void);
	virtual void display(GLContextData& contextData) const;
	};

}

}

#endif
