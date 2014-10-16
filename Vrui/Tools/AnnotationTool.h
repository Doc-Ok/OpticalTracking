/***********************************************************************
AnnotationTool - Tool to interactively annotate 3D models.
Copyright (c) 2011-2013 Oliver Kreylos

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

#ifndef VRUI_ANNOTATIONTOOL_INCLUDED
#define VRUI_ANNOTATIONTOOL_INCLUDED

#include <vector>
#include <Geometry/Point.h>
#include <Geometry/OrthogonalTransformation.h>
#include <GL/GLLabel.h>
#include <GLMotif/RadioBox.h>
#include <Vrui/Vrui.h>
#include <Vrui/CoordinateManager.h>
#include <Vrui/UtilityTool.h>

/* Forward declarations: */
class GLFont;
namespace GLMotif {
class PopupWindow;
}

namespace Vrui {

class AnnotationTool;

class AnnotationToolFactory:public ToolFactory
	{
	friend class AnnotationTool;
	
	/* Elements: */
	public:
	Scalar markerSize; // Size of measurement markers in physical units
	GLFont* labelFont; // Font to draw labels etc.
	
	/* Constructors and destructors: */
	public:
	AnnotationToolFactory(ToolManager& toolManager);
	virtual ~AnnotationToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual const char* getButtonFunction(int buttonSlotIndex) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class AnnotationTool:public UtilityTool
	{
	friend class AnnotationToolFactory;
	
	/* Embedded classes: */
	private:
	class Annotation // Base class for annotation objects
		{
		/* Embedded classes: */
		public:
		class CreationState // Base class for creation state objects used during object creation
			{
			/* Constructors and destructors: */
			public:
			virtual ~CreationState(void);
			
			/* Methods: */
			virtual bool isFinished(void) const =0; // Returns true when object creation is complete
			};
		
		class PickResult // Base class containing the result of a successful picking operation on an object
			{
			/* Elements: */
			private:
			Scalar pickDistance2; // Squared distance of picking position from object itself
			
			/* Constructors and destructors: */
			public:
			PickResult(Scalar sPickDistance2); // Elementwise constructor
			virtual ~PickResult(void);
			
			/* Methods: */
			Scalar getPickDistance2(void) const // Returns squared distance of picking position from object
				{
				return pickDistance2;
				}  
			};
		
		typedef OGTransform DragTransform; // Type for dragging transformations
		
		class DragState // Base class to store an object's dragging state during a drag operation
			{
			/* Constructors and destructors: */
			public:
			virtual ~DragState(void);
			};
		
		/* Elements: */
		private:
		bool selected; // Flag if the object is currently selected
		
		/* Protected helper methods: */
		protected:
		static void drawMarker(const Point& pos); // Draws a marker at the given physical-space position
		
		/* Constructors and destructors: */
		public:
		Annotation(void);
		virtual ~Annotation(void);
		
		/* Methods: */
		bool isSelected(void) const // Returns true if the object is currently selected
			{
			return selected;
			}
		virtual void select(bool newSelected); // Selects or deselects the object
		virtual PickResult* pick(const Point& pickPos) const; // Picks the object using a 3D position and returns a pick result
		virtual DragState* startDrag(const DragTransform& drag,CreationState* creationState,PickResult* pickResult); // Starts dragging during creation or after picking
		virtual void drag(const DragTransform& drag,CreationState* creationState,PickResult* pickResult,DragState* dragState); // Drags during creation or after picking
		virtual void endDrag(const DragTransform& drag,CreationState* creationState,PickResult* pickResult,DragState* dragState); // Ends dragging during creation of after picking
		virtual void glRenderAction(GLContextData& contextData) const; // Renders the object
		virtual void glRenderAction(CreationState* creationState,PickResult* pickResult,DragState* dragState,GLContextData& contextData) const; // Ditto, with at least one state object supplied
		};
	
	class Position:public Annotation // Class to annotate a 3D position
		{
		/* Embedded classes: */
		public:
		class CreationState:public Annotation::CreationState
			{
			friend class Position;
			
			/* Elements: */
			private:
			unsigned int numPoints; // Number of already created points
			
			/* Constructors and destructors: */
			public:
			CreationState(void);
			
			/* Methods from Annotation::CreationState: */
			virtual bool isFinished(void) const;
			};
		
		class DragState:public Annotation::DragState
			{
			friend class Position;
			
			/* Elements: */
			private:
			Point localPos; // Position of dragged point in drag transformation's coordinate space
			
			/* Constructors and destructors: */
			public:
			DragState(const Point& sLocalPos);
			};
		
		/* Elements: */
		private:
		Color bgColor,fgColor; // The background and foreground colors
		Point pos; // The annotated 3D position
		GLLabel posLabels[3]; // Array of 3D text labels to draw the position's coordinates
		
		/* Private methods: */
		void updateLabels(void); // Updates the position labels
		void draw(GLContextData& contextData) const; // Draws the object
		
		/* Constructors and destructors: */
		public:
		Position(const GLFont& labelFont); // Initializes the position object with the given label font
		
		/* Methods from Annotation: */
		virtual Annotation::PickResult* pick(const Point& pickPos) const;
		virtual Annotation::DragState* startDrag(const DragTransform& drag,Annotation::CreationState* creationState,Annotation::PickResult* pickResult);
		virtual void drag(const DragTransform& drag,Annotation::CreationState* creationState,Annotation::PickResult* pickResult,Annotation::DragState* dragState);
		virtual void glRenderAction(GLContextData& contextData) const;
		virtual void glRenderAction(Annotation::CreationState* creationState,Annotation::PickResult* pickResult,Annotation::DragState* dragState,GLContextData& contextData) const;
		};
	
	class Distance:public Annotation // Class to annotate a 3D distance
		{
		/* Embedded classes: */
		public:
		class CreationState:public Annotation::CreationState
			{
			friend class Distance;
			
			/* Elements: */
			private:
			unsigned int numPoints; // Number of already created points
			
			/* Constructors and destructors: */
			public:
			CreationState(void);
			
			/* Methods from Annotation::CreationState: */
			virtual bool isFinished(void) const;
			};
		
		class PickResult:public Annotation::PickResult
			{
			friend class Distance;
			
			/* Elements: */
			private:
			unsigned int pointIndex; // Index of picked point
			
			/* Constructors and destructors: */
			public:
			PickResult(unsigned int sPointIndex,Scalar sPickDistance2);
			};
		
		class DragState:public Annotation::DragState
			{
			friend class Distance;
			
			/* Elements: */
			private:
			unsigned int pointIndex; // Index of the dragged point
			Point localPos; // Position of dragged point in drag transformation's coordinate space
			
			/* Constructors and destructors: */
			public:
			DragState(unsigned int sPointIndex,const Point& sLocalPos);
			};
		
		/* Elements: */
		private:
		Color bgColor,fgColor; // The background and foreground colors
		Point pos[2]; // The end points of the annotated distance
		GLLabel distLabel; // 3D text label to draw the distance's length
		
		/* Private methods: */
		void updateLabel(void); // Updates the distance label
		void draw(GLContextData& contextData) const; // Draws the object
		
		/* Constructors and destructors: */
		public:
		Distance(const GLFont& labelFont); // Initializes the distance object with the given label font
		
		/* Methods from Annotation: */
		virtual Annotation::PickResult* pick(const Point& pickPos) const;
		virtual Annotation::DragState* startDrag(const DragTransform& drag,Annotation::CreationState* creationState,Annotation::PickResult* pickResult);
		virtual void drag(const DragTransform& drag,Annotation::CreationState* creationState,Annotation::PickResult* pickResult,Annotation::DragState* dragState);
		virtual void glRenderAction(GLContextData& contextData) const;
		virtual void glRenderAction(Annotation::CreationState* creationState,Annotation::PickResult* pickResult,Annotation::DragState* dragState,GLContextData& contextData) const;
		};
	
	class Angle:public Annotation // Class to annotate a 3D angle
		{
		/* Embedded classes: */
		public:
		class CreationState:public Annotation::CreationState
			{
			friend class Angle;
			
			/* Elements: */
			private:
			unsigned int numPoints; // Number of already created points
			
			/* Constructors and destructors: */
			public:
			CreationState(void);
			
			/* Methods from Annotation::CreationState: */
			virtual bool isFinished(void) const;
			};
		
		class PickResult:public Annotation::PickResult
			{
			friend class Angle;
			
			/* Elements: */
			private:
			unsigned int pointIndex; // Index of picked point
			
			/* Constructors and destructors: */
			public:
			PickResult(unsigned int sPointIndex,Scalar sPickDistance2);
			};
		
		class DragState:public Annotation::DragState
			{
			friend class Angle;
			
			/* Elements: */
			private:
			unsigned int pointIndex; // Index of the dragged point
			Point localPos; // Position of dragged point in drag transformation's coordinate space
			
			/* Constructors and destructors: */
			public:
			DragState(unsigned int sPointIndex,const Point& sLocalPos);
			};
		
		/* Elements: */
		private:
		Color bgColor,fgColor; // The background and foreground colors
		Point pos[3]; // The end points of the annotated angle
		Scalar angle; // The subtended angle in radians
		GLLabel angleLabel; // 3D text label to draw the angle's angle
		
		/* Private methods: */
		void updateLabel(void); // Updates the angle label
		void draw(GLContextData& contextData) const; // Draws the object
		
		/* Constructors and destructors: */
		public:
		Angle(const GLFont& labelFont); // Initializes the angle object with the given label font
		
		/* Methods from Annotation: */
		virtual Annotation::PickResult* pick(const Point& pickPos) const;
		virtual Annotation::DragState* startDrag(const DragTransform& drag,Annotation::CreationState* creationState,Annotation::PickResult* pickResult);
		virtual void drag(const DragTransform& drag,Annotation::CreationState* creationState,Annotation::PickResult* pickResult,Annotation::DragState* dragState);
		virtual void glRenderAction(GLContextData& contextData) const;
		virtual void glRenderAction(Annotation::CreationState* creationState,Annotation::PickResult* pickResult,Annotation::DragState* dragState,GLContextData& contextData) const;
		};
	
	/* Elements: */
	static AnnotationToolFactory* factory; // Pointer to the factory object for this class
	GLMotif::PopupWindow* annotationDialogPopup; // Pointer to the annotation dialog window
	GLMotif::RadioBox* creationButtons; // Container for annotation object creation buttons
	std::vector<Annotation*> annotations; // List of created annotation objects
	CoordinateTransform* userTransform; // The currently valid user-space coordinate transformation
	Annotation* newObject; // Currently created annotation object
	Annotation::CreationState* newCreationState; // Creation state of currently created annotation object
	Annotation* pickedObject; // Object picked at beginning of current dragging operation
	Annotation::PickResult* pickResult; // Pick result from beginning of current dragging operation
	InputDevice* dragDevice; // Input device tracked during the current dragging operation
	Annotation::DragState* dragState; // Dragging state of current dragging operation
	
	/* Private methods: */
	void creationButtonsCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData);
	void coordTransformChangedCallback(CoordinateManager::CoordinateTransformChangedCallbackData* cbData);
	
	/* Constructors and destructors: */
	public:
	AnnotationTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	virtual ~AnnotationTool(void);
	
	/* Methods from Tool: */
	virtual void configure(const Misc::ConfigurationFileSection& configFileSection);
	virtual void storeState(Misc::ConfigurationFileSection& configFileSection) const;
	virtual const ToolFactory* getFactory(void) const;
	virtual void buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData);
	virtual void frame(void);
	virtual void display(GLContextData& contextData) const;
	};

}

#endif
