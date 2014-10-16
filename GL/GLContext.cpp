/***********************************************************************
GLContext - Class to encapsulate state relating to a single OpenGL
context, to facilitate context sharing between windows.
Copyright (c) 2014 Oliver Kreylos

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

#include <GL/GLContext.h>

#include <stdexcept>
#include <Misc/ThrowStdErr.h>
#include <GL/GLExtensionManager.h>
#include <GL/GLContextData.h>

/**************************
Methods of class GLContext:
**************************/

GLContext::GLContext(const char* displayName,int* visualProperties)
	:display(0),visual(0),depth(-1),
	 extensionManager(0),contextData(0)
	{
	/* Open connection to the X server: */
	display=XOpenDisplay(displayName);
	if(display==0)
		Misc::throwStdErr("GLContext: Unable to open display %s",displayName!=0?displayName:"default");
	int screen=DefaultScreen(display);
	
	/* Query for GLX extension: */
	int errorBase,eventBase;
	if(!glXQueryExtension(display,&errorBase,&eventBase))
		Misc::throwStdErr("GLContext: GLX extension not supported on display %s",displayName!=0?displayName:"default");
	
	/* Use default visual properties if none were provided: */
	int defaultVisualProperties[]={GLX_RGBA,GLX_RED_SIZE,8,GLX_GREEN_SIZE,8,GLX_BLUE_SIZE,8,GLX_DEPTH_SIZE,16,GLX_DOUBLEBUFFER,None};
	if(visualProperties==0)
		visualProperties=defaultVisualProperties;
	
	/* Look for a matching visual: */
	XVisualInfo* visInfo=glXChooseVisual(display,screen,visualProperties);
	if(visInfo==0)
		{
		/* Reduce any requested color channel sizes, and try again: */
		for(int i=0;visualProperties[i]!=None;++i)
			if(visualProperties[i]==GLX_RED_SIZE||visualProperties[i]==GLX_GREEN_SIZE||visualProperties[i]==GLX_BLUE_SIZE)
				{
				/* Ask for at least one bit per channel: */
				++i;
				visualProperties[i]=1;
				}
		
		/* Search again: */
		visInfo=glXChooseVisual(display,screen,visualProperties);
		if(visInfo==0)
			{
			/* Reduce any requested depth channel sizes, and try yet again: */
			for(int i=0;visualProperties[i]!=None;++i)
				if(visualProperties[i]==GLX_DEPTH_SIZE)
					{
					/* Ask for at least one bit of depth channel: */
					++i;
					visualProperties[i]=1;
					}
			
			/* Search one last time: */
			visInfo=glXChooseVisual(display,screen,visualProperties);
			if(visInfo==0)
				{
				/* Now fail: */
				Misc::throwStdErr("GLContext: No suitable visual found on display %s",displayName!=0?displayName:"default");
				}
			}
		}
	
	/* Create an OpenGL context: */
	context=glXCreateContext(display,visInfo,0,GL_TRUE);
	if(context==0)
		Misc::throwStdErr("GLContext: Unable to create OpenGL context on display %s",displayName!=0?displayName:"default");
	
	/* Remember the chosen visual and display bit depth: */
	visual=visInfo->visual;
	depth=visInfo->depth;
	
	/* Delete the visual information structure: */
	XFree(visInfo);
	}

GLContext::~GLContext(void)
	{
	/* Release this GLX context if it is the current one: */
	if(glXGetCurrentContext()==context)
		glXMakeCurrent(display,None,0);
	
	/* Destroy this GLX context: */
	glXDestroyContext(display,context);
	
	/* Close the X server connection: */
	XCloseDisplay(display);
	}

bool GLContext::isDirect(void) const
	{
	return glXIsDirect(display,context);
	}

void GLContext::init(GLXDrawable drawable)
	{
	/* Check if the extension manager already exists: */
	if(extensionManager==0)
		{
		/* Associate the GLX context with the current thread and the given drawable: */
		if(!glXMakeCurrent(display,drawable,context))
			throw std::runtime_error("GLContext::init: Unable to bind GLX context");
		
		/* Create and initialize the extension manager: */
		extensionManager=new GLExtensionManager;
		
		/* Install this context's GL extension manager: */
		GLExtensionManager::makeCurrent(extensionManager);
		
		/* Create a context data manager: */
		contextData=new GLContextData(101);
		}
	}

void GLContext::deinit(void)
	{
	/* Delete the context data and extension managers: */
	GLContextData::makeCurrent(0);
	delete contextData;
	contextData=0;
	GLExtensionManager::makeCurrent(0);
	delete extensionManager;
	extensionManager=0;
	}

void GLContext::makeCurrent(GLXDrawable drawable)
	{
	/* Associate the GLX context with the current thread and the given drawable: */
	if(!glXMakeCurrent(display,drawable,context))
		throw std::runtime_error("GLContext::makeCurrent: Unable to set current GLX context");
	
	/* Install this context's GL extension manager: */
	GLExtensionManager::makeCurrent(extensionManager);
	
	/* Install the this context's GL context data manager: */
	GLContextData::makeCurrent(contextData);
	}

void GLContext::swapBuffers(GLXDrawable drawable)
	{
	/* Swap buffers in the given drawable: */
	glXSwapBuffers(display,drawable);
	}

void GLContext::release(void)
	{
	/* Check if this GLX context is the current one: */
	if(glXGetCurrentContext()==context)
		{
		/* Release this context's context data and extension managers: */
		GLContextData::makeCurrent(0);
		GLExtensionManager::makeCurrent(0);
		
		/* Release the GLX context: */
		glXMakeCurrent(display,None,0);
		}
	}
