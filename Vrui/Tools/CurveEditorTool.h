/***********************************************************************
CurveEditorTool - Tool to create and edit 3D curves (represented as
splines in hermite form).
Copyright (c) 2007-2013 Oliver Kreylos

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

#ifndef VRUI_CURVEEDITORTOOL_INCLUDED
#define VRUI_CURVEEDITORTOOL_INCLUDED

#include <string>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <GLMotif/RadioBox.h>
#include <GLMotif/Slider.h>
#include <GLMotif/ToggleButton.h>
#include <Vrui/FileSelectionHelper.h>
#include <Vrui/Geometry.h>
#include <Vrui/UtilityTool.h>

/* Forward declarations: */
namespace Math {
class Matrix;
}
namespace GLMotif {
class PopupWindow;
class TextField;
}

namespace Vrui {

class CurveEditorTool;

class CurveEditorToolFactory:public ToolFactory
	{
	friend class CurveEditorTool;
	
	/* Elements: */
	private:
	std::string curveFileName; // Name of file into which curve data is saved
	Scalar vertexRadius; // Radius of vertex glyphs
	Scalar handleRadius; // Radius of vertex tangent handle glyphs
	Scalar curveRadius; // Radius of cylindrical tube around curve
	
	/* Constructors and destructors: */
	public:
	CurveEditorToolFactory(ToolManager& toolManager);
	virtual ~CurveEditorToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual const char* getButtonFunction(int buttonSlotIndex) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class CurveEditorTool:public UtilityTool
	{
	friend class CurveEditorToolFactory;
	
	/* Embedded classes: */
	private:
	struct ControlPoint // Structure for curve control points
		{
		/* Elements: */
		public:
		Point center; // Position of display center in navigational coordinates
		Scalar size; // Size of display in navigational coordinates
		Vector forward,up; // Normalized forward and up vectors in navigational coordinates
		
		/* Methods: */
		static ControlPoint affineCombination(const ControlPoint& cp1,const ControlPoint& cp2,Scalar w2) // Returns affine combination of two control points
			{
			ControlPoint result;
			result.center=Geometry::affineCombination(cp1.center,cp2.center,w2);
			Scalar w1=Scalar(1)-w2;
			result.size=cp1.size*w1+cp2.size*w2;
			result.forward=cp1.forward*w1+cp2.forward*w2;
			result.forward.normalize();
			result.up=cp1.up*w1+cp2.up*w2;
			result.up.normalize();
			return result;
			}
		};
	
	struct Vertex; // Forward declaration
	
	struct Segment // Structure for a curve segment (a cubic Bezier curve)
		{
		/* Elements: */
		public:
		Vertex* vertices[2]; // Pointers to the segments's start and end vertices
		ControlPoint mid[2]; // The two segment midpoints defining the start and end tangent vectors in navigational coordinates
		Scalar parameterInterval; // Length of segment's parameter interval
		bool forceStraight; // Flag forcing the segment to a straight line
		};
	
	struct Vertex:public ControlPoint // Structure for curve vertices
		{
		/* Embedded classes: */ 
		public:
		enum Continuity // Enumerated type for segment-segment continuities
			{
			NONE,TANGENT,DERIVATIVE
			};
		
		/* Elements: */
		Segment* segments[2]; // Pointers to the (up to) two segments sharing the vertex
		Continuity continuity; // Vertex' continuity type
		};
	
	enum C2BoundaryCondition // Enumerated type for boundary conditions of C^2-continuous curves
		{
		ZERO_VELOCITY,ZERO_ACCELERATION
		};
	
	enum EditingMode // Enumerated type for editing modes
		{
		IDLE,
		CREATING_VERTEX,
		DRAGGING_VERTEX,
		DRAGGING_HANDLE
		};
	
	/* Elements: */
	static CurveEditorToolFactory* factory; // Pointer to the factory object for this class
	GLMotif::PopupWindow* curveEditorDialogPopup; // Pointer to the curve editor dialog window
	GLMotif::ToggleButton* forceC2ContinuityToggle; // Toggle button to force C^2 continuity
	GLMotif::RadioBox* c2BoundaryConditionBox; // Radio box to select boundary conditions for C^2 continuity
	GLMotif::TextField* parameterValueText; // Text field displaying the current parameter value
	GLMotif::Slider* parameterValueSlider; // Slider to select parameter values along the curve
	GLMotif::ToggleButton* scrubToggle; // Toggle to enable "scrubbing" of the viewpoint along the curve
	GLMotif::ToggleButton* autoPlayToggle; // Toggle to enable automatic playback from the current curve point
	GLMotif::ToggleButton* snapVertexToViewToggle; // Toggle button to snap the picked vertex to the current view
	GLMotif::RadioBox* vertexContinuityBox; // Radio box to set vertex continuity
	GLMotif::ToggleButton* forceSegmentStraightToggle; // Toggle button to force a segment to a straight line
	GLMotif::TextField* segmentParameterIntervalText; // Text field displaying the width of a segment's parameter interval
	GLMotif::Slider* segmentParameterIntervalSlider; // Slider to select the width of a segment's parameter interval
	
	/* Editor state: */
	unsigned int numVertices; // Current number of vertices in curve
	Vertex* firstVertex; // Pointer to first vertex in curve
	Vertex* lastVertex; // Pointer to last vertex in curve
	Scalar parameterInterval; // Upper bound of curve's total parameter interval
	bool forceC2Continuity; // Flag whether the curve is constrained to be C^2-continuous
	C2BoundaryCondition c2BoundaryCondition; // Boundary condition for C^2-continuous curves
	Vertex* pickedVertex; // Pointer to currently picked vertex
	Segment* pickedHandleSegment; // Pointer to segment containing currently picked vertex tangent handle
	int pickedHandleIndex; // Index of currently picked vertex tangent handle in its segment
	Segment* pickedSegment; // Pointer to currently picked curve segment
	Scalar pickedSegmentParameter; // Relative parameter value of pick point on currently picked curve segment
	bool scrub; // Flag whether the view should follow the current curve point
	bool play; // Flag whether the curve point should move automatically with time
	double playStartTime; // Time offset for automatic curve movement
	FileSelectionHelper curveSelectionHelper; // Helper object to load/save curve files
	
	/* Editing operation state: */
	EditingMode editingMode; // Current editing mode
	bool snapVertexToView; // Flag wether the picked control point follows the current view
	Vector dragVertexOffset; // Offset from device to dragged vertex in navigational coordinates
	Vector dragHandleOffset; // Offset from device to dragged vertex tangent handle in navigational coordinates
	
	/* Private methods: */
	static void writeControlPoint(const ControlPoint& cp,Math::Matrix& b,unsigned int rowIndex);
	void calculateC2Spline(void); // Adjusts the curve to form a C^2-continuous spline
	void updateCurve(void); // Called to update derived curve state after the curve has been modified
	void moveToControlPoint(const ControlPoint& cp); // Sets the navigation transformation to move to the given vertex
	void pickSegment(Scalar parameterValue); // Picks the curve segment containing the given parameter value
	void setParameterValue(Scalar newParameterValue); // Sets a new curve parameter value and updates the user interface
	void forceC2ContinuityToggleValueChangedCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData);
	void c2BoundaryConditionBoxValueChangedCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData);
	void previousControlPointCallback(Misc::CallbackData* cbData);
	void parameterValueSliderValueChangedCallback(GLMotif::Slider::ValueChangedCallbackData* cbData);
	void nextControlPointCallback(Misc::CallbackData* cbData);
	void scrubToggleValueChangedCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData);
	void autoPlayToggleValueChangedCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData);
	void loadCurveCallback(GLMotif::FileSelectionDialog::OKCallbackData* cbData); // Loads selected curve file
	void saveCurveCallback(GLMotif::FileSelectionDialog::OKCallbackData* cbData); // Saves selected curve file
	void appendVertexCallback(Misc::CallbackData* cbData); // Appends current view as new vertex to curve
	void snapVertexToViewToggleValueChangedCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData);
	void deleteVertexCallback(Misc::CallbackData* cbData); // Deletes picked vertex from curve
	void vertexContinuityBoxValueChangedCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData);
	void forceSegmentStraightToggleValueChangedCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData);
	void splitSegmentCallback(Misc::CallbackData* cbData); // Splits picked segment at picked parameter value
	void segmentParameterIntervalSliderValueChangedCallback(GLMotif::Slider::ValueChangedCallbackData* cbData);
	void updateDialog(void); // Updates the curve editor dialog to reflect the newest settings
	void renderSegment(const Point& p0,const Point& p1,const Point& p2,const Point& p3,int level) const;
	
	/* Constructors and destructors: */
	public:
	CurveEditorTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	virtual ~CurveEditorTool(void);
	
	/* Methods from Tool: */
	virtual const ToolFactory* getFactory(void) const;
	virtual void buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData);
	virtual void frame(void);
	virtual void display(GLContextData& contextData) const;
	};

}

#endif
