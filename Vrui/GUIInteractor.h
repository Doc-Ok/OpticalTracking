/***********************************************************************
GUIInteractor - Helper class to implement tool classes that interact
with graphical user interface elements.
Copyright (c) 2010-2014 Oliver Kreylos

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

#ifndef VRUI_GUIINTERACTOR_INCLUDED
#define VRUI_GUIINTERACTOR_INCLUDED

#include <Geometry/Ray.h>
#include <Geometry/OrthogonalTransformation.h>
#include <GL/gl.h>
#include <GL/GLColor.h>
#include <Vrui/Geometry.h>

/* Forward declarations: */
class GLContextData;
namespace GLMotif {
class TextControlEvent;
class Widget;
}
namespace Vrui {
class InputDevice;
}

namespace Vrui {

class GUIInteractor
	{
	/* Elements: */
	private:
	static GUIInteractor* activeInteractor; // Static pointer to the currently active GUI interactor, or null
	bool useEyeRays; // Flag whether ray directions come from the device or from an eye line to the viewer
	Scalar rayOffset; // Ray origin offset for 6-DOF devices
	InputDevice* device; // Pointer to input device with which the interaction tool is associated
	Ray ray; // The current interaction ray
	bool pointing; // Flag if the interactor is pointing at a GUI widget
	bool interacting; // Flag if the interactor is sending pointer button and motion events to GUI widgets
	GLMotif::Widget* draggedWidget; // Pointer to the currently dragged widget, or 0 if not dragging
	NavTrackerState draggingTransform; // Current dragging transformation
	
	/* Constructors and destructors: */
	public:
	GUIInteractor(bool sUseEyeRays,Scalar sRayOffset,InputDevice* sDevice); // Creates an inactive interactor
	virtual ~GUIInteractor(void); // Destroys the interactor
	
	/* Methods: */
	void updateRay(void); // Calculates the interaction ray based on the current input device state
	const Ray& getRay(void) const // Returns the current interaction ray
		{
		return ray;
		}
	NavTrackerState calcInteractionTransform(void) const; // Calculates a transformation corresponding to the current interaction ray
	bool canActivate(void) const // Returns true if the interactor can be activated, or is currently active
		{
		return activeInteractor==0||activeInteractor==this;
		}
	bool isActive(void) const // Returns true if the interactor is distributing events
		{
		return interacting;
		}
	bool buttonDown(bool force); // Reacts to a button press with the current interaction ray; forces activation if flag is true; returns true if interactor became active
	void buttonUp(void); // Reacts to a button release with the current interaction ray
	void move(void); // Reacts to a change in interaction ray
	bool textControl(const GLMotif::TextControlEvent& textControlEvent); // Sends a text control event
	void glRenderAction(GLfloat rayWidth,const GLColor<GLfloat,4>& rayColor,GLContextData& contextData) const; // Draws the interactor's current state
	virtual Point calcHotSpot(void) const; // Returns current interaction position of GUI interactor
	};

}

#endif
