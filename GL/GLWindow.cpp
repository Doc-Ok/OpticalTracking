/***********************************************************************
GLWindow - Class to encapsulate details of the underlying window system
implementation from an application wishing to use OpenGL windows.
Copyright (c) 2001-2014 Oliver Kreylos

This file is part of the OpenGL/GLX Support Library (GLXSupport).

The OpenGL/GLX Support Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The OpenGL/GLX Support Library is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the OpenGL/GLX Support Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <GL/GLWindow.h>

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdexcept>
#include <X11/cursorfont.h>
#include <GL/glx.h>
#include <GL/GLExtensionManager.h>
#include <Misc/SizedTypes.h>
#include <Misc/ThrowStdErr.h>
#include <Misc/CallbackData.h>

/**************************
Methods of class GLWindow:
**************************/

void GLWindow::initWindow(const char* windowName,bool decorate)
	{
	/* Check if the screen index is valid: */
	if(screen<0||screen>=ScreenCount(context->getDisplay()))
		Misc::throwStdErr("GLWindow: Screen %d does not exist on display %s",screen,DisplayString(context->getDisplay()));
	
	/* Get a handle to the root window: */
	root=RootWindow(context->getDisplay(),screen);
	
	/* Create an X colormap (visual might not be default): */
	colorMap=XCreateColormap(context->getDisplay(),root,context->getVisual(),AllocNone);
	
	/* Create an X window with the selected visual: */
	XSetWindowAttributes swa;
	swa.colormap=colorMap;
	swa.border_pixel=0;
	if(fullscreen) // Create a fullscreen window
		{
		windowPos.origin[0]=0;
		windowPos.origin[1]=0;
		windowPos.size[0]=DisplayWidth(context->getDisplay(),screen);
		windowPos.size[1]=DisplayHeight(context->getDisplay(),screen);
		decorate=false;
		}
	swa.override_redirect=False;
	swa.event_mask=PointerMotionMask|ButtonPressMask|ButtonReleaseMask|KeyPressMask|KeyReleaseMask|ExposureMask|StructureNotifyMask;
	unsigned long attributeMask=CWBorderPixel|CWColormap|CWOverrideRedirect|CWEventMask;
	window=XCreateWindow(context->getDisplay(),root,
	                     windowPos.origin[0],windowPos.origin[1],windowPos.size[0],windowPos.size[1],
	                     0,context->getDepth(),InputOutput,context->getVisual(),attributeMask,&swa);
	XSetStandardProperties(context->getDisplay(),window,windowName,windowName,None,0,0,0);
	
	if(!decorate)
		{
		/*******************************************************************
		Ask the window manager not to decorate this window:
		*******************************************************************/
		
		/* Create and fill in window manager hint structure inherited from Motif: */
		struct MotifHints // Structure to pass hints to window managers
			{
			/* Elements: */
			public:
			Misc::UInt32 flags;
			Misc::UInt32 functions;
			Misc::UInt32 decorations;
			Misc::SInt32 inputMode;
			Misc::UInt32 status;
			} hints;
		hints.flags=2U; // Only change decorations bit
		hints.functions=0U;
		hints.decorations=0U;
		hints.inputMode=0;
		hints.status=0U;
		
		/* Get the X atom to set hint properties: */
		Atom hintProperty=XInternAtom(context->getDisplay(),"_MOTIF_WM_HINTS",True);
		if(hintProperty!=None)
			{
			/* Set the window manager hint property: */
			XChangeProperty(context->getDisplay(),window,hintProperty,hintProperty,32,PropModeReplace,reinterpret_cast<unsigned char*>(&hints),5);
			}
		}
	
	/* Initiate window manager communication: */
	wmProtocolsAtom=XInternAtom(context->getDisplay(),"WM_PROTOCOLS",False);
	wmDeleteWindowAtom=XInternAtom(context->getDisplay(),"WM_DELETE_WINDOW",False);
	XSetWMProtocols(context->getDisplay(),window,&wmDeleteWindowAtom,1);
	
	/* Display the window on the screen: */
	XMapWindow(context->getDisplay(),window);
	
	/*********************************************************************
	Since modern window managers ignore window positions when opening
	windows, we now need to move the window to its requested position.
	Fix suggested by William Sherman.
	*********************************************************************/
	
	if(decorate)
		{
		/* Query the window tree to get the window's parent (the one containing the decorations): */
		Window win_root,win_parent;
		Window* win_children;
		unsigned int win_numChildren;
		XQueryTree(context->getDisplay(),window,&win_root,&win_parent,&win_children,&win_numChildren);

		/* Query the window's and the parent's geometry to calculate the window's offset inside its parent: */
		int win_parentX,win_parentY,win_x,win_y;
		unsigned int win_width,win_height,win_borderWidth,win_depth;
		XGetGeometry(context->getDisplay(),win_parent,&win_root,&win_parentX,&win_parentY,&win_width,&win_height,&win_borderWidth,&win_depth);
		XGetGeometry(context->getDisplay(),window,&win_root,&win_x,&win_y,&win_width,&win_height,&win_borderWidth,&win_depth);

		/* Move the window's interior's top-left corner to the requested position: */
		XMoveWindow(context->getDisplay(),window,windowPos.origin[0]-(win_x-win_parentX),windowPos.origin[1]-(win_y-win_parentY));
		
		/* Clean up: */
		XFree(win_children);
		}
	else
		{
		/* Move the window's top-left corner to the requested position: */
		XMoveWindow(context->getDisplay(),window,windowPos.origin[0],windowPos.origin[1]);
		}
	
	if(fullscreen)
		{
		/* Grab pointer and keyboard: */
		XGrabPointer(context->getDisplay(),window,True,0,GrabModeAsync,GrabModeAsync,None,None,CurrentTime);
		XGrabKeyboard(context->getDisplay(),window,True,GrabModeAsync,GrabModeAsync,CurrentTime);
		}
	
	/* Gobble up the initial rush of X events regarding window creation: */
	#if 1
	XEvent event;
	while(XCheckWindowEvent(context->getDisplay(),window,ExposureMask|StructureNotifyMask,&event))
		{
		switch(event.type)
			{
			case Expose:
				/* Put the event back into the queue to let caller handle it: */
				XPutBackEvent(context->getDisplay(),&event);
				goto doneWithEvents;
				break;
			
			case ConfigureNotify:
				/* Retrieve the final window size: */
				windowPos.origin[0]=event.xconfigure.x;
				windowPos.origin[1]=event.xconfigure.y;
				windowPos.size[0]=event.xconfigure.width;
				windowPos.size[1]=event.xconfigure.height;
				break;
			}
		}
	doneWithEvents:
	;
	#else
	while(true)
		{
		/* Look at the next event: */
		XEvent event;
		XPeekEvent(context->getDisplay(),&event);
		if(event.type==Expose)
			break; // Leave this event for the caller to process
		
		/* Process the next event: */
		XNextEvent(context->getDisplay(),&event);
		switch(event.type)
			{
			case ConfigureNotify:
				/* Retrieve the final window position and size: */
				windowPos.origin[0]=event.xconfigure.x;
				windowPos.origin[1]=event.xconfigure.y;
				windowPos.size[0]=event.xconfigure.width;
				windowPos.size[1]=event.xconfigure.height;
				break;
			}
		}
	#endif
	
	/* Initialize the OpenGL context: */
	context->init(window);
	
	/* Query needed GLX extension entry points: */
	glXSwapIntervalEXTProc=GLExtensionManager::getFunction<PFNGLXSWAPINTERVALEXTPROC>("glXSwapIntervalEXT");
	glXWaitVideoSyncSGIProc=GLExtensionManager::getFunction<PFNGLXWAITVIDEOSYNCSGIPROC>("glXWaitVideoSyncSGI");
	}

GLWindow::GLWindow(GLContext* sContext,int sScreen,const char* windowName,const GLWindow::WindowPos& sWindowPos,bool decorate)
	:context(sContext),
	 screen(sScreen),
	 windowPos(sWindowPos),fullscreen(windowPos.size[0]==0||windowPos.size[1]==0)
	{
	/* Call common part of window initialization routine: */
	initWindow(windowName,decorate);
	}

GLWindow::GLWindow(const char* displayName,const char* windowName,const GLWindow::WindowPos& sWindowPos,bool decorate,int* visualProperties)
	:context(new GLContext(displayName,visualProperties)),
	 screen(context->getDefaultScreen()),
	 windowPos(sWindowPos),fullscreen(windowPos.size[0]==0||windowPos.size[1]==0)
	{
	/* Call common part of window initialization routine: */
	initWindow(windowName,decorate);
	}

GLWindow::GLWindow(const char* windowName,const GLWindow::WindowPos& sWindowPos,bool decorate,int* visualProperties)
	:context(new GLContext(0,visualProperties)),
	 screen(context->getDefaultScreen()),
	 windowPos(sWindowPos),fullscreen(windowPos.size[0]==0||windowPos.size[1]==0)
	{
	/* Call common part of window initialization routine: */
	initWindow(windowName,decorate);
	}

GLWindow::GLWindow(GLWindow* source,int sScreen,const char* windowName,const GLWindow::WindowPos& sWindowPos,bool decorate)
	:context(source->context),
	 screen(sScreen),
	 windowPos(sWindowPos),fullscreen(windowPos.size[0]==0||windowPos.size[1]==0)
	{
	/* Call common part of window initialization routine: */
	initWindow(windowName,decorate);
	}

GLWindow::GLWindow(GLWindow* source,const char* windowName,const GLWindow::WindowPos& sWindowPos,bool decorate)
	:context(source->context),
	 screen(source->screen),
	 windowPos(sWindowPos),fullscreen(windowPos.size[0]==0||windowPos.size[1]==0)
	{
	/* Call common part of window initialization routine: */
	initWindow(windowName,decorate);
	}

GLWindow::~GLWindow(void)
	{
	if(fullscreen)
		{
		/* Release the pointer and keyboard grab: */
		XUngrabPointer(context->getDisplay(),CurrentTime);
		XUngrabKeyboard(context->getDisplay(),CurrentTime);
		}
	
	/* Close the window: */
	XUnmapWindow(context->getDisplay(),window);
	context->release();
	XDestroyWindow(context->getDisplay(),window);
	XFreeColormap(context->getDisplay(),colorMap);
	
	/* Context pointer's destructor will detach from GL context and possible destroy it */
	}

GLWindow::WindowPos GLWindow::getRootWindowPos(void) const
	{
	return WindowPos(DisplayWidth(context->getDisplay(),screen),DisplayHeight(context->getDisplay(),screen));
	}

double GLWindow::getScreenWidthMM(void) const
	{
	return double(DisplayWidthMM(context->getDisplay(),screen));
	}

double GLWindow::getScreenHeightMM(void) const
	{
	return double(DisplayHeightMM(context->getDisplay(),screen));
	}

void GLWindow::setWindowPos(const GLWindow::WindowPos& newWindowPos)
	{
	/* Query the window tree to get the window's parent (the one containing the decorations): */
	Window win_root,win_parent;
	Window* win_children;
	unsigned int win_numChildren;
	XQueryTree(context->getDisplay(),window,&win_root,&win_parent,&win_children,&win_numChildren);
	
	/* Check if the window has decorations: */
	int deltaX=0;
	int deltaY=0;
	if(win_parent!=win_root)
		{
		/* Query the window's and the parent's geometry to calculate the window's offset inside its parent: */
		int win_parentX,win_parentY,win_x,win_y;
		unsigned int win_width,win_height,win_borderWidth,win_depth;
		XGetGeometry(context->getDisplay(),win_parent,&win_root,&win_parentX,&win_parentY,&win_width,&win_height,&win_borderWidth,&win_depth);
		XGetGeometry(context->getDisplay(),window,&win_root,&win_x,&win_y,&win_width,&win_height,&win_borderWidth,&win_depth);
		deltaX=win_x-win_parentX;
		deltaY=win_y-win_parentY;
		}
	
	/* Move the window's interior's top-left corner to the requested position: */
	XMoveResizeWindow(context->getDisplay(),window,newWindowPos.origin[0]-deltaX,newWindowPos.origin[1]-deltaY,newWindowPos.size[0],newWindowPos.size[1]);
	XFlush(context->getDisplay());
	
	/* Clean up: */
	XFree(win_children);
	
	/* Don't update the window size structure yet; this method will cause a ConfigureNotify event which will handle it */
	}

void GLWindow::bypassCompositor(void)
	{
	/* Get relevant window manager protocol atoms: */
	Atom netwmBypassCompositorAtom=XInternAtom(context->getDisplay(),"_NET_WM_BYPASS_COMPOSITOR",True);
	if(netwmBypassCompositorAtom!=None)
		{
		/* Ask the window manager to let this window bypass the compositor: */
		XEvent bypassCompositorEvent;
		memset(&bypassCompositorEvent,0,sizeof(XEvent));
		bypassCompositorEvent.xclient.type=ClientMessage;
		bypassCompositorEvent.xclient.serial=0;
		bypassCompositorEvent.xclient.send_event=True;
		bypassCompositorEvent.xclient.display=context->getDisplay();
		bypassCompositorEvent.xclient.window=window;
		bypassCompositorEvent.xclient.message_type=netwmBypassCompositorAtom;
		bypassCompositorEvent.xclient.format=32;
		bypassCompositorEvent.xclient.data.l[0]=1; // Bypass compositor
		XSendEvent(context->getDisplay(),RootWindow(context->getDisplay(),screen),False,SubstructureRedirectMask|SubstructureNotifyMask,&bypassCompositorEvent);
		XFlush(context->getDisplay());
		}
	}

void GLWindow::makeFullscreen(void)
	{
	/*********************************************************************
	"Sane" version of fullscreen switch: Use the window manager protocol
	when supported; otherwise, fall back to hacky method.
	*********************************************************************/
	
	/* Get relevant window manager protocol atoms: */
	Atom netwmStateAtom=XInternAtom(context->getDisplay(),"_NET_WM_STATE",True);
	Atom netwmStateFullscreenAtom=XInternAtom(context->getDisplay(),"_NET_WM_STATE_FULLSCREEN",True);
	if(netwmStateAtom!=None&&netwmStateFullscreenAtom!=None)
		{
		/* Ask the window manager to make this window fullscreen: */
		XEvent fullscreenEvent;
		memset(&fullscreenEvent,0,sizeof(XEvent));
		fullscreenEvent.xclient.type=ClientMessage;
		fullscreenEvent.xclient.serial=0;
		fullscreenEvent.xclient.send_event=True;
		fullscreenEvent.xclient.display=context->getDisplay();
		fullscreenEvent.xclient.window=window;
		fullscreenEvent.xclient.message_type=netwmStateAtom;
		fullscreenEvent.xclient.format=32;
		fullscreenEvent.xclient.data.l[0]=1; // Should be _NET_WM_STATE_ADD, but that doesn't work for some reason
		fullscreenEvent.xclient.data.l[1]=netwmStateFullscreenAtom;
		fullscreenEvent.xclient.data.l[2]=0;
		fullscreenEvent.xclient.data.l[3]=1; // Request source is application
		XSendEvent(context->getDisplay(),RootWindow(context->getDisplay(),screen),False,SubstructureRedirectMask|SubstructureNotifyMask,&fullscreenEvent);
		XFlush(context->getDisplay());
		}
	else
		{
		/*******************************************************************
		Use hacky method of adjusting window size just beyond the root
		window. Only method available if there is no window manager, like on
		dedicated cluster rendering nodes.
		*******************************************************************/
		
		/* Query the window's geometry to calculate its offset inside its parent window (the window manager decoration): */
		Window win_root;
		int win_x,win_y;
		unsigned int win_width,win_height,win_borderWidth,win_depth;
		XGetGeometry(context->getDisplay(),window,&win_root,&win_x,&win_y,&win_width,&win_height,&win_borderWidth,&win_depth);
		
		/* Set the window's position and size such that the window manager decoration falls outside the root window: */
		XMoveResizeWindow(context->getDisplay(),window,-win_x,-win_y,DisplayWidth(context->getDisplay(),screen),DisplayHeight(context->getDisplay(),screen));
		}
	
	/* Raise the window to the top of the stacking hierarchy: */
	XRaiseWindow(context->getDisplay(),window);
	}

bool GLWindow::canVsync(bool frontBufferRendering) const
	{
	if(frontBufferRendering)
		return glXWaitVideoSyncSGIProc!=0&&context->isDirect();
	else
		return glXSwapIntervalEXTProc!=0;
	}

void GLWindow::setVsyncInterval(int newInterval)
	{
	/* Check if the GLX_EXT_swap_control extension is supported: */
	if(glXSwapIntervalEXTProc!=0)
		{
		/* Set the vsync interval: */
		glXSwapIntervalEXTProc(context->getDisplay(),window,newInterval);
		}
	}

void GLWindow::disableMouseEvents(void)
	{
	/* Get the window's current event mask: */
	XWindowAttributes wa;
	XGetWindowAttributes(context->getDisplay(),window,&wa);
	
	/* Disable mouse-related events: */
	XSetWindowAttributes swa;
	swa.event_mask=wa.all_event_masks&~(PointerMotionMask|EnterWindowMask|LeaveWindowMask|ButtonPressMask|ButtonReleaseMask);
	XChangeWindowAttributes(context->getDisplay(),window,CWEventMask,&swa);
	}

void GLWindow::hideCursor(void)
	{
	/* Why is it so goshdarn complicated to just hide the friggin' cursor? */
	static char emptyCursorBits[]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	                               0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	                               0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	                               0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	Pixmap emptyCursorPixmap=XCreatePixmapFromBitmapData(context->getDisplay(),window,emptyCursorBits,16,16,1,0,1);
	XColor black,white; // Actually, both are dummy colors
	Cursor emptyCursor=XCreatePixmapCursor(context->getDisplay(),emptyCursorPixmap,emptyCursorPixmap,&black,&white,0,0);
	XDefineCursor(context->getDisplay(),window,emptyCursor);
	XFreeCursor(context->getDisplay(),emptyCursor);
	XFreePixmap(context->getDisplay(),emptyCursorPixmap);
	}

void GLWindow::showCursor(void)
	{
	XUndefineCursor(context->getDisplay(),window);
	}

bool GLWindow::grabPointer(void)
	{
	/* Do nothing if the window is in fullscreen mode: */
	if(fullscreen)
		return true;
	
	bool result;
	
	/* Try grabbing the pointer: */
	result=XGrabPointer(context->getDisplay(),window,False,
	                    ButtonPressMask|ButtonReleaseMask|PointerMotionMask,
	                    GrabModeAsync,GrabModeAsync,None,None,CurrentTime)==GrabSuccess;
	if(result)
		{
		/* Try grabbing the keyboard as well: */
		result=XGrabKeyboard(context->getDisplay(),window,False,GrabModeAsync,GrabModeAsync,CurrentTime)==GrabSuccess;
		if(!result)
			{
			/* Release the pointer again: */
			XUngrabPointer(context->getDisplay(),CurrentTime);
			}
		}
	
	return result;
	}

void GLWindow::releasePointer(void)
	{
	/* Do nothing if the window is in fullscreen mode: */
	if(fullscreen)
		return;
	
	XUngrabPointer(context->getDisplay(),CurrentTime);
	XUngrabKeyboard(context->getDisplay(),CurrentTime);
	}

void GLWindow::setCursorPos(int newCursorX,int newCursorY)
	{
	XWarpPointer(context->getDisplay(),None,window,0,0,0,0,newCursorX,newCursorY);
	}

void GLWindow::redraw(void)
	{
	/* Send an expose X event for the entire area to this window: */
	XEvent event;
	memset(&event,0,sizeof(XEvent));
	event.type=Expose;
	event.xexpose.display=context->getDisplay();
	event.xexpose.window=window;
	event.xexpose.x=0;
	event.xexpose.y=0;
	event.xexpose.width=windowPos.size[0];
	event.xexpose.height=windowPos.size[1];
	event.xexpose.count=0;
	XSendEvent(context->getDisplay(),window,False,0x0,&event);
	XFlush(context->getDisplay());
	}

void GLWindow::waitForVsync(void)
	{
	/* Check if the GLX_SGI_video_sync extension is supported: */
	if(glXWaitVideoSyncSGIProc!=0)
		{
		/* Wait for the next vertical retrace synchronization pulse: */
		unsigned int count;
		glXWaitVideoSyncSGIProc(1,0,&count);
		}
	}

void GLWindow::processEvent(const XEvent& event)
	{
	switch(event.type)
		{
		case ConfigureNotify:
			{
			/* Retrieve the new window size: */
			windowPos.size[0]=event.xconfigure.width;
			windowPos.size[1]=event.xconfigure.height;
			
			/* Calculate the window's position on the screen: */
			Window child;
			XTranslateCoordinates(context->getDisplay(),window,root,0,0,&windowPos.origin[0],&windowPos.origin[1],&child);
			break;
			}
		
		case ClientMessage:
			if(event.xclient.message_type==wmProtocolsAtom&&event.xclient.format==32&&(Atom)(event.xclient.data.l[0])==wmDeleteWindowAtom)
				{
				/* Call the close callbacks: */
				Misc::CallbackData cbData;
				closeCallbacks.call(&cbData);
				}
			break;
		}
	}
