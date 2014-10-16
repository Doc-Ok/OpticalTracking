/***********************************************************************
VRMeshEditor - VR application to manipulate triangle meshes.
Copyright (c) 2003-2005 Oliver Kreylos
***********************************************************************/

#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/OrthogonalTransformation.h>
#include <GL/gl.h>
#include <GL/GLGeometryWrappers.h>
#include <Vrui/GlyphRenderer.h>
#include <Vrui/Vrui.h>

#include "SphereRenderer.h"
#include "VRMeshEditor.h"

/*************************************************
Methods of class VRMeshEditor::FaceRemovalLocator:
*************************************************/

VRMeshEditor::FaceRemovalLocator::FaceRemovalLocator(Vrui::LocatorTool* sTool,VRMeshEditor* sApplication)
	:Locator(sTool,sApplication),
	 influenceRadius(Vrui::getGlyphRenderer()->getGlyphSize()*5.0),
	 active(false)
	{
	}

VRMeshEditor::FaceRemovalLocator::~FaceRemovalLocator(void)
	{
	}

void VRMeshEditor::FaceRemovalLocator::motionCallback(Vrui::LocatorTool::MotionCallbackData* cbData)
	{
	/* Update the locator's position and radius in model coordinates: */
	influenceCenter=AutoTriangleMesh::Point(cbData->currentTransformation.getOrigin());
	scaledInfluenceRadius=influenceRadius*cbData->currentTransformation.getScaling();
	
	if(active)
		{
		/* Remove all faces from the mesh that touch the influence sphere: */
		AutoTriangleMesh::FaceIterator fIt=application->mesh->beginFaces();
		while(fIt!=application->mesh->endFaces())
			{
			AutoTriangleMesh::FaceIterator nextFIt=fIt;
			++nextFIt;
			
			/* Check if the triangle touches the influence sphere: */
			bool inside=false;
			for(AutoTriangleMesh::FaceEdgeIterator feIt=fIt.beginEdges();feIt!=fIt.endEdges();++feIt)
				if(Geometry::sqrDist(influenceCenter,*feIt->getStart())<=Math::sqr(scaledInfluenceRadius))
					inside=true;
			
			/* Remove the face: */
			if(inside)
				application->mesh->removeFace(fIt);
			
			fIt=nextFIt;
			}
		}
	}

void VRMeshEditor::FaceRemovalLocator::buttonPressCallback(Vrui::LocatorTool::ButtonPressCallbackData* cbData)
	{
	active=true;
	}

void VRMeshEditor::FaceRemovalLocator::buttonReleaseCallback(Vrui::LocatorTool::ButtonReleaseCallbackData* cbData)
	{
	active=false;
	}

void VRMeshEditor::FaceRemovalLocator::glRenderAction(GLContextData& contextData) const
	{
	/* Render the influence sphere: */
	glPushMatrix();
	glTranslate(influenceCenter-AutoTriangleMesh::Point::origin);
	glScale(scaledInfluenceRadius);
	application->sphereRenderer->glRenderAction(contextData);
	glPopMatrix();
	}
