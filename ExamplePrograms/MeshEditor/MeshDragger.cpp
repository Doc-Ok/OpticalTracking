/***********************************************************************
VRMeshEditor - VR application to manipulate triangle meshes.
Copyright (c) 2003-2005 Oliver Kreylos
***********************************************************************/

#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/OrthogonalTransformation.h>
#include <Vrui/GlyphRenderer.h>
#include <Vrui/Vrui.h>

#include "VRMeshEditor.h"

/******************************************
Methods of class VRMeshEditor::MeshDragger:
******************************************/

VRMeshEditor::MeshDragger::MeshDragger(Vrui::DraggingTool* sTool,VRMeshEditor* sApplication)
	:Dragger(sTool,sApplication),
	 Influence(application->sphereRenderer,0.0),
	 influenceRadius(Vrui::getGlyphRenderer()->getGlyphSize()*5.0),
	 active(0)
	{
	/* Set influence's action type: */
	Influence::setAction(application->defaultActionType);
	}

void VRMeshEditor::MeshDragger::idleMotionCallback(Vrui::DraggingTool::IdleMotionCallbackData* cbData)
	{
	/* Update influence object's state: */
	Influence::setPositionOrientation(Influence::ONTransform(cbData->currentTransformation.getTranslation(),cbData->currentTransformation.getRotation()));
	
	/* Set the influence object's radius: */
	Influence::setRadius(influenceRadius*cbData->currentTransformation.getScaling());
	}

void VRMeshEditor::MeshDragger::dragStartCallback(Vrui::DraggingTool::DragStartCallbackData* cbData)
	{
	/* Start performing influence's action: */
	active=true;
	}

void VRMeshEditor::MeshDragger::dragCallback(Vrui::DraggingTool::DragCallbackData* cbData)
	{
	/* Update influence object's state: */
	Influence::setPositionOrientation(Influence::ONTransform(cbData->currentTransformation.getTranslation(),cbData->currentTransformation.getRotation()));
	
	/* Set the influence object's radius: */
	Influence::setRadius(influenceRadius*cbData->currentTransformation.getScaling());
	
	/* Perform influence's action: */
	if(active)
		Influence::actOnMesh(*application->mesh);
	}

void VRMeshEditor::MeshDragger::dragEndCallback(Vrui::DraggingTool::DragEndCallbackData* cbData)
	{
	/* Stop performing influence's action: */
	active=false;
	}

void VRMeshEditor::MeshDragger::glRenderAction(GLContextData& contextData) const
	{
	Influence::glRenderAction(contextData);
	}
