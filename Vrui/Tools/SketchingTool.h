/***********************************************************************
SketchingTool - Tool to create and edit 3D curves.
Copyright (c) 2009-2013 Oliver Kreylos

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

#ifndef VRUI_SKETCHINGTOOL_INCLUDED
#define VRUI_SKETCHINGTOOL_INCLUDED

#include <string>
#include <vector>
#include <Geometry/Point.h>
#include <GL/gl.h>
#include <GL/GLColor.h>
#include <GLMotif/NewButton.h>
#include <GLMotif/Slider.h>
#include <Vrui/FileSelectionHelper.h>
#include <Vrui/Geometry.h>
#include <Vrui/UtilityTool.h>

/* Forward declarations: */
namespace GLMotif {
class PopupWindow;
class RowColumn;
class TextField;
}

namespace Vrui {

class SketchingTool;

class SketchingToolFactory:public ToolFactory
	{
	friend class SketchingTool;
	
	/* Elements: */
	private:
	Scalar detailSize; // Minimal length of line segments in curves
	std::string curveFileName; // Name of file into which curve data is saved
	
	/* Constructors and destructors: */
	public:
	SketchingToolFactory(ToolManager& toolManager);
	virtual ~SketchingToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual const char* getButtonFunction(int buttonSlotIndex) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class SketchingTool:public UtilityTool
	{
	friend class SketchingToolFactory;
	
	/* Embedded classes: */
	private:
	struct Curve // Structure to represent single-stroke curves
		{
		/* Embedded classes: */
		public:
		typedef GLColor<GLubyte,4> Color; // Type for colors
		
		struct ControlPoint // Structure for curve control points
			{
			/* Elements: */
			public:
			Point pos; // Control point position
			Scalar t; // Control point sample time
			};
		
		/* Elements: */
		GLfloat lineWidth; // Curve's cosmetic line width
		Color color; // Curve's color
		std::vector<ControlPoint> controlPoints; // The curve's control points
		};
	
	/* Elements: */
	static SketchingToolFactory* factory; // Pointer to the factory object for this class
	static const Curve::Color curveColors[8]; // Standard line color palette
	GLMotif::PopupWindow* controlDialogPopup;
	GLMotif::TextField* lineWidthValue;
	GLMotif::RowColumn* colorBox;
	std::vector<Curve*> curves; // The list of existing curves
	GLfloat newLineWidth; // Line width for new curves
	Curve::Color newColor; // Color for new curves
	bool active; // Flag whether the tool is currently creating a curve
	Curve* currentCurve; // Pointer to the currently created curve
	Point lastPoint; // The last point appended to the curve
	Point currentPoint; // The current dragging position
	FileSelectionHelper curvesSelectionHelper; // Helper object to load and save curve files
	
	/* Constructors and destructors: */
	public:
	SketchingTool(const Vrui::ToolFactory* sFactory,const Vrui::ToolInputAssignment& inputAssignment);
	virtual ~SketchingTool(void);
	
	/* Methods from Vrui::Tool: */
	virtual const Vrui::ToolFactory* getFactory(void) const
		{
		return factory;
		}
	virtual void buttonCallback(int buttonSlotIndex,Vrui::InputDevice::ButtonCallbackData* cbData);
	virtual void frame(void);
	virtual void display(GLContextData& contextData) const;
	
	/* New methods: */
	void lineWidthSliderCallback(GLMotif::Slider::ValueChangedCallbackData* cbData);
	void colorButtonSelectCallback(GLMotif::NewButton::SelectCallbackData* cbData);
	void saveCurvesCallback(GLMotif::FileSelectionDialog::OKCallbackData* cbData);
	void loadCurvesCallback(GLMotif::FileSelectionDialog::OKCallbackData* cbData);
	void deleteAllCurvesCallback(Misc::CallbackData* cbData);
	};

}

#endif
