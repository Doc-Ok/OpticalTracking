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

#include <Vrui/Tools/AnnotationTool.h>

#include <stdio.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Math/Math.h>
#include <Geometry/Rotation.h>
#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLVertexTemplates.h>
#include <GL/GLGeometryWrappers.h>
#include <GL/GLTransformationWrappers.h>
#include <GLMotif/PopupWindow.h>
#include <GLMotif/RowColumn.h>
#include <GLMotif/Button.h>
#include <GLMotif/WidgetStateHelper.h>
#include <Vrui/Viewer.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/**************************************
Methods of class AnnotationToolFactory:
**************************************/

AnnotationToolFactory::AnnotationToolFactory(ToolManager& toolManager)
	:ToolFactory("AnnotationTool",toolManager),
	 markerSize(getUiSize()),
	 labelFont(getUiFont())
	{
	/* Initialize tool layout: */
	layout.setNumButtons(1);
	
	/* Insert class into class hierarchy: */
	ToolFactory* toolFactory=toolManager.loadClass("UtilityTool");
	toolFactory->addChildClass(this);
	addParentClass(toolFactory);
	
	/* Load class settings: */
	Misc::ConfigurationFileSection cfs=toolManager.getToolClassSection(getClassName());
	markerSize=cfs.retrieveValue<Scalar>("./markerSize",markerSize);
	std::string labelFontName=cfs.retrieveString("./labelFont","");
	if(!labelFontName.empty())
		{
		/* Load a different label font: */
		labelFont=loadFont(labelFontName.c_str());
		}
	
	/* Set tool class' factory pointer: */
	AnnotationTool::factory=this;
	}

AnnotationToolFactory::~AnnotationToolFactory(void)
	{
	/* Delete a non-standard label font: */
	if(labelFont!=getUiFont())
		delete labelFont;
	
	/* Reset tool class' factory pointer: */
	AnnotationTool::factory=0;
	}

const char* AnnotationToolFactory::getName(void) const
	{
	return "Annotation Tool";
	}

const char* AnnotationToolFactory::getButtonFunction(int) const
	{
	return "Annotate";
	}

Tool* AnnotationToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new AnnotationTool(this,inputAssignment);
	}

void AnnotationToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveAnnotationToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("UtilityTool");
	}

extern "C" ToolFactory* createAnnotationToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	AnnotationToolFactory* annotationToolFactory=new AnnotationToolFactory(*toolManager);
	
	/* Return factory object: */
	return annotationToolFactory;
	}

extern "C" void destroyAnnotationToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/**********************************************************
Methods of class AnnotationTool::Annotation::CreationState:
**********************************************************/

AnnotationTool::Annotation::CreationState::~CreationState(void)
	{
	}

/*******************************************************
Methods of class AnnotationTool::Annotation::PickResult:
*******************************************************/

AnnotationTool::Annotation::PickResult::PickResult(Scalar sPickDistance2)
	:pickDistance2(sPickDistance2)
	{
	}

AnnotationTool::Annotation::PickResult::~PickResult(void)
	{
	}

/******************************************************
Methods of class AnnotationTool::Annotation::DragState:
******************************************************/

AnnotationTool::Annotation::DragState::~DragState(void)
	{
	}

/*******************************************
Methods of class AnnotationTool::Annotation:
*******************************************/

void AnnotationTool::Annotation::drawMarker(const Point& pos)
	{
	/* Draw the marker: */
	Scalar ms=factory->markerSize;
	glBegin(GL_LINES);
	glVertex(pos[0]-ms,pos[1],pos[2]);
	glVertex(pos[0]+ms,pos[1],pos[2]);
	glVertex(pos[0],pos[1]-ms,pos[2]);
	glVertex(pos[0],pos[1]+ms,pos[2]);
	glVertex(pos[0],pos[1],pos[2]-ms);
	glVertex(pos[0],pos[1],pos[2]+ms);
	glEnd();
	}

AnnotationTool::Annotation::Annotation(void)
	:selected(false)
	{
	}

AnnotationTool::Annotation::~Annotation(void)
	{
	}

void AnnotationTool::Annotation::select(bool newSelected)
	{
	selected=newSelected;
	}

AnnotationTool::Annotation::PickResult* AnnotationTool::Annotation::pick(const Point& pickPos) const
	{
	/* Signal unsuccessful pick: */
	return 0;
	}

AnnotationTool::Annotation::DragState* AnnotationTool::Annotation::startDrag(const AnnotationTool::Annotation::DragTransform& drag,AnnotationTool::Annotation::CreationState* creationState,AnnotationTool::Annotation::PickResult* pickResult)
	{
	/* No drag state required: */
	return 0;
	}

void AnnotationTool::Annotation::drag(const AnnotationTool::Annotation::DragTransform& drag,AnnotationTool::Annotation::CreationState* creationState,AnnotationTool::Annotation::PickResult* pickResult,AnnotationTool::Annotation::DragState* dragState)
	{
	/* Do nothing */
	}

void AnnotationTool::Annotation::endDrag(const AnnotationTool::Annotation::DragTransform& drag,AnnotationTool::Annotation::CreationState* creationState,AnnotationTool::Annotation::PickResult* pickResult,AnnotationTool::Annotation::DragState* dragState)
	{
	/* Do nothing */
	}

void AnnotationTool::Annotation::glRenderAction(GLContextData& contextData) const
	{
	/* Do nothing */
	}

void AnnotationTool::Annotation::glRenderAction(AnnotationTool::Annotation::CreationState* creationState,AnnotationTool::Annotation::PickResult* pickResult,AnnotationTool::Annotation::DragState* dragState,GLContextData& contextData) const
	{
	/* Do nothing */
	}

/********************************************************
Methods of class AnnotationTool::Position::CreationState:
********************************************************/

AnnotationTool::Position::CreationState::CreationState(void)
	:numPoints(0)
	{
	}

bool AnnotationTool::Position::CreationState::isFinished(void) const
	{
	return numPoints>=1;
	}

/****************************************************
Methods of class AnnotationTool::Position::DragState:
****************************************************/

AnnotationTool::Position::DragState::DragState(const Point& sLocalPos)
	:localPos(sLocalPos)
	{
	}

/*****************************************
Methods of class AnnotationTool::Position:
*****************************************/

void AnnotationTool::Position::updateLabels(void)
	{
	GLfloat y=GLfloat(factory->markerSize);
	for(int i=2;i>=0;--i)
		{
		char posLabelText[40];
		snprintf(posLabelText,sizeof(posLabelText),"%g",double(pos[i]));
		posLabels[i].setString(posLabelText);
		GLLabel::Box::Vector labelSize=posLabels[i].getLabelSize();
		posLabels[i].setOrigin(GLLabel::Box::Vector(-labelSize[0]*0.5f,y,0.0f));
		y+=labelSize[1];
		}
	}

void AnnotationTool::Position::draw(GLContextData& contextData) const
	{
	/* Draw the position: */
	glDisable(GL_LIGHTING);
	
	/* Draw the position marker: */
	Point physPos=getNavigationTransformation().transform(pos);
	glLineWidth(3.0f);
	glColor(bgColor);
	drawMarker(physPos);
	glLineWidth(1.0f);
	glColor(fgColor);
	drawMarker(physPos);
	
	/* Draw the position labels: */
	glPushMatrix();
	glTranslate(physPos-Point::origin);
	Vector z=getMainViewer()->getHeadPosition()-physPos;
	Vector x=getUpDirection()^z;
	Vector y=z^x;
	glRotate(Rotation::fromBaseVectors(x,y));
	
	for(int i=0;i<3;++i)
		posLabels[i].draw(contextData);
	
	glPopMatrix();
	}

AnnotationTool::Position::Position(const GLFont& labelFont)
	{
	/* Determine default background and foreground colors: */
	bgColor=getBackgroundColor();
	for(int i=0;i<3;++i)
		fgColor[i]=1.0f-bgColor[i];
	fgColor[3]=1.0f;
	
	/* Initialize the position labels: */
	for(int i=0;i<3;++i)
		{
		posLabels[i].setFont(labelFont);
		Color lbgColor=bgColor;
		lbgColor[3]=0.0f;
		posLabels[i].setBackground(lbgColor);
		posLabels[i].setForeground(fgColor);
		}
	}

AnnotationTool::Annotation::PickResult* AnnotationTool::Position::pick(const Point& pickPos) const
	{
	/* Compare the given position against the annotated position: */
	Scalar dist2=Geometry::sqrDist(pickPos,pos);
	if(dist2<Math::sqr(getPointPickDistance()))
		return new Annotation::PickResult(dist2);
	else
		return 0;
	}

AnnotationTool::Annotation::DragState* AnnotationTool::Position::startDrag(const AnnotationTool::Annotation::DragTransform& drag,AnnotationTool::Annotation::CreationState* creationState,AnnotationTool::Annotation::PickResult* pickResult)
	{
	/* Check if we're in creation-dragging or pick-dragging mode: */
	Point localPos;
	if(creationState!=0)
		{
		/* Get the creation state object: */
		CreationState* myCs=static_cast<CreationState*>(creationState);
		
		/* Increment the number of points: */
		++myCs->numPoints;
		
		/* Initialize the new point's local position: */
		localPos=Point::origin;
		}
	else
		{
		/* Calculate the dragged point's local position: */
		localPos=drag.inverseTransform(pos);
		}
	
	/* Return a drag state: */
	return new DragState(localPos);
	}

void AnnotationTool::Position::drag(const AnnotationTool::Annotation::DragTransform& drag,AnnotationTool::Annotation::CreationState* creationState,AnnotationTool::Annotation::PickResult* pickResult,AnnotationTool::Annotation::DragState* dragState)
	{
	/* Get the drag state object: */
	DragState* myDs=static_cast<DragState*>(dragState);
	
	/* Update the position of the dragged point: */
	pos=drag.transform(myDs->localPos);
	updateLabels();
	}

void AnnotationTool::Position::glRenderAction(GLContextData& contextData) const
	{
	/* Draw the position: */
	draw(contextData);
	}

void AnnotationTool::Position::glRenderAction(AnnotationTool::Annotation::CreationState* creationState,AnnotationTool::Annotation::PickResult* pickResult,AnnotationTool::Annotation::DragState* dragState,GLContextData& contextData) const
	{
	if(creationState==0||static_cast<CreationState*>(creationState)->numPoints>0)
		{
		/* Draw the position: */
		draw(contextData);
		}
	}

/********************************************************
Methods of class AnnotationTool::Distance::CreationState:
********************************************************/

AnnotationTool::Distance::CreationState::CreationState(void)
	:numPoints(0)
	{
	}

bool AnnotationTool::Distance::CreationState::isFinished(void) const
	{
	return numPoints>=2;
	}

/*****************************************************
Methods of class AnnotationTool::Distance::PickResult:
*****************************************************/

AnnotationTool::Distance::PickResult::PickResult(unsigned int sPointIndex,Scalar sPickDistance2)
	:Annotation::PickResult(sPickDistance2),
	 pointIndex(sPointIndex)
	{
	}

/****************************************************
Methods of class AnnotationTool::Distance::DragState:
****************************************************/

AnnotationTool::Distance::DragState::DragState(unsigned int sPointIndex,const Point& sLocalPos)
	:pointIndex(sPointIndex),localPos(sLocalPos)
	{
	}

/*****************************************
Methods of class AnnotationTool::Distance:
*****************************************/

void AnnotationTool::Distance::updateLabel(void)
	{
	char distLabelText[40];
	snprintf(distLabelText,sizeof(distLabelText),"%g",double(Geometry::dist(pos[0],pos[1])));
	distLabel.setString(distLabelText);
	GLLabel::Box::Vector labelSize=distLabel.getLabelSize();
	distLabel.setOrigin(GLLabel::Box::Vector(-labelSize[0]*0.5f,0.0f,0.0f));
	}

void AnnotationTool::Distance::draw(GLContextData& contextData) const
	{
	/* Draw the distance: */
	glDisable(GL_LIGHTING);
	
	Point physPos[2];
	for(int i=0;i<2;++i)
		physPos[i]=getNavigationTransformation().transform(pos[i]);
	
	Vector x=physPos[1]-physPos[0];
	Vector z=getMainViewer()->getHeadPosition()-physPos[0];
	Vector y=z^x;
	y.normalize();
	
	/* Draw the distance measure: */
	glLineWidth(3.0f);
	glColor(bgColor);
	for(int i=0;i<2;++i)
		{
		glBegin(GL_LINE_STRIP);
		glVertex(physPos[i]);
		glVertex(physPos[i]+y*factory->markerSize*Scalar(1.5));
		glVertex(physPos[i]+y*factory->markerSize*Scalar(2));
		glEnd();
		}
	glBegin(GL_LINES);
	glVertex(physPos[0]+y*factory->markerSize*Scalar(1.5));
	glVertex(physPos[1]+y*factory->markerSize*Scalar(1.5));
	glEnd();
	
	glLineWidth(1.0f);
	glColor(fgColor);
	for(int i=0;i<2;++i)
		{
		glBegin(GL_LINE_STRIP);
		glVertex(physPos[i]);
		glVertex(physPos[i]+y*factory->markerSize*Scalar(1.5));
		glVertex(physPos[i]+y*factory->markerSize*Scalar(2));
		glEnd();
		}
	glBegin(GL_LINES);
	glVertex(physPos[0]+y*factory->markerSize*Scalar(1.5));
	glVertex(physPos[1]+y*factory->markerSize*Scalar(1.5));
	glEnd();
	
	/* Draw the distance label: */
	glPushMatrix();
	Point p=Geometry::mid(physPos[0],physPos[1])+y*factory->markerSize*Scalar(2);
	glTranslate(p-Point::origin);
	z=getMainViewer()->getHeadPosition()-p+y*factory->markerSize*Scalar(0.5);
	y=z^x;
	x=y^z;
	glRotate(Rotation::fromBaseVectors(x,y));
	
	distLabel.draw(contextData);
	
	glPopMatrix();
	}

AnnotationTool::Distance::Distance(const GLFont& labelFont)
	{
	/* Determine default background and foreground colors: */
	bgColor=getBackgroundColor();
	for(int i=0;i<3;++i)
		fgColor[i]=1.0f-bgColor[i];
	fgColor[3]=1.0f;
	
	/* Initialize the distance label: */
	distLabel.setFont(labelFont);
	Color lbgColor=bgColor;
	lbgColor[3]=0.0f;
	distLabel.setBackground(lbgColor);
	distLabel.setForeground(fgColor);
	}

AnnotationTool::Annotation::PickResult* AnnotationTool::Distance::pick(const Point& pickPos) const
	{
	/* Compare the given position against the distance measure's end points: */
	Scalar pickDistance2=Math::sqr(getPointPickDistance());
	unsigned int pickPointIndex=2;
	for(unsigned int i=0;i<2;++i)
		{
		Scalar dist2=Geometry::sqrDist(pickPos,pos[i]);
		if(pickDistance2>dist2)
			{
			pickDistance2=dist2;
			pickPointIndex=i;
			}
		}
	
	if(pickPointIndex<2)
		return new PickResult(pickPointIndex,pickDistance2);
	else
		return 0;
	}

AnnotationTool::Annotation::DragState* AnnotationTool::Distance::startDrag(const AnnotationTool::Annotation::DragTransform& drag,AnnotationTool::Annotation::CreationState* creationState,AnnotationTool::Annotation::PickResult* pickResult)
	{
	/* Check if we're in creation-dragging or pick-dragging mode: */
	unsigned int pointIndex;
	Point localPos;
	if(creationState!=0)
		{
		/* Get the creation state object: */
		CreationState* myCs=static_cast<CreationState*>(creationState);
		
		/* Initialize the new point's local position: */
		pointIndex=myCs->numPoints;
		localPos=Point::origin;
		
		/* Increment the number of points: */
		++myCs->numPoints;
		}
	else
		{
		/* Get the pick result object: */
		PickResult* myPr=static_cast<PickResult*>(pickResult);
		
		/* Calculate the dragged point's local position: */
		pointIndex=myPr->pointIndex;
		localPos=drag.inverseTransform(pos[pointIndex]);
		}
	
	/* Return a drag state: */
	return new DragState(pointIndex,localPos);
	}

void AnnotationTool::Distance::drag(const AnnotationTool::Annotation::DragTransform& drag,AnnotationTool::Annotation::CreationState* creationState,AnnotationTool::Annotation::PickResult* pickResult,AnnotationTool::Annotation::DragState* dragState)
	{
	/* Get the drag state object: */
	DragState* myDs=static_cast<DragState*>(dragState);
	
	/* Update the position of the dragged point: */
	pos[myDs->pointIndex]=drag.transform(myDs->localPos);
	updateLabel();
	}

void AnnotationTool::Distance::glRenderAction(GLContextData& contextData) const
	{
	/* Draw the distance: */
	draw(contextData);
	}

void AnnotationTool::Distance::glRenderAction(AnnotationTool::Annotation::CreationState* creationState,AnnotationTool::Annotation::PickResult* pickResult,AnnotationTool::Annotation::DragState* dragState,GLContextData& contextData) const
	{
	if(creationState==0||static_cast<CreationState*>(creationState)->numPoints>1)
		{
		/* Draw the distance: */
		draw(contextData);
		}
	else if(static_cast<CreationState*>(creationState)->numPoints>0)
		{
		/* Draw a marker for the initial position: */
		glDisable(GL_LIGHTING);
		
		Point physPos=getNavigationTransformation().transform(pos[0]);
		glLineWidth(3.0f);
		glColor(bgColor);
		drawMarker(physPos);
		glLineWidth(1.0f);
		glColor(fgColor);
		drawMarker(physPos);
		}
	}

/*****************************************************
Methods of class AnnotationTool::Angle::CreationState:
*****************************************************/

AnnotationTool::Angle::CreationState::CreationState(void)
	:numPoints(0)
	{
	}

bool AnnotationTool::Angle::CreationState::isFinished(void) const
	{
	return numPoints>=3;
	}

/**************************************************
Methods of class AnnotationTool::Angle::PickResult:
**************************************************/

AnnotationTool::Angle::PickResult::PickResult(unsigned int sPointIndex,Scalar sPickAngle2)
	:Annotation::PickResult(sPickAngle2),
	 pointIndex(sPointIndex)
	{
	}

/*************************************************
Methods of class AnnotationTool::Angle::DragState:
*************************************************/

AnnotationTool::Angle::DragState::DragState(unsigned int sPointIndex,const Point& sLocalPos)
	:pointIndex(sPointIndex),localPos(sLocalPos)
	{
	}

/**************************************
Methods of class AnnotationTool::Angle:
**************************************/

void AnnotationTool::Angle::updateLabel(void)
	{
	char angleLabelText[40];
	Vector d1=pos[1]-pos[0];
	Vector d2=pos[2]-pos[0];
	angle=Math::acos((d1*d2)/(Geometry::mag(d1)*Geometry::mag(d2)));
	snprintf(angleLabelText,sizeof(angleLabelText),"%g",double(Math::deg(angle)));
	angleLabel.setString(angleLabelText);
	GLLabel::Box::Vector labelSize=angleLabel.getLabelSize();
	angleLabel.setOrigin(GLLabel::Box::Vector(-labelSize[0]*0.5f,-labelSize[1]*0.5f,0.0f));
	}

void AnnotationTool::Angle::draw(GLContextData& contextData) const
	{
	/* Draw the distance: */
	glDisable(GL_LIGHTING);
	
	Point physPos[3];
	for(int i=0;i<3;++i)
		physPos[i]=getNavigationTransformation().transform(pos[i]);
	Vector x[2];
	Scalar xLen[2];
	Vector y[2];
	for(int i=0;i<2;++i)
		{
		x[i]=physPos[i+1]-physPos[0];
		xLen[i]=Geometry::mag(x[i]);
		Vector z=getMainViewer()->getHeadPosition()-physPos[i+1];
		y[i]=z^x[i];
		y[i].normalize();
		}
	Vector c=x[0]*(factory->markerSize*Scalar(6)/xLen[0]);
	Vector s=x[1]-x[0]*((x[1]*x[0])/Math::sqr(xLen[0]));
	s=s*(factory->markerSize*Scalar(6)/Geometry::mag(s));
	int numSegments=int(Math::ceil(angle/Math::rad(Scalar(10))));
	
	/* Draw the angle measure: */
	glLineWidth(3.0f);
	glColor(bgColor);
	glBegin(GL_LINES);
	for(int i=0;i<2;++i)
		{
		glVertex(physPos[0]);
		glVertex(physPos[i+1]);
		if(xLen[i]<factory->markerSize*Scalar(7))
			{
			glVertex(physPos[i+1]);
			glVertex(physPos[0]+x[i]*(factory->markerSize*Scalar(7)/xLen[i]));
			}
		glVertex(physPos[i+1]-y[i]*factory->markerSize);
		glVertex(physPos[i+1]+y[i]*factory->markerSize);
		}
	glEnd();
	glBegin(GL_LINE_STRIP);
	for(int i=0;i<=numSegments;++i)
		{
		Scalar a=angle*Scalar(i)/Scalar(numSegments);
		glVertex(physPos[0]+c*Math::cos(a)+s*Math::sin(a));
		}
	glEnd();
	glLineWidth(1.0f);
	glColor(fgColor);
	glBegin(GL_LINES);
	for(int i=0;i<2;++i)
		{
		glVertex(physPos[0]);
		glVertex(physPos[i+1]);
		if(xLen[i]<factory->markerSize*Scalar(7))
			{
			glVertex(physPos[i+1]);
			glVertex(physPos[0]+x[i]*(factory->markerSize*Scalar(7)/xLen[i]));
			}
		glVertex(physPos[i+1]-y[i]*factory->markerSize);
		glVertex(physPos[i+1]+y[i]*factory->markerSize);
		}
	glEnd();
	glBegin(GL_LINE_STRIP);
	for(int i=0;i<=numSegments;++i)
		{
		Scalar a=angle*Scalar(i)/Scalar(numSegments);
		glVertex(physPos[0]+c*Math::cos(a)+s*Math::sin(a));
		}
	glEnd();
	
	/* Draw the angle label: */
	glPushMatrix();
	Point p=physPos[0]+(c*Math::cos(angle*Scalar(0.5))+s*Math::sin(angle*Scalar(0.5)))*Scalar(7.0/6.0);
	glTranslate(p-Point::origin);
	Vector lz=getMainViewer()->getHeadPosition()-p;
	Vector lx=getUpDirection()^lz;
	Vector ly=lz^lx;
	glRotate(Rotation::fromBaseVectors(lx,ly));
	
	angleLabel.draw(contextData);
	
	glPopMatrix();
	}

AnnotationTool::Angle::Angle(const GLFont& labelFont)
	{
	/* Determine default background and foreground colors: */
	bgColor=getBackgroundColor();
	for(int i=0;i<3;++i)
		fgColor[i]=1.0f-bgColor[i];
	fgColor[3]=1.0f;
	
	/* Initialize the angle label: */
	angleLabel.setFont(labelFont);
	Color lbgColor=bgColor;
	lbgColor[3]=0.0f;
	angleLabel.setBackground(lbgColor);
	angleLabel.setForeground(fgColor);
	}

AnnotationTool::Annotation::PickResult* AnnotationTool::Angle::pick(const Point& pickPos) const
	{
	/* Compare the given position against the distance measure's end points: */
	Scalar pickAngle2=Math::sqr(getPointPickDistance());
	unsigned int pickPointIndex=3;
	for(unsigned int i=0;i<3;++i)
		{
		Scalar dist2=Geometry::sqrDist(pickPos,pos[i]);
		if(pickAngle2>dist2)
			{
			pickAngle2=dist2;
			pickPointIndex=i;
			}
		}
	
	if(pickPointIndex<3)
		return new PickResult(pickPointIndex,pickAngle2);
	else
		return 0;
	}

AnnotationTool::Annotation::DragState* AnnotationTool::Angle::startDrag(const AnnotationTool::Annotation::DragTransform& drag,AnnotationTool::Annotation::CreationState* creationState,AnnotationTool::Annotation::PickResult* pickResult)
	{
	/* Check if we're in creation-dragging or pick-dragging mode: */
	unsigned int pointIndex;
	Point localPos;
	if(creationState!=0)
		{
		/* Get the creation state object: */
		CreationState* myCs=static_cast<CreationState*>(creationState);
		
		/* Initialize the new point's local position: */
		pointIndex=myCs->numPoints;
		localPos=Point::origin;
		
		/* Increment the number of points: */
		++myCs->numPoints;
		}
	else
		{
		/* Get the pick result object: */
		PickResult* myPr=static_cast<PickResult*>(pickResult);
		
		/* Calculate the dragged point's local position: */
		pointIndex=myPr->pointIndex;
		localPos=drag.inverseTransform(pos[pointIndex]);
		}
	
	/* Return a drag state: */
	return new DragState(pointIndex,localPos);
	}

void AnnotationTool::Angle::drag(const AnnotationTool::Annotation::DragTransform& drag,AnnotationTool::Annotation::CreationState* creationState,AnnotationTool::Annotation::PickResult* pickResult,AnnotationTool::Annotation::DragState* dragState)
	{
	/* Get the drag state object: */
	DragState* myDs=static_cast<DragState*>(dragState);
	
	/* Update the position of the dragged point: */
	pos[myDs->pointIndex]=drag.transform(myDs->localPos);
	updateLabel();
	}

void AnnotationTool::Angle::glRenderAction(GLContextData& contextData) const
	{
	/* Draw the angle: */
	draw(contextData);
	}

void AnnotationTool::Angle::glRenderAction(AnnotationTool::Annotation::CreationState* creationState,AnnotationTool::Annotation::PickResult* pickResult,AnnotationTool::Annotation::DragState* dragState,GLContextData& contextData) const
	{
	if(creationState==0||static_cast<CreationState*>(creationState)->numPoints>2)
		{
		/* Draw the angle: */
		draw(contextData);
		}
	else if(static_cast<CreationState*>(creationState)->numPoints>0)
		{
		glDisable(GL_LIGHTING);
		
		/* Draw a marker for each position: */
		Point physPos[2];
		unsigned int np=static_cast<CreationState*>(creationState)->numPoints;
		for(unsigned int i=0;i<np;++i)
			{
			physPos[i]=getNavigationTransformation().transform(pos[i]);
			glLineWidth(3.0f);
			glColor(bgColor);
			drawMarker(physPos[i]);
			glLineWidth(1.0f);
			glColor(fgColor);
			drawMarker(physPos[i]);
			}
		if(np>1)
			{
			/* Draw a line connecting the first two points: */
			glLineWidth(3.0f);
			glColor(bgColor);
			glBegin(GL_LINES);
			glVertex(physPos[0]);
			glVertex(physPos[1]);
			glEnd();
			glLineWidth(1.0f);
			glColor(fgColor);
			glBegin(GL_LINES);
			glVertex(physPos[0]);
			glVertex(physPos[1]);
			glEnd();
			}
		}
	}

/***************************************
Static elements of class AnnotationTool:
***************************************/

AnnotationToolFactory* AnnotationTool::factory=0;

/*******************************
Methods of class AnnotationTool:
*******************************/

void AnnotationTool::creationButtonsCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData)
	{
	/* Cancel any ongoing object creation: */
	delete newObject;
	newObject=0;
	delete newCreationState;
	newCreationState=0;
	
	if(cbData->newSelectedToggle!=0)
		{
		/* Start creating a new object: */
		switch(cbData->radioBox->getToggleIndex(cbData->newSelectedToggle))
			{
			case 0:
				/* Create a new position object: */
				newObject=new Position(*factory->labelFont);
				newCreationState=new Position::CreationState;
				break;
			
			case 1:
				/* Create a new distance object: */
				newObject=new Distance(*factory->labelFont);
				newCreationState=new Distance::CreationState;
				break;
			
			case 2:
				/* Create a new angle object: */
				newObject=new Angle(*factory->labelFont);
				newCreationState=new Angle::CreationState;
				break;
			}
		}
	}

void AnnotationTool::coordTransformChangedCallback(CoordinateManager::CoordinateTransformChangedCallbackData* cbData)
	{
	/* Update the user transformation: */
	userTransform=cbData->newTransform;
	}

AnnotationTool::AnnotationTool(const ToolFactory* sFactory,const ToolInputAssignment& inputAssignment)
	:UtilityTool(sFactory,inputAssignment),
	 annotationDialogPopup(0),creationButtons(0),
	 userTransform(getCoordinateManager()->getCoordinateTransform()),
	 newObject(0),newCreationState(0),
	 pickedObject(0),pickResult(0),
	 dragDevice(0),dragState(0)
	{
	/* Create the annotation dialog window: */
	annotationDialogPopup=new GLMotif::PopupWindow("AnnotationDialogPopup",getWidgetManager(),"Annotation Dialog");
	
	GLMotif::RowColumn* annotationDialog=new GLMotif::RowColumn("AnnotationDialog",annotationDialogPopup,false);
	annotationDialog->setOrientation(GLMotif::RowColumn::VERTICAL);
	annotationDialog->setPacking(GLMotif::RowColumn::PACK_TIGHT);
	annotationDialog->setNumMinorWidgets(1);
	
	creationButtons=new GLMotif::RadioBox("CreationButtons",annotationDialog,false);
	creationButtons->setOrientation(GLMotif::RowColumn::HORIZONTAL);
	creationButtons->setPacking(GLMotif::RowColumn::PACK_TIGHT);
	creationButtons->setAlignment(GLMotif::Alignment::LEFT);
	creationButtons->setSelectionMode(GLMotif::RadioBox::ATMOST_ONE);
	
	creationButtons->addToggle("Position");
	creationButtons->addToggle("Distance");
	creationButtons->addToggle("Angle");
	creationButtons->addToggle("Arrow");
	creationButtons->addToggle("Balloon");
	
	creationButtons->getValueChangedCallbacks().add(this,&AnnotationTool::creationButtonsCallback);
	creationButtons->manageChild();
	
	new GLMotif::Button("DeleteAnnotation",annotationDialog,"Delete Annotation");
	
	annotationDialog->manageChild();
	
	/* Pop up the annotation dialog: */
	popupPrimaryWidget(annotationDialogPopup);
	
	/* Register a callback with the coordinate manager: */
	getCoordinateManager()->getCoordinateTransformChangedCallbacks().add(this,&AnnotationTool::coordTransformChangedCallback);
	}

AnnotationTool::~AnnotationTool(void)
	{
	/* Unregister the callback from the coordinate manager: */
	getCoordinateManager()->getCoordinateTransformChangedCallbacks().remove(this,&AnnotationTool::coordTransformChangedCallback);
	
	/* Delete the annotation dialog: */
	delete annotationDialogPopup;
	
	/* Delete all created annotation objects: */
	for(std::vector<Annotation*>::iterator aIt=annotations.begin();aIt!=annotations.end();++aIt)
		delete *aIt;
	
	/* Delete annotation creation state: */
	delete newObject;
	delete newCreationState;
	
	/* Delete dragging state: */
	delete pickResult;
	delete dragState;
	}

void AnnotationTool::configure(const Misc::ConfigurationFileSection& configFileSection)
	{
	/* Read the currently selected toggle: */
	int creationState=configFileSection.retrieveValue<int>("./creationState",-1);
	if(creationState>=0&&creationState<3)
		{
		/* Select the toggle and start creating a new annotation object: */
		creationButtons->setSelectedToggle(creationState);
		
		switch(creationState)
			{
			case 0:
				/* Create a new position object: */
				newObject=new Position(*factory->labelFont);
				newCreationState=new Position::CreationState;
				break;
			
			case 1:
				/* Create a new distance object: */
				newObject=new Distance(*factory->labelFont);
				newCreationState=new Distance::CreationState;
				break;
			
			case 2:
				/* Create a new angle object: */
				newObject=new Angle(*factory->labelFont);
				newCreationState=new Angle::CreationState;
				break;
			}
		}
	
	/* Read the annotation dialog's position, orientation, and size: */
	GLMotif::readTopLevelPosition(annotationDialogPopup,configFileSection);
	}

void AnnotationTool::storeState(Misc::ConfigurationFileSection& configFileSection) const
	{
	/* Write the index of the currently selected toggle or -1 if no toggle is selected: */
	configFileSection.storeValue<int>("./creationState",creationButtons->getToggleIndex(creationButtons->getSelectedToggle()));
	
	/* Write the annotation dialog's current position, orientation, and size: */
	GLMotif::writeTopLevelPosition(annotationDialogPopup,configFileSection);
	}

const ToolFactory* AnnotationTool::getFactory(void) const
	{
	return factory;
	}

void AnnotationTool::buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData)
	{
	if(cbData->newButtonState)
		{
		if(newObject!=0)
			{
			/* Start a dragging operation on the new object: */
			dragDevice=cbData->inputDevice;
			dragState=newObject->startDrag(getDeviceTransformation(dragDevice),newCreationState,0);
			}
		else
			{
			/* Perform a pick query on all finished objects: */
			for(std::vector<Annotation*>::iterator aIt=annotations.begin();aIt!=annotations.end();++aIt)
				{
				Annotation::PickResult* pr=(*aIt)->pick(getDevicePosition(cbData->inputDevice));
				if(pr!=0&&(pickResult==0||pickResult->getPickDistance2()>pr->getPickDistance2()))
					{
					pickedObject=*aIt;
					delete pickResult;
					pickResult=pr;
					}
				}
			
			if(pickedObject!=0)
				{
				/* Select the picked object: */
				pickedObject->select(true);
				
				/* Start dragging the picked object: */
				dragDevice=cbData->inputDevice;
				dragState=pickedObject->startDrag(getDeviceTransformation(dragDevice),0,pickResult);
				}
			}
		}
	else if(dragDevice!=0)
		{
		if(newObject!=0)
			{
			/* Finish a dragging operation on the new object: */
			newObject->endDrag(getDeviceTransformation(dragDevice),newCreationState,0,dragState);
			delete dragState;
			dragState=0;
			dragDevice=0;
			
			/* Check if the new object is finished: */
			if(newCreationState->isFinished())
				{
				/* Store the finished object and stop the creation process: */
				annotations.push_back(newObject);
				newObject=0;
				delete newCreationState;
				newCreationState=0;
				creationButtons->setSelectedToggle(static_cast<GLMotif::ToggleButton*>(0));
				}
			}
		else if(pickedObject!=0)
			{
			/* Finish a dragging operation on the picked object: */
			pickedObject->endDrag(getDeviceTransformation(dragDevice),0,pickResult,dragState);
			delete dragState;
			dragState=0;
			dragDevice=0;
			
			/* Unselect the picked object: */
			pickedObject->select(false);
			pickedObject=0;
			delete pickResult;
			pickResult=0;
			}
		}
	}

void AnnotationTool::frame(void)
	{
	if(dragDevice!=0)
		{
		if(newObject!=0)
			{
			/* Continue a dragging operation on the new object: */
			newObject->drag(getDeviceTransformation(dragDevice),newCreationState,0,dragState);
			}
		else if(pickedObject!=0)
			{
			/* Continue a dragging operation on the picked object: */
			pickedObject->drag(getDeviceTransformation(dragDevice),0,pickResult,dragState);
			}
		}
	}

void AnnotationTool::display(GLContextData& contextData) const
	{
	/* Save OpenGL state: */
	glPushAttrib(GL_ENABLE_BIT|GL_LINE_BIT|GL_POINT_BIT);
	
	/* Draw all complete annotation objects: */
	for(std::vector<Annotation*>::const_iterator aIt=annotations.begin();aIt!=annotations.end();++aIt)
		(*aIt)->glRenderAction(contextData);
	
	if(newObject!=0)
		{
		/* Draw the currently created object: */
		newObject->glRenderAction(newCreationState,0,0,contextData);
		}
	
	/* Restore OpenGL state: */
	glPopAttrib();
	}

}
