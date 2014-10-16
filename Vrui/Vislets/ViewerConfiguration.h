/***********************************************************************
ViewerConfiguration - Vislet class to configure the settings of a Vrui
Viewer object from inside a running Vrui application.
Copyright (c) 2013-2014 Oliver Kreylos

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

#ifndef VRUI_VISLETS_VIEWERCONFIGURATION_INCLUDED
#define VRUI_VISLETS_VIEWERCONFIGURATION_INCLUDED

#include <vector>
#include <Geometry/Point.h>
#include <Geometry/LinearUnit.h>
#include <GLMotif/DropdownBox.h>
#include <GLMotif/TextFieldSlider.h>

#include <Vrui/Geometry.h>
#include <Vrui/Vislet.h>

/* Forward declarations: */
namespace GLMotif {
class PopupWindow;
class RowColumn;
class Button;
}
namespace Vrui {
class Viewer;
class VRScreen;
class VisletManager;
}

namespace Vrui {

namespace Vislets {

class ViewerConfiguration;

class ViewerConfigurationFactory:public Vrui::VisletFactory
	{
	friend class ViewerConfiguration;
	
	/* Embedded classes: */
	private:
	struct OculusRiftViewer // Structure associating viewers with Oculus Rift head-mounted displays to adjust display geometry to viewer configuration
		{
		/* Elements: */
		public:
		Viewer* viewer; // Pointer to the viewer to which this Oculus Rift configuration applies
		VRScreen* screens[2]; // Pointers to the left and right screen halves
		Scalar lensDist; // The distance between the left and right lenses' centers
		};
	
	/* Elements: */
	Geometry::LinearUnit configUnit; // Unit of measurement to use for configuration settings
	std::vector<OculusRiftViewer> oculusRiftViewers; // List of viewers associates with Oculus Rift head-mounted displays in this environment
	
	/* Constructors and destructors: */
	public:
	ViewerConfigurationFactory(Vrui::VisletManager& visletManager);
	virtual ~ViewerConfigurationFactory(void);
	
	/* Methods: */
	virtual Vislet* createVislet(int numVisletArguments,const char* const visletArguments[]) const;
	virtual void destroyVislet(Vislet* vislet) const;
	};

class ViewerConfiguration:public Vrui::Vislet
	{
	friend class ViewerConfigurationFactory;
	
	/* Elements: */
	private:
	static ViewerConfigurationFactory* factory; // Pointer to the factory object for this class
	Scalar unitScale; // Scale factor between configuration units and Vrui units
	bool firstEnable; // Flag to indicate the first time the vislet is enabled (and ignore it)
	Viewer* viewer; // The viewer currently selected for configuration
	ViewerConfigurationFactory::OculusRiftViewer* oculusRiftViewer; // Pointer to an Oculus Rift configuration structure if the currently selected viewer is associated with one, NULL otherwise
	Point eyePos[3]; // The current positions of the current viewer's mono, left, and right eyes
	Scalar eyeDist; // The current viewer's current eye separation distance
	
	GLMotif::PopupWindow* dialogWindow; // ViewerConfiguration controls dialog window
	GLMotif::DropdownBox* viewerMenu; // Drop-down menu to select the viewer to be configured
	GLMotif::TextFieldSlider* eyePosSliders[3][3]; // Array of sliders controlling the (x, y, z) coordinates of the viewer's mono, left, and right eye positions
	GLMotif::TextFieldSlider* eyeDistanceSlider; // Slider to directly adjust the viewer's eye distance
	
	/* Private methods: */
	void updateViewer(void);
	void setViewer(Viewer* newViewer);
	void viewerMenuCallback(GLMotif::DropdownBox::ValueChangedCallbackData* cbData);
	void eyePosSliderCallback(GLMotif::TextFieldSlider::ValueChangedCallbackData* cbData,const int& sliderIndex);
	void eyeDistanceSliderCallback(GLMotif::TextFieldSlider::ValueChangedCallbackData* cbData);
	void buildViewerConfigurationControls(void); // Creates the filming controls dialog window
	
	/* Constructors and destructors: */
	public:
	ViewerConfiguration(int numArguments,const char* const arguments[]);
	virtual ~ViewerConfiguration(void);
	
	/* Methods from Vislet: */
	public:
	virtual VisletFactory* getFactory(void) const;
	virtual void disable(void);
	virtual void enable(void);
	};

}

}

#endif
