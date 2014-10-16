/***********************************************************************
VRMeshEditor - VR application to manipulate triangle meshes.
Copyright (c) 2003-2005 Oliver Kreylos
***********************************************************************/

#ifndef VRMESHEDITOR_INCLUDED
#define VRMESHEDITOR_INCLUDED

#include <vector>
#include <GL/gl.h>
#include <GL/GLMaterial.h>
#include <GL/GLObject.h>
#include <GLMotif/Popup.h>
#include <GLMotif/PopupMenu.h>
#include <GLMotif/RadioBox.h>
#include <GLMotif/ToggleButton.h>
#include <Vrui/LocatorTool.h>
#include <Vrui/LocatorToolAdapter.h>
#include <Vrui/DraggingTool.h>
#include <Vrui/DraggingToolAdapter.h>
#include <Vrui/ToolManager.h>
#include <Vrui/Application.h>

#include "PolygonMesh.h"
#include "AutoTriangleMesh.h"
#include "BallPivoting.h"

#include "Influence.h"
#include "MorphBox.h"

/* Forward declarations: */
class SphereRenderer;

struct VRMeshEditor:public Vrui::Application,public GLObject
	{
	/* Embedded classes: */
	public:
	typedef AutoTriangleMesh MyMesh;
	typedef MyMesh::VertexIterator MyVIt;
	typedef MorphBox MyMorphBox;
	
	enum LocatorType // Enumerated type for locator types
		{
		BALLPIVOTLOCATOR,VERTEXREMOVALLOCATOR,FACEREMOVALLOCATOR
		};
	
	enum DraggerType // Enumerated type for dragger types
		{
		MESHDRAGGER,MORPHBOXDRAGGER,BALLPIVOTDRAGGER
		};
	
	enum RenderMode // Enumerated type for rendering modes
		{
		SHADED,WIREFRAME
		};
	
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		public:
		GLuint vertexBufferId; // ID of vertex buffer object for point data
		
		/* Constructors and destructors: */
		DataItem(void);
		virtual ~DataItem(void);
		};
	
	class Locator:public Vrui::LocatorToolAdapter // Base class for application locators
		{
		/* Elements: */
		protected:
		VRMeshEditor* application; // Pointer to the application object
		
		/* Constructors and destructors: */
		Locator(Vrui::LocatorTool* sTool,VRMeshEditor* sApplication)
			:LocatorToolAdapter(sTool),
			 application(sApplication)
			{
			};
		public:
		virtual ~Locator(void)
			{
			};
		
		/* Methods: */
		virtual void glRenderAction(GLContextData& contextData) const
			{
			};
		};
	
	typedef std::vector<Locator*> LocatorList;
	
	class BallPivotLocator:public Locator // Class to triangulate point sets using ball pivoting
		{
		/* Elements: */
		private:
		double ballRadius; // Radius of pivoting ball in physical coordinates
		bool active;
		ShootBallResult sbr; // Result of shooting a ball from the dragger's position
		BPState* bpState; // Ball pivoting state object
		
		/* Constructors and destructors: */
		public:
		BallPivotLocator(Vrui::LocatorTool* sTool,VRMeshEditor* sApplication);
		virtual ~BallPivotLocator(void);
		
		/* Methods: */
		virtual void motionCallback(Vrui::LocatorTool::MotionCallbackData* cbData);
		virtual void buttonPressCallback(Vrui::LocatorTool::ButtonPressCallbackData* cbData);
		virtual void buttonReleaseCallback(Vrui::LocatorTool::ButtonReleaseCallbackData* cbData);
		virtual void glRenderAction(GLContextData& contextData) const;
		};
	
	class VertexRemovalLocator:public Locator // Class to remove vertices from a mesh
		{
		/* Elements: */
		private:
		double influenceRadius; // Radius of removal influence sphere in physical coordinates
		AutoTriangleMesh::Point influenceCenter;
		double scaledInfluenceRadius;
		bool active;
		
		/* Constructors and destructors: */
		public:
		VertexRemovalLocator(Vrui::LocatorTool* sTool,VRMeshEditor* sApplication);
		virtual ~VertexRemovalLocator(void);
		
		/* Methods: */
		virtual void motionCallback(Vrui::LocatorTool::MotionCallbackData* cbData);
		virtual void buttonPressCallback(Vrui::LocatorTool::ButtonPressCallbackData* cbData);
		virtual void buttonReleaseCallback(Vrui::LocatorTool::ButtonReleaseCallbackData* cbData);
		virtual void glRenderAction(GLContextData& contextData) const;
		};
	
	class FaceRemovalLocator:public Locator // Class to remove triangles from a mesh
		{
		/* Elements: */
		private:
		double influenceRadius; // Radius of removal influence sphere in physical coordinates
		AutoTriangleMesh::Point influenceCenter;
		double scaledInfluenceRadius;
		bool active;
		
		/* Constructors and destructors: */
		public:
		FaceRemovalLocator(Vrui::LocatorTool* sTool,VRMeshEditor* sApplication);
		virtual ~FaceRemovalLocator(void);
		
		/* Methods: */
		virtual void motionCallback(Vrui::LocatorTool::MotionCallbackData* cbData);
		virtual void buttonPressCallback(Vrui::LocatorTool::ButtonPressCallbackData* cbData);
		virtual void buttonReleaseCallback(Vrui::LocatorTool::ButtonReleaseCallbackData* cbData);
		virtual void glRenderAction(GLContextData& contextData) const;
		};
	
	class Dragger:public Vrui::DraggingToolAdapter // Base class for application draggers
		{
		/* Elements: */
		protected:
		VRMeshEditor* application; // Pointer to the application object
		
		/* Constructors and destructors: */
		Dragger(Vrui::DraggingTool* sTool,VRMeshEditor* sApplication)
			:DraggingToolAdapter(sTool),
			 application(sApplication)
			{
			};
		public:
		virtual ~Dragger(void)
			{
			};
		
		/* Methods: */
		virtual void glRenderAction(GLContextData& contextData) const
			{
			};
		};
	
	typedef std::vector<Dragger*> DraggerList;
	
	class MeshDragger:public Dragger,public Influence // Class to drag meshes with a dragging tool
		{
		/* Elements: */
		private:
		double influenceRadius; // Radius of influence sphere in physical coordinates
		bool active; // Flag whether the influence is active
		
		/* Constructors and destructors: */
		public:
		MeshDragger(Vrui::DraggingTool* sTool,VRMeshEditor* sApplication);
		
		/* Methods: */
		virtual void idleMotionCallback(Vrui::DraggingTool::IdleMotionCallbackData* cbData);
		virtual void dragStartCallback(Vrui::DraggingTool::DragStartCallbackData* cbData);
		virtual void dragCallback(Vrui::DraggingTool::DragCallbackData* cbData);
		virtual void dragEndCallback(Vrui::DraggingTool::DragEndCallbackData* cbData);
		virtual void glRenderAction(GLContextData& contextData) const;
		};
	
	class MorphBoxDragger:public Dragger // Class to drag morph boxes (and morph meshes)
		{
		/* Elements: */
		private:
		bool creatingMorphBox; // Flag if the dragger is currently creating a morph box
		Vrui::Point p1,p2; // Diagonally opposite box corners during box creation
		bool draggingMorphBox; // Flag if the dragger is currently dragging a morph box
		
		/* Constructors and destructors: */
		public:
		MorphBoxDragger(Vrui::DraggingTool* sTool,VRMeshEditor* sApplication);
		
		/* Methods: */
		virtual void dragStartCallback(Vrui::DraggingTool::DragStartCallbackData* cbData);
		virtual void dragCallback(Vrui::DraggingTool::DragCallbackData* cbData);
		virtual void dragEndCallback(Vrui::DraggingTool::DragEndCallbackData* cbData);
		virtual void glRenderAction(GLContextData& contextData) const;
		};
	
	friend class BallPivotLocator;
	friend class VertexRemovalLocator;
	friend class FaceRemovalLocator;
	friend class MeshDragger;
	friend class MorphBoxDragger;
	
	/* Elements: */
	
	/* Mesh state: */
	MyMesh* mesh; // The mesh being edited
	MyMorphBox* morphBox; // Pointer to currently active morph box
	
	/* Interaction state: */
	LocatorType defaultLocatorType; // Type of locators to be created
	DraggerType defaultDraggerType; // Type of draggers to be created
	Influence::ActionType defaultActionType; // Default influence action type for new mesh draggers
	bool overrideTools; // Flag to override action types of existing tools when default action is changed
	LocatorList locators; // List of currently instantiated locators
	DraggerList draggers; // List of currently instantiated draggers
	
	/* Rendering state: */
	SphereRenderer* sphereRenderer;
	RenderMode renderMode; // Current rendering mode for surface
	bool showVertexColors; // Flag whether to use colors supplied with vertices
	GLMaterial meshMaterial; // Material for rendering the mesh when showing true colors
	GLMaterial frontMaterial,backMaterial; // Material for rendering the mesh when showing surface orientation
	bool renderMeshVertices; // Flag whether mesh boundary vertices should be rendered
	bool renderMeshVerticesTransparent; // Flag whether to render them using transparency
	
	/* Vrui state: */
	GLMotif::PopupMenu* mainMenu; // The main menu widget
	
	/* Private methods: */
	GLMotif::Popup* createLocatorTypesMenu(void); // Creates program's locator type menu
	GLMotif::Popup* createDraggerTypesMenu(void); // Creates program's dragger type menu
	GLMotif::Popup* createInfluenceActionsMenu(void); // Creates program's influence actions menu
	GLMotif::Popup* createSettingsMenu(void); // Creates program's settings menu
	GLMotif::PopupMenu* createMainMenu(void); // Creates program's main menu
	void renderMesh(DataItem* dataItem) const; // Renders the mesh
	void renderMeshWireframe(DataItem* dataItem) const; // Renders the mesh in wireframe mode
	void renderMeshBoundaryVertices(DataItem* dataItem) const; // Renders the mesh's boundary vertices
	void setMeshDraggerActionType(Influence::ActionType newActionType); // Sets the editing modes of all mesh dragging tools
	AutoTriangleMesh* loadEarthquakeEvents(const char* filename); // Loads points from an earthquake event file
	AutoTriangleMesh* loadSphericalPoints(const char* filename); // Loads points from a spherical point file
	AutoTriangleMesh* loadVolumePoints(const char* filename); // Loads points from a point volume file
	
	/* Constructors and destructors: */
	VRMeshEditor(int& argc,char**& argv,char**& appDefaults);
	virtual ~VRMeshEditor(void);
	
	/* Methods: */
	virtual void initContext(GLContextData& contextData) const;
	virtual void toolCreationCallback(Vrui::ToolManager::ToolCreationCallbackData* cbData);
	virtual void toolDestructionCallback(Vrui::ToolManager::ToolDestructionCallbackData* cbData);
	virtual void frame(void);
	virtual void display(GLContextData& contextData) const;
	void locatorTypeValueChangedCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData);
	void draggerTypeValueChangedCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData);
	void influenceActionValueChangedCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData);
	void renderModeValueChangedCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData);
	void toggleButtonValueChangedCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData);
	void centerDisplayCallback(Misc::CallbackData* cbData);
	void createMorphBoxCallback(Misc::CallbackData* cbData);
	void deleteMorphBoxCallback(Misc::CallbackData* cbData);
	void saveMeshCallback(Misc::CallbackData* cbData);
	void createInputDeviceCallback(Misc::CallbackData* cbData);
	};

#endif
