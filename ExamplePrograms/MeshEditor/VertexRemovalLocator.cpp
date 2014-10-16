/***********************************************************************
VRMeshEditor - VR application to manipulate triangle meshes.
Copyright (c) 2003-2005 Oliver Kreylos
***********************************************************************/

#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/OrthogonalTransformation.h>
#include <GL/gl.h>
#include <GL/GLModels.h>
#include <GL/GLGeometryWrappers.h>
#include <Vrui/GlyphRenderer.h>
#include <Vrui/Vrui.h>

#include "SphereRenderer.h"
#include "VRMeshEditor.h"

/***************************************************
Methods of class VRMeshEditor::VertexRemovalLocator:
***************************************************/

VRMeshEditor::VertexRemovalLocator::VertexRemovalLocator(Vrui::LocatorTool* sTool,VRMeshEditor* sApplication)
	:Locator(sTool,sApplication),
	 influenceRadius(Vrui::getGlyphRenderer()->getGlyphSize()*5.0),
	 active(false)
	{
	}

VRMeshEditor::VertexRemovalLocator::~VertexRemovalLocator(void)
	{
	}

void VRMeshEditor::VertexRemovalLocator::motionCallback(Vrui::LocatorTool::MotionCallbackData* cbData)
	{
	/* Update the locator's position and radius in model coordinates: */
	influenceCenter=AutoTriangleMesh::Point(cbData->currentTransformation.getOrigin());
	scaledInfluenceRadius=influenceRadius*cbData->currentTransformation.getScaling();
	
	if(active)
		{
		/* Remove all vertices from the mesh that touch the influence sphere: */
		AutoTriangleMesh::VertexIterator vIt=application->mesh->beginVertices();
		while(vIt!=application->mesh->endVertices())
			{
			AutoTriangleMesh::VertexIterator nextVIt=vIt;
			++nextVIt;
			
			/* Check if the vertex is inside the influence sphere: */
			if(Geometry::sqrDist(influenceCenter,*vIt)<=Math::sqr(scaledInfluenceRadius))
				{
				/* Remove the vertex: */
				application->mesh->removeSingularVertex(vIt);
				}
			
			vIt=nextVIt;
			}
		}
	}

void VRMeshEditor::VertexRemovalLocator::buttonPressCallback(Vrui::LocatorTool::ButtonPressCallbackData* cbData)
	{
	active=true;
	}

void VRMeshEditor::VertexRemovalLocator::buttonReleaseCallback(Vrui::LocatorTool::ButtonReleaseCallbackData* cbData)
	{
	active=false;
	}

void VRMeshEditor::VertexRemovalLocator::glRenderAction(GLContextData& contextData) const
	{
	/* Render the influence sphere: */
	glPushMatrix();
	glTranslate(influenceCenter-AutoTriangleMesh::Point::origin);
	glScale(scaledInfluenceRadius);
	application->sphereRenderer->glRenderAction(contextData);
	glPopMatrix();
	}
