/***********************************************************************
Jello - VR program to interact with "virtual Jell-O" using a simplified
force interaction model based on the Nanotech Construction Kit.
Copyright (c) 2006-2013 Oliver Kreylos

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

#ifndef JELLO_INCLUDED
#define JELLO_INCLUDED

#include <GL/gl.h>
#include <GLMotif/ToggleButton.h>
#include <GLMotif/TextFieldSlider.h>
#include <Vrui/ToolManager.h>
#include <Vrui/DraggingToolAdapter.h>
#include <Vrui/Application.h>

#include "JelloAtom.h"
#include "JelloCrystal.h"
#include "JelloRenderer.h"

/* Forward declarations: */
namespace GLMotif {
class PopupMenu;
class PopupWindow;
}

class Jello:public Vrui::Application
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
	
	class AtomDragger:public Vrui::DraggingToolAdapter // Class to drag Jell-O atoms
		{
		/* Elements: */
		private:
		Jello* application; // Pointer to the application object "owning" this dragger
		bool dragging; // Flag whether the draggedAtom is valid
		AtomID draggedAtom; // Pointer to the dragged atom
		ONTransform dragTransform; // The dragging transformation applied to the dragged atom
		
		/* Constructors and destructors: */
		public:
		AtomDragger(Vrui::DraggingTool* sTool,Jello* sApplication);
		
		/* Methods: */
		virtual void dragStartCallback(Vrui::DraggingTool::DragStartCallbackData* cbData);
		virtual void dragCallback(Vrui::DraggingTool::DragCallbackData* cbData);
		virtual void dragEndCallback(Vrui::DraggingTool::DragEndCallbackData* cbData);
		};
	
	typedef std::vector<AtomDragger*> AtomDraggerList; // Type for lists of atom draggers
	
	friend class AtomDragger;
	
	/* Elements: */
	JelloCrystal crystal; // The virtual Jell-O crystal
	JelloRenderer renderer; // A renderer for the Jell-O crystal
	double targetFrameRate; // The target frame rate for graphical updates (defaults to 50Hz)
	int numMiniSteps; // Number of integration steps per frame (slowly adjusts to achieve the target frame rate)
	double lastFrameTime; // Application time of last frame
	
	/* Interaction parameters: */
	AtomDraggerList atomDraggers; // List of active atom draggers
	
	GLMotif::PopupMenu* mainMenu; // The program's main menu
	GLMotif::ToggleButton* showSettingsDialogToggle;
	GLMotif::PopupWindow* settingsDialog; // The settings dialog
	GLMotif::TextFieldSlider* jigglinessSlider;
	GLMotif::TextFieldSlider* viscositySlider;
	GLMotif::TextFieldSlider* gravitySlider;
	
	/* Private methods: */
	GLMotif::PopupMenu* createMainMenu(void);
	GLMotif::PopupWindow* createSettingsDialog(void);
	
	/* Constructors and destructors: */
	public:
	Jello(int& argc,char**& argv);
	virtual ~Jello(void);
	
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
