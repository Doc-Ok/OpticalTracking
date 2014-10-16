/***********************************************************************
ScaleBar - Class to draw a scale bar in Vrui applications. Scale bar is
implemented as a special top-level GLMotif widget for simplicity.
Copyright (c) 2010-2012 Oliver Kreylos

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

#ifndef VRUI_INTERNAL_SCALEBAR_INCLUDED
#define VRUI_INTERNAL_SCALEBAR_INCLUDED

#include <GLMotif/Draggable.h>
#include <GLMotif/Widget.h>
#include <Vrui/Geometry.h>
#include <Vrui/Vrui.h>

/* Forward declarations: */
class GLLabel;
namespace GLMotif {
class WidgetManager;
}

namespace Vrui {

class ScaleBar:public GLMotif::Widget,public GLMotif::Draggable
	{
	/* Elements: */
	private:
	GLMotif::WidgetManager* manager; // Pointer to the widget manager
	Scalar targetLength; // Physical-space target length of scale bar
	int currentMantissa; // Current step in quasi-binary length progression (1, 2, 5, 10, ...)
	int currentExponent; // Current base-10 exponent of navigation-space length of scale bar
	Scalar currentNavLength; // Current navigation-space length of scale bar in coordinate manager units
	Scalar currentScale; // Current scaling factor from navigational space to physical space
	GLLabel* lengthLabel; // Label to display the scale bar's current navigation-space length
	GLLabel* scaleLabel; // Label to display the current scaling factor from navigational space to physical space
	Scalar currentPhysLength; // Current physical-space length of scale bar
	
	/* Private methods: */
	void calcSize(const NavTransform& newNavigationTransformation); // Recalculates the scale bar's layout after a change in navigation scale
	void navigationChangedCallback(NavigationTransformationChangedCallbackData* cbData); // Callback called when the navigation transformation changes
	
	/* Constructors and destructors: */
	public:
	ScaleBar(const char* sName,GLMotif::WidgetManager* sManager); // Creates a scale bar of default size
	virtual ~ScaleBar(void);
	
	/* Methods from GLMotif::Widget: */
	virtual const GLMotif::WidgetManager* getManager(void) const
		{
		return manager;
		}
	virtual GLMotif::WidgetManager* getManager(void)
		{
		return manager;
		}
	virtual GLMotif::Vector calcNaturalSize(void) const;
	virtual void resize(const GLMotif::Box& newExterior);
	virtual void draw(GLContextData& contextData) const;
	virtual void pointerButtonDown(GLMotif::Event& event);
	virtual void pointerButtonUp(GLMotif::Event& event);
	};

}

#endif
