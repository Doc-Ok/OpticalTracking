/***********************************************************************
ToolManager - Class to manage tool classes, and dynamic assignment of
tools to input devices.
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

#ifndef VRUI_TOOLMANAGER_INCLUDED
#define VRUI_TOOLMANAGER_INCLUDED

#include <vector>
#include <Misc/CallbackData.h>
#include <Misc/CallbackList.h>
#include <Plugins/FactoryManager.h>
#include <Vrui/Tool.h>

/* Forward declarations: */
namespace Misc {
class ConfigurationFileSection;
}
namespace Plugins {
class Factory;
}
class GLContextData;
namespace GLMotif {
class Popup;
class PopupMenu;
}
namespace Vrui {
class InputDevice;
class InputDeviceFeature;
class InputGraphManager;
class InputDeviceManager;
class ToolInputAssignment;
class MenuTool;
class MutexMenu;
class ToolKillZone;
class ToolManagerToolCreationState;
}

namespace Vrui {

class ToolManager:public Plugins::FactoryManager<ToolFactory>
	{
	/* Embedded classes: */
	private:
	typedef Plugins::FactoryManager<ToolFactory> BaseClass; // Base class type
	public:
	typedef std::vector<Tool*> ToolList; // Data type for list of tools
	
	private:
	struct ToolManagementQueueItem // Structure for items in the tool management queue
		{
		/* Embedded classes: */
		public:
		enum ItemFunction // Enumerated types for queue item functions
			{
			CREATE_TOOL,DESTROY_TOOL
			};
		
		/* Elements: */
		ItemFunction itemFunction; // Function of this item
		ToolFactory* createToolFactory; // Pointer to a tool factory if the item function is CREATE_TOOL
		ToolInputAssignment* tia; // Pointer to a tool input assignment if the item function is CREATE_TOOL
		Tool* tool; // Pointer to the to-be-destroyed tool if the item function is DESTROY_TOOL
		};
	
	typedef std::vector<ToolManagementQueueItem> ToolManagementQueue;
	
	public:
	class ToolCreationCallbackData:public Misc::CallbackData // Callback data sent when a tool is created
		{
		/* Elements: */
		public:
		Tool* tool; // Pointer to newly created tool
		const Misc::ConfigurationFileSection* cfg; // Optional pointer to the configuration file section from which the tool read its settings
		
		/* Constructors and destructors: */
		ToolCreationCallbackData(Tool* sTool,const Misc::ConfigurationFileSection* sCfg)
			:tool(sTool),cfg(sCfg)
			{
			}
		};
	
	class ToolDestructionCallbackData:public Misc::CallbackData // Callback data sent when a tool is destroyed
		{
		/* Elements: */
		public:
		Tool* tool; // Pointer to tool to be destroyed
		
		/* Constructors and destructors: */
		ToolDestructionCallbackData(Tool* sTool)
			:tool(sTool)
			{
			}
		};
	
	/* Elements: */
	private:
	InputGraphManager* inputGraphManager; // Pointer to the input graph manager
	InputDeviceManager* inputDeviceManager; // Pointer to input device manager
	const Misc::ConfigurationFileSection* configFileSection; // The tool manager's configuration file section - valid throughout the manager's entire lifetime
	
	/* Tool management state: */
	ToolList tools; // List of currently instantiated tools
	ToolManagementQueue toolManagementQueue; // Queue of management tasks that have to be performed on the next call to update
	
	/* Tool creation state: */
	InputDevice* toolCreationDevice; // A virtual input device with associated tool selection tool, dynamically hooked into the first tool assignment slot during tool creation
	MenuTool* toolCreationTool; // The tool selection tool used during the tool creation process
	GLMotif::PopupMenu* toolMenuPopup; // Hierarchical popup menu for tool selection
	MutexMenu* toolMenu; // Shell for tool selection menu
	ToolManagerToolCreationState* toolCreationState; // Current state of tool creation procedure
	Misc::CallbackList toolCreationCallbacks; // List of callbacks to be called after a new tool has been created
	
	/* Tool destruction state: */
	ToolKillZone* toolKillZone; // Pointer to tool "kill zone"
	Misc::CallbackList toolDestructionCallbacks; // List of callbacks to be called before a tool will be destroyed
	
	/* Private methods: */
	GLMotif::Popup* createToolSubmenu(const Plugins::Factory& factory); // Returns submenu containing all subclasses of the given class
	GLMotif::PopupMenu* createToolMenu(void); // Returns top level of tool selection menu
	void inputDeviceDestructionCallback(Misc::CallbackData* cbData); // Callback called when an input device is destroyed
	void toolMenuSelectionCallback(Misc::CallbackData* cbData); // Callback called when a tool class is selected from the selection menu; continues tool creation process
	void toolCreationDeviceMotionCallback(Misc::CallbackData* cbData); // Callback called when the device for which a tool is being created moves during tool creation
	
	/* Constructors and destructors: */
	public:
	ToolManager(InputDeviceManager* sInputDeviceManager,const Misc::ConfigurationFileSection& sConfigFileSection); // Initializes tool manager by reading given configuration file section
	~ToolManager(void); // Destroys tool manager
	
	/* Methods from Plugins::FactoryManager: */
	void addClass(ToolFactory* newFactory,DestroyFactoryFunction newDestroyFactoryFunction =0); // Overrides base class method; adds new tool class to tool selection menu
	void releaseClass(const char* className); // Overrides base class method; destroys all tools of the given class before destroying the class
	
	/* Methods: */
	void addAbstractClass(ToolFactory* newFactory,DestroyFactoryFunction newDestroyFactoryFunction =0); // Same as addClass method, but does not add to tool selection menu (derived concrete tool classes will create the cascade button)
	static void defaultToolFactoryDestructor(ToolFactory* factory); // Default destructor for tool factories; simply deletes them
	Misc::ConfigurationFileSection getToolClassSection(const char* toolClassName) const; // Returns the configuration file section a tool class should use for its initialization
	MutexMenu* getToolMenu(void) // Returns tool menu
		{
		return toolMenu;
		}
	void loadToolBinding(const char* toolSectionName); // Loads a tool binding from a configuration file section; names are relative to tool manager's section
	void loadDefaultTools(void); // Creates default tool associations
	bool isCreatingTool(void) const // Returns true if the tool manager is in the middle of the interactive tool creation process
		{
		return toolCreationState!=0;
		}
	void startToolCreation(const InputDeviceFeature& feature); // Starts the interactive tool creation process with the given input device feature
	void prepareFeatureAssignment(const InputDeviceFeature& feature); // Prepares the given input device feature for tool assignment
	void assignFeature(const InputDeviceFeature& feature); // Assigns the given feature to the next available tool assignment slot
	Tool* createTool(ToolFactory* factory,const ToolInputAssignment& tia,const Misc::ConfigurationFileSection* cfg =0); // Programmatically creates a new tool of the given class and input assignment; optionally lets tool initialize itself by reading from the given configuration file section
	void destroyTool(Tool* tool,bool destroyImmediately =true); // Destroys a tool programmatically, either right away or during the next call to update()
	void update(void); // Called once every frame so that the tool manager has a well-defined place to create new tools
	ToolList::const_iterator beginTools(void) const // Returns iterator to first instantiated tool
		{
		return tools.begin();
		}
	ToolList::iterator beginTools(void) // Ditto
		{
		return tools.begin();
		}
	ToolList::const_iterator endTools(void) const // Returns iterator after last instantiated tool
		{
		return tools.end();
		}
	ToolList::iterator endTools(void) // Ditto
		{
		return tools.end();
		}
	void glRenderAction(GLContextData& contextData) const; // Renders the tool manager (not the tools)
	Misc::CallbackList& getToolCreationCallbacks(void) // Returns list of tool creation callbacks
		{
		return toolCreationCallbacks;
		}
	Misc::CallbackList& getToolDestructionCallbacks(void) // Returns list of tool destruction callbacks
		{
		return toolDestructionCallbacks;
		}
	ToolKillZone* getToolKillZone(void) // Returns pointer to the tool kill zone
		{
		return toolKillZone;
		}
	bool isDeviceInToolKillZone(const InputDevice* device) const; // Returns true if the given device is in (or pointing at) the tool kill zone
	};

}

#endif
