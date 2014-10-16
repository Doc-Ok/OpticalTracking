/***********************************************************************
ThumbstickTransformTool - Class to convert a two-axis analog thumb stick
into a 6-DOF input device moving inside a rectangle.
Copyright (c) 2014 Oliver Kreylos

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

#ifndef VRUI_THUMBSTICKTRANSFORMTOOL_INCLUDED
#define VRUI_THUMBSTICKTRANSFORMTOOL_INCLUDED

#include <Misc/FixedArray.h>
#include <Geometry/OrthonormalTransformation.h>
#include <GL/GLMaterial.h>
#include <Vrui/TransformTool.h>

/* Forward declarations: */
namespace Misc {
class ConfigurationFileSection;
}

namespace Vrui {

class ThumbstickTransformTool;

class ThumbstickTransformToolFactory:public ToolFactory
	{
	friend class ThumbstickTransformTool;
	
	/* Embedded classes: */
	private:
	struct Configuration // Structure holding tool (class) configuration
		{
		/* Elements: */
		public:
		TrackerState baseTransform; // Position and orientation around which the virtual device moves; rectangle is spanned by X and Y axes
		Misc::FixedArray<Scalar,2> translateFactors; // Scale factors from valuator values to displacements along the X and Y axes
		std::string deviceGlyphType; // Name of glyph type to use to visualize the virtual input device
		GLMaterial deviceGlyphMaterial; // Material properties for the device glyph
		
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
	ThumbstickTransformToolFactory(ToolManager& toolManager);
	virtual ~ThumbstickTransformToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual const char* getButtonFunction(int buttonSlotIndex) const;
	virtual const char* getValuatorFunction(int valuatorSlotIndex) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class ThumbstickTransformTool:public TransformTool
	{
	friend class ThumbstickTransformToolFactory;
	
	/* Elements: */
	private:
	static ThumbstickTransformToolFactory* factory; // Pointer to the factory object for this class
	ThumbstickTransformToolFactory::Configuration config; // The tool configuration
	
	/* Constructors and destructors: */
	public:
	ThumbstickTransformTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	
	/* Methods from Tool: */
	virtual void configure(const Misc::ConfigurationFileSection& configFileSection);
	virtual void storeState(Misc::ConfigurationFileSection& configFileSection) const;
	virtual void initialize(void);
	virtual const ToolFactory* getFactory(void) const;
	virtual void frame(void);
	};

}

#endif
