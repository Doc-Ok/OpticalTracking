/***********************************************************************
EarthquakeTool - Vrui tool class to snap a virtual input device to
events in an earthquake data set.
Copyright (c) 2009-2013 Oliver Kreylos

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

#ifndef EARTHQUAKETOOL_INCLUDED
#define EARTHQUAKETOOL_INCLUDED

#include <Vrui/TransformTool.h>

/* Forward declarations: */
class EarthquakeSet;

class EarthquakeTool; // Forward declaration of the earthquake tool class

class EarthquakeToolFactory:public Vrui::ToolFactory // Class for factories that create/destroy earthquake tool objects
	{
	friend class EarthquakeTool;
	
	/* Elements: */
	private:
	const std::vector<EarthquakeSet*>& earthquakeSets; // Reference to the list of earthquake sets queried by all tools created by this factory
	
	/* Constructors and destructors: */
	public:
	EarthquakeToolFactory(Vrui::ToolManager& toolManager,const std::vector<EarthquakeSet*>& sEarthquakeSets);
	static void factoryDestructor(Vrui::ToolFactory* factory)
		{
		delete factory;
		}
	virtual ~EarthquakeToolFactory(void);
	
	/* Methods from Vrui::ToolFactory: */
	virtual const char* getName(void) const;
	virtual Vrui::Tool* createTool(const Vrui::ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Vrui::Tool* tool) const;
	};

class EarthquakeTool:public Vrui::TransformTool // The earthquake tool class
	{
	friend class EarthquakeToolFactory;
	
	/* Elements: */
	private:
	static EarthquakeToolFactory* factory; // Pointer to the factory object for this class
	
	Vrui::Scalar lastRayParameter; // Last successful event intersection ray parameter for ray-based devices
	
	/* Constructors and destructors: */
	public:
	EarthquakeTool(const Vrui::ToolFactory* factory,const Vrui::ToolInputAssignment& inputAssignment);
	
	/* Methods from Vrui::Tool: */
	virtual void initialize(void);
	virtual const Vrui::ToolFactory* getFactory(void) const;
	virtual void frame(void);
	};

#endif
