/***********************************************************************
MultiShiftButtonTool - Class to switch between mulitple planes of
buttons and/or valuators by pressing one from an array of "radio
buttons."
Copyright (c) 2012-2014 Oliver Kreylos

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

#ifndef VRUI_MULTISHIFTBUTTONTOOL_INCLUDED
#define VRUI_MULTISHIFTBUTTONTOOL_INCLUDED

#include <Vrui/TransformTool.h>

namespace Vrui {

class MultiShiftButtonTool;

class MultiShiftButtonToolFactory:public ToolFactory
	{
	friend class MultiShiftButtonTool;
	
	/* Embedded classes: */
	private:
	struct Configuration // Structure containing tool settings
		{
		/* Elements: */
		public:
		int numPlanes; // Default number of button/valuator planes (and radio buttons) for new tools of this class
		bool forwardRadioButtons; // Flag whether the radio buttons themselves are forwarded to their button/valuator planes
		bool resetFeatures; // Flag whether to reset buttons and valuators when shifting away from their plane
		int initialPlane; // Plane to be activated when a new tool is created
		
		/* Constructors and destructors: */
		Configuration(void); // Creates default configuration
		
		/* Methods: */
		void read(const Misc::ConfigurationFileSection& cfs); // Overrides configuration from configuration file section
		void write(Misc::ConfigurationFileSection& cfs) const; // Writes configuration to configuration file section
		};
	
	/* Elements: */
	Configuration config; // Default configuration for all tools
	
	/* Constructors and destructors: */
	public:
	MultiShiftButtonToolFactory(ToolManager& toolManager);
	virtual ~MultiShiftButtonToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual const char* getButtonFunction(int buttonSlotIndex) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class MultiShiftButtonTool:public TransformTool
	{
	friend class MultiShiftButtonToolFactory;
	
	/* Elements: */
	private:
	static MultiShiftButtonToolFactory* factory; // Pointer to the factory object for this class
	MultiShiftButtonToolFactory::Configuration config; // Private configuration of this tool
	int numForwardedButtons; // Total number of forwarded buttons per plane, optionally including the radio button itself
	int firstForwardedButton; // Index of first forwarded button in each button plane
	int requestedPlane; // Index of button/valuator plane requested by a radio button
	int nextPlane; // Index of button/valuator plane to be installed on next frame
	int currentPlane; // Index of currently active button/valuator plane
	
	/* Constructors and destructors: */
	public:
	MultiShiftButtonTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	virtual ~MultiShiftButtonTool(void);
	
	/* Methods from Tool: */
	virtual void configure(const Misc::ConfigurationFileSection& configFileSection);
	virtual void storeState(Misc::ConfigurationFileSection& configFileSection) const;
	virtual void initialize(void);
	virtual void deinitialize(void);
	virtual const ToolFactory* getFactory(void) const;
	virtual void buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData);
	virtual void valuatorCallback(int valuatorSlotIndex,InputDevice::ValuatorCallbackData* cbData);
	virtual void frame(void);
	
	/* Methods from class DeviceForwarder: */
	virtual InputDeviceFeatureSet getSourceFeatures(const InputDeviceFeature& forwardedFeature);
	virtual InputDeviceFeatureSet getForwardedFeatures(const InputDeviceFeature& sourceFeature);
	};

}

#endif
