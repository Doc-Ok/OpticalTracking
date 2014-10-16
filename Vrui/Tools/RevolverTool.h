/***********************************************************************
RevolverTool - Class to control multiple buttons (and tools) from a
single button using a revolver metaphor. Generalized from the rotator
tool initially developed by Braden Pellett and Jordan van Aalsburg.
Copyright (c) 2008-2013 Oliver Kreylos

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

#ifndef VRUI_REVOLVERTOOL_INCLUDED
#define VRUI_REVOLVERTOOL_INCLUDED

#include <GL/GLNumberRenderer.h>
#include <Vrui/TransformTool.h>

namespace Vrui {

class RevolverTool;

class RevolverToolFactory:public ToolFactory
	{
	friend class RevolverTool;
	
	/* Elements: */
	private:
	int numChambers; // Number of chambers (button/valuator sets) on the revolver tool's virtual input device
	
	/* Constructors and destructors: */
	public:
	RevolverToolFactory(ToolManager& toolManager);
	virtual ~RevolverToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual const char* getButtonFunction(int buttonSlotIndex) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class RevolverTool:public TransformTool
	{
	friend class RevolverToolFactory;
	
	/* Elements: */
	private:
	static RevolverToolFactory* factory; // Pointer to the factory object for this class
	GLNumberRenderer numberRenderer; // Helper class to render numbers using a HUD-style font
	
	int currentChamber; // Index of the currently mapped chamber on the virtual input device
	double showNumbersTime; // Application time until which to show the virtual button numbers
	
	/* Constructors and destructors: */
	public:
	RevolverTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	virtual ~RevolverTool(void);
	
	/* Methods from Tool: */
	virtual void configure(const Misc::ConfigurationFileSection& configFileSection);
	virtual void storeState(Misc::ConfigurationFileSection& configFileSection) const;
	virtual void initialize(void);
	virtual const ToolFactory* getFactory(void) const;
	virtual void buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData);
	virtual void valuatorCallback(int valuatorSlotIndex,InputDevice::ValuatorCallbackData* cbData);
	virtual void frame(void);
	virtual void display(GLContextData& contextData) const;
	
	/* Methods from class DeviceForwarder: */
	virtual InputDeviceFeatureSet getSourceFeatures(const InputDeviceFeature& forwardedFeature);
	virtual InputDeviceFeatureSet getForwardedFeatures(const InputDeviceFeature& sourceFeature);
	};

}

#endif
