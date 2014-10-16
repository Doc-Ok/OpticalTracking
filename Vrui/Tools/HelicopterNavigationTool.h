/***********************************************************************
HelicopterNavigationTool - Class for navigation tools using a simplified
helicopter flight model, a la Enemy Territory: Quake Wars' Anansi. Yeah,
I like that -- wanna fight about it?
Copyright (c) 2007-2013 Oliver Kreylos

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

#ifndef VRUI_HELICOPTERNAVIGATIONTOOL_INCLUDED
#define VRUI_HELICOPTERNAVIGATIONTOOL_INCLUDED

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

class HelicopterNavigationTool;

class HelicopterNavigationToolFactory:public ToolFactory
	{
	friend class HelicopterNavigationTool;
	
	/* Embedded classes: */
	private:
	struct Configuration // Structure holding tool (class) configuration
		{
		/* Elements: */
		public:
		bool activationToggle; // Flag whether the activation button acts as a toggle
		Misc::FixedArray<Scalar,3> rotateFactors; // Array of rotation speeds around the (pitch, roll, yaw) axes in radians/s
		Scalar g; // Acceleration of gravity in physical coordinate units/s^2
		Scalar collectiveMin,collectiveMax; // Min and max amounts of collective acceleration in physical coordinate units/s^2
		Scalar thrust; // Thrust acceleration in physical coordinate units/s^2
		Scalar brake; // Reverse thrust acceleration in physical coordinate units/s^2
		Misc::FixedArray<Scalar,3> dragCoefficients; // Drag coefficients in local x, y, z directions
		Misc::FixedArray<Scalar,2> viewAngleFactors; // View offset angle factors for hat switch valuators in radians
		Scalar probeSize; // Size of probe to use when aligning surface frames
		Scalar maxClimb; // Maximum amount of climb per frame
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
	HelicopterNavigationToolFactory(ToolManager& toolManager);
	virtual ~HelicopterNavigationToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual const char* getButtonFunction(int buttonSlotIndex) const;
	virtual const char* getValuatorFunction(int valuatorSlotIndex) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class HelicopterNavigationTool:public SurfaceNavigationTool
	{
	friend class HelicopterNavigationToolFactory;
	
	/* Elements: */
	private:
	static HelicopterNavigationToolFactory* factory; // Pointer to the factory object for this class
	GLNumberRenderer* numberRenderer; // Helper object to render numbers using a HUD-like font
	
	/* Configuration state: */
	HelicopterNavigationToolFactory::Configuration config; // The tool configuration
	
	/* Transient navigation state: */
	NavTransform surfaceFrame; // Current local coordinate frame aligned to the surface in navigation coordinates
	Rotation orientation; // Current orientation of virtual helicopter relative to current surface frame
	Scalar elevation; // Current elevation of virtual helicopter above the surface
	Vector velocity; // Current linear velocity of the virtual helicopter
	
	/* Private methods: */
	void applyNavState(void);
	void initNavState(void);
	
	/* Constructors and destructors: */
	public:
	HelicopterNavigationTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	virtual ~HelicopterNavigationTool(void);
	
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
