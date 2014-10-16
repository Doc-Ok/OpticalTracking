/***********************************************************************
SingleChildContainer - Base class for containers that contain at most
one child.
Copyright (c) 2008-2010 Oliver Kreylos

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

#include <GLMotif/SingleChildContainer.h>

#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLVertexTemplates.h>
#include <GLMotif/Event.h>

namespace GLMotif {

/*************************************
Methods of class SingleChildContainer:
*************************************/

Vector SingleChildContainer::calcInteriorSize(const Vector& childSize) const
	{
	/* Return the child's size unchanged: */
	return childSize;
	}

Box SingleChildContainer::calcChildBox(const Box& interior) const
	{
	/* Return the interior box unchanged: */
	return interior;
	}

SingleChildContainer::SingleChildContainer(const char* sName,Container* sParent,bool sManageChild)
	:Container(sName,sParent,false),
	 child(0)
	{
	/* Manage me: */
	if(sManageChild)
		manageChild();
	}

SingleChildContainer::~SingleChildContainer(void)
	{
	deleteChild(child);
	}

Vector SingleChildContainer::calcNaturalSize(void) const
	{
	if(child!=0)
		return calcExteriorSize(calcInteriorSize(child->calcNaturalSize()));
	else
		return calcExteriorSize(calcInteriorSize(Vector(0.0f,0.0f,0.0f)));
	}

ZRange SingleChildContainer::calcZRange(void) const
	{
	/* Calculate the parent class widget's z range: */
	ZRange myZRange=Container::calcZRange();
	
	/* Check the child widget: */
	if(child!=0)
		myZRange+=child->calcZRange();
	
	return myZRange;
	}

void SingleChildContainer::resize(const Box& newExterior)
	{
	/* Resize the parent class widget: */
	Container::resize(newExterior);
	
	/* Resize the child: */
	if(child!=0)
		{
		/* Resize and reposition the child widget based on the new interior size: */
		child->resize(calcChildBox(getInterior()));
		}
	}

void SingleChildContainer::draw(GLContextData& contextData) const
	{
	/* Draw the parent class widget: */
	Container::draw(contextData);
	
	if(child!=0)
		{
		/* Draw the child widget: */
		child->draw(contextData);
		}
	else
		{
		/* Fill the widget's interior with the background color: */
		glBegin(GL_QUADS);
		glColor(backgroundColor);
		glNormal3f(0.0f,0.0f,1.0f);
		glVertex(getInterior().getCorner(0));
		glVertex(getInterior().getCorner(1));
		glVertex(getInterior().getCorner(3));
		glVertex(getInterior().getCorner(2));
		glEnd();
		}
	}

bool SingleChildContainer::findRecipient(Event& event)
	{
	/* Distribute the question to the child widget: */
	if(child==0||!child->findRecipient(event))
		{
		/* Check ourselves: */
		Event::WidgetPoint wp=event.calcWidgetPoint(this);
		if(isInside(wp.getPoint()))
			return event.setTargetWidget(this,wp);
		else
			return false;
		}
	else
		return true;
	}

void SingleChildContainer::addChild(Widget* newChild)
	{
	/* Delete the current child: */
	deleteChild(child);
	
	/* Add the new child: */
	child=newChild;
	
	if(isManaged)
		{
		/* Try to resize the widget to accomodate the new child: */
		parent->requestResize(this,calcNaturalSize());
		}
	}

void SingleChildContainer::removeChild(Widget* removeChild)
	{
	/* Check if the given widget is really the child: */
	if(child!=0&&child==removeChild)
		{
		/* Tell the child that it is being removed: */
		child->unmanageChild();
		
		/* Remove the child: */
		child=0;
		
		/* Notify parent widgets that the visual representation has changed: */
		update();
		}
	}

void SingleChildContainer::requestResize(Widget* child,const Vector& newExteriorSize)
	{
	/* Just grant the request if nothing really changed: */
	if(!isManaged)
		{
		/* Resize the child without directly: */
		child->resize(Box(child->getExterior().origin,newExteriorSize));
		}
	else if(newExteriorSize[0]==child->getExterior().size[0]&&newExteriorSize[1]==child->getExterior().size[1])
		{
		/* Resize the child at its current position and size: */
		child->resize(child->getExterior());
		
		/* Notify parent widgets that the visual representation has changed: */
		update();
		}
	else
		{
		/* Calculate the new preferred exterior size: */
		Vector myExteriorSize=calcExteriorSize(calcInteriorSize(newExteriorSize));
		
		/* Try adjusting the widget size to accomodate the child's new size: */
		parent->requestResize(this,myExteriorSize);
		}
	}

Widget* SingleChildContainer::getFirstChild(void)
	{
	/* Return the only child: */
	return child;
	}

Widget* SingleChildContainer::getNextChild(Widget*)
	{
	/* Since there is only one child, always return null: */
	return 0;
	}

}
