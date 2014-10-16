/***********************************************************************
VRWindow - Class for OpenGL windows that are used to map one or two eyes
of a viewer onto a VR screen.
Copyright (c) 2004-2014 Oliver Kreylos
ZMap stereo mode additions copyright (c) 2011 Matthias Deller.

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

#ifndef VRUI_VRWINDOW_INCLUDED
#define VRUI_VRWINDOW_INCLUDED

#include <string>
#include <Geometry/ComponentArray.h>
#include <Geometry/Point.h>
#include <Geometry/Ray.h>
#include <GL/gl.h>
#include <GL/GLWindow.h>
#include <Vrui/Geometry.h>
#include <Vrui/Internal/KeyMapper.h>
#include <Vrui/Internal/GetOutputConfiguration.h>

/* Forward declarations: */
namespace Misc {
class ConfigurationFileSection;
}
class GLExtensionManager;
class GLShader;
class GLContextData;
class GLFont;
namespace Vrui {
class InputDevice;
class Viewer;
class VRScreen;
struct WindowProperties;
class ViewSpecification;
class DisplayState;
class InputDeviceAdapterMouse;
class MovieSaver;
class VruiState;
}
namespace Vrui {
struct VruiWindowGroup;
class LensCorrector;
}

namespace Vrui {

class VRWindow:public GLWindow
	{
	/* Embedded classes: */
	public:
	enum WindowType // Enumerated type for VR window types
		{
		MONO,LEFT,RIGHT,QUADBUFFER_STEREO,ANAGLYPHIC_STEREO,SPLITVIEWPORT_STEREO,INTERLEAVEDVIEWPORT_STEREO,AUTOSTEREOSCOPIC_STEREO
		};
	
	/* Elements: */
	private:
	VruiState* vruiState; // Pointer to the Vrui state object this window belongs to
	VruiWindowGroup* windowGroup; // Pointer to the window group to which this window belongs
	InputDeviceAdapterMouse* mouseAdapter; // Pointer to the mouse input device adapter (if one exists; 0 otherwise)
	int windowMousePos[2]; // Last reported mouse position in window coordinates
	GLbitfield clearBufferMask; // Bit mask of OpenGL buffers that need to be cleared before rendering to this window
	bool vsync; // Flag if the window uses vertical retrace synchronization
	bool frontBufferRendering; // Flag if the window uses front buffer rendering
	DisplayState* displayState; // The display state object associated with this window's OpenGL context; updated before each rendering pass
	VRScreen* screens[2]; // Pointer to the two VR screens this window projects onto (left and right, usually identical)
	Viewer* viewers[2]; // Pointers to the two viewers viewing this window (left and right, usually identical)
	std::string outputName; // Name of output connector or connected monitor to which to limit this window
	OutputConfiguration outputConfiguration; // Size and panning domain of output to which window is bound
	int xrandrEventBase; // Base index for XRANDR events
	WindowType windowType; // Type of this window
	int multisamplingLevel; // Level of multisampling (FSAA) for this window (1 == multisampling disabled)
	GLWindow::WindowPos splitViewportPos[2]; // Positions and sizes of viewports for split-viewport stereo windows
	bool panningViewport; // Flag whether the window's viewport depends on the window's position on the display screen
	bool navigate; // Flag if the window should move the display when it is moved/resized
	bool movePrimaryWidgets; // Flag if the window should move primary popped-up widgets when it is moved/resized
	Scalar viewports[2][4]; // Viewport borders (left, right, bottom, top) for each VR screen in VR screen coordinates
	bool hasFramebufferObjectExtension; // Flag whether the local OpenGL supports GL_EXT_framebuffer_object (for interleaved viewport and autostereoscopic stereo modes)
	
	/* Interaction state: */
	KeyMapper::QualifiedKey exitKey; // Key to exit the Vrui application
	KeyMapper::QualifiedKey screenshotKey; // Key to save the window contents as an image
	KeyMapper::QualifiedKey burnModeToggleKey; // Key to toggle "burn mode"
	
	/* State for interleaved-viewport stereoscopic rendering: */
	int ivTextureSize[2]; // Size of off-screen buffer and textures used for interleaved-viewport rendering
	float ivTexCoord[2]; // Texture coordinates to map the viewport textures to the window
	int ivEyeIndexOffset; // Index offset to account for the odd/even position of the window's top-left corner pixel
	GLuint ivRightViewportTextureID; // Texture ID of the right viewport texture (left viewport uses window drawable)
	GLuint ivRightDepthbufferObjectID; // Object ID of the right viewport depthbuffer (alas, can't reuse window's depthbuffer)
	GLuint ivRightFramebufferObjectID; // Object ID of the right viewport framebuffer (left viewport uses window drawable)
	GLubyte* ivRightStipplePatterns[4]; // Four stipple patterns for the right viewport (which one is used depends on window origin position)
	
	/* State for rendering to autostereoscopic displays with multiple viewing zones: */
	int asNumViewZones; // Number of viewing zones for autostereoscopic rendering
	Scalar asViewZoneOffset; // Distance between viewing zone eye points in horizontal direction at viewer position
	int asNumTiles[2]; // Number of view zone tile columns and rows
	int asTextureSize[2]; // Size of off-screen buffer and textures used for autostereoscopic rendering
	GLuint asViewMapTextureID; // Texture ID of the view map mapping display subpixels to view zone pixels
	GLuint asViewZoneTextureID; // Texture ID of the intermediate frame buffer holding the view zone images
	GLuint asDepthBufferObjectID; // Object ID of the depth buffer used to render the view zone images
	GLuint asFrameBufferObjectID; // Object ID of the frame buffer used to render the view zone images
	GLShader* asInterzigShader; // GLSL shader to "interzig" separate view zone images into autostereoscopic image
	int asQuadSizeUniformIndex; // Index of quad size uniform variable in interzigging shader
	
	/* State for post-rendering lens distortion correction: */
	LensCorrector* lensCorrector; // Pointer to lens distortion correction object; will replace all state below
	
	VRScreen* mouseScreen; // Pointer to an optional screen used to map mouse positions from window coordinates to physical coordinates
	GLFont* showFpsFont; // Font to render the current update frequency
	bool showFps; // Flag if the window is to display the current update frequency
	bool burnMode; // Flag if the window is currently in burn mode, i.e., if it runs Vrui frames as quickly as possible and displays smoothed FPS
	unsigned int burnModeNumFrames; // Number of frames rendered in burn mode
	double burnModeStartTime; // Application time at which the window entered burn mode
	bool protectScreens; // Flag if the window's screen(s) need to be protected from nearby input devices
	bool trackToolKillZone; // Flag if the tool manager's tool kill zone should follow the window when moved/resized
	Scalar toolKillZonePos[2]; // Position of tool kill zone in relative window coordinates (0.0-1.0 in both directions)
	bool dirty; // Flag if the window needs to be redrawn
	bool resizeViewport; // Flag if the window's OpenGL viewport needs to be resized on the next draw() call
	bool saveScreenshot; // Flag if the window is to save its contents after the next draw() call
	std::string screenshotImageFileName; // Name of the image file into which to save the next screen shot
	MovieSaver* movieSaver; // Pointer to a movie saver object if the window is supposed to write contents to a movie
	
	/* Private methods: */
	void render(const GLWindow::WindowPos& viewportPos,int screenIndex,const Point& eye);
	
	/* Constructors and destructors: */
	public:
	static GLContext* createContext(const WindowProperties& properties,const Misc::ConfigurationFileSection& configFileSection); // Creates an OpenGL context based on settings from the given window properties and configuration file section
	VRWindow(GLContext* sContext,int sScreen,const char* windowName,const Misc::ConfigurationFileSection& configFileSection,VruiState* sVruiState,InputDeviceAdapterMouse* sMouseAdapter); // Initializes VR window using given OpenGL context and settings from given configuration file section
	virtual ~VRWindow(void);
	
	/* Methods: */
	void setWindowGroup(VruiWindowGroup* newWindowGroup); // Sets the window's window group
	void setVRScreen(int screenIndex,VRScreen* newScreen); // Overrides the window's screen; caller must know what he's doing
	void setVRScreen(VRScreen* newScreen); // Ditto; sets both windows
	void setScreenViewport(const Scalar newViewport[4]); // Overrides the window's viewport on its screen in screen coordinates
	void setViewer(int viewerIndex,Viewer* newViewer); // Overrides the window's viewer; caller must know what he's doing
	void setViewer(Viewer* newViewer); // Ditto; sets both viewers
	void deinit(void); // Releases a window's resources before destruction
	const int* getViewportSize(void) const // Returns window's viewport size in pixels
		{
		if(windowType==SPLITVIEWPORT_STEREO)
			return splitViewportPos[0].size; // Return size of left viewport for now
		else
			return GLWindow::getWindowSize();
		}
	int getViewportSize(int dimension) const // Returns one component of the window's viewport size in pixels
		{
		if(windowType==SPLITVIEWPORT_STEREO)
			return splitViewportPos[0].size[dimension]; // Return size of left viewport for now
		else
			return GLWindow::getWindowSize()[dimension];
		}
	const VRScreen* getVRScreen(int screenIndex =0) const // Returns the VR screen this window renders to
		{
		return screens[screenIndex];
		}
	VRScreen* getVRScreen(int screenIndex =0) // Ditto
		{
		return screens[screenIndex];
		}
	const Scalar* getScreenViewport(void) const // Returns the window's viewport on its screen in screen coordinates
		{
		return viewports[0];
		}
	Scalar* getScreenViewport(Scalar resultViewport[4]) const // Ditto, but copies viewport into provided array and returns pointer to array
		{
		for(int i=0;i<4;++i)
			resultViewport[i]=viewports[0][i];
		return resultViewport;
		}
	const Viewer* getViewer(int viewerIndex =0) const // Returns the viewer this window renders from
		{
		return viewers[viewerIndex];
		}
	Viewer* getViewer(int viewerIndex =0) // Ditto
		{
		return viewers[viewerIndex];
		}
	int getNumEyes(void) const; // Returns the number of eyes this window renders from
	Point getEyePosition(int eyeIndex) const; // Returns the position of the given eye in physical coordinates
	void updateMouseDevice(const int windowPos[2],InputDevice* mouse) const; // Positions a 3D mouse device based on the given pointer position in window coordinates
	ViewSpecification calcViewSpec(int eyeIndex) const; // Returns a view specification for the given eye in physical coordinates
	int* getWindowCenterPos(int windowCenterPos[2]) const // Returns the center of the window in window coordinates
		{
		for(int i=0;i<2;++i)
			windowCenterPos[i]=Math::div2(getWindowSize()[i]);
		return windowCenterPos;
		}
	bool processEvent(const XEvent& event); // Processes an X event; returns true if the Vrui mainloop should stop processing events for this frame
	bool isDirty(void) const // Returns true if the window needs to be redrawn
		{
		return dirty;
		}
	void requestScreenshot(const char* sScreenshotImageFileName); // Asks the window to save its contents to the given image file on the next render pass
	void draw(void); // Redraws the window's contents
	void swapBuffers(void); // Overridden method from GLWindow
	};

}

#endif
