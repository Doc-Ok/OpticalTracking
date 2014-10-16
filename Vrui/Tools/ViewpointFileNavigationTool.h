/***********************************************************************
ViewpointFileNavigationTool - Class for tools to play back previously
saved viewpoint data files.
Copyright (c) 2007-2014 Oliver Kreylos

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

#ifndef VRUI_VIEWPOINTFILENAVIGATIONTOOL_INCLUDED
#define VRUI_VIEWPOINTFILENAVIGATIONTOOL_INCLUDED

#include <string>
#include <vector>
#include <GLMotif/TextFieldSlider.h>
#include <Vrui/FileSelectionHelper.h>
#include <Vrui/NavigationTool.h>

/* Forward declarations: */
namespace Misc {
class ConfigurationFileSection;
}
namespace Math {
class Matrix;
}
namespace GLMotif {
class PopupWindow;
}

namespace Vrui {

class ViewpointFileNavigationTool;

class ViewpointFileNavigationToolFactory:public ToolFactory
	{
	friend class ViewpointFileNavigationTool;
	
	/* Embedded classes: */
	private:
	struct Configuration // Structure containing tool settings
		{
		/* Elements: */
		public:
		std::string viewpointFileName; // Name of file from which viewpoint data is loaded. Tool will show file selection dialog if empty
		bool showGui; // Flag whether to show the playback control GUI
		bool showKeyframes; // Flag whether to render the current target keyframe during animation
		std::string pauseFileName; // Name of file from which scheduled pauses are loaded
		bool autostart; // Flag if new viewpoint file navigation tools start animation immediately
		
		/* Constructors and destructors: */
		Configuration(void); // Creates a default configuration
		
		/* Methods: */
		void read(const Misc::ConfigurationFileSection& cfs); // Overrides configuration from configuration file section
		void write(Misc::ConfigurationFileSection& cfs) const; // Writes configuration to configuration file section
		};
	
	/* Elements: */
	Configuration configuration; // Default configuration for all tools
	FileSelectionHelper viewpointSelectionHelper; // Helper object to load viewpoint curves from files
	
	/* Constructors and destructors: */
	public:
	ViewpointFileNavigationToolFactory(ToolManager& toolManager);
	virtual ~ViewpointFileNavigationToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual const char* getButtonFunction(int buttonSlotIndex) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class ViewpointFileNavigationTool:public NavigationTool
	{
	friend class ViewpointFileNavigationToolFactory;
	
	/* Embedded classes: */
	private:
	struct ControlPoint // Structure defining a viewpoint in environment-independent format
		{
		/* Elements: */
		public:
		Point center;
		Scalar size;
		Vector forward,up;
		};
	
	struct SplineSegment // Structure defining a spline segment interpolating between two keyframe viewpoints
		{
		/* Elements: */
		public:
		Scalar t[2]; // Time marks for the endpoints of the spline segment
		ControlPoint p[4]; // Array of spline segment control points (Bezier polygon)
		};
	
	/* Elements: */
	private:
	static ViewpointFileNavigationToolFactory* factory; // Pointer to the factory object for this class
	ViewpointFileNavigationToolFactory::Configuration configuration; // Private configuration of this tool
	GLMotif::PopupWindow* controlDialogPopup; // Playback control dialog window
	GLMotif::TextFieldSlider* positionSlider; // Slider to set the playback position
	std::vector<Scalar> times; // List of viewpoint times read from the viewpoint file
	std::vector<ControlPoint> viewpoints; // List of viewpoints read from the viewpoint file
	std::vector<SplineSegment> splines; // List of interpolating splines
	std::vector<Scalar> pauses; // List of scheduled pauses along the viewpoint curve
	unsigned int nextViewpointIndex; // Index of next viewpoint to be set
	Scalar speed; // Playback speed
	bool firstFrame; // Flag if the viewpoint animation has started on the current frame
	bool paused; // Flag if the viewpoint animation is currently paused
	Scalar parameter; // Current curve parameter
	GLMotif::FileSelectionDialog* loadViewpointFileDialog; // Pointer to file selection dialog opened to select viewpoint file
	
	/* Private methods: */
	void positionSliderCallback(GLMotif::TextFieldSlider::ValueChangedCallbackData* cbData);
	void speedSliderCallback(GLMotif::TextFieldSlider::ValueChangedCallbackData* cbData);
	void createGui(void); // Creates the playback control GUI
	void readViewpointFile(const char* fileName); // Reads the viewpoint file of the given name
	void loadViewpointFileCallback(GLMotif::FileSelectionDialog::OKCallbackData* cbData);
	void writeControlPoint(const ControlPoint& cp,Math::Matrix& b,unsigned int rowIndex); // Writes a control point to the spline calculation matrix
	void interpolate(const ControlPoint& p0,const ControlPoint& p1,Scalar t,ControlPoint& result); // Interpolates between two control points
	bool navigate(Scalar parameter); // Navigates to the given curve parameter
	
	/* Constructors and destructors: */
	public:
	ViewpointFileNavigationTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	virtual ~ViewpointFileNavigationTool(void);
	
	/* Methods from Tool: */
	virtual void configure(const Misc::ConfigurationFileSection& configFileSection);
	virtual void storeState(Misc::ConfigurationFileSection& configFileSection) const;
	virtual void initialize(void);
	virtual void deinitialize(void);
	virtual const ToolFactory* getFactory(void) const;
	virtual void buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData);
	virtual void frame(void);
	virtual void display(GLContextData& contextData) const;
	};

}

#endif
