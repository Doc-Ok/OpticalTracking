/***********************************************************************
UserInterfaceTool - Base class for tools related to user interfaces
(interaction with dialog boxes, context menus, virtual input devices).
Copyright (c) 2008-2014 Oliver Kreylos

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

#ifndef VRUI_USERINTERFACETOOL_INCLUDED
#define VRUI_USERINTERFACETOOL_INCLUDED

#include <Geometry/Ray.h>
#include <GL/gl.h>
#include <GL/GLColor.h>
#include <Vrui/Geometry.h>
#include <Vrui/Tool.h>

namespace Vrui {

/* Forward declarations: */
class UserInterfaceTool;

class UserInterfaceToolFactory:public ToolFactory
	{
	friend class UserInterfaceTool;
	
	/* Embedded classes: */
	private:
	struct Configuration // Structure containing tool settings
		{
		/* Elements: */
		public:
		bool useEyeRay; // Flag whether to use an eyeline from the main viewer or the device's ray direction for ray-based interaction
		Scalar rayOffset; // Amount by which to shift the selection ray backwards to simplify interaction
		bool drawRay; // Flag whether to draw the interaction ray
		GLColor<GLfloat,4> rayColor; // Color to draw the ray
		GLfloat rayWidth; // Cosmetic line width to draw the ray
		
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
	UserInterfaceToolFactory(ToolManager& toolManager);
	virtual ~UserInterfaceToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	};

class UserInterfaceTool:public Tool
	{
	friend class UserInterfaceToolFactory;
	
	/* Elements: */
	private:
	static UserInterfaceToolFactory* factory; // Pointer to the factory object for this class
	UserInterfaceToolFactory::Configuration configuration; // Private configuration of this tool
	protected:
	InputDevice* interactionDevice; // Pointer to the input device used for user interface interaction
	
	/* Protected methods: */
	protected:
	bool isUseEyeRay(void) const // Returns true if input devices use eyelines for ray-based interaction
		{
		return configuration.useEyeRay;
		}
	Scalar getRayOffset(void) const // Returns the ray origin offset for ray-based interaction
		{
		return configuration.rayOffset;
		}
	Point getInteractionPosition(void) const // Returns a position for point-based interaction
		{
		return interactionDevice->getPosition();
		}
	Ray calcInteractionRay(void) const; // Returns a ray for ray-based interaction
	Point calcRayPoint(const Ray& ray) const; // Returns an interaction point for ray-based tools
	ONTransform calcRayTransform(const Ray& ray) const; // Returns an interaction transformation for ray-based tools
	bool isDrawRay(void) const // Returns true if the interaction ray is to be drawn
		{
		return configuration.drawRay;
		}
	const GLColor<GLfloat,4>& getRayColor(void) const // Returns the color with which to draw the interaction ray
		{
		return configuration.rayColor;
		}
	GLfloat getRayWidth(void) const // Returns the cosmetic line width with which to draw the interaction ray
		{
		return configuration.rayWidth;
		}
	
	/* Constructors and destructors: */
	public:
	UserInterfaceTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	
	/* Methods from Tool: */
	virtual void configure(const Misc::ConfigurationFileSection& configFileSection);
	virtual void storeState(Misc::ConfigurationFileSection& configFileSection) const;
	};

}

#endif
