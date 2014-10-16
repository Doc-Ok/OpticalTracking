/***********************************************************************
Filming - Vislet class to assist shooting of video inside an immersive
environment by providing run-time control over viewers and environment
settings.
Copyright (c) 2012-2013 Oliver Kreylos

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

#ifndef VRUI_VISLETS_FILMING_INCLUDED
#define VRUI_VISLETS_FILMING_INCLUDED

#include <IO/Directory.h>
#include <Geometry/Point.h>
#include <Geometry/OrthonormalTransformation.h>
#include <GLMotif/ToggleButton.h>
#include <GLMotif/DropdownBox.h>
#include <GLMotif/TextFieldSlider.h>
#include <GLMotif/HSVColorSelector.h>

#include <Vrui/Vrui.h>
#include <Vrui/FileSelectionHelper.h>
#include <Vrui/Tool.h>
#include <Vrui/GenericToolFactory.h>
#include <Vrui/ToolManager.h>
#include <Vrui/Vislet.h>

/* Forward declarations: */
namespace GLMotif {
class PopupWindow;
class RowColumn;
class Button;
}
namespace Vrui {
class InputDevice;
class Viewer;
class VisletManager;
}

namespace Vrui {

namespace Vislets {

class Filming;

class FilmingFactory:public Vrui::VisletFactory
	{
	friend class Filming;
	
	/* Elements: */
	private:
	Point initialViewerPosition; // Initial position for a non-tracked filming viewer
	Scalar moveViewerSpeed; // Movement speed of viewer move tools in physical coordinate units per second
	
	/* Constructors and destructors: */
	public:
	FilmingFactory(Vrui::VisletManager& visletManager);
	virtual ~FilmingFactory(void);
	
	/* Methods: */
	virtual Vislet* createVislet(int numVisletArguments,const char* const visletArguments[]) const;
	virtual void destroyVislet(Vislet* vislet) const;
	};

class Filming:public Vrui::Vislet
	{
	friend class FilmingFactory;
	
	/* Embedded classes: */
	private:
	class Tool // Mix-in class for tool classes related to filming vislets
		{
		/* Elements: */
		protected:
		Filming* vislet; // Pointer to the vislet with which this tool is associated
		
		/* Constructors and destructors: */
		public:
		Tool(void) // Default constructor
			:vislet(0)
			{
			}
		
		/* Methods: */
		void setVislet(Filming* newVislet) // Associates the tool with a vislet
			{
			vislet=newVislet;
			}
		};
	
	class MoveViewerTool; // Forward declaration
	typedef GenericToolFactory<MoveViewerTool> MoveViewerToolFactory; // Factory class for viewer moving tools
	
	class MoveViewerTool:public Vrui::Tool,public Tool // Tool class to move a fixed-position viewer using an input device
		{
		friend class GenericToolFactory<MoveViewerTool>;
		
		/* Elements: */
		private:
		static MoveViewerToolFactory* factory; // Pointer to the tool's factory
		
		/* Constructors and destructors: */
		public:
		static void initClass(void);
		MoveViewerTool(const ToolFactory* sFactory,const ToolInputAssignment& inputAssignment);
		
		/* Methods: */
		virtual const Vrui::ToolFactory* getFactory(void) const;
		virtual void frame(void);
		};
	
	class MoveGridTool; // Forward declaration
	typedef GenericToolFactory<MoveGridTool> MoveGridToolFactory; // Factory class for grid moving tools
	
	class MoveGridTool:public Vrui::Tool,public Tool // Tool class to move the calibration grid using a 6-DOF input device
		{
		friend class GenericToolFactory<MoveGridTool>;
		
		/* Elements: */
		private:
		static MoveGridToolFactory* factory; // Pointer to the tool's factory
		ONTransform dragTransform; // Dragging transformation for a grid dragging operation
		
		/* Constructors and destructors: */
		public:
		static void initClass(void);
		MoveGridTool(const ToolFactory* sFactory,const ToolInputAssignment& inputAssignment);
		
		/* Methods: */
		virtual const Vrui::ToolFactory* getFactory(void) const;
		virtual void buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData);
		virtual void frame(void);
		};
	
	friend class MoveViewerTool;
	friend class MoveGridTool;
	
	/* Elements: */
	private:
	static FilmingFactory* factory; // Pointer to the factory object for this class
	Viewer* viewer; // The private filming viewer
	InputDevice* viewerDevice; // Tracking device to which the filming viewer is attached, or NULL
	Point viewerPosition; // Current position of the filming viewer if not head tracked
	Point eyePosition; // Position of viewer's eye if head tracked
	Viewer** windowViewers; // Array of viewers that were originally assigned to each window
	bool* windowFilmings; // Array of flags indicating which windows have the filming viewer attached
	bool* originalHeadlightStates; // Array of original headlight enable states of all viewers
	bool* headlightStates; // Array of headlight states of all viewers while the vislet is active
	Color originalBackgroundColor; // Environment's original background color
	Color backgroundColor; // Background color used while the vislet is active
	bool drawGrid; // Flag whether to draw the calibration grid
	ONTransform gridTransform; // Position and orientation of the calibration grid
	MoveGridTool* gridDragger; // Pointer to the grid dragging tool that is currently dragging the grid
	bool drawDevices; // Flag whether to draw coordinate frames for all physical devices
	GLMotif::PopupWindow* dialogWindow; // Filming controls dialog window
	GLMotif::DropdownBox* viewerDeviceMenu; // Drop-down menu to select tracking devices for the filming viewer
	GLMotif::TextFieldSlider* posSliders[3]; // Array of sliders controlling the viewer's fixed position
	GLMotif::RowColumn* windowButtonBox; // Box containing toggles to select filming windows
	GLMotif::RowColumn* headlightButtonBox; // Box containing toggles to enable viewers' headlights
	GLMotif::HSVColorSelector* backgroundColorSelector; // Color selector to change the background color
	GLMotif::ToggleButton* drawGridToggle;
	GLMotif::ToggleButton* drawDevicesToggle;
	FileSelectionHelper settingsSelectionHelper; // Helper to load and save settings
	
	/* Private methods: */
	void changeViewerMode(void); // Updates the GUI after a viewer mode change
	void viewerDeviceMenuCallback(GLMotif::DropdownBox::ValueChangedCallbackData* cbData);
	void posSliderCallback(GLMotif::TextFieldSlider::ValueChangedCallbackData* cbData,const int& sliderIndex);
	void windowToggleCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData,const int& windowIndex);
	void headlightToggleCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData,const int& viewerIndex);
	void backgroundColorSelectorCallback(GLMotif::HSVColorSelector::ValueChangedCallbackData* cbData);
	void drawGridToggleCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData);
	void resetGridCallback(Misc::CallbackData* cbData);
	void drawDevicesToggleCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData);
	void loadSettingsCallback(GLMotif::FileSelectionDialog::OKCallbackData* cbData);
	void saveSettingsCallback(GLMotif::FileSelectionDialog::OKCallbackData* cbData);
	void buildFilmingControls(void); // Creates the filming controls dialog window
	void toolCreationCallback(ToolManager::ToolCreationCallbackData* cbData); // Callback called when a new tool is created
	
	/* Constructors and destructors: */
	public:
	Filming(int numArguments,const char* const arguments[]);
	virtual ~Filming(void);
	
	/* Methods from Vislet: */
	public:
	virtual VisletFactory* getFactory(void) const;
	virtual void disable(void);
	virtual void enable(void);
	virtual void frame(void);
	virtual void display(GLContextData& contextData) const;
	};

}

}

#endif
