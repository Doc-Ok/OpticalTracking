/***********************************************************************
VRMeshEditor - VR application to manipulate triangle meshes.
Copyright (c) 2003-2005 Oliver Kreylos
***********************************************************************/

#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/OrthogonalTransformation.h>
#include <GL/gl.h>
#include <GL/GLModels.h>
#include <Vrui/Vrui.h>

#include "VRMeshEditor.h"

/**********************************************
Methods of class VRMeshEditor::MorphBoxDragger:
**********************************************/

VRMeshEditor::MorphBoxDragger::MorphBoxDragger(Vrui::DraggingTool* sTool,VRMeshEditor* sApplication)
	:Dragger(sTool,sApplication),
	 creatingMorphBox(false),draggingMorphBox(false)
	{
	}

void VRMeshEditor::MorphBoxDragger::dragStartCallback(Vrui::DraggingTool::DragStartCallbackData* cbData)
	{
	if(application->morphBox!=0)
		{
		/* Pick the morph box: */
		MyMorphBox::Scalar pd=MyMorphBox::Scalar(Vrui::getInchFactor()*cbData->startTransformation.getScaling());
		if(application->morphBox->pickBox(pd*MyMorphBox::Scalar(0.75),pd*MyMorphBox::Scalar(0.5),pd*MyMorphBox::Scalar(0.333),cbData->startTransformation.getOrigin()))
			{
			draggingMorphBox=true;
			application->morphBox->startDragBox(cbData->startTransformation);
			}
		}
	else
		{
		/* Start creating a morph box: */
		creatingMorphBox=true;
		p1=p2=cbData->startTransformation.getOrigin();
		}
	}

void VRMeshEditor::MorphBoxDragger::dragCallback(Vrui::DraggingTool::DragCallbackData* cbData)
	{
	if(draggingMorphBox)
		application->morphBox->dragBox(cbData->currentTransformation);
	else if(creatingMorphBox)
		p2=cbData->currentTransformation.getOrigin();
	}

void VRMeshEditor::MorphBoxDragger::dragEndCallback(Vrui::DraggingTool::DragEndCallbackData* cbData)
	{
	if(draggingMorphBox)
		{
		application->morphBox->stopDragBox();
		draggingMorphBox=false;
		}
	else if(creatingMorphBox)
		{
		/* Create a morph box: */
		MyMorphBox::Point origin;
		MyMorphBox::Scalar size[3];
		for(int i=0;i<3;++i)
			{
			origin[i]=MyMorphBox::Scalar(p1[i]<=p2[i]?p1[i]:p2[i]);
			size[i]=Math::abs(MyMorphBox::Scalar(p2[i]-p1[i]));
			}
		application->morphBox=new MyMorphBox(application->mesh,origin,size);
		creatingMorphBox=false;
		}
	}

void VRMeshEditor::MorphBoxDragger::glRenderAction(GLContextData& contextData) const
	{
	if(creatingMorphBox)
		{
		/* Render the current morph box: */
		GLfloat min[3],max[3];
		for(int i=0;i<3;++i)
			{
			if(p1[i]<=p2[i])
				{
				min[i]=GLfloat(p1[i]);
				max[i]=GLfloat(p2[i]);
				}
			else
				{
				min[i]=GLfloat(p2[i]);
				max[i]=GLfloat(p1[i]);
				}
			}
		glDrawBox(min,max);
		}
	}
