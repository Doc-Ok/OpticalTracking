/***********************************************************************
Widget - Base class for GLMotif UI components.
Copyright (c) 2001-2014 Oliver Kreylos

This file is part of the GLMotif Widget Library (GLMotif).

The GLMotif Widget Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GLMotif Widget Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the GLMotif Widget Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <GLMotif/Widget.h>

#include <string.h>
#include <Math/Math.h>
#include <Math/Constants.h>
#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLVertexTemplates.h>
#include <GLMotif/WidgetManager.h>
#include <GLMotif/Event.h>
#include <GLMotif/Container.h>

namespace GLMotif {

/***********************
Methods of class Widget:
***********************/

void Widget::unmanageChild(void)
	{
	/* Unmanage the child: */
	isManaged=false;
	}

Widget::Widget(const char* sName,Container* sParent,bool sManageChild)
	:parent(sParent),isManaged(false),name(new char[strlen(sName)+1]),
	 exterior(Vector(0.0f,0.0f,0.0f),Vector(0.0f,0.0f,0.0f)),
	 borderWidth(0.0f),borderType(PLAIN),
	 interior(Vector(0.0f,0.0f,0.0f),Vector(0.0f,0.0f,0.0f)),
	 zRange(0.0f,0.0f),
	 enabled(true)
	{
	/* Copy the widget name: */
	strcpy(name,sName);
	
	if(parent!=0)
		{
		/* Inherit most settings from parent: */
		borderWidth=parent->borderWidth;
		borderType=parent->borderType;
		borderColor=parent->borderColor;
		backgroundColor=parent->backgroundColor;
		foregroundColor=parent->foregroundColor;
		
		if(sManageChild)
			{
			/* Add yourself to the parent widget: */
			parent->addChild(this);
			isManaged=true;
			}
		}
	}

Widget::~Widget(void)
	{
	/* Tell the widget manager that the widget is to be destroyed: */
	WidgetManager* manager=getManager();
	if(manager!=0)
		manager->unmanageWidget(this);
	
	/* Tell the parent container that the widget is to be destroyed: */
	if(isManaged)
		parent->removeChild(this);
	
	delete[] name;
	}

void Widget::reparent(Container* newParent,bool manageChild)
	{
	/* Remove the widget from its current parent: */
	if(isManaged)
		parent->removeChild(this);
	isManaged=false;
	
	/* Attach the widget to the new parent: */
	parent=newParent;
	if(parent!=0)
		{
		/* Inherit most settings from parent: */
		borderWidth=parent->borderWidth;
		borderType=parent->borderType;
		borderColor=parent->borderColor;
		backgroundColor=parent->backgroundColor;
		foregroundColor=parent->foregroundColor;
		
		if(manageChild)
			{
			/* Add the widget to the parent widget: */
			parent->addChild(this);
			isManaged=true;
			}
		}
	}

void Widget::manageChild(void)
	{
	if(!isManaged)
		{
		parent->addChild(this);
		isManaged=true;
		}
	}

const Widget* Widget::getRoot(void) const
	{
	/* Move up the widget tree until the root is reached: */
	const Widget* result=this;
	while(result->parent!=0)
		result=result->parent;

	return result;
	}

Widget* Widget::getRoot(void)
	{
	/* Move up the widget tree until the root is reached: */
	Widget* result=this;
	while(result->parent!=0)
		result=result->parent;

	return result;
	}

const WidgetManager* Widget::getManager(void) const
	{
	if(parent!=0)
		return parent->getManager();
	else
		return 0;
	}

WidgetManager* Widget::getManager(void)
	{
	if(parent!=0)
		return parent->getManager();
	else
		return 0;
	}

const StyleSheet* Widget::getStyleSheet(void) const
	{
	return getManager()->getStyleSheet();
	}

Vector Widget::calcExteriorSize(const Vector& interiorSize) const
	{
	Vector result=interiorSize;
	result[0]+=2.0f*borderWidth;
	result[1]+=2.0f*borderWidth;
	
	return result;
	}

ZRange Widget::calcZRange(void) const
	{
	GLfloat minZ=exterior.origin[2];
	GLfloat maxZ=exterior.origin[2];
	if(borderType==RAISED)
		maxZ+=borderWidth;
	else if(borderType==LOWERED)
		minZ-=borderWidth;
	
	return ZRange(minZ,maxZ);
	}

void Widget::resize(const Box& newExterior)
	{
	/* Adjust the exterior rectangle: */
	exterior=newExterior;
	
	/* Adjust the interior rectangle: */
	interior=exterior.inset(Vector(borderWidth,borderWidth,0.0f));
	if(borderType==RAISED)
		interior.origin[2]+=borderWidth;
	else if(borderType==LOWERED)
		interior.origin[2]-=borderWidth;
	
	/* Calculate the z range: */
	zRange=calcZRange();
	
	/* Invalidate the visual representation: */
	update();
	}

Vector Widget::calcHotSpot(void) const
	{
	Vector result;
	for(int i=0;i<3;++i)
		result[i]=exterior.origin[i]+exterior.size[i]*0.5f;
	return result;
	}

void Widget::setBorderWidth(GLfloat newBorderWidth)
	{
	borderWidth=newBorderWidth;
	
	/* Try resizing the widget: */
	Box newExterior=interior.outset(Vector(borderWidth,borderWidth,0.0f));
	if(parent!=0&&isManaged)
		parent->requestResize(this,newExterior.size);
	else
		resize(newExterior);
	}

void Widget::setBorderType(Widget::BorderType newBorderType)
	{
	borderType=newBorderType;
	
	/* Try resizing the widget: */
	if(parent!=0&&isManaged)
		parent->requestResize(this,exterior.size);
	else
		resize(exterior);
	}

void Widget::setBorderColor(const Color& newBorderColor)
	{
	/* Set the border color: */
	borderColor=newBorderColor;
	
	/* Update the widget: */
	update();
	}

void Widget::setBackgroundColor(const Color& newBackgroundColor)
	{
	/* Set the background color: */
	backgroundColor=newBackgroundColor;
	
	/* Update the widget: */
	update();
	}

void Widget::setForegroundColor(const Color& newForegroundColor)
	{
	/* Set the foreground color: */
	foregroundColor=newForegroundColor;
	
	/* Update the widget: */
	update();
	}

void Widget::update(void)
	{
	if(parent!=0&&isManaged)
		{
		/* Notify the parent widget of the update: */
		parent->update();
		}
	}

void Widget::draw(GLContextData&) const
	{
	/* Draw the widget's border: */
	if(borderWidth>0.0f)
		{
		/* Draw the border: */
		glColor(borderColor);
		switch(borderType)
			{
			case PLAIN:
				glBegin(GL_QUAD_STRIP);
				glNormal3f(0.0f,0.0f,1.0f);
				glVertex(interior.getCorner(0));
				glVertex(exterior.getCorner(0));
				glVertex(interior.getCorner(1));
				glVertex(exterior.getCorner(1));
				glVertex(interior.getCorner(3));
				glVertex(exterior.getCorner(3));
				glVertex(interior.getCorner(2));
				glVertex(exterior.getCorner(2));
				glVertex(interior.getCorner(0));
				glVertex(exterior.getCorner(0));
				glEnd();
				break;
			
			case RAISED:
			case LOWERED:
				{
				GLfloat normalComponent=borderType==RAISED?0.707f:-0.707f;
				glBegin(GL_QUADS);
				glNormal3f(0.0f,-normalComponent,0.707f);
				glVertex(interior.getCorner(0));
				glVertex(exterior.getCorner(0));
				glVertex(exterior.getCorner(1));
				glVertex(interior.getCorner(1));
				glNormal3f(normalComponent,0.0f,0.707f);
				glVertex(interior.getCorner(1));
				glVertex(exterior.getCorner(1));
				glVertex(exterior.getCorner(3));
				glVertex(interior.getCorner(3));
				glNormal3f(0.0f,normalComponent,0.707f);
				glVertex(interior.getCorner(3));
				glVertex(exterior.getCorner(3));
				glVertex(exterior.getCorner(2));
				glVertex(interior.getCorner(2));
				glNormal3f(-normalComponent,0.0f,0.707f);
				glVertex(interior.getCorner(2));
				glVertex(exterior.getCorner(2));
				glVertex(exterior.getCorner(0));
				glVertex(interior.getCorner(0));
				glEnd();
				break;
				}
			}
		}
	}

bool Widget::isInside(const Point& p) const
	{
	Vector p0=exterior.getCorner(0);
	Vector p3=exterior.getCorner(3);
	
	return p[0]>=p0[0]&&p[0]<=p3[0]&&p[1]>=p0[1]&&p[1]<=p3[1]&&p[2]>=zRange.first&&p[2]<=zRange.second;
	}

Scalar Widget::intersectRay(const Ray& ray,Point& intersection) const
	{
	/* Intersect the ray with the widget's center plane: */
	if(ray.getDirection()[2]==0.0)
		{
		/* Bail out and assign a position inside the widget: */
		intersection[0]=exterior.origin[0]+0.5f*exterior.size[0];
		intersection[1]=exterior.origin[1]+0.5f*exterior.size[1];
		intersection[2]=exterior.origin[2];
		return Math::Constants<Scalar>::max;
		}
	Scalar lambda=(Math::mid(zRange.first,zRange.second)-ray.getOrigin()[2])/ray.getDirection()[2];
	intersection=ray(lambda);
	intersection[2]=Math::mid(zRange.first,zRange.second); // Force the intersection's z value to the desired plane
	return lambda;
	}

void Widget::setEnabled(bool newEnabled)
	{
	enabled=newEnabled;
	}

bool Widget::findRecipient(Event& event)
	{
	/* Reject events if the widget is disabled: */
	if(!enabled)
		return false;
	
	/* Find the event's point in our coordinate system: */
	Event::WidgetPoint wp=event.calcWidgetPoint(this);
	
	/* If the point is inside our bounding box, put us down as recipient: */
	if(isInside(wp.getPoint()))
		return event.setTargetWidget(this,wp);
	else
		return false;
	}

void Widget::pointerButtonDown(Event&)
	{
	/* No default action */
	}

void Widget::pointerButtonUp(Event&)
	{
	/* No default action */
	}

void Widget::pointerMotion(Event&)
	{
	/* No default action */
	}

bool Widget::giveTextFocus(void)
	{
	/* Default behavior is to reject focus: */
	return false;
	}

void Widget::takeTextFocus(void)
	{
	/* No default action */
	}

void Widget::textEvent(const TextEvent&)
	{
	/* No default action */
	}

void Widget::textControlEvent(const TextControlEvent&)
	{
	/* No default action */
	}

}
