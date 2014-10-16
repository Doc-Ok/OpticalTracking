/***********************************************************************
PanelButtonTool - Class to map a single input device button to several
virtual input device buttons by presenting an extensible panel with GUI
buttons.
Copyright (c) 2013 Oliver Kreylos

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

#ifndef PANELBUTTONTOOL_INCLUDED
#define PANELBUTTONTOOL_INCLUDED

#include <Vrui/TransformTool.h>

/* Forward declarations: */
namespace Misc {
class ConfigurationFileSection;
}
namespace GLMotif {
class PopupWindow;
class RowColumn;
}

namespace Vrui {

class PanelButtonTool;

class PanelButtonToolFactory:public ToolFactory
	{
	friend class PanelButtonTool;
	
	/* Embedded classes: */
	private:
	struct Configuration // Structure holding tool (class) configuration
		{
		/* Elements: */
		public:
		bool panelVertical; // Flag whether panel buttons are arranged vertically or horizontally
		bool dynamic; // Flag whether panel buttons can be added or removed dynamically
		int numButtons; // Initial number of buttons on the button panel
		
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
	PanelButtonToolFactory(ToolManager& toolManager);
	virtual ~PanelButtonToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual const char* getButtonFunction(int buttonSlotIndex) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class PanelButtonTool:public TransformTool
	{
	friend class PanelButtonToolFactory;
	
	/* Elements: */
	private:
	static PanelButtonToolFactory* factory; // Pointer to the factory object for this class
	
	/* Configuration state: */
	PanelButtonToolFactory::Configuration config; // The tool configuration
	
	GLMotif::PopupWindow* panelPopup; // Pointer to the top-level widget containing the button panel
	GLMotif::RowColumn* panel; // Pointer to the button panel widget
	int currentButton; // Index of the currently active virtual button
	
	/* Constructors and destructors: */
	public:
	PanelButtonTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	virtual ~PanelButtonTool(void);
	
	/* Methods from Tool: */
	virtual void configure(const Misc::ConfigurationFileSection& configFileSection);
	virtual void storeState(Misc::ConfigurationFileSection& configFileSection) const;
	virtual void initialize(void);
	virtual const ToolFactory* getFactory(void) const;
	virtual void buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData);
	virtual void frame(void);
	
	/* Methods from class DeviceForwarder: */
	virtual InputDeviceFeatureSet getSourceFeatures(const InputDeviceFeature& forwardedFeature);
	virtual InputDeviceFeatureSet getForwardedFeatures(const InputDeviceFeature& sourceFeature);
	};

}

#endif
