/***********************************************************************
EarthquakeQueryTool - Vrui tool class to query and display meta data
about earthquake events.
Copyright (c) 2013 Oliver Kreylos

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

#ifndef EARTHQUAKEQUERYTOOL_INCLUDED
#define EARTHQUAKEQUERYTOOL_INCLUDED

#include <Misc/FunctionCalls.h>
#include <Vrui/Tool.h>

#include "EarthquakeSet.h"

/* Forward declarations: */
namespace GLMotif {
class PopupWindow;
class TextField;
}

class EarthquakeQueryTool; // Forward declaration of the earthquake tool class

class EarthquakeQueryToolFactory:public Vrui::ToolFactory // Class for factories that create/destroy earthquake tool objects
	{
	friend class EarthquakeQueryTool;
	
	/* Embedded classes: */
	public:
	typedef Misc::FunctionCall<double> SetTimeFunction; // Function call to set an earthquake event's time in a caller
	
	/* Elements: */
	private:
	const std::vector<EarthquakeSet*>& earthquakeSets; // Reference to the list of earthquake sets queried by all tools created by this factory
	SetTimeFunction* setTimeFunction; // Function called when a user selects a data dialog's "Set Time" button
	
	/* Constructors and destructors: */
	public:
	EarthquakeQueryToolFactory(Vrui::ToolManager& toolManager,const std::vector<EarthquakeSet*>& sEarthquakeSets,SetTimeFunction* sSetTimeFunction =0);
	static void factoryDestructor(Vrui::ToolFactory* factory)
		{
		delete factory;
		}
	virtual ~EarthquakeQueryToolFactory(void);
	
	/* Methods from Vrui::ToolFactory: */
	virtual const char* getName(void) const;
	virtual Vrui::Tool* createTool(const Vrui::ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Vrui::Tool* tool) const;
	};

class EarthquakeQueryTool:public Vrui::Tool // The earthquake query tool class
	{
	friend class EarthquakeQueryToolFactory;
	
	/* Elements: */
	private:
	static EarthquakeQueryToolFactory* factory; // Pointer to the factory object for this class
	
	GLMotif::PopupWindow* dataDialog;
	GLMotif::TextField* timeTextField;
	GLMotif::TextField* magnitudeTextField;
	const EarthquakeSet::Event* selectedEvent; // The currently selected and displayed event
	
	/* Private methods: */
	void setTimeButtonCallback(Misc::CallbackData* cbData);
	
	/* Constructors and destructors: */
	public:
	EarthquakeQueryTool(const Vrui::ToolFactory* factory,const Vrui::ToolInputAssignment& inputAssignment);
	virtual ~EarthquakeQueryTool(void);
	
	/* Methods from Vrui::Tool: */
	virtual const Vrui::ToolFactory* getFactory(void) const;
	virtual void frame(void);
	virtual void display(GLContextData& contextData) const;
	};

#endif
