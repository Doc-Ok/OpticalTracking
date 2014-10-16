/***********************************************************************
MouseNavigationTool - Class encapsulating the navigation behaviour of a
mouse in the OpenInventor SoXtExaminerViewer.
Copyright (c) 2004-2013 Oliver Kreylos

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

#ifndef VRUI_MOUSENAVIGATIONTOOL_INCLUDED
#define VRUI_MOUSENAVIGATIONTOOL_INCLUDED

#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/OrthogonalTransformation.h>
#include <Vrui/GUIInteractor.h>
#include <Vrui/NavigationTool.h>

/* Forward declarations: */
class GLContextData;

namespace Vrui {

class MouseNavigationTool;

class MouseNavigationToolFactory:public ToolFactory
	{
	friend class MouseNavigationTool;
	
	/* Embedded classes: */
	private:
	struct Configuration // Structure containing tool settings
		{
		/* Elements: */
		public:
		Scalar rotatePlaneOffset; // Offset of rotation plane from screen plane
		Scalar rotateFactor; // Distance the device has to be moved to rotate by one radians
		bool invertDolly; // Flag whether to invert the switch between dollying/zooming
		Vector dollyingDirection; // Direction of dollying line in physical coordinates
		Vector scalingDirection; // Direction of scaling line in physical coordinates
		Scalar dollyFactor; // Distance the device has to be moved along the scaling line to dolly by one physical unit
		Scalar scaleFactor; // Distance the device has to be moved along the scaling line to scale by factor of e
		Scalar wheelDollyFactor; // Physical unit dolly amount for one wheel click
		Scalar wheelScaleFactor; // Scaling factor for one wheel click
		Scalar spinThreshold; // Distance the device has to be moved on the last step of rotation to activate spinning
		bool showScreenCenter; // Flag whether to draw the center of the screen during navigation
		bool interactWithWidgets; // Flag if the mouse navigation tool doubles as a widget tool (this is an evil hack)
		
		/* Constructors and destructors: */
		Configuration(void); // Creates default configuration
		
		/* Methods: */
		void read(const Misc::ConfigurationFileSection& cfs); // Overrides configuration from configuration file section
		void write(Misc::ConfigurationFileSection& cfs) const; // Writes configuration to configuration file section
		};
	
	/* Elements: */
	Configuration configuration; // Default configuration for all tools
	
	/* Constructors and destructors: */
	public:
	MouseNavigationToolFactory(ToolManager& toolManager);
	virtual ~MouseNavigationToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual const char* getButtonFunction(int buttonSlotIndex) const;
	virtual const char* getValuatorFunction(int valuatorSlotIndex) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class MouseNavigationTool:public NavigationTool,public GUIInteractor
	{
	friend class MouseNavigationToolFactory;
	
	/* Embedded classes: */
	private:
	enum NavigationMode // Enumerated type for states the tool can be in
		{
		IDLE,WIDGETING,ROTATING,SPINNING,PANNING,DOLLYING,SCALING,DOLLYING_WHEEL,SCALING_WHEEL
		};
	
	/* Elements: */
	static MouseNavigationToolFactory* factory; // Pointer to the factory object for this class
	MouseNavigationToolFactory::Configuration configuration; // Private configuration of this tool
	
	/* Transient navigation state: */
	Point currentPos; // Current projected position of mouse input device on screen
	double lastMoveTime; // Application time at which the projected position last changed
	Scalar currentValue; // Value of the associated valuator
	bool dolly; // Flag whether to dolly instead of scale
	NavigationMode navigationMode; // The tool's current navigation mode
	Point screenCenter; // Center of screen; center of rotation and scaling operations
	Vector dollyDirection; // Transformation direction of dollying (vector from eye to screen center)
	Point motionStart; // Start position of mouse motion
	Vector rotateOffset; // Offset vector applied to device position during rotations
	Point lastRotationPos; // Last mouse position during rotation
	Vector spinAngularVelocity; // Angular velocity when spinning
	Scalar currentWheelScale; // Current scale factor during mouse wheel scaling
	NavTrackerState preScale; // Transformation to be applied to the navigation transformation before scaling
	NavTrackerState rotation; // Current accumulated rotation transformation
	NavTrackerState postScale; // Transformation to be applied to the navigation transformation after scaling
	
	/* Private methods: */
	Point calcInteractionPos(void) const; // Returns the current device position in the interaction plane
	void startRotating(void); // Sets up rotation
	void startPanning(void); // Sets up panning
	void startDollying(void); // Sets up dollying
	void startScaling(void); // Sets up scaling
	
	/* Constructors and destructors: */
	public:
	MouseNavigationTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	
	/* Methods from Tool: */
	virtual void configure(const Misc::ConfigurationFileSection& configFileSection);
	virtual void storeState(Misc::ConfigurationFileSection& configFileSection) const;
	virtual const ToolFactory* getFactory(void) const;
	virtual void buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData);
	virtual void valuatorCallback(int valuatorSlotIndex,InputDevice::ValuatorCallbackData* cbData);
	virtual void frame(void);
	virtual void display(GLContextData& contextData) const;
	};

}

#endif
