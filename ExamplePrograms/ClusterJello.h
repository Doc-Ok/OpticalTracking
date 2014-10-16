/***********************************************************************
ClusterJello - VR program to interact with "virtual Jell-O" using a
simplified force interaction model based on the Nanotech Construction
Kit. This version of Virtual Jell-O uses multithreading and explicit
cluster communication to split the computation work and rendering work
between the CPUs and nodes of a distributed rendering cluster.
Copyright (c) 2007-2013 Oliver Kreylos

This file is part of the Virtual Jell-O interactive VR demonstration.

Virtual Jell-O is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

Virtual Jell-O is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with Virtual Jell-O; if not, write to the Free Software Foundation,
Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef CLUSTERJELLO_INCLUDED
#define CLUSTERJELLO_INCLUDED

#include <Misc/HashTable.h>
#include <Threads/Thread.h>
#include <GL/gl.h>
#include <GLMotif/ToggleButton.h>
#include <GLMotif/TextFieldSlider.h>
#include <Vrui/ToolManager.h>
#include <Vrui/DraggingToolAdapter.h>
#include <Vrui/Application.h>

#include "JelloAtom.h"
#include "JelloCrystal.h"
#include "JelloRenderer.h"
#include "TripleBuffer.h"

/* Forward declarations: */
namespace Cluster {
class MulticastPipe;
}
namespace GLMotif {
class PopupMenu;
class PopupWindow;
}

class ClusterJello:public Vrui::Application
	{
	/* Embedded classes: */
	private:
	typedef JelloCrystal::Scalar Scalar;
	typedef JelloCrystal::Point Point;
	typedef JelloCrystal::Vector Vector;
	typedef JelloCrystal::Rotation Rotation;
	typedef JelloCrystal::Ray Ray;
	typedef JelloCrystal::Box Box;
	typedef JelloCrystal::ONTransform ONTransform;
	typedef JelloCrystal::AtomID AtomID;
	typedef JelloCrystal::Index Index;
	
	struct SimulationParameters // Structure to communicate simulation parameters to the simulation thread
		{
		/* Elements: */
		public:
		Scalar atomMass; // Mass of a single Jell-O atom
		Scalar attenuation; // The velocity attenuation factor
		Scalar gravity; // The gravity acceleration constant
		};
	
	struct DraggerStates // Structure to communicate application dragger states to the simulation thread
		{
		/* Elements: */
		public:
		int numDraggers; // Number of application draggers
		unsigned int* draggerIDs; // Array of unique IDs for each dragger, to detect dynamic creation/deletion
		bool* draggerRayBaseds; // Array of flags if a dragger has ray-based selection
		Ray* draggerRays; // Array of ray directions for each dragger
		ONTransform* draggerTransformations; // Array of dragger positions/orientations
		bool* draggerActives; // Array of active flags for each dragger
		
		/* Constructors and destructors: */
		DraggerStates(void)
			:numDraggers(0),draggerIDs(0),draggerRayBaseds(0),draggerRays(0),draggerTransformations(0),draggerActives(0)
			{
			};
		~DraggerStates(void)
			{
			delete[] draggerIDs;
			delete[] draggerRayBaseds;
			delete[] draggerRays;
			delete[] draggerTransformations;
			delete[] draggerActives;
			};
		
		/* Methods: */
		void setNumDraggers(int newNumDraggers); // Sets the number of draggers in the dragger state
		};
	
	struct AtomLock // Structure to connect a dragger to a locked Jell-O atom
		{
		/* Elements: */
		public:
		AtomID draggedAtom; // ID of the locked atom
		ONTransform dragTransformation; // The dragging transformation applied to the locked atom
		};
	
	typedef Misc::HashTable<unsigned int,AtomLock> AtomLockHasher; // Hash table to associate atom locks with dragger IDs
	
	class AtomDragger:public Vrui::DraggingToolAdapter // Class to drag Jell-O atoms
		{
		/* Elements: */
		private:
		ClusterJello* application; // Pointer to the application object "owning" this dragger
		public:
		unsigned int draggerID; // Unique ID for each dragger in the application
		bool draggerRayBased; // Flag if the dragger is ray-based
		Ray draggerRay; // Ray defining dragger's selection
		ONTransform draggerTransformation; // Transformation of the dragger in model coordinates
		bool active; // Flag if the dragger is active
		
		/* Constructors and destructors: */
		public:
		AtomDragger(Vrui::DraggingTool* sTool,ClusterJello* sApplication,unsigned int sDraggerID);
		
		/* Methods: */
		virtual void idleMotionCallback(Vrui::DraggingTool::IdleMotionCallbackData* cbData);
		virtual void dragStartCallback(Vrui::DraggingTool::DragStartCallbackData* cbData);
		virtual void dragCallback(Vrui::DraggingTool::DragCallbackData* cbData);
		virtual void dragEndCallback(Vrui::DraggingTool::DragEndCallbackData* cbData);
		};
	
	typedef std::vector<AtomDragger*> AtomDraggerList; // Type for lists of atom draggers
	
	friend class AtomDragger;
	
	/* Elements: */
	
	/* Simulation thread state: */
	Threads::Thread simulationThread; // Handle of the simulation thread
	Cluster::MulticastPipe* clusterPipe; // Pipe connecting the nodes in a rendering cluster
	JelloCrystal* crystal; // Pointer to actual Jell-O crystal (only valid on master node)
	AtomLockHasher atomLocks; // Hash table of current atom locks
	double updateTime; // Time between updates of the Jell-O graphics state (inverse of target frame rate)
	
	/* Communication between application and simulation thread: */
	TripleBuffer<SimulationParameters> simulationParameters; // Triple buffer of Jell-O simulation parameters
	TripleBuffer<DraggerStates> draggerStates; // Triple buffer of application dragger states, updated during the frame method
	TripleBuffer<JelloCrystal> proxyCrystal; // Triple buffer of proxy (rendering-only) Jell-O crystals
	
	/* Application state: */
	SimulationParameters currentSimulationParameters; // Current simulation parameters
	JelloRenderer* renderer; // Jell-O crystal renderer that can be associated with any of the proxy Jell-O crystals in the triple buffer
	GLMotif::PopupMenu* mainMenu; // The program's main menu
	GLMotif::ToggleButton* showSettingsDialogToggle;
	GLMotif::PopupWindow* settingsDialog; // The settings dialog
	GLMotif::TextFieldSlider* jigglinessSlider;
	GLMotif::TextFieldSlider* viscositySlider;
	GLMotif::TextFieldSlider* gravitySlider;
	
	unsigned int nextDraggerID; // ID to be assigned to the next atom dragger
	AtomDraggerList atomDraggers; // List of active atom draggers
	
	/* Private methods: */
	GLMotif::PopupMenu* createMainMenu(void);
	GLMotif::PopupWindow* createSettingsDialog(void);
	void* simulationThreadMethodMaster(void); // The simulation thread method running on the master
	void* simulationThreadMethodSlave(void); // The simulation thread method running on the slaves
	
	/* Constructors and destructors: */
	public:
	ClusterJello(int& argc,char**& argv);
	virtual ~ClusterJello(void);
	
	/* Methods: */
	virtual void toolCreationCallback(Vrui::ToolManager::ToolCreationCallbackData* cbData);
	virtual void toolDestructionCallback(Vrui::ToolManager::ToolDestructionCallbackData* cbData);
	virtual void frame(void);
	virtual void display(GLContextData& contextData) const;
	void centerDisplayCallback(Misc::CallbackData* cbData);
	void showSettingsDialogCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData);
	void jigglinessSliderCallback(GLMotif::TextFieldSlider::ValueChangedCallbackData* cbData);
	void viscositySliderCallback(GLMotif::TextFieldSlider::ValueChangedCallbackData* cbData);
	void gravitySliderCallback(GLMotif::TextFieldSlider::ValueChangedCallbackData* cbData);
	void settingsDialogCloseCallback(Misc::CallbackData* cbData);
	};

#endif
