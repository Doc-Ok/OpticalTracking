/***********************************************************************
SixAxisSurfaceNavigationTool - Class to convert an input device with six
valuators into a surface-aligned navigation tool.
Copyright (c) 2011-2013 Oliver Kreylos

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

#ifndef VRUI_SIXAXISSURFACENAVIGATIONTOOL_INCLUDED
#define VRUI_SIXAXISSURFACENAVIGATIONTOOL_INCLUDED

#include <Misc/FixedArray.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/OrthogonalTransformation.h>
#include <GL/gl.h>
#include <Vrui/Vrui.h>
#include <Vrui/SurfaceNavigationTool.h>

/* Forward declarations: */
namespace Misc {
class ConfigurationFileSection;
}
class GLNumberRenderer;

namespace Vrui {

class SixAxisSurfaceNavigationTool;

class SixAxisSurfaceNavigationToolFactory:public ToolFactory
	{
	friend class SixAxisSurfaceNavigationTool;
	
	/* Embedded classes: */
	private:
	struct Configuration // Structure holding tool (class) configuration
		{
		/* Elements: */
		public:
		bool activationToggle; // Flag whether the activation button acts as a toggle
		Misc::FixedArray<Scalar,3> translateFactors; // Array of translation speeds along the (x, y, z) axes in physical units/s
		Misc::FixedArray<Scalar,3> rotateFactors; // Array of rotation speeds around the (pitch, roll, yaw) axes in radians/s
		bool canRoll; // Flag whether to the tool is allowed to roll around the local Y axis
		bool bankTurns; // Flag whether the roll angle is locked to the yaw angular velocity
		Scalar bankFactor; // Amount of rotation during banking turns
		Scalar levelSpeed; // Relative speed at which the navigation tool levels to a zero roll angle
		bool canFly; // Flag whether the tool is allowed to "fly" above the surface
		Scalar probeSize; // Size of probe to use when aligning surface frames
		Scalar maxClimb; // Maximum amount of climb per frame
		bool fixAzimuth; // Flag whether to fix the tool's azimuth angle during movement
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
	SixAxisSurfaceNavigationToolFactory(ToolManager& toolManager);
	virtual ~SixAxisSurfaceNavigationToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual const char* getButtonFunction(int buttonSlotIndex) const;
	virtual const char* getValuatorFunction(int valuatorSlotIndex) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class SixAxisSurfaceNavigationTool:public SurfaceNavigationTool
	{
	friend class SixAxisSurfaceNavigationToolFactory;
	
	/* Elements: */
	private:
	static SixAxisSurfaceNavigationToolFactory* factory; // Pointer to the factory object for this class
	GLNumberRenderer* numberRenderer; // Helper object to render numbers using a HUD-like font
	
	/* Configuration state: */
	SixAxisSurfaceNavigationToolFactory::Configuration config; // The tool configuration
	
	/* Transient navigation state: */
	Point headPos; // Current head position in physical coordinates
	NavTransform surfaceFrame; // Current local coordinate frame aligned to the surface in navigation coordinates
	Scalar angles[3]; // Current orientation relative to current surface frame as Euler angles
	
	/* Private methods: */
	void applyNavState(void);
	void initNavState(void);
	
	/* Constructors and destructors: */
	public:
	SixAxisSurfaceNavigationTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	virtual ~SixAxisSurfaceNavigationTool(void);
	
	/* Methods from Tool: */
	virtual void configure(const Misc::ConfigurationFileSection& configFileSection);
	virtual void storeState(Misc::ConfigurationFileSection& configFileSection) const;
	virtual void initialize(void);
	virtual const ToolFactory* getFactory(void) const;
	virtual void buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData);
	virtual void frame(void);
	virtual void display(GLContextData& contextData) const;
	};

}

#endif
