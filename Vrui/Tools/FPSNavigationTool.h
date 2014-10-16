/***********************************************************************
FPSNavigationTool - Class encapsulating the navigation behaviour of a
typical first-person shooter (FPS) game.
Copyright (c) 2005-2014 Oliver Kreylos

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

#ifndef VRUI_FPSNAVIGATIONTOOL_INCLUDED
#define VRUI_FPSNAVIGATIONTOOL_INCLUDED

#include <Misc/FixedArray.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/OrthogonalTransformation.h>
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
class InputDeviceAdapterMouse;
}

namespace Vrui {

class FPSNavigationTool;

class FPSNavigationToolFactory:public ToolFactory
	{
	friend class FPSNavigationTool;
	
	/* Embedded classes: */
	private:
	struct Configuration // Structure holding tool (class) configuration
		{
		/* Elements: */
		public:
		bool activationToggle; // Flag whether the activation button acts as a toggle
		Misc::FixedArray<Scalar,2> rotateFactors; // Distance the mouse has to be moved to rotate by one radians horizontally and vertically, respectively
		Misc::FixedArray<Scalar,2> moveSpeeds; // Moving speed when pressing move buttons in physical space units per second, for forward/back and left/right, respectively
		Scalar fallAcceleration; // Acceleration when falling in physical space units per second^2, defaults to g
		Scalar jumpVelocity; // Instantaneous vertical velocity when jumping in physical space units per second
		Scalar probeSize; // Size of probe to use when aligning surface frames in physical space units
		Scalar maxClimb; // Maximum amount of climb per frame in physical space units
		Scalar azimuthStep; // Discrete increment step in azimuth angle to fight motion sickness for HMD users
		bool fixAzimuth; // Flag whether to fix the tool's azimuth angle during movement
		bool levelOnExit; // Flag whether to reset the elevation angle to zero upon deactivating the tool
		bool drawHud; // Flag whether to draw the navigation heads-up display
		Color hudColor; // Color to draw the HUD
		float hudDist; // Distance of HUD plane from eye point in physical coordinate units
		float hudRadius; // Radius of HUD on HUD plane
		float hudFontSize; // HUD font size in physical coordinate units
		
		/* Constructors and destructors: */
		Configuration(void); // Creates default configuration
		
		/* Methods: */
		void load(const Misc::ConfigurationFileSection& cfs); // Loads configuration from configuration file section
		void save(Misc::ConfigurationFileSection& cfs) const; // Saves configuration to configuration file section
		};
	
	/* Elements: */
	private:
	Configuration config; // The class configuration
	
	/* Constructors and destructors: */
	public:
	FPSNavigationToolFactory(ToolManager& toolManager);
	virtual ~FPSNavigationToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual const char* getButtonFunction(int buttonSlotIndex) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class FPSNavigationTool:public SurfaceNavigationTool,public DeviceForwarder
	{
	friend class FPSNavigationToolFactory;
	
	/* Elements: */
	private:
	static FPSNavigationToolFactory* factory; // Pointer to the factory object for this class
	FPSNavigationToolFactory::Configuration config; // The tool configuration
	InputDevice* buttonDevice; // Pointer to the input device representing the forwarded movement buttons
	InputDeviceAdapterMouse* mouseAdapter; // Mouse adapter controlling the tool's input device (0 if none)
	GLNumberRenderer* numberRenderer; // Helper object to render numbers using a HUD-style font
	
	/* Transient navigation state: */
	Point footPos; // Current position of main viewer's foot in physical coordinates
	Scalar headHeight; // Height of viewer's head above the foot point
	NavTransform surfaceFrame; // Current local coordinate frame aligned to the surface in navigation coordinates
	Scalar azimuth; // Current azimuth of viewer position relative to local coordinate frame
	Scalar ratchetedAzimuth; // Current azimuth of viewer position as applied to navigation matrix, locked to multiples of ratchet amount
	Scalar elevation; // Current elevation of viewer position relative to local coordinate frame
	Vector controlVelocity; // Movement velocity prescribed by controls in frame coordinates
	Vector moveVelocity; // Current movement velocity in frame coordinates
	bool jump; // Flag whether the user requested a jump
	
	/* Private methods: */
	void applyNavState(void); // Applies the tool's current navigation state to the navigation transformation
	void initNavState(void); // Initializes the tool's navigation state when it is activated
	void stopNavState(void); // Leaves navigation mode
	
	/* Constructors and destructors: */
	public:
	FPSNavigationTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	~FPSNavigationTool(void);
	
	/* Methods from Tool: */
	virtual void configure(const Misc::ConfigurationFileSection& configFileSection);
	virtual void storeState(Misc::ConfigurationFileSection& configFileSection) const;
	virtual void initialize(void);
	virtual void deinitialize(void);
	virtual const ToolFactory* getFactory(void) const;
	virtual void buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData);
	virtual void frame(void);
	virtual void display(GLContextData& contextData) const;
	
	/* Methods from DeviceForwarder: */
	virtual std::vector<InputDevice*> getForwardedDevices(void);
	virtual InputDeviceFeatureSet getSourceFeatures(const InputDeviceFeature& forwardedFeature);
	virtual InputDevice* getSourceDevice(const InputDevice* forwardedDevice);
	virtual InputDeviceFeatureSet getForwardedFeatures(const InputDeviceFeature& sourceFeature);
	};

}

#endif
