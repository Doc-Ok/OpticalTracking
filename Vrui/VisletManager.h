/***********************************************************************
VisletManager - Class to manage vislet classes.
Copyright (c) 2006-2014 Oliver Kreylos

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

#ifndef VRUI_VISLETMANAGER_INCLUDED
#define VRUI_VISLETMANAGER_INCLUDED

#include <vector>
#include <Misc/ConfigurationFile.h>
#include <Plugins/FactoryManager.h>
#include <GLMotif/ToggleButton.h>
#include <Vrui/Vislet.h>

/* Forward declarations: */
class GLContextData;
namespace GLMotif {
class Popup;
class SubMenu;
}
class ALContextData;

namespace Vrui {

class VisletManager:public Plugins::FactoryManager<VisletFactory>
	{
	/* Embedded classes: */
	private:
	typedef std::vector<Vislet*> VisletList; // Data type for list of loaded vislets
	
	/* Elements: */
	Misc::ConfigurationFileSection configFileSection; // The vislet manager's configuration file section - valid throughout the manager's entire lifetime
	VisletList vislets; // List of all loaded vislets
	GLMotif::SubMenu* visletMenu; // Submenu to activate or deactivate individual vislets
	
	/* Private methods: */
	void visletMenuToggleButtonCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData);
	
	/* Constructors and destructors: */
	public:
	VisletManager(const Misc::ConfigurationFileSection& sConfigFileSection); // Initializes vislet manager by reading given configuration file section
	~VisletManager(void); // Destroys vislet manager and all loaded vislets
	
	/* Methods: */
	Misc::ConfigurationFileSection getVisletClassSection(const char* visletClassName) const; // Returns the configuration file section a vislet class should use for its initialization
	Vislet* createVislet(VisletFactory* factory,int numVisletArguments,const char* const visletArguments[]); // Loads a vislet of the given class and initializes it with the given list of parameters
	size_t getNumVislets(void) const // Returns the number of vislets managed by the vislet manager
		{
		return vislets.size();
		}
	Vislet* getVislet(size_t visletIndex) // Returns the vislet of the given index
		{
		return vislets[visletIndex];
		}
	GLMotif::Popup* buildVisletMenu(void); // Returns a popup containing a submenu with one toggle button for each managed vislet
	void enable(void); // Enables all vislets
	void disable(void); // Disables all vislets
	void frame(void); // Calls the frame function of all loaded vislets
	void display(GLContextData& contextData) const; // Renders all loaded vislets
	void sound(ALContextData& contextData) const; // Renders the sound of all loaded vislets
	};

}

#endif
