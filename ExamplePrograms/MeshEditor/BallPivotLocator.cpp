/***********************************************************************
VRMeshEditor - VR application to manipulate triangle meshes.
Copyright (c) 2003-2005 Oliver Kreylos
***********************************************************************/

#include <iostream>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/OrthogonalTransformation.h>
#include <GL/gl.h>
#include <GL/GLGeometryWrappers.h>
#include <Vrui/GlyphRenderer.h>
#include <Vrui/Vrui.h>

#include "SphereRenderer.h"
#include "VRMeshEditor.h"

/***********************************************
Methods of class VRMeshEditor::BallPivotLocator:
***********************************************/

VRMeshEditor::BallPivotLocator::BallPivotLocator(Vrui::LocatorTool* sTool,VRMeshEditor* sApplication)
	:Locator(sTool,sApplication),
	 ballRadius(Vrui::getGlyphRenderer()->getGlyphSize()*5.0),
	 active(false),
	 bpState(0)
	{
	}

VRMeshEditor::BallPivotLocator::~BallPivotLocator(void)
	{
	if(bpState!=0)
		finishBallPivoting(bpState);
	}

void VRMeshEditor::BallPivotLocator::motionCallback(Vrui::LocatorTool::MotionCallbackData* cbData)
	{
	if(active)
		{
		/* Continue triangulating if in progress: */
		if(bpState!=0)
			{
			if(pivotOnce(bpState,250))
				{
				finishBallPivoting(bpState);
				bpState=0;
				}
			//Vrui::requestUpdate();
			}
		}
	else
		{
		/* Shoot a ball against the mesh: */
		Vrui::Point ballStart=cbData->currentTransformation.getOrigin();
		Vrui::Vector ballDirection=cbData->currentTransformation.getDirection(1);
		Vrui::Scalar scaledBallRadius=ballRadius*cbData->currentTransformation.getScaling();
		
		//std::cout<<"("<<ballStart[0]<<", "<<ballStart[1]<<", "<<ballStart[2]<<"), ";
		//std::cout<<"("<<ballDirection[0]<<", "<<ballDirection[1]<<", "<<ballDirection[2]<<"), ";
		//std::cout<<scaledBallRadius<<std::endl;
		
		sbr=shootBall(*application->mesh,ballStart,ballDirection,scaledBallRadius);
		// sbr=shootBall(*application->mesh,ballStart,ballDirection,1.44998);
		// sbr=shootBall(*application->mesh,ballStart,ballDirection,3.0);
		}
	}

void VRMeshEditor::BallPivotLocator::buttonPressCallback(Vrui::LocatorTool::ButtonPressCallbackData* cbData)
	{
	/* Triangulate based on the last shoot ball result: */
	if(sbr.numVertices==3)
		bpState=startBallPivoting(*application->mesh,sbr);
	
	active=true;
	}

void VRMeshEditor::BallPivotLocator::buttonReleaseCallback(Vrui::LocatorTool::ButtonReleaseCallbackData* cbData)
	{
	/* Stop the current triangulation process: */
	if(bpState!=0)
		{
		finishBallPivoting(bpState);
		bpState=0;
		}
	
	active=false;
	}

void VRMeshEditor::BallPivotLocator::glRenderAction(GLContextData& contextData) const
	{
	if(active)
		{
		if(bpState!=0)
			{
			/* Render the state of the triangulator: */
			GLboolean lightingEnabled=glIsEnabled(GL_LIGHTING);
			if(lightingEnabled)
				glDisable(GL_LIGHTING);
			renderState(bpState);
			if(lightingEnabled)
				glEnable(GL_LIGHTING);
			}
		}
	else
		{
		/* Render the pivoting ball: */
		glPushMatrix();
		glTranslate(sbr.ballCenter-Geometry::Point<double,3>::origin);
		glScale(sbr.ballRadius);
		application->sphereRenderer->glRenderAction(contextData);
		glPopMatrix();
		
		GLboolean lightingEnabled=glIsEnabled(GL_LIGHTING);
		if(lightingEnabled)
			glDisable(GL_LIGHTING);
		GLfloat pointSize;
		glGetFloatv(GL_POINT_SIZE,&pointSize);
		glPointSize(5.0f);
		glBegin(GL_POINTS);
		glColor3f(1.0f,0.0f,0.0f);
		for(int i=0;i<sbr.numVertices;++i)
			{
			glVertex(*sbr.vertices[i]);
			glColor3f(1.0f,1.0f,1.0f);
			}
		glEnd();
		glPointSize(pointSize);
		GLfloat lineWidth;
		glGetFloatv(GL_LINE_WIDTH,&lineWidth);
		glLineWidth(3.0f);
		glBegin(GL_LINE_LOOP);
		for(int i=0;i<sbr.numVertices;++i)
			glVertex(*sbr.vertices[i]);
		glEnd();
		glLineWidth(lineWidth);
		if(sbr.valid)
			{
			glBegin(GL_TRIANGLES);
			glColor3f(0.0f,1.0f,0.0f);
			glVertex(*sbr.vertices[0]);
			glVertex(*sbr.vertices[1]);
			glVertex(*sbr.vertices[2]);
			glEnd();
			}
		if(lightingEnabled)
			glEnable(GL_LIGHTING);
		}
	}
