/***********************************************************************
MeasureEnvironment - Utility for guided surveys of a single-screen
VR environment using a Total Station.
Copyright (c) 2009-2013 Oliver Kreylos

This file is part of the Vrui calibration utility package.

The Vrui calibration utility package is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The Vrui calibration utility package is distributed in the hope that it
will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Vrui calibration utility package; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#ifndef MEASUREENVIRONMENT_INCLUDED
#define MEASUREENVIRONMENT_INCLUDED

#include <vector>
#include <Threads/Mutex.h>
#include <Threads/Thread.h>
#define GEOMETRY_NONSTANDARD_TEMPLATES
#include <Geometry/Point.h>
#include <Geometry/Ray.h>
#include <Geometry/OrthogonalTransformation.h>
#include <Geometry/ProjectiveTransformation.h>
#include <GLMotif/RadioBox.h>
#include <GLMotif/TextField.h>
#include <GLMotif/FileSelectionDialog.h>
#include <Vrui/Tool.h>
#include <Vrui/GenericToolFactory.h>
#include <Vrui/TransformTool.h>
#include <Vrui/Application.h>

#include "TotalStation.h"

/* Forward declarations: */
namespace GLMotif {
class PopupWindow;
}
class NaturalPointClient;

class MeasureEnvironment:public Vrui::Application
	{
	/* Embedded classes: */
	private:
	typedef double Scalar;
	typedef Geometry::Point<Scalar,3> Point;
	typedef Geometry::Ray<Scalar,3> Ray;
	typedef Geometry::OrthogonalTransformation<Scalar,3> OGTransform;
	typedef Geometry::ProjectiveTransformation<Scalar,3> PTransform;
	typedef std::vector<Point> PointList;
	typedef size_t PickResult;
	
	class PointSnapperTool; // Forward declaration
	typedef Vrui::GenericToolFactory<PointSnapperTool> PointSnapperToolFactory; // Tool class uses the generic factory class
	
	class PointSnapperTool:public Vrui::TransformTool,public Vrui::Application::Tool<MeasureEnvironment> // Tool to snap to measured points
		{
		friend class Vrui::GenericToolFactory<PointSnapperTool>;
		
		/* Elements: */
		private:
		static PointSnapperToolFactory* factory;
		
		/* Constructors and destructors: */
		public:
		PointSnapperTool(const Vrui::ToolFactory* factory,const Vrui::ToolInputAssignment& inputAssignment);
		
		/* Methods from Vrui::Tool: */
		virtual void initialize(void);
		virtual const Vrui::ToolFactory* getFactory(void) const;
		virtual void frame(void);
		};
	
	class PointQueryTool; // Forward declaration
	typedef Vrui::GenericToolFactory<PointQueryTool> PointQueryToolFactory; // Tool class uses the generic factory class
	
	class PointQueryTool:public Vrui::Tool,public Vrui::Application::Tool<MeasureEnvironment> // Tool class to query and delete measured points
		{
		friend class Vrui::GenericToolFactory<PointQueryTool>;
		
		/* Elements: */
		private:
		static PointQueryToolFactory* factory;
		GLMotif::PopupWindow* dialogPopup; // Point data display dialog
		GLMotif::TextField* pointType; // Point type (floor, screen, or ball)
		GLMotif::TextField* position[3]; // Text fields to display selected point positions
		bool dragging; // Flag if the tool is currently in a dragging selection
		PickResult pickResult; // Result of last pick operation
		
		/* Private methods: */
		void textFieldLayoutChangedCallback(GLMotif::TextField::LayoutChangedCallbackData* cbData);
		void deleteCallback(Misc::CallbackData* cbData);
		
		/* Constructors and destructors: */
		public:
		PointQueryTool(const Vrui::ToolFactory* factory,const Vrui::ToolInputAssignment& inputAssignment);
		virtual ~PointQueryTool(void);
		
		/* Methods from Vrui::Tool: */
		virtual const Vrui::ToolFactory* getFactory(void) const
			{
			return factory;
			}
		virtual void buttonCallback(int buttonSlotIndex,Vrui::InputDevice::ButtonCallbackData* cbData);
		virtual void frame(void);
		virtual void display(GLContextData& contextData) const;
		};
	
	friend class PointSnapperTool;
	friend class PointQueryTool;
	
	/* Elements: */
	TotalStation* totalStation;
	TotalStation::Scalar basePrismOffset; // Offset that must be applied to all distance measurements
	TotalStation::Scalar initialPrismOffset;
	NaturalPointClient* naturalPointClient;
	bool naturalPointFlipZ;
	OGTransform pointTransform; // Transformation applied to all incoming points
	Threads::Thread pointCollectorThread;
	mutable Threads::Mutex measuringMutex;
	int measuringMode;
	TotalStation::Scalar ballRadius;
	PointList floorPoints;
	int screenPixelSize[2];
	int gridSize;
	PointList screenPoints;
	PointList ballPoints;
	PointList trackerPoints;
	bool measurementsDirty; // Flag if there are unsaved measurement changes
	GLMotif::PopupMenu* mainMenu; // The program's main menu
	
	/* Private methods: */
	static PTransform calcNormalization(const PointList& points);
	GLMotif::PopupMenu* createMainMenu(void); // Creates the program's main menu
	void* pointCollectorThreadMethod(void);
	void loadMeasurementFile(IO::Directory& directory,const char* fileName);
	void saveMeasurementFile(const char* fileName);
	void loadOptitrackSampleFile(IO::Directory& directory,const char* fileName,bool flipZ);
	
	/* Constructors and destructors: */
	public:
	MeasureEnvironment(int& argc,char**& argv,char**& appDefaults);
	~MeasureEnvironment(void);
	
	/* Methods: */
	virtual void frame(void);
	virtual void display(GLContextData& contextData) const;
	
	/* New methods: */
	PickResult pickPoint(const Point& point) const;
	PickResult pickPoint(const Ray& ray) const;
	std::pair<int,int> classifyPickResult(const PickResult& pickResult) const;
	Point snapToPoint(const Point& point,const PickResult& pickResult) const;
	void deletePoint(const PickResult& pickResult);
	void changeMeasuringModeCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData);
	void loadMeasurementFileCallback(Misc::CallbackData* cbData);
	void loadMeasurementFileOKCallback(GLMotif::FileSelectionDialog::OKCallbackData* cbData);
	void loadOptitrackSampleFileCallback(Misc::CallbackData* cbData);
	void loadOptitrackSampleFileOKCallback(GLMotif::FileSelectionDialog::OKCallbackData* cbData);
	void saveMeasurementFileCallback(Misc::CallbackData* cbData);
	void createTransformationCallback(Misc::CallbackData* cbData);
	void fitScreenTransformationCallback(Misc::CallbackData* cbData);
	};

#endif
