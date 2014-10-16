/***********************************************************************
GridEditor - Vrui application for interactive virtual clay modeling
using a density grid and interactive isosurface extraction.
Copyright (c) 2006-2014 Oliver Kreylos

This file is part of the Virtual Clay Editing Package.

The Virtual Clay Editing Package is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The Virtual Clay Editing Package is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Virtual Clay Editing Package; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#ifndef GRIDEDITOR_INCLUDED
#define GRIDEDITOR_INCLUDED

#include <string>
#include <vector>
#include <Misc/Array.h>
#include <Geometry/Point.h>
#include <Geometry/OrthogonalTransformation.h>
#include <GL/gl.h>
#include <GL/GLObject.h>
#include <GLMotif/RadioBox.h>
#include <GLMotif/TextFieldSlider.h>
#include <Vrui/GenericToolFactory.h>
#include <Vrui/ToolManager.h>
#include <Vrui/Application.h>
#include <Vrui/FileSelectionHelper.h>
#include <Vrui/TransparentObject.h>

#include "EditableGrid.h"

/* Forward declarations: */
namespace Misc {
class CallbackData;
}
namespace GLMotif {
class PopupMenu;
class PopupWindow;
}

class GridEditor:public Vrui::Application,GLObject
	{
	/* Embedded classes: */
	private:
	typedef Geometry::Point<float,3> Point; // Data type for affine points
	
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		public:
		GLuint influenceSphereDisplayListId; // ID of display list to render transparent spheres
		GLuint domainBoxDisplayListId; // ID of display list to render the editable grid's domain box
		
		/* Constructors and destructors: */
		DataItem(void);
		virtual ~DataItem(void);
		};
	
	class EditTool; // Forward declaration
	
	typedef Vrui::GenericToolFactory<EditTool> EditToolFactory; // Generic factory class for the edit tool class
	
	class EditTool:public Vrui::Tool,public Vrui::Application::Tool<GridEditor>,public Vrui::TransparentObject // Tool class to edit the editable grid
		{
		friend class Vrui::GenericToolFactory<EditTool>;
		
		/* Embedded classes: */
		public:
		enum EditMode // Enumerated type for editing modes
			{
			ADD,SUBTRACT,SMOOTH,DRAG
			};
		
		/* Elements: */
		private:
		static EditToolFactory* factory; // Pointer to the factory object for this class
		EditableGrid* grid; // Pointer to the applications' grid
		Vrui::Scalar influenceRadius; // Radius of selector's influence in physical coordinates
		float fudgeSize; // Half width of smooth area around brush radius in model size
		EditMode editMode; // Editing mode of this locator
		Vrui::NavTrackerState lastTrackerState; // Position and orientation of tool at previous frame
		Point modelCenter; // Locator's position in model coordinates
		float modelRadius; // Locator's influence radius in model coordinates
		bool active; // Flag whether the locator is active
		Misc::Array<float,3> newValues; // Temporary storage for new grid values
		GLMotif::PopupWindow* settingsDialog;
		GLMotif::RadioBox* editModeBox;
		
		/* Private methods: */
		void brushSizeSliderCallback(GLMotif::TextFieldSlider::ValueChangedCallbackData* cbData);
		void fudgeSizeSliderCallback(GLMotif::TextFieldSlider::ValueChangedCallbackData* cbData);
		void changeEditModeCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData);
		
		/* Constructors and destructors: */
		public:
		static EditToolFactory* initClass(Vrui::ToolManager& toolManager); // Initializes the edit tool class
		EditTool(const Vrui::ToolFactory* factory,const Vrui::ToolInputAssignment& inputAssignment);
		virtual ~EditTool(void);
		
		/* Methods from Vrui::Tool: */
		virtual const Vrui::ToolFactory* getFactory(void) const;
		virtual void initialize(void);
		virtual void deinitialize(void);
		virtual void buttonCallback(int buttonSlotIndex,Vrui::InputDevice::ButtonCallbackData* cbData);
		virtual void frame(void);
		
		/* Methods from Vrui::TransparentObject: */
		virtual void glRenderActionTransparent(GLContextData& contextData) const;
		};
	
	friend class EditTool;
	
	/* Elements: */
	private:
	EditableGrid* grid; // Editable 3D grid
	Vrui::FileSelectionHelper saveGridHelper; // Helper object to save grids to .vol files
	Vrui::FileSelectionHelper exportSurfaceHelper; // Helper object to export surfaces to PLY files
	GLMotif::PopupMenu* mainMenu; // The program's main menu
	
	/* Private methods: */
	void centerDisplayCallback(Misc::CallbackData* cbData);
	void saveGridCallback(GLMotif::FileSelectionDialog::OKCallbackData* cbData);
	void exportSurfaceCallback(GLMotif::FileSelectionDialog::OKCallbackData* cbData);
	GLMotif::PopupMenu* createMainMenu(void); // Creates the program's main menu
	
	/* Constructors and destructors: */
	public:
	GridEditor(int& argc,char**& argv);
	virtual ~GridEditor(void);
	
	/* Methods from Vrui::Application: */
	virtual void display(GLContextData& contextData) const;
	
	/* Methods from GLObject: */
	virtual void initContext(GLContextData& contextData) const;
	};

#endif
