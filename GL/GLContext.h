/***********************************************************************
GLContext - Class to encapsulate state relating to a single OpenGL
context, to facilitate context sharing between windows.
Copyright (c) 2013-2014 Oliver Kreylos

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

#ifndef GLCONTEXT_INCLUDED
#define GLCONTEXT_INCLUDED

#include <Misc/Autopointer.h>
#include <Threads/RefCounted.h>
#include <X11/X.h>
#include <GL/glx.h>

/* Forward declarations: */
class GLExtensionManager;
class GLContextData;

class GLContext:public Threads::RefCounted
	{
	/* Elements: */
	private:
	Display* display; // Display connection for this context
	GLXContext context; // GLX context handle
	Visual* visual; // Pointer to the visual for which the GLX context was created
	int depth; // Bit depth of the visual associated with the GLX context
	GLExtensionManager* extensionManager; // Pointer to an extension manager for this GLX context
	GLContextData* contextData; // Pointer to an object associating per-context application state with this GLX context
	
	/* Constructors and destructors: */
	public:
	GLContext(const char* displayName,int* visualProperties =0); // Creates an OpenGL context for the given display name using the given visual properties (or default properties if null pointer is passed)
	private:
	GLContext(const GLContext& source); // Prohibit copy constructor
	GLContext& operator=(const GLContext& source); // Prohibit assignment operator
	public:
	virtual ~GLContext(void); // Destroys the OpenGL context and all associated data
	
	/* Methods: */
	Display* getDisplay(void) // Returns the context's display connection
		{
		return display;
		}
	int getDefaultScreen(void) // Returns the context's default screen
		{
		return XDefaultScreen(display);
		}
	Visual* getVisual(void) // Returns the context's visual
		{
		return visual;
		}
	int getDepth(void) // Returns the context's bit depth
		{
		return depth;
		}
	bool isDirect(void) const; // Returns true if the OpenGL context supports direct rendering
	void init(GLXDrawable drawable); // Creates the context's extension and context data managers; context will be bound to the given drawable
	void deinit(void); // Destroys the context's extension and context data managers; context must be current on some drawable
	GLExtensionManager& getExtensionManager(void) // Returns the context's extension manager
		{
		return *extensionManager;
		}
	GLContextData& getContextData(void) // Returns the context's context data manager
		{
		return *contextData;
		}
	void makeCurrent(GLXDrawable drawable); // Makes this OpenGL context current in the current thread and the given drawable (window or off-screen buffer)
	void swapBuffers(GLXDrawable drawable); // Swaps front and back buffers in the given drawable
	void release(void); // Detaches the OpenGL context from the current thread and drawable if it is the current context
	};

typedef Misc::Autopointer<GLContext> GLContextPtr; // Type for automatic pointers to GLContext objects

#endif
