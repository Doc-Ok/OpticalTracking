/***********************************************************************
SixAxisTransformTool - Class to convert an input device with six
valuators into a virtual 6-DOF input device.
Copyright (c) 2010-2013 Oliver Kreylos

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

#ifndef VRUI_SIXAXISTRANSFORMTOOL_INCLUDED
#define VRUI_SIXAXISTRANSFORMTOOL_INCLUDED

#include <string>
#include <Misc/FixedArray.h>
#include <Misc/Optional.h>
#include <Geometry/Vector.h>
#include <GL/GLMaterial.h>
#include <Vrui/TransformTool.h>

/* Forward declarations: */
namespace Misc {
class ConfigurationFileSection;
}

namespace Vrui {

class SixAxisTransformTool;

class SixAxisTransformToolFactory:public ToolFactory
	{
	friend class SixAxisTransformTool;
	
	/* Embedded classes: */
	private:
	struct Configuration // Structure holding tool (class) configuration
		{
		/* Elements: */
		public:
		Scalar translateFactor; // Scaling factor for all translation vectors
		Misc::FixedArray<Vector,3> translations; // Translation vectors in physical space
		Scalar rotateFactor; // Scaling factor for all scaled rotation axes
		Misc::FixedArray<Vector,3> rotations; // Scaled rotation axes in physical space
		Misc::Optional<TrackerState> homePosition; // Position/orientation to which to return the virtual device when the home button is pressed
		Misc::Optional<TrackerState> position; // Initial position/orientation of virtual device when the tool is created
		std::string deviceGlyphType; // Name of glyph type to use to visualize the virtual input device
		GLMaterial deviceGlyphMaterial; // Material properties for the device glyph
		
		/* Constructors and destructors: */
		Configuration(void); // Creates default configuration
		
		/* Methods: */
		void load(const Misc::ConfigurationFileSection& cfs); // Loads configuration from configuration file section
		void save(Misc::ConfigurationFileSection& cfs) const; // Saves configuration to configuration file section
		TrackerState getHomePosition(void) const; // Returns the current home position
		};
	
	/* Elements: */
	private:
	Configuration config; // The class configuration
	
	/* Constructors and destructors: */
	public:
	SixAxisTransformToolFactory(ToolManager& toolManager);
	virtual ~SixAxisTransformToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual const char* getButtonFunction(int buttonSlotIndex) const;
	virtual const char* getValuatorFunction(int valuatorSlotIndex) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class SixAxisTransformTool:public TransformTool
	{
	friend class SixAxisTransformToolFactory;
	
	/* Elements: */
	private:
	static SixAxisTransformToolFactory* factory; // Pointer to the factory object for this class
	
	/* Configuration state: */
	SixAxisTransformToolFactory::Configuration config; // The tool configuration
	Vector translations[3]; // Scaled translation vectors
	Vector rotations[3]; // Scaled scaled rotation axes
	
	/* Constructors and destructors: */
	public:
	SixAxisTransformTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	
	/* Methods from Tool: */
	virtual void configure(const Misc::ConfigurationFileSection& configFileSection);
	virtual void storeState(Misc::ConfigurationFileSection& configFileSection) const;
	virtual void initialize(void);
	virtual const ToolFactory* getFactory(void) const;
	virtual void buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData);
	virtual void frame(void);
	};

}

#endif
