/***********************************************************************
InputGraphManager - Class to maintain the bipartite input device / tool
graph formed by tools being assigned to input devices, and input devices
in turn being grabbed by tools.
Copyright (c) 2004-2013 Oliver Kreylos

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

#ifndef VRUI_INPUTGRAPHMANAGER_INCLUDED
#define VRUI_INPUTGRAPHMANAGER_INCLUDED

#include <vector>
#include <Misc/HashTable.h>
#include <Geometry/OrthogonalTransformation.h>
#include <SceneGraph/GraphNode.h>
#include <Vrui/Geometry.h>
#include <Vrui/GlyphRenderer.h>
#include <Vrui/InputDevice.h>
#include <Vrui/InputDeviceFeature.h>

/* Forward declarations: */
namespace Misc {
class ConfigurationFileSection;
}
namespace IO {
class Directory;
}
namespace Vrui {
class VirtualInputDevice;
class Tool;
struct InputGraphManagerToolStackState;
}

namespace Vrui {

class InputGraphManager
	{
	/* Embedded classes: */
	private:
	struct GraphTool // Structure to represent a tool in the input graph
		{
		/* Elements: */
		public:
		Tool* tool; // Pointer to the tool
		int level; // Index of the graph level containing the tool
		GraphTool* levelPred; // Pointer to the previous tool in the same graph level
		GraphTool* levelSucc; // Pointer to the next tool in the same graph level
		
		/* Constructors and destructors: */
		GraphTool(Tool* sTool,int sLevel); // Creates a graph wrapper for the given tool
		};
	
	struct ToolSlot // Structure to store assignments of input device features to tool input slots
		{
		/* Elements: */
		public:
		InputDeviceFeature feature; // The input device feature managed by this tool slot
		GraphTool* tool; // Pointer to the tool assigned to this feature slot
		bool preempted; // Flag whether a button press or valuator push event on this slot was preempted
		bool inKillZone; // Flag if this slot's device was inside the tool kill zone during the button press or valuator push event
		
		/* Constructors and destructors: */
		ToolSlot(void); // Creates an unassigned tool slot
		~ToolSlot(void); // Removes the slot's callbacks and destroys it
		
		/* Methods: */
		void initialize(InputDevice* sDevice,int sFeatureIndex); // Initializes a slot and installs callbacks
		void inputDeviceButtonCallback(InputDevice::ButtonCallbackData* cbData); // Callback for button events
		void inputDeviceValuatorCallback(InputDevice::ValuatorCallbackData* cbData); // Callback for valuator events
		bool pressed(void); // Processes a button press or valuator push event
		bool released(void); // Processes a button or valuator release event
		};
	
	struct GraphInputDevice // Structure to represent an input device in the input graph
		{
		/* Elements: */
		public:
		InputDevice* device; // Pointer to the input device
		Glyph deviceGlyph; // Glyph used to visualize the device's position and orientation
		ToolSlot* toolSlots; // Array of tool slots for this device
		int level; // Index of the graph level containing the input device
		bool navigational; // Flag whether this device, if ungrabbed, follows the navigation transformation
		NavTrackerState fromNavTransform; // Transformation from navigation coordinates to device's coordinates while device is in navigational mode
		GraphInputDevice* levelPred; // Pointer to the previous input device in the same graph level
		GraphInputDevice* levelSucc; // Pointer to the next input device in the same graph level
		GraphTool* grabber; // Pointer to the tool currently holding a grab on the input device
		
		/* Constructors and destructors: */
		GraphInputDevice(InputDevice* sDevice); // Creates a graph wrapper for the given input device
		~GraphInputDevice(void); // Destroys the graph wrapper
		};
	
	typedef Misc::HashTable<InputDevice*,GraphInputDevice*> DeviceMap; // Hash table to map from input devices to graph input devices
	typedef Misc::HashTable<Tool*,GraphTool*> ToolMap; // Hash table to map from tools to graph tools
	
	/* Elements: */
	GlyphRenderer* glyphRenderer; // Pointer to the glyph renderer
	VirtualInputDevice* virtualInputDevice; // Pointer to helper class for handling ungrabbed virtual input devices
	GraphTool inputDeviceManager; // A fake graph tool to grab physical input devices
	DeviceMap deviceMap; // Hash table mapping from input devices to graph input devices
	ToolMap toolMap; // Hash table mapping from tools to graph tools
	int maxGraphLevel; // Maximum level in the input graph that has input devices or tools
	std::vector<GraphInputDevice*> deviceLevels; // Vector of pointers to the first input device in each graph level
	std::vector<GraphTool*> toolLevels; // Vector of pointers to the first tool in each graph level
	SceneGraph::GraphNodePointer toolStackNode; // Scene graph node displaying an input device feature's tool stack
	InputDeviceFeature toolStackBaseFeature; // Base input device feature for the currently displayed tool stack
	
	/* Private methods: */
	void linkInputDevice(GraphInputDevice* gid); // Links a graph input device to its current graph level
	void unlinkInputDevice(GraphInputDevice* gid); // Unlinks a graph input device from its current graph level
	void linkTool(GraphTool* gt); // Links a graph tool to its current graph level
	void unlinkTool(GraphTool* gt); // Unlinks a graph tool from its current graph level
	int calcToolLevel(const Tool* tool) const; // Returns the correct graph level for the given tool
	void growInputGraph(int level); // Grows the input graph to represent the given level
	void shrinkInputGraph(void); // Removes all empty levels from the end of the input graph
	void updateInputGraph(void); // Reorders graph levels after input device grab/release
	SceneGraph::GraphNodePointer showToolStack(const ToolSlot& ts,InputGraphManagerToolStackState& tss) const; // Returns a scene graph visualizing the given tool slot's tool stack
	
	/* Constructors and destructors: */
	public:
	InputGraphManager(GlyphRenderer* sGlyphRenderer,VirtualInputDevice* sVirtualInputDevice); // Creates an empty input graph manager using the given glyph renderer and virtual input device
	private:
	InputGraphManager(const InputGraphManager& source); // Prohibit copy constructor
	InputGraphManager& operator=(const InputGraphManager& source); // Prohibit assignment operator
	public:
	~InputGraphManager(void);
	
	/* Methods: */
	void clear(void); // Removes all tools and virtual input devices from the input graph
	void addInputDevice(InputDevice* newDevice); // Adds an ungrabbed input device to the graph
	void removeInputDevice(InputDevice* device); // Removes an input device from the graph
	void addTool(Tool* newTool); // Adds a tool to the input graph, based on its current input assignment
	void removeTool(Tool* tool); // Removes a tool from the input graph
	void loadInputGraph(const Misc::ConfigurationFileSection& configFileSection); // Loads all virtual input devices and tools defined in the given configuration file section
	void loadInputGraph(IO::Directory& directory,const char* configurationFileName,const char* baseSectionName); // Ditto
	void saveInputGraph(IO::Directory& directory,const char* configurationFileName,const char* baseSectionName) const; // Saves the current state of all virtual input devices and assigned tools to the given section in the given configuration file
	bool isNavigational(InputDevice* device) const; // Returns whether the given device will follow navigation coordinates while ungrabbed
	void setNavigational(InputDevice* device,bool newNavigational); // Sets whether the given device will follow navigation coordinates while ungrabbed
	Glyph& getInputDeviceGlyph(InputDevice* device); // Returns the glyph associated with the given input device
	bool isReal(InputDevice* device) const; // Returns true if the given input device is a real device
	bool isGrabbed(InputDevice* device) const; // Returns true if the given input device is currently grabbed by a tool
	InputDevice* getFirstInputDevice(void); // Returns pointer to the first ungrabbed input device
	InputDevice* getNextInputDevice(InputDevice* device); // Returns pointer to the next input device in the same level after the given one
	InputDevice* findInputDevice(const Point& position,bool ungrabbedOnly =true); // Finds an ungrabbed input device based on a position in physical coordinates
	InputDevice* findInputDevice(const Ray& ray,bool ungrabbedOnly =true); // Finds an ungrabbed input device based on a ray in physical coordinates
	bool grabInputDevice(InputDevice* device,Tool* grabber); // Allows a tool (or physical layer if tool==0) to grab an input device; returns true on success
	void releaseInputDevice(InputDevice* device,Tool* grabber); // Allows the current grabber of an input device to release the grab
	InputDevice* getRootDevice(InputDevice* device); // Returns the input device forming the base of the transformation chain containing the given (virtual) input device
	InputDeviceFeature findFirstUnassignedFeature(const InputDeviceFeature& feature) const; // Returns the first unassigned input device feature forwarded from the given feature
	void showToolStack(const InputDeviceFeature& feature); // Displays the stack of tools assigned to the given input device feature
	void update(void); // Updates state of all tools and non-physical input devices in the graph
	void glRenderDevices(GLContextData& contextData) const; // Renders current state of all input devices
	void glRenderTools(GLContextData& contextData) const; // Renders current state of all tools
	};

}

#endif
