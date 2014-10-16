/***********************************************************************
WalkNavigationTool - Class to navigate in a VR environment by walking
around a fixed center position.
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

#include <Vrui/Tools/WalkNavigationTool.h>

#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Math/Math.h>
#include <Math/Constants.h>
#include <Geometry/Rotation.h>
#include <Geometry/OrthonormalTransformation.h>
#include <Geometry/GeometryValueCoders.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLContextData.h>
#include <GL/GLValueCoders.h>
#include <GL/GLGeometryWrappers.h>
#include <Vrui/Viewer.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/******************************************
Methods of class WalkNavigationToolFactory:
******************************************/

WalkNavigationToolFactory::WalkNavigationToolFactory(ToolManager& toolManager)
	:ToolFactory("WalkNavigationTool",toolManager),
	 centerOnActivation(false),
	 centerPoint(getDisplayCenter()),
	 moveSpeed(getDisplaySize()),
	 innerRadius(getDisplaySize()*Scalar(0.5)),outerRadius(getDisplaySize()*Scalar(0.75)),
	 centerViewDirection(getForwardDirection()),
	 rotateSpeed(Math::rad(Scalar(120))),
	 innerAngle(Math::rad(Scalar(30))),outerAngle(Math::rad(Scalar(120))),
	 drawMovementCircles(true),
	 movementCircleColor(0.0f,1.0f,0.0f)
	{
	/* Initialize tool layout: */
	layout.setNumButtons(1);
	
	/* Insert class into class hierarchy: */
	ToolFactory* navigationToolFactory=toolManager.loadClass("NavigationTool");
	navigationToolFactory->addChildClass(this);
	addParentClass(navigationToolFactory);
	
	/* Load class settings: */
	Misc::ConfigurationFileSection cfs=toolManager.getToolClassSection(getClassName());
	centerOnActivation=cfs.retrieveValue<bool>("./centerOnActivation",centerOnActivation);
	centerPoint=cfs.retrieveValue<Point>("./centerPoint",centerPoint);
	centerPoint=getFloorPlane().project(centerPoint);
	moveSpeed=cfs.retrieveValue<Scalar>("./moveSpeed",moveSpeed);
	innerRadius=cfs.retrieveValue<Scalar>("./innerRadius",innerRadius);
	outerRadius=cfs.retrieveValue<Scalar>("./outerRadius",outerRadius);
	centerViewDirection=cfs.retrieveValue<Vector>("./centerViewDirection",centerViewDirection);
	centerViewDirection-=getUpDirection()*((centerViewDirection*getUpDirection())/Geometry::sqr(getUpDirection()));
	centerViewDirection.normalize();
	rotateSpeed=Math::rad(cfs.retrieveValue<Scalar>("./rotateSpeed",Math::deg(rotateSpeed)));
	innerAngle=Math::rad(cfs.retrieveValue<Scalar>("./innerAngle",Math::deg(innerAngle)));
	outerAngle=Math::rad(cfs.retrieveValue<Scalar>("./outerAngle",Math::deg(outerAngle)));
	drawMovementCircles=cfs.retrieveValue<bool>("./drawMovementCircles",drawMovementCircles);
	movementCircleColor=cfs.retrieveValue<Color>("./movementCircleColor",movementCircleColor);
	
	/* Set tool class' factory pointer: */
	WalkNavigationTool::factory=this;
	}

WalkNavigationToolFactory::~WalkNavigationToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	WalkNavigationTool::factory=0;
	}

const char* WalkNavigationToolFactory::getName(void) const
	{
	return "Walk";
	}

const char* WalkNavigationToolFactory::getButtonFunction(int) const
	{
	return "Start / Stop";
	}

Tool* WalkNavigationToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new WalkNavigationTool(this,inputAssignment);
	}

void WalkNavigationToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveWalkNavigationToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("NavigationTool");
	}

extern "C" ToolFactory* createWalkNavigationToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	WalkNavigationToolFactory* walkNavigationToolFactory=new WalkNavigationToolFactory(*toolManager);
	
	/* Return factory object: */
	return walkNavigationToolFactory;
	}

extern "C" void destroyWalkNavigationToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/*********************************************
Methods of class WalkNavigationTool::DataItem:
*********************************************/

WalkNavigationTool::DataItem::DataItem(void)
	{
	/* Create tools' model display list: */
	movementCircleListId=glGenLists(1);
	}

WalkNavigationTool::DataItem::~DataItem(void)
	{
	/* Destroy tools' model display list: */
	glDeleteLists(movementCircleListId,1);
	}

/*******************************************
Static elements of class WalkNavigationTool:
*******************************************/

WalkNavigationToolFactory* WalkNavigationTool::factory=0;

/***********************************
Methods of class WalkNavigationTool:
***********************************/

Point WalkNavigationTool::projectToFloor(const Point& p)
	{
	/* Project the given point onto the floor plane along the up direction: */
	const Vector& normal=getFloorPlane().getNormal();
	Scalar lambda=(getFloorPlane().getOffset()-p*normal)/(getUpDirection()*normal);
	return p+getUpDirection()*lambda;
	}

WalkNavigationTool::WalkNavigationTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment)
	:NavigationTool(factory,inputAssignment),
	 centerPoint(static_cast<const WalkNavigationToolFactory*>(factory)->centerPoint)
	{
	}

const ToolFactory* WalkNavigationTool::getFactory(void) const
	{
	return factory;
	}

void WalkNavigationTool::buttonCallback(int,InputDevice::ButtonCallbackData* cbData)
	{
	if(cbData->newButtonState) // Button has just been pressed
		{
		/* Act depending on this tool's current state: */
		if(isActive())
			{
			/* Deactivate this tool: */
			deactivate();
			}
		else
			{
			/* Try activating this tool: */
			if(activate())
				{
				/* Store the center point for this navigation sequence: */
				if(factory->centerOnActivation)
					centerPoint=projectToFloor(getMainViewer()->getHeadPosition());
				
				/* Initialize the navigation transformation: */
				preScale=Vrui::getNavigationTransformation();
				translation=Vector::zero;
				azimuth=Scalar(0);
				}
			}
		}
	}

void WalkNavigationTool::frame(void)
	{
	/* Act depending on this tool's current state: */
	if(isActive())
		{
		/* Calculate azimuth angle change based on the current viewing direction: */
		Vector viewDir=getMainViewer()->getViewDirection();
		viewDir-=getUpDirection()*((viewDir*getUpDirection())/Geometry::sqr(getUpDirection()));
		Scalar viewDir2=Geometry::sqr(viewDir);
		if(viewDir2!=Scalar(0))
			{
			/* Calculate the rotation speed: */
			Scalar viewAngleCos=(viewDir*factory->centerViewDirection)/Math::sqrt(viewDir2);
			Scalar viewAngle;
			if(viewAngleCos>Scalar(1)-Math::Constants<Scalar>::epsilon)
				viewAngle=Scalar(0);
			else if(viewAngleCos<Scalar(-1)+Math::Constants<Scalar>::epsilon)
				viewAngle=Math::Constants<Scalar>::pi;
			else
				viewAngle=Math::acos(viewAngleCos);
			Scalar rotateSpeed=Scalar(0);
			if(viewAngle>=factory->outerAngle)
				rotateSpeed=factory->rotateSpeed;
			else if(viewAngle>factory->innerAngle)
				rotateSpeed=factory->rotateSpeed*(viewAngle-factory->innerAngle)/(factory->outerAngle-factory->innerAngle);
			Vector x=factory->centerViewDirection^getUpDirection();
			if(viewDir*x<Scalar(0))
				rotateSpeed=-rotateSpeed;
			
			/* Update the accumulated rotation angle: */
			azimuth+=rotateSpeed*getFrameTime();
			if(azimuth<-Math::Constants<Scalar>::pi)
				azimuth+=Scalar(2)*Math::Constants<Scalar>::pi;
			else if(azimuth>=Math::Constants<Scalar>::pi)
				azimuth-=Scalar(2)*Math::Constants<Scalar>::pi;
			}
		
		/* Calculate the movement direction and speed: */
		Point footPos=projectToFloor(getMainViewer()->getHeadPosition());
		Vector moveDir=centerPoint-footPos;
		Scalar moveDirLen=moveDir.mag();
		Scalar speed=Scalar(0);
		if(moveDirLen>=factory->outerRadius)
			speed=factory->moveSpeed;
		else if(moveDirLen>factory->innerRadius)
			speed=factory->moveSpeed*(moveDirLen-factory->innerRadius)/(factory->outerRadius-factory->innerRadius);
		moveDir*=speed/moveDirLen;
		
		/* Accumulate the transformation: */
		NavTransform::Rotation rot=NavTransform::Rotation::rotateAxis(getUpDirection(),azimuth);
		translation+=rot.inverseTransform(moveDir*getFrameTime());
		
		/* Set the navigation transformation: */
		NavTransform nav=NavTransform::identity;
		nav*=Vrui::NavTransform::translateFromOriginTo(centerPoint);
		nav*=Vrui::NavTransform::rotate(rot);
		nav*=Vrui::NavTransform::translateToOriginFrom(centerPoint);
		nav*=Vrui::NavTransform::translate(translation);
		nav*=preScale;
		setNavigationTransformation(nav);
		
		if(speed!=Scalar(0))
			{
			/* Request another frame: */
			scheduleUpdate(getApplicationTime()+1.0/125.0);
			}
		}
	}

void WalkNavigationTool::display(GLContextData& contextData) const
	{
	if(factory->drawMovementCircles)
		{
		/* Get a pointer to the context entry: */
		WalkNavigationTool::DataItem* dataItem=contextData.retrieveDataItem<WalkNavigationTool::DataItem>(this);
		
		/* Translate to the center point: */
		glPushMatrix();
		glTranslate(centerPoint-Point::origin);
		
		/* Execute the tool model display list: */
		glCallList(dataItem->movementCircleListId);
		
		glPopMatrix();
		}
	}

void WalkNavigationTool::initContext(GLContextData& contextData) const
	{
	if(factory->drawMovementCircles)
		{
		/* Create a new data item: */
		DataItem* dataItem=new DataItem;
		contextData.addDataItem(this,dataItem);
		
		/* Create the movement circle display list: */
		glNewList(dataItem->movementCircleListId,GL_COMPILE);
		
		/* Set up OpenGL state: */
		glPushAttrib(GL_ENABLE_BIT|GL_LINE_BIT);
		glDisable(GL_LIGHTING);
		glLineWidth(1.0f);
		glColor(factory->movementCircleColor);
		
		/* Create a coordinate system for the floor plane: */
		Vector y=factory->centerViewDirection;
		Vector x=y^getFloorPlane().getNormal();
		x.normalize();
		
		/* Draw the inner circle: */
		glBegin(GL_LINE_LOOP);
		for(int i=0;i<64;++i)
			{
			Scalar angle=Scalar(2)*Math::Constants<Scalar>::pi*Scalar(i)/Scalar(64);
			glVertex(Point::origin-x*(Math::sin(angle)*factory->innerRadius)+y*(Math::cos(angle)*factory->innerRadius));
			}
		glEnd();
		
		/* Draw the outer circle: */
		glBegin(GL_LINE_LOOP);
		for(int i=0;i<64;++i)
			{
			Scalar angle=Scalar(2)*Math::Constants<Scalar>::pi*Scalar(i)/Scalar(64);
			glVertex(Point::origin-x*(Math::sin(angle)*factory->outerRadius)+y*(Math::cos(angle)*factory->outerRadius));
			}
		glEnd();
		
		/* Draw the inner angle: */
		glBegin(GL_LINE_STRIP);
		glVertex(Point::origin-x*(Math::sin(factory->innerAngle)*factory->innerRadius)+y*(Math::cos(factory->innerAngle)*factory->innerRadius));
		glVertex(Point::origin);
		glVertex(Point::origin-x*(Math::sin(-factory->innerAngle)*factory->innerRadius)+y*(Math::cos(-factory->innerAngle)*factory->innerRadius));
		glEnd();
		
		/* Draw the outer angle: */
		glBegin(GL_LINE_STRIP);
		glVertex(Point::origin-x*(Math::sin(factory->outerAngle)*factory->outerRadius)+y*(Math::cos(factory->outerAngle)*factory->outerRadius));
		glVertex(Point::origin);
		glVertex(Point::origin-x*(Math::sin(-factory->outerAngle)*factory->outerRadius)+y*(Math::cos(-factory->outerAngle)*factory->outerRadius));
		glEnd();
		
		/* Reset OpenGL state: */
		glPopAttrib();
		
		glEndList();
		}
	}

}
