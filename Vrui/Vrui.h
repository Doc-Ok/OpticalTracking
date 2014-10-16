/***********************************************************************
Vrui - Public kernel interface of the Vrui virtual reality development
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

#ifndef VRUI_INCLUDED
#define VRUI_INCLUDED

#include <utility>
#include <Misc/CallbackData.h>
#include <GL/gl.h>
#include <GL/GLColor.h>
#include <Vrui/Geometry.h>

/* Forward declarations: */
namespace Misc {
class Time;
class CallbackList;
class TimerEventScheduler;
}
namespace Cluster {
class Multiplexer;
class MulticastPipe;
}
class GLContextData;
struct GLMaterial;
class GLFont;
namespace GLMotif {
struct StyleSheet;
class Widget;
class WidgetManager;
class PopupMenu;
}
class ALContextData;
namespace Vrui {
class Glyph;
class GlyphRenderer;
class InputDevice;
class VirtualInputDevice;
class InputGraphManager;
class InputDeviceManager;
class TextEventDispatcher;
class MutexMenu;
class LightsourceManager;
class ClipPlaneManager;
class Viewer;
class VRScreen;
struct WindowProperties;
class VRWindow;
class ViewSpecification;
class Listener;
class SoundContext;
class CoordinateManager;
class Tool;
class ToolManager;
class VisletManager;
class DisplayState;
}

namespace Vrui {

/**************************
Additional Vrui data types:
**************************/

typedef GLColor<GLfloat,4> Color; // Data type for colors

struct NavigationTransformationChangedCallbackData:public Misc::CallbackData // Callback data passed to callbacks when the navigation transformation changes
	{
	/* Elements: */
	public:
	const NavTransform& oldTransform; // Previous navigation transformation
	const NavTransform& oldInverseTransform; // Inverse of previous navigation transformation
	const NavTransform& newTransform; // Next navigation transformation
	const NavTransform& newInverseTransform; // Inverse of next navigation transformation
	
	/* Constructors and destructors: */
	NavigationTransformationChangedCallbackData(const NavTransform& sOldTransform,const NavTransform& sOldInverseTransform,const NavTransform& sNewTransform,const NavTransform& sNewInverseTransform)
		:oldTransform(sOldTransform),oldInverseTransform(sOldInverseTransform),
		 newTransform(sNewTransform),newInverseTransform(sNewInverseTransform)
		{
		}
	};

/***********************************************************************
Vrui functions called from inside an application's main function. These
functions are deprecated; applications should use the Vrui::Application
class instead.
***********************************************************************/

/* Initializes the Vrui toolkit; must be called first: */
void init(int& argc,char**& argv,char**& appdefaults);

/* Sets the function that is called exactly once for each frame: */
typedef void (*FrameFunctionType)(void* userData);
void setFrameFunction(FrameFunctionType frameFunction,void* userData);

/* Sets the function that renders the application's current state: */
typedef void (*DisplayFunctionType)(GLContextData& contextData,void* userData);
void setDisplayFunction(DisplayFunctionType displayFunction,void* userData);

/* Sets the function that renders the application's current sound state: */
typedef void (*SoundFunctionType)(ALContextData& contextData,void* userData);
void setSoundFunction(SoundFunctionType soundFunction,void* userData);

/* Initializes the graphics and sound subsystems and starts the toolkit's main loop: */
void mainLoop(void);

/* Deinitializes the Vrui toolkit, must be called last: */
void deinit(void);

/**********************
Vrui control functions:
**********************/

/* Exits from a Vrui application by terminating the main loop: */
void shutdown(void);

/* Manage multipipe rendering: */
Cluster::Multiplexer* getClusterMultiplexer(void); // Returns the intra-cluster communication multiplexer in a cluster-based Vrui environment; returns 0 if called in a non-cluster environment)
bool isMaster(void); // Returns true if the multipipe node the caller is running on is the master
int getNodeIndex(void); // Returns index of the multipipe node the caller is running on (0: master node)
int getNumNodes(void); // Returns number of multipipe nodes, including master
Cluster::MulticastPipe* getMainPipe(void); // Returns Vrui's main frame pipe; safe to use inside frame function, user must call finishMessage() when done (returns 0 if called in a non-cluster environment)
Cluster::MulticastPipe* openPipe(void); // Opens a pipe for 1-to-n communication from master to all slaves (returns 0 if called in a non-cluster environment)

/* Manage glyph rendering: */
GlyphRenderer* getGlyphRenderer(void); // Returns pointer to the glyph renderer
void renderGlyph(const Glyph& glyph,const OGTransform& transformation,GLContextData& contextData); // Renders the given glyph with the given transformation

/* Manage the input graph: */
VirtualInputDevice* getVirtualInputDevice(void); // Returns pointer to the root virtual input device
InputGraphManager* getInputGraphManager(void); // Returns pointer to the input graph manager

/* Manage input devices: */
InputDeviceManager* getInputDeviceManager(void); // Returns pointer to the input device manager
int getNumInputDevices(void); // Returns number of input devices
InputDevice* getInputDevice(int index); // Returns pointer to input device of given index
InputDevice* findInputDevice(const char* name); // Returns pointer to input device of given name (returns 0 if name is not found)
InputDevice* addVirtualInputDevice(const char* name,int numButtons,int numValuators); // Creates a new ungrabbed virtual input device with the given number of buttons and valuators

/* Manage light sources and clipping planes: */
LightsourceManager* getLightsourceManager(void); // Returns the light source manager
ClipPlaneManager* getClipPlaneManager(void); // Returns the clipping plane manager

/* Query information about viewers: */
Viewer* getMainViewer(void); // Returns pointer to the "main" viewer (the one to use when clueless)
int getNumViewers(void); // Returns number of viewers
Viewer* getViewer(int index); // Returns pointer to viewer of given index
Viewer* findViewer(const char* name); // Returns pointer to viewer of given name (returns 0 if name is not found)

/* Query information about screens: */
VRScreen* getMainScreen(void); // Returns pointer to the "main" screen (the one to use when clueless)
int getNumScreens(void); // Returns number of screens
VRScreen* getScreen(int index); // Returns pointer to screen of given index
VRScreen* findScreen(const char* name); // Returns pointer to screen of given name (returns 0 if name is not found)
std::pair<VRScreen*,Scalar> findScreen(const Ray& ray); // Returns pointer to closest screen intersected by ray, and intersection ray parameter (returns 0 if no screen is intersected)

/* Query information about rendering windows: */
void requestWindowProperties(const WindowProperties& properties); // Requests the given set of properties from all subsequently opened display windows (must be called before main loop)
int getNumWindows(void); // Returns the total number of active rendering windows in the environment
VRWindow* getWindow(int index); // Returns pointer to window of given index; windows on other cluster nodes have NULL pointers
ViewSpecification calcViewSpec(int windowIndex,int eyeIndex); // Returns viewing specification in navigation coordinates for given eye in given window; returns garbage if given window is on another cluster node

/* Query information about listeners: */
Listener* getMainListener(void); // Returns pointer to the "main" listener (the one to use when clueless)
int getNumListeners(void); // Returns number of listeners
Listener* getListener(int index); // Returns pointer to listener of given index
Listener* findListener(const char* name); // Returns pointer to listener of given name (returns 0 if name is not found)

/* Query information about sound contexts: */
void requestSound(void); // Called during Vrui initialization to request sound processing during the main loop
int getNumSoundContexts(void); // Returns the number of active sound contexts
SoundContext* getSoundContext(int index); // Returns pointer to sound context of given index

/* Query information about environment: */
Scalar getInchFactor(void); // Returns the length of an inch, expressed in Vrui physical units
Scalar getMeterFactor(void); // Returns the length of a meter, expressed in Vrui physical units
Scalar getDisplaySize(void); // Returns the approximate size of the display environment in physical units
const Point& getDisplayCenter(void); // Returns the center of the display environment in physical coordinates
const Vector& getForwardDirection(void); // Returns a vector pointing in the main viewing direction
const Vector& getUpDirection(void); // Returns a vector pointing "up" in physical coordinates
const Plane& getFloorPlane(void); // Returns a plane equation for the "floor" of the display environment in physical coordinates

/* Manage rendering parameters: */
void setFrontplaneDist(Scalar newFrontplaneDist); // Sets the distance of the OpenGL front plane in physical units
Scalar getFrontplaneDist(void); // Returns the distance of the OpenGL front plane in physical units
void setBackplaneDist(Scalar newBackplaneDist); // Sets the distance of the OpenGL back plane in physical units
Scalar getBackplaneDist(void); // Returns the distance of the OpenGL back plane in physical units
void setBackgroundColor(const Color& newBackgroundColor); // Sets the OpenGL background color
const Color& getBackgroundColor(void); // Returns the OpenGL background color

/* Manage primary widgets and popup menus: */
GLFont* loadFont(const char* fontName); // Load and return a pointer to the font of the given name
const GLMotif::StyleSheet* getUiStyleSheet(void); // Returns the default Vrui look for UI components
float getUiSize(void); // Returns an appropriate size to use for UI components
const Color& getUiBgColor(void); // Returns the default background color for UI components
const Color& getUiFgColor(void); // Returns the default foreground color for UI components
const Color& getUiTextFieldBgColor(void); // Returns the default background color for text field UI components
const Color& getUiTextFieldFgColor(void); // Returns the default foreground color for text field UI components
GLFont* getUiFont(void); // Returns pointer to the default font for UI components
void setWidgetMaterial(const GLMaterial& newWidgetMaterial); // Sets the material property for rendering UI components
const GLMaterial& getWidgetMaterial(void); // Returns the material property for rendering UI components
void setMainMenu(GLMotif::PopupMenu* newMainMenu); // Sets the application's main menu (associated to all menu tools)
MutexMenu* getMainMenu(void); // Returns pointer to the application's main menu
Misc::TimerEventScheduler* getTimerEventScheduler(void); // Returns pointer to the scheduler for application timer events
TextEventDispatcher* getTextEventDispatcher(void); // Returns pointer to the GLMotif text event dispatcher
GLMotif::WidgetManager* getWidgetManager(void); // Returns pointer to the UI component manager
const ONTransform& getUiPlane(void); // Returns a plane to create / interact with UI components
Point calcUiPoint(const Ray& ray); // Returns a position to create / interact with UI components for the given input device
ONTransform calcUiTransform(const Ray& ray); // Returns a transformation to create / interact with UI components for the given input device
ONTransform calcHUDTransform(const Point& hotSpot); // Returns a transformation for showing a HUD or GUI element at the given hot spot
void popupPrimaryWidget(GLMotif::Widget* topLevel); // Shows a top-level UI component at a default position in the environment
void popupPrimaryWidget(GLMotif::Widget* topLevel,const Point& hotSpot,bool navigational =true); // Shows a top-level UI component at the given position in physical or navigational coordinates
void popupPrimaryScreenWidget(GLMotif::Widget* topLevel,Scalar x,Scalar y); // Shows a top-level UI component aligned to the given screen in the environment
void popdownPrimaryWidget(GLMotif::Widget* topLevel); // Hides a top-level UI component
void showErrorMessage(const char* title,const char* message); // Pops up a temporary dialog window to show an error message with an acknowledgment button

/* Manage 3D picking and selection: */
Scalar getPointPickDistance(void); // Returns the maximum distance to be used for point-based pick queries in navigational space
Scalar getRayPickCosine(void); // Returns the cosine of the maximum angle to be used for ray-based pick queries

/* Navigation transformation management: */
void setNavigationTransformation(const NavTransform& newNavigationTransformation); // Sets the navigation transformation
void setNavigationTransformation(const Point& center,Scalar radius); // Sets the navigation transformation such that a model inside the given sphere appears in the middle of the environment
void setNavigationTransformation(const Point& center,Scalar radius,const Vector& up); // Sets the navigation transformation such that a model inside the given sphere appears in the middle of the environment, and the given direction points "up"
void concatenateNavigationTransformation(const NavTransform& t); // Concatenates the given transformation to the navigation transformation from the right
void concatenateNavigationTransformationLeft(const NavTransform& t); // Concatenates the given transformation to the navigation transformation from the left
const NavTransform& getNavigationTransformation(void); // Returns the navigation transformation
const NavTransform& getInverseNavigationTransformation(void); // Returns the inverse of the navigation transformation
void disableNavigationTransformation(void); // Disable all navigation; from this point on, navigational coordinates are physical coordinates
Misc::CallbackList& getNavigationTransformationChangedCallbacks(void); // Returns the lists of callbacks called when the navigation transformation changes
CoordinateManager* getCoordinateManager(void); // Returns pointer to the coordinate manager

/* Pointer/device position/orientation management: */
Point getHeadPosition(void); // Returns the current position of the main viewer in navigational coordinates
Vector getViewDirection(void); // Returns the view direction of the main viewer in navigational coordinates
Point getDevicePosition(InputDevice* device); // Returns the position of the given input device in navigational coordinates
NavTrackerState getDeviceTransformation(InputDevice* device); // Returns the transformation of the given input device in navigational coordiantes

/* Tool management: */
ToolManager* getToolManager(void); // Returns pointer to the tool manager
bool activateNavigationTool(const Tool* tool); // Activates a navigation tool (prohibits all other navigation tools from becoming active)
void deactivateNavigationTool(const Tool* tool); // Deactivates a navigation tool such that other navigation tools may become active

/* Vislet maangement: */
VisletManager* getVisletManager(void); // Returns pointer to the vislet manager

/* Time management: */
Misc::Time getTimeOfDay(void); // Returns the system's wall clock time; requires a multicast data exchange in cluster environments
double getApplicationTime(void); // Returns the time since the application was started in seconds; is identical throughout a Vrui frame and across a cluster
double getFrameTime(void); // Returns the duration of the last frame in seconds
double getCurrentFrameTime(void); // Returns the current average time between frames (1/framerate) in seconds

/* Rendering management: */
void updateContinuously(void); // Tells Vrui to continuously update its state (must be called before mainLoop)
void requestUpdate(void); // Tells Vrui to update its internal state and redraw the VR windows; can be called from any thread
void scheduleUpdate(double nextFrameTime); // Asks Vrui to update its internal state and redraw the VR windows at the given application time; must be called from main thread
const DisplayState& getDisplayState(GLContextData& contextData); // Returns the Vrui display state valid for the current display method call

}

#endif
