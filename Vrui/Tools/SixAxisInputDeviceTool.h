/***********************************************************************
SixAxisInputDeviceTool - Class for tools using six valuators for
translational and rotational axes to control virtual input devices.
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

#ifndef VRUI_SIXAXISINPUTDEVICETOOL_INCLUDED
#define VRUI_SIXAXISINPUTDEVICETOOL_INCLUDED

#include <Misc/FixedArray.h>
#include <Geometry/Vector.h>
#include <Vrui/InputDeviceTool.h>

/* Forward declarations: */
namespace Misc {
class ConfigurationFileSection;
}

namespace Vrui {

class SixAxisInputDeviceTool;

class SixAxisInputDeviceToolFactory:public ToolFactory
	{
	friend class SixAxisInputDeviceTool;
	
	/* Embedded classes: */
	private:
	struct Configuration // Structure holding tool (class) configuration
		{
		/* Elements: */
		public:
		bool selectButtonToggle; // Flag whether the input device selection button has toggle behavior
		Scalar translateFactor; // Scaling factor for all translation vectors
		Misc::FixedArray<Vector,3> translations; // Translation vectors in physical space
		Scalar rotateFactor; // Scaling factor for all scaled rotation axes
		Misc::FixedArray<Vector,3> rotations; // Scaled rotation axes in physical space
		
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
	SixAxisInputDeviceToolFactory(ToolManager& toolManager);
	virtual ~SixAxisInputDeviceToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual const char* getButtonFunction(int buttonSlotIndex) const;
	virtual const char* getValuatorFunction(int valuatorSlotIndex) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class SixAxisInputDeviceTool:public InputDeviceTool
	{
	friend class SixAxisInputDeviceToolFactory;
	
	/* Elements: */
	private:
	static SixAxisInputDeviceToolFactory* factory; // Pointer to the factory object for this class
	
	/* Configuration state: */
	SixAxisInputDeviceToolFactory::Configuration config; // The tool configuration
	Vector translations[3]; // Scaled translation vectors
	Vector rotations[3]; // Scaled scaled rotation axes
	
	/* Constructors and destructors: */
	public:
	SixAxisInputDeviceTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	
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
