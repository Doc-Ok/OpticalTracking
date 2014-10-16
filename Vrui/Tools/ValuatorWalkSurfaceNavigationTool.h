/***********************************************************************
ValuatorWalkSurfaceNavigationTool - Version of the
WalkSurfaceNavigationTool that uses a pair of valuators to move instead
of head position.
Copyright (c) 2013-2014 Oliver Kreylos

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

#ifndef VRUI_VALUATORWALKSURFACENAVIGATIONTOOL_INCLUDED
#define VRUI_VALUATORWALKSURFACENAVIGATIONTOOL_INCLUDED

#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/OrthogonalTransformation.h>
#include <Geometry/Plane.h>
#include <GL/gl.h>
#include <GL/GLObject.h>
#include <Vrui/Vrui.h>
#include <Vrui/DeviceForwarder.h>
#include <Vrui/SurfaceNavigationTool.h>

/* Forward declarations: */
namespace Misc {
class ConfigurationFileSection;
}
class GLContextData;
class GLNumberRenderer;
namespace Vrui {

class ValuatorWalkSurfaceNavigationTool;

class ValuatorWalkSurfaceNavigationToolFactory:public ToolFactory
	{
	friend class ValuatorWalkSurfaceNavigationTool;
	
	/* Embedded classes: */
	private:
	struct Configuration // Structure containing tool settings
		{
		/* Elements: */
		public:
		bool activationToggle; // Flag whether the activation button acts as a toggle
		bool centerOnActivation; // Flag if to center navigation on the head position when the tool is activated
		Point centerPoint; // Center point of movement circles on floor
		Scalar moveSpeed; // Maximum movement speed
		Scalar innerRadius; // Radius of circle of no motion around center point
		Scalar outerRadius; // Radius where maximum movement speed is reached
		Scalar valuatorMoveSpeeds[2]; // Maximum movement speeds in X and Y when using valuators
		Scalar valuatorViewFollowFactor; // Blending factor for valuator move direction between 0 (move along forward direction) to 1 (move along view direction)
		Vector centerViewDirection; // Central view direction
		Scalar rotateSpeed; // Maximum rotation speed in radians per second
		Scalar innerAngle; // Angle of no rotation around central view direction
		Scalar outerAngle; // Angle where maximum rotation speed is reached
		bool valuatorSnapRotate; // Flag whether valuator-based rotation is in discrete increments of valuatorRotateSpeed
		Scalar valuatorRotateSpeed; // Maximum horizontal rotation speed when using valuators in radians per second
		Scalar fallAcceleration; // Acceleration when falling in physical space units per second^2, defaults to g
		Scalar jetpackAcceleration; // Maximum acceleration of virtual jetpack in physical space units per second^2, defaults to 1.5*fallAcceleration
		Scalar probeSize; // Size of probe to use when aligning surface frames
		Scalar maxClimb; // Maximum amount of climb per frame
		bool fixAzimuth; // Flag whether to fix the tool's azimuth angle during panning
		bool drawMovementCircles; // Flag whether to draw the movement circles
		Color movementCircleColor; // Color for drawing movement circles
		bool drawHud; // Flag whether to draw a heads-up display
		float hudRadius; // Radius of heads-up display in Vrui physical units
		float hudFontSize; // Font size for heads-up display
		
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
	ValuatorWalkSurfaceNavigationToolFactory(ToolManager& toolManager);
	virtual ~ValuatorWalkSurfaceNavigationToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual const char* getButtonFunction(int buttonSlotIndex) const;
	virtual const char* getValuatorFunction(int valuatorSlotIndex) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class ValuatorWalkSurfaceNavigationTool:public SurfaceNavigationTool,public DeviceForwarder,public GLObject
	{
	friend class ValuatorWalkSurfaceNavigationToolFactory;
	
	/* Embedded classes: */
	private:
	struct ForwardedDevice // Helper structure to associate source input devices with forwarded input devices
		{
		/* Elements: */
		public:
		InputDevice* sourceDevice; // Pointer to source device
		InputDevice* virtualDevice; // Pointer to forwarded virtual device
		};
	
	struct ForwardedValuator // Helper structure to associate input valuators with valuator slots on forwarded valuator devices
		{
		/* Elements: */
		public:
		InputDevice* device; // Pointer to forwarded device
		int valuatorIndex; // Index of valuator feature on forwarded device
		};
	
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
	static ValuatorWalkSurfaceNavigationToolFactory* factory; // Pointer to the factory object for this class
	ValuatorWalkSurfaceNavigationToolFactory::Configuration configuration; // Private configuration of this tool
	int numValuatorDevices; // Number of forwarded valuator devices
	ForwardedDevice* valuatorDevices; // Array of pointers to the input devices representing the forwarded movement valuators
	ForwardedValuator* forwardedValuators; // Array of structures associating input valuator slots with forwarded valuators
	GLNumberRenderer* numberRenderer; // Helper class to render numbers using a HUD-style font
	
	/* Transient navigation state: */
	Point centerPoint; // Center point of movement circle while the navigation tool is active
	Point footPos; // Position of the main viewer's foot on the last frame
	Scalar headHeight; // Height of viewer's head above the foot point
	NavTransform surfaceFrame; // Current local coordinate frame aligned to the surface in navigation coordinates
	Scalar azimuth; // Current azimuth of view relative to local coordinate frame
	Scalar elevation; // Current elevation of view relative to local coordinate frame
	Scalar rotate; // Current valuator rotation speed in radians per second
	int lastSnapRotate,snapRotate; // Previous and current trivariate state of the rotation valuator in snap rotation mode
	Scalar jetpack; // Current acceleration of virtual jetpack in units per second^2
	Scalar fallVelocity; // Current falling velocity while airborne in units per second^2
	
	/* Private methods: */
	void applyNavState(void) const; // Sets the navigation transformation based on the tool's current navigation state
	void initNavState(void); // Initializes the tool's navigation state when it is activated
	
	/* Constructors and destructors: */
	public:
	ValuatorWalkSurfaceNavigationTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	virtual ~ValuatorWalkSurfaceNavigationTool(void);
	
	/* Methods from Tool: */
	virtual void configure(const Misc::ConfigurationFileSection& configFileSection);
	virtual void storeState(Misc::ConfigurationFileSection& configFileSection) const;
	virtual void initialize(void);
	virtual void deinitialize(void);
	virtual const ToolFactory* getFactory(void) const;
	virtual void buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData);
	virtual void valuatorCallback(int valuatorSlotIndex,InputDevice::ValuatorCallbackData* cbData);
	virtual void frame(void);
	virtual void display(GLContextData& contextData) const;
	
	/* Methods from DeviceForwarder: */
	virtual std::vector<InputDevice*> getForwardedDevices(void);
	virtual InputDeviceFeatureSet getSourceFeatures(const InputDeviceFeature& forwardedFeature);
	virtual InputDevice* getSourceDevice(const InputDevice* forwardedDevice);
	virtual InputDeviceFeatureSet getForwardedFeatures(const InputDeviceFeature& sourceFeature);
	
	/* Methods from GLObject: */
	virtual void initContext(GLContextData& contextData) const;
	};

}

#endif
