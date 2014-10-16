/***********************************************************************
WalkSurfaceNavigationTool - Version of the WalkNavigationTool that lets
a user navigate along an application-defined surface.
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

#ifndef VRUI_WALKSURFACENAVIGATIONTOOL_INCLUDED
#define VRUI_WALKSURFACENAVIGATIONTOOL_INCLUDED

#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/OrthogonalTransformation.h>
#include <Geometry/Plane.h>
#include <GL/gl.h>
#include <GL/GLObject.h>
#include <GL/GLNumberRenderer.h>
#include <Vrui/Vrui.h>
#include <Vrui/SurfaceNavigationTool.h>

/* Forward declarations: */
class GLContextData;

namespace Vrui {

class WalkSurfaceNavigationTool;

class WalkSurfaceNavigationToolFactory:public ToolFactory
	{
	friend class WalkSurfaceNavigationTool;
	
	/* Elements: */
	private:
	bool centerOnActivation; // Flag if to center navigation on the head position when the tool is activated
	Point centerPoint; // Center point of movement circles on floor
	Scalar moveSpeed; // Maximum movement speed
	Scalar innerRadius; // Radius of circle of no motion around center point
	Scalar outerRadius; // Radius where maximum movement speed is reached
	Vector centerViewDirection; // Central view direction
	Scalar rotateSpeed; // Maximum rotation speed in radians per second
	Scalar innerAngle; // Angle of no rotation around central view direction
	Scalar outerAngle; // Angle where maximum rotation speed is reached
	Scalar fallAcceleration; // Acceleration when falling in physical space units per second^2, defaults to g
	Scalar jetpackAcceleration; // Maximum acceleration of virtual jetpack in physical space units per second^2, defaults to 1.5*fallAcceleration
	Scalar probeSize; // Size of probe to use when aligning surface frames
	Scalar maxClimb; // Maximum amount of climb per frame
	bool fixAzimuth; // Flag whether to fix the tool's azimuth angle during panning
	bool drawMovementCircles; // Flag whether to draw the movement circles
	Color movementCircleColor; // Color for drawing movement circles
	bool drawHud; // Flag whether to draw a heads-up display
	float hudFontSize; // Font size for heads-up display
	
	/* Constructors and destructors: */
	public:
	WalkSurfaceNavigationToolFactory(ToolManager& toolManager);
	virtual ~WalkSurfaceNavigationToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual const char* getButtonFunction(int buttonSlotIndex) const;
	virtual const char* getValuatorFunction(int valuatorSlotIndex) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class WalkSurfaceNavigationTool:public SurfaceNavigationTool,public GLObject
	{
	friend class WalkSurfaceNavigationToolFactory;
	
	/* Embedded classes: */
	private:
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		public:
		GLuint movementCircleListId; // Display list ID to render movement circles
		GLuint hudListId; // Display list ID to render the hud
		
		/* Constructors and destructors: */
		DataItem(void);
		virtual ~DataItem(void);
		};
	
	/* Elements: */
	private:
	static WalkSurfaceNavigationToolFactory* factory; // Pointer to the factory object for this class
	GLNumberRenderer numberRenderer; // Helper class to render numbers using a HUD-style font
	
	/* Transient navigation state: */
	Point centerPoint; // Center point of movement circle while the navigation tool is active
	Point footPos; // Position of the main viewer's foot on the last frame
	Scalar headHeight; // Height of viewer's head above the foot point
	NavTransform surfaceFrame; // Current local coordinate frame aligned to the surface in navigation coordinates
	Scalar azimuth; // Current azimuth of view relative to local coordinate frame
	Scalar elevation; // Current elevation of view relative to local coordinate frame
	Scalar jetpack; // Current acceleration of virtual jetpack in units per second^2
	Scalar fallVelocity; // Current falling velocity while airborne in units per second^2
	
	/* Private methods: */
	void applyNavState(void) const; // Sets the navigation transformation based on the tool's current navigation state
	void initNavState(void); // Initializes the tool's navigation state when it is activated
	
	/* Constructors and destructors: */
	public:
	WalkSurfaceNavigationTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	
	/* Methods from Tool: */
	virtual const ToolFactory* getFactory(void) const;
	virtual void buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData);
	virtual void valuatorCallback(int valuatorSlotIndex,InputDevice::ValuatorCallbackData* cbData);
	virtual void frame(void);
	virtual void display(GLContextData& contextData) const;
	
	/* Methods from GLObject: */
	virtual void initContext(GLContextData& contextData) const;
	};

}

#endif
