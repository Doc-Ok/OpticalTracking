/***********************************************************************
ShowEarthModel - Simple Vrui application to render a model of Earth,
with the ability to additionally display earthquake location data and
other geology-related stuff.
Copyright (c) 2005-2013 Oliver Kreylos

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef SHOWEARTHMODEL_INCLUDED
#define SHOWEARTHMODEL_INCLUDED

#include <vector>
#include <GL/gl.h>
#include <GL/GLMaterial.h>
#include <GL/GLObject.h>
#include <Images/RGBImage.h>
#include <GLMotif/Slider.h>
#include <GLMotif/ToggleButton.h>
#include <Vrui/GeodeticCoordinateTransform.h>
#include <Vrui/ToolManager.h>
#include <Vrui/SurfaceNavigationTool.h>
#include <Vrui/Application.h>

#include "EarthquakeSet.h"

/* Forward declarations: */
namespace Misc {
class CallbackData;
}
class GLPolylineTube;
namespace GLMotif {
class Popup;
class PopupMenu;
class PopupWindow;
class TextField;
}
namespace SceneGraph {
class GroupNode;
}
class PointSet;
class SeismicPath;

class ShowEarthModel:public Vrui::Application,public GLObject
	{
	/* Embedded classes: */
	private:
	class RotatedGeodeticCoordinateTransform:public Vrui::GeodeticCoordinateTransform
		{
		/* Elements: */
		private:
		Vrui::Scalar rotationAngle; // Current rotation angle of the Earth model
		Vrui::Scalar raSin,raCos; // Sine and cosine of rotation angle
		
		/* Constructors and destructors: */
		public:
		RotatedGeodeticCoordinateTransform(void);
		
		/* Methods from GeodeticCoordinateTransform: */
		virtual const char* getUnitName(int componentIndex) const;
		virtual const char* getUnitAbbreviation(int componentIndex) const;
		virtual Vrui::Point transform(const Vrui::Point& navigationPoint) const;
		virtual Vrui::Point inverseTransform(const Vrui::Point& userPoint) const;
		
		/* New methods: */
		void setRotationAngle(Vrui::Scalar newRotationAngle);
		};
	
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		public:
		bool hasVertexBufferObjectExtension; // Flag if buffer objects are supported by the local GL
		GLuint surfaceVertexBufferObjectId; // Vertex buffer object ID for Earth surface
		GLuint surfaceIndexBufferObjectId; // Index buffer object ID for Earth surface
		GLuint surfaceTextureObjectId; // Texture object ID for Earth surface texture
		GLuint displayListIdBase; // Base ID of set of display lists for Earth model components
		
		/* Constructors and destructors: */
		DataItem(void);
		virtual ~DataItem(void);
		};
	
	/* Elements: */
	std::vector<EarthquakeSet*> earthquakeSets; // Vector of earthquake sets to render
	EarthquakeSet::TimeRange earthquakeTimeRange; // Range to earthquake event times
	std::vector<PointSet*> pointSets; // Vector of additional point sets to render
	std::vector<SeismicPath*> seismicPaths; // Vector of seismic paths to render
	std::vector<GLPolylineTube*> sensorPaths; // Vector of sensor paths to render
	std::vector<SceneGraph::GroupNode*> sceneGraphs; // Vector of scene graphs to render
	bool scaleToEnvironment; // Flag if the Earth model should be scaled to fit the environment
	bool rotateEarth; // Flag if the Earth model should be rotated
	double lastFrameTime; // Application time when last frame was rendered (to determine Earth angle updates)
	float rotationAngle; // Current Earth rotation angle
	float rotationSpeed; // Earth rotation speed in degree/second
	RotatedGeodeticCoordinateTransform* userTransform; // Coordinate transformation from user space to navigation space
	Images::RGBImage surfaceImage; // Texture image for the Earth surface
	bool showSurface; // Flag if the Earth surface is rendered
	bool surfaceTransparent; // Flag if the Earth surface is rendered transparently
	GLMaterial surfaceMaterial; // OpenGL material properties for the Earth surface
	bool showGrid; // Flag if the long/lat grid is rendered
	std::vector<bool> showEarthquakeSets; // Vector of flags if each of the earthquake sets is rendered
	std::vector<bool> showPointSets; // Vector of flags if each of the additional point sets is rendered
	std::vector<bool> showSceneGraphs; // Vector of flags if each of the scene graphs is rendered
	bool showSeismicPaths; // Flag if the seismic paths are rendered
	bool showOuterCore; // Flag if the outer core is rendered
	bool outerCoreTransparent; // Flag if the outer core is rendered transparently
	GLMaterial outerCoreMaterial; // OpenGL material properties for the outer core
	bool showInnerCore; // Flag if the inner core is rendered
	bool innerCoreTransparent; // Flag if the inner core is rendered transparently
	GLMaterial innerCoreMaterial; // OpenGL material properties for the inner core
	float earthquakePointSize; // Point size to render earthquake hypocenters
	GLMaterial sensorPathMaterial; // OpenGL material properties for sensor paths
	bool fog; // Flag whether depth cueing via fog is enabled
	float bpDist; // Current backplane distance for clipping and fog attenuation
	double currentTime; // Current animation time in seconds since the epoch in UTC
	double playSpeed; // Animation playback speed in real-world seconds per visualization second
	bool play; // Flag if automatic playback is enabled
	bool lockToSphere; // Flag whether the navigation transformation is locked to a fixed-radius sphere
	Vrui::Scalar sphereRadius; // Radius of the fixed sphere to which to lock the navigation transformation
	Vrui::NavTransform sphereTransform; // Transformation pre-applied to navigation transformation to lock it to a sphere
	GLMotif::PopupMenu* mainMenu; // The program's main menu
	GLMotif::ToggleButton* showRenderDialogToggle;
	GLMotif::ToggleButton* showAnimationDialogToggle;
	GLMotif::PopupWindow* renderDialog; // The rendering settings dialog
	GLMotif::PopupWindow* animationDialog; // The animation dialog
	GLMotif::TextField* currentTimeValue; // Text field showing the current animation time
	GLMotif::Slider* currentTimeSlider; // Slider to adjust the current animation time
	GLMotif::TextField* playSpeedValue; // Text field showing the animation speed
	GLMotif::Slider* playSpeedSlider; // Slider to adjust the animation speed
	GLMotif::ToggleButton* playToggle; // Toggle button for automatic playback
	
	/* Private methods: */
	GLMotif::Popup* createRenderTogglesMenu(void); // Creates the "Rendering Modes" submenu
	GLMotif::PopupMenu* createMainMenu(void); // Creates the program's main menu
	GLMotif::PopupWindow* createRenderDialog(void); // Creates the rendering settings dialog
	void updateCurrentTime(void); // Updates the current time text field
	GLMotif::PopupWindow* createAnimationDialog(void); // Create the animation dialog
	GLPolylineTube* readSensorPathFile(const char* sensorPathFileName,double scaleFactor);
	
	/* Constructors and destructors: */
	public:
	ShowEarthModel(int& argc,char**& argv);
	virtual ~ShowEarthModel(void);
	
	/* Methods: */
	virtual void initContext(GLContextData& contextData) const;
	virtual void toolCreationCallback(Vrui::ToolManager::ToolCreationCallbackData* cbData);
	virtual void frame(void);
	virtual void display(GLContextData& contextData) const;
	void alignSurfaceFrame(Vrui::SurfaceNavigationTool::AlignmentData& alignmentData);
	void menuToggleSelectCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData);
	void renderDialogCloseCallback(Misc::CallbackData* cbData);
	void animationDialogCloseCallback(Misc::CallbackData* cbData);
	void sliderCallback(GLMotif::Slider::ValueChangedCallbackData* cbData);
	void centerDisplayCallback(Misc::CallbackData* cbData);
	void setEventTime(double newEventTime);
	};

#endif
