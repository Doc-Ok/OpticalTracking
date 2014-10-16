/***********************************************************************
FlashlightTool - Class for tools that add an additional light source
into an environment when activated.
Copyright (c) 2004-2014 Oliver Kreylos

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

#ifndef VRUI_FLASHLIGHTTOOL_INCLUDED
#define VRUI_FLASHLIGHTTOOL_INCLUDED

#include <Geometry/Ray.h>
#include <Geometry/OrthogonalTransformation.h>
#include <GL/GLLight.h>
#include <Vrui/Geometry.h>
#include <Vrui/PointingTool.h>

/* Forward declarations: */
namespace Vrui {
class Lightsource;
class ToolManager;
}

namespace Vrui {

class FlashlightTool;

class FlashlightToolFactory:public ToolFactory
	{
	friend class FlashlightTool;
	
	/* Embedded classes: */
	private:
	struct Configuration // Structure to hold tool configuration data
		{
		/* Elements: */
		public:
		GLLight light; // OpenGL light source parameters for flashlight tools
		
		/* Constructors and destructors: */
		Configuration(void); // Creates a default configuration
		
		/* Methods: */
		void read(const Misc::ConfigurationFileSection& cfs); // Overrides configuration from configuration file section
		void write(Misc::ConfigurationFileSection& cfs) const; // Writes configuration to configuration file section
		};
	
	/* Elements: */
	Configuration configuration; // Default configuration for all tools
	
	/* Constructors and destructors: */
	public:
	FlashlightToolFactory(ToolManager& toolManager);
	virtual ~FlashlightToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class FlashlightTool:public PointingTool
	{
	friend class FlashlightToolFactory;
	
	/* Elements: */
	private:
	static FlashlightToolFactory* factory; // Pointer to the factory object for this class
	FlashlightToolFactory::Configuration configuration; // Private configuration of this tool
	Lightsource* lightsource; // Light source object allocated for this flashlight tool
	
	/* Transient state: */
	bool active; // Flag if the tool is currently active
	
	/* Constructors and destructors: */
	public:
	FlashlightTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	virtual ~FlashlightTool(void);
	
	/* Methods from Tool: */
	virtual void configure(const Misc::ConfigurationFileSection& configFileSection);
	virtual void storeState(Misc::ConfigurationFileSection& configFileSection) const;
	virtual void initialize(void);
	virtual void deinitialize(void);
	virtual const ToolFactory* getFactory(void) const;
	virtual void buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData);
	virtual void frame(void);
	};

}

#endif
