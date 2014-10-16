/***********************************************************************
Internal kernel interface of the Vrui virtual reality development
toolkit.
Copyright (c) 2000-2014 Oliver Kreylos

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

#ifndef VRUI_INTERNAL_INCLUDED
#define VRUI_INTERNAL_INCLUDED

#include <vector>
#include <deque>
#include <Misc/Timer.h>
#include <Misc/CallbackList.h>
#include <Threads/Mutex.h>
#include <IO/Directory.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/OrthogonalTransformation.h>
#include <Geometry/Plane.h>
#include <GL/gl.h>
#include <GL/GLMaterial.h>
#include <GL/GLObject.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/WidgetManager.h>
#include <GLMotif/SubMenu.h>
#include <GLMotif/ToggleButton.h>
#include <Vrui/Vrui.h>
#include <Vrui/FileSelectionHelper.h>
#include <Vrui/GlyphRenderer.h>
#include <Vrui/WindowProperties.h>
#include <Vrui/DisplayState.h>
#include <Vrui/ToolManager.h>

/* Forward declarations: */
namespace Misc {
class ConfigurationFileSection;
class CallbackData;
class TimerEventScheduler;
}
namespace Cluster {
class Multiplexer;
class MulticastPipe;
}
namespace GLMotif {
class Container;
class Popup;
class PopupMenu;
class PopupWindow;
class CascadeButton;
}
namespace Vrui {
class InputDeviceDataSaver;
class MultipipeDispatcher;
class ScaleBar;
class VisletManager;
class GUIInteractor;
}

namespace Vrui {

/********************
Global program state:
********************/

struct VruiState
	{
	/* Embedded classes: */
	public:
	struct ScreenProtector // Structure describing an input device that needs to be protected from bumping into a screen
		{
		/* Elements: */
		public:
		InputDevice* inputDevice; // Pointer to input device
		Point center; // Center of protective sphere in input device's coordinates
		Scalar radius; // Radius of protective sphere around input device's position
		};
	
	class DisplayStateMapper:public GLObject // Helper class to associate DisplayState objects with each VRWindow's GL context
		{
		/* Embedded classes: */
		public:
		struct DataItem:public GLObject::DataItem
			{
			/* Elements: */
			public:
			DisplayState displayState; // The display state object
			};
		
		/* Methods from GLObject: */
		void initContext(GLContextData& contextData) const
			{
			/* No need to do anything */
			}
		};
	
	/* Elements: */
	
	/* Multipipe management: */
	Cluster::Multiplexer* multiplexer;
	bool master;
	Cluster::MulticastPipe* pipe;
	
	/* Random number management: */
	unsigned int randomSeed; // Seed value for random number generator
	
	/* Environment dimensions: */
	Scalar inchScale; // Length of an inch expressed in the (arbitrary) Vrui physical coordinate units
	Scalar meterScale; // Length of a meter expressed in the (arbitrary) Vrui physical coordinate units
	Point displayCenter;
	Scalar displaySize;
	Vector forwardDirection;
	Vector upDirection;
	Plane floorPlane;
	
	/* Glyph management: */
	GlyphRenderer* glyphRenderer;
	
	/* Input graph management: */
	Point newInputDevicePosition;
	VirtualInputDevice* virtualInputDevice;
	InputGraphManager* inputGraphManager;
	FileSelectionHelper inputGraphSelectionHelper; // Helper to load and save input graph files
	bool loadInputGraph; // Flag whether to replace the current input graph with one loaded from the given file at the next opportune moment
	std::string inputGraphFileName; // Name of input graph file to load if loadInputGraph is true
	
	/* Input device management: */
	TextEventDispatcher* textEventDispatcher;
	InputDeviceManager* inputDeviceManager;
	InputDeviceDataSaver* inputDeviceDataSaver;
	MultipipeDispatcher* multipipeDispatcher;
	
	/* Light source management: */
	LightsourceManager* lightsourceManager;
	
	/* Clipping plane management: */
	ClipPlaneManager* clipPlaneManager;
	
	/* Viewer management: */
	int numViewers;
	Viewer* viewers;
	Viewer* mainViewer;
	
	/* Screen management: */
	int numScreens;
	VRScreen* screens;
	VRScreen* mainScreen;
	
	/* Screen protection management: */
	int numProtectors;
	ScreenProtector* protectors;
	
	/* Window management: */
	WindowProperties windowProperties;
	DisplayStateMapper displayStateMapper;
	
	/* Listener management: */
	int numListeners;
	Listener* listeners;
	Listener* mainListener;
	
	/* Rendering parameters: */
	Scalar frontplaneDist;
	Scalar backplaneDist;
	Color backgroundColor;
	Color ambientLightColor;
	
	/* Sound rendering parameters: */
	bool useSound;
	
	/* Widget management: */
	GLMaterial widgetMaterial;
	GLMotif::StyleSheet uiStyleSheet;
	Misc::TimerEventScheduler* timerEventScheduler; // Scheduler for timer events
	GLMotif::WidgetManager* widgetManager;
	bool popWidgetsOnScreen;
	ONTransform widgetPlane; // Plane to which to lock widgets when popWidgetsOnScreen is true
	GLMotif::SubMenu* dialogsMenu;
	std::vector<GLMotif::PopupWindow*> poppedDialogs;
	GLMotif::PopupMenu* systemMenuPopup;
	GLMotif::CascadeButton* dialogsMenuCascade;
	MutexMenu* mainMenu;
	FileSelectionHelper viewSelectionHelper; // Helper to load and save view files
	
	/* 3D picking management: */
	Scalar pointPickDistance;
	Scalar rayPickCosine;
	
	/* Navigation transformation management: */
	std::string viewpointFileName;
	bool navigationTransformationEnabled;
	bool delayNavigationTransformation; // Flag whether changes to the navigation transformation are delayed until the beginning of the next frame
	int navigationTransformationChangedMask; // Bit mask for changed parts of the navigation transformation (0x1-transform,0x2-display center/size)
	NavTransform newNavigationTransformation;
	NavTransform navigationTransformation,inverseNavigationTransformation;
	std::vector<NavTransform> storedNavigationTransformations;
	Misc::CallbackList navigationTransformationChangedCallbacks; // List of callbacks called when the navigation transformation changes
	CoordinateManager* coordinateManager;
	ScaleBar* scaleBar;
	
	/* Tool management: */
	ToolManager* toolManager;
	
	/* Vislet management: */
	VisletManager* visletManager;
	
	/* Application function callbacks: */
	FrameFunctionType frameFunction;
	void* frameFunctionData;
	DisplayFunctionType displayFunction;
	void* displayFunctionData;
	SoundFunctionType soundFunction;
	void* soundFunctionData;
	
	/* Time management: */
	Misc::Timer appTime; // Free-running application timer
	double minimumFrameTime; // Lower limit on frame times; Vrui's main loop will block to pad frame times to this minimum
	double lastFrame; // Application time at which the last frame was started
	double lastFrameDelta; // Duration of last frame
	double nextFrameTime; // Scheduled time to start next frame, or 0.0 if no frame scheduled
	double synchFrameTime; // Precise time to be used for next frame
	bool synchWait; // Flag whether to delay the next frame until wallclock time matches synch time
	int numRecentFrameTimes; // Number of recent frame times to average from
	double* recentFrameTimes; // Array of recent times to complete a frame
	int nextFrameTimeIndex; // Index at which the next frame time is stored in the array
	double* sortedFrameTimes; // Helper array to calculate median of frame times
	double currentFrameTime; // Current frame time average
	
	/* Transient dragging/moving/scaling state: */
	const Tool* activeNavigationTool;
	
	/* Transient popup menu/primary widget interaction state: */
	GUIInteractor* mostRecentGUIInteractor; // Pointer to the most-recently used GUI interactor, to calculate an appropriate position to pop up dialog windows
	Point mostRecentHotSpot; // Final hot spot position when the most-recently used GUI interactor is destroyed
	
	/* List of created virtual input devices: */
	std::deque<InputDevice*> createdVirtualInputDevices;
	
	/* Rendering management state: */
	bool updateContinuously; // Flag if the inner Vrui loop never blocks
	
	/* Private methods: */
	GLMotif::Popup* buildDialogsMenu(void); // Builds the dialogs submenu
	GLMotif::Popup* buildViewMenu(void); // Builds the view submenu
	GLMotif::Popup* buildDevicesMenu(void); // Builds the input devices submenu
	void buildSystemMenu(GLMotif::Container* parent); // Builds the Vrui system menu inside the given container widget
	void updateNavigationTransformation(const NavTransform& newTransform); // Updates the working version of the navigation transformation
	void loadViewpointFile(IO::Directory& directory,const char* viewpointFileName); // Overrides the navigation transformation with viewpoint data stored in the given viewpoint file
	void toolDestructionCallback(ToolManager::ToolDestructionCallbackData* cbData); // Callback method called when a tool is destroyed
	
	/* Constructors and destructors: */
	VruiState(Cluster::Multiplexer* sMultiplexer,Cluster::MulticastPipe* sPipe); // Initializes basic Vrui state
	~VruiState(void);
	
	/* Methods: */
	
	/* Initialization methods: */
	void initialize(const Misc::ConfigurationFileSection& configFileSection); // Initializes complete Vrui state
	void createSystemMenu(void); // Creates Vrui's system menu
	DisplayState* registerContext(GLContextData& contextData) const; // Registers a newly created OpenGL context with the Vrui state object
	void prepareMainLoop(void); // Performs last steps of initialization before main loop is run
	
	/* Frame processing methods: */
	void update(void); // Update Vrui state for current frame
	void display(DisplayState* displayState,GLContextData& contextData) const; // Vrui display function
	void sound(ALContextData& contextData) const; // Vrui sound function
	
	/* De-initialization methods: */
	void finishMainLoop(void); // Performs first steps of shutdown after mainloop finishes
	
	/* System menu callback methods: */
	void dialogsMenuCallback(GLMotif::SubMenu::EntrySelectCallbackData* cbData);
	void widgetPopCallback(GLMotif::WidgetManager::WidgetPopCallbackData* cbData);
	void loadViewCallback(GLMotif::FileSelectionDialog::OKCallbackData* cbData);
	void saveViewCallback(GLMotif::FileSelectionDialog::OKCallbackData* cbData);
	void pushViewCallback(Misc::CallbackData* cbData);
	void popViewCallback(Misc::CallbackData* cbData);
	void createInputDeviceCallback(Misc::CallbackData* cbData,const int& numButtons);
	void destroyInputDeviceCallback(Misc::CallbackData* cbData);
	void loadInputGraphCallback(GLMotif::FileSelectionDialog::OKCallbackData* cbData);
	void saveInputGraphCallback(GLMotif::FileSelectionDialog::OKCallbackData* cbData);
	void showScaleBarToggleCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData);
	void quitCallback(Misc::CallbackData* cbData);
	};

extern VruiState* vruiState;

/********************************
Private Vrui function prototypes:
********************************/

/* Forward declarations: */
struct VruiWindowGroup;

extern void setRandomSeed(unsigned int newRandomSeed); // Sets Vrui's random seed; can only be called by InputDeviceAdapterPlayback during its initialization
extern void vruiDelay(double interval);
extern void synchronize(double nextFrameTime,bool wait); // Gives a precise time value to use for the next frame; delays frame until wall-clock time matches if wait is true; can only be called by InputDeviceAdapterPlayback during playback
extern void setDisplayCenter(const Point& newDisplayCenter,Scalar newDisplaySize); // Sets the center and size of Vrui's display environment
extern void setMostRecentGUIInteractor(GUIInteractor* interactor); // Sets the most-recently-used GUI interaction tool
extern void resizeWindow(VruiWindowGroup* windowGroup,const VRWindow* window,const int newViewportSize[2],const int newFrameSize[2]); // Notifies the run-time environment that a window has changes viewport and/or frame buffer size
extern void getMaxWindowSizes(VruiWindowGroup* windowGroup,int viewportSize[2],int frameSize[2]); // Returns the maximum viewport and frame buffer sizes for the given window group

}

#endif
