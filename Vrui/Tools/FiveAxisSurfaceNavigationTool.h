/***********************************************************************
FiveAxisSurfaceNavigationTool - Class for navigation tools that use a
six-axis spaceball or similar input device to move along an application-
defined surface.
Copyright (c) 2012-2013 Oliver Kreylos

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

#ifndef VRUI_FIVEAXISSURFACENAVIGATIONTOOL_INCLUDED
#define VRUI_FIVEAXISSURFACENAVIGATIONTOOL_INCLUDED

#include <Misc/FixedArray.h>
#include <GL/gl.h>
#include <GL/GLObject.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/OrthogonalTransformation.h>
#include <Vrui/SurfaceNavigationTool.h>

/* Forward declarations: */
namespace Misc {
class ConfigurationFileSection;
}

namespace Vrui {

class FiveAxisSurfaceNavigationTool;

class FiveAxisSurfaceNavigationToolFactory:public ToolFactory
	{
	friend class FiveAxisSurfaceNavigationTool;
	
	/* Embedded classes: */
	private:
	struct Configuration // Structure holding tool (class) configuration
		{
		/* Elements: */
		public:
		Misc::FixedArray<Scalar,2> translateFactors; // Array of translation speeds along the (x, y) axes in physical units/s
		Misc::FixedArray<Scalar,2> rotateFactors; // Array of rotation speeds around the (yaw, pitch) axes in radians/s
		Scalar zoomFactor; // Factor for the zoom axis
		Scalar probeSize; // Size of probe to use when aligning surface frames
		Scalar maxClimb; // Maximum amount of climb per frame
		bool fixAzimuth; // Flag whether to fix the tool's azimuth angle during movement
		bool showCompass; // Flag whether to draw a virtual compass
		Scalar compassSize; // Size of compass rose
		Scalar compassThickness; // Thickness of compass rose's ring
		bool showScreenCenter; // Flag whether to draw the center of the screen during navigation
		
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
	FiveAxisSurfaceNavigationToolFactory(ToolManager& toolManager);
	virtual ~FiveAxisSurfaceNavigationToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual const char* getValuatorFunction(int valuatorSlotIndex) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class FiveAxisSurfaceNavigationTool:public SurfaceNavigationTool,public GLObject
	{
	friend class FiveAxisSurfaceNavigationToolFactory;
	
	/* Embedded classes: */
	private:
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		public:
		GLuint compassDisplayList; // ID of display list to draw the compass rose
		
		/* Constructors and destructors: */
		DataItem(void);
		virtual ~DataItem(void);
		};
	
	/* Elements: */
	static FiveAxisSurfaceNavigationToolFactory* factory; // Pointer to the factory object for this class
	
	/* Configuration state: */
	FiveAxisSurfaceNavigationToolFactory::Configuration config; // The tool configuration
	
	/* Transient navigation state: */
	int numActiveAxes; // Number of non-zero valuators, to determine when to activate and deactivate the tool
	NavTransform surfaceFrame; // Current local coordinate frame aligned to the surface in navigation coordinates
	Scalar azimuth; // Current azimuth of viewer position relative to local coordinate frame
	Scalar elevation; // Current elevation of viewer position relative to local coordinate frame
	bool showCompass; // Flag if the virtual compass is currently shown
	
	/* Private methods: */
	void applyNavState(void) const; // Sets the navigation transformation based on the tool's current navigation state
	void initNavState(void); // Initializes the tool's navigation state when it is activated
	void realignSurfaceFrame(NavTransform& newSurfaceFrame); // Re-aligns the tool's surface frame after a relevant change
	void navigationTransformationChangedCallback(Misc::CallbackData* cbData); // Callback called when the navigation transformation changes
	
	/* Constructors and destructors: */
	public:
	FiveAxisSurfaceNavigationTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	virtual ~FiveAxisSurfaceNavigationTool(void);
	
	/* Methods from Tool: */
	virtual void configure(const Misc::ConfigurationFileSection& configFileSection);
	virtual void storeState(Misc::ConfigurationFileSection& configFileSection) const;
	virtual const ToolFactory* getFactory(void) const;
	virtual void valuatorCallback(int valuatorSlotIndex,InputDevice::ValuatorCallbackData* cbData);
	virtual void frame(void);
	virtual void display(GLContextData& contextData) const;
	
	/* Methods from GLObject: */
	virtual void initContext(GLContextData& contextData) const;
	};

}

#endif
