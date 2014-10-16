/***********************************************************************
ScreenshotTool - Class for tools to save save screenshots from immersive
environments by overriding a selected window's screen and viewer with
virtual ones attached to an input device.
Copyright (c) 2008-2013 Oliver Kreylos

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

#ifndef VRUI_SCREENSHOTTOOL_INCLUDED
#define VRUI_SCREENSHOTTOOL_INCLUDED

#include <string>
#include <Geometry/Point.h>
#include <Vrui/Geometry.h>
#include <Vrui/UtilityTool.h>

/* Forward declarations: */
namespace Vrui {
class VRWindow;
class VRScreen;
class Viewer;
}

namespace Vrui {

class ScreenshotTool;

class ScreenshotToolFactory:public ToolFactory
	{
	friend class ScreenshotTool;
	
	/* Elements: */
	private:
	std::string screenshotFileName; // Name of file into which screenshots are saved
	int windowIndex; // Index of master node window from which to save screenshots
	Scalar screenSize; // Diagonal size for virtual screen; aspect ration determined by screenshot window
	Vector horizontal,vertical; // Screen directions in device coordinates
	bool useMainViewer; // Flag whether to use the environment's main viewer for the virtual camera
	Point monoEyePosition; // Mono eye position in virtual screen coordinates (y is up) relative to screen center for fixed viewer
	Vector eyeOffset; // Vector from mono eye position to right eye in screen coordinates for fixed viewer
	
	/* Constructors and destructors: */
	public:
	ScreenshotToolFactory(ToolManager& toolManager);
	virtual ~ScreenshotToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual const char* getButtonFunction(int buttonSlotIndex) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class ScreenshotTool:public UtilityTool
	{
	friend class ScreenshotToolFactory;
	
	/* Elements: */
	private:
	static ScreenshotToolFactory* factory; // Pointer to the factory object for this class
	
	/* Master node state: */
	VRWindow* window; // Pointer to the window from which to save screenshots
	VRScreen *originalScreen; // Pointer to the original screen used by the screenshot window
	Scalar originalViewport[4]; // Pointer to the original screen viewport used by the screenshot window
	Viewer* originalViewer; // Pointer to the original viewer used by the screenshot window
	VRScreen* virtualScreen; // Virtual screen used while the tool is active
	Viewer* virtualViewer; // Virtual viewer used while the tool is active
	Scalar screenW,screenH; // Width and height of virtual screen
	
	/* Shared state: */
	Point screenCenter; // Center point of screen in device coordinates
	Point screenBox[4]; // Four corner points of the virtual screen in device coordinates
	Point eyePosition; // Fixed eye position in device coordinates if not using the main viewer
	
	/* Transient state: */
	double showFrameTime; // Application time after which the frame will be drawn (to disable frame when a screenshot is requested)
	
	/* Constructors and destructors: */
	public:
	ScreenshotTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	
	/* Methods from Tool: */
	virtual void initialize(void);
	virtual void deinitialize(void);
	virtual const ToolFactory* getFactory(void) const;
	virtual void buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData);
	virtual void frame(void);
	virtual void display(GLContextData& contextData) const;
	};

}

#endif
