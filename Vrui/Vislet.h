/***********************************************************************
Vislet - Abstract base class for application-independent visualization
plug-ins that can be loaded into Vrui applications on demand.
Copyright (c) 2006-2012 Oliver Kreylos

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

#ifndef VRUI_VISLET_INCLUDED
#define VRUI_VISLET_INCLUDED

#include <Plugins/Factory.h>

/* Forward declarations: */
class GLContextData;
class ALContextData;
namespace Vrui {
class VisletManager;
}

namespace Vrui {

class Vislet;

class VisletFactory:public Plugins::Factory
	{
	/* Constructors and destructors: */
	public:
	VisletFactory(const char* sClassName,VisletManager& visletManager); // Initializes vislet factory settings
	
	/* Methods: */
	virtual Vislet* createVislet(int numVisletArguments,const char* const visletArguments[]) const; // Creates a vislet of the class represented by this factory with the given arguments
	virtual void destroyVislet(Vislet* vislet) const; // Destroys a vislet of the class represented by this factory
	};

class Vislet
	{
	/* Elements: */
	protected:
	bool active; // Flag if the vislet is active (enabled)
	
	/* Constructors and destructors: */
	public:
	Vislet(void); // Initializes vislet with defaults from given factory; vislet is created in "disabled" state
	virtual ~Vislet(void); // Destroys the vislet
	
	/* Methods: */
	virtual VisletFactory* getFactory(void) const; // Returns pointer to factory that created this vislet
	bool isActive(void) const // Returns the vislet's active flag
		{
		return active;
		}
	virtual void disable(void); // Disables the vislet; called for the last time after the end of Vrui's main loop if vislet is active at that time
	virtual void enable(void); // Enables the vislet; called for the first time before the start of Vrui's main loop
	virtual void frame(void); // Method called exactly once every frame
	virtual void display(GLContextData& contextData) const; // Method for rendering the vislet's current state into the given OpenGL context
	virtual void sound(ALContextData& contextData) const; // Method for rendering the vislet's current sound state into the given OpenAL context
	};

}

#endif
