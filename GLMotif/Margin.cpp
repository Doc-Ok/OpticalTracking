/***********************************************************************
Margin - Class to create a margin around a widget, or pad a widget's
natural size into a larger frame.
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

#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLVertexTemplates.h>

#include <GLMotif/Margin.h>

namespace GLMotif {

/***********************
Methods of class Margin:
***********************/

Vector Margin::calcInteriorSize(const Vector& childSize) const
	{
	return Vector(childSize[0]+2.0f*marginWidth,childSize[1]+2.0f*marginWidth,childSize[2]);
	}

Box Margin::calcChildBox(const Box& interior) const
	{
	/* Calculate the maximum extent of the child box: */
	Box childBox=interior.inset(Vector(marginWidth,marginWidth,0.0f));
	
	/* Adjust the child box to fit the child's natural size: */
	Vector childSize=child->calcNaturalSize();
	if(childBox.size[0]>childSize[0])
		{
		/* Pad the child horizontally: */
		switch(alignment.hAlignment)
			{
			case Alignment::HFILL:
				/* Do nothing */
				break;
			
			case Alignment::LEFT:
				childBox.size[0]=childSize[0];
				break;
			
			case Alignment::HCENTER:
				childBox.origin[0]+=(childBox.size[0]-childSize[0])*0.5f;
				childBox.size[0]=childSize[0];
				break;
			
			case Alignment::RIGHT:
				childBox.origin[0]+=childBox.size[0]-childSize[0];
				childBox.size[0]=childSize[0];
				break;
			}
		}
	if(childBox.size[1]>childSize[1])
		{
		/* Pad the child vertically: */
		switch(alignment.vAlignment)
			{
			case Alignment::VFILL:
				/* Do nothing */
				break;
			
			case Alignment::BOTTOM:
				childBox.size[1]=childSize[1];
				break;
			
			case Alignment::VCENTER:
				childBox.origin[1]+=(childBox.size[1]-childSize[1])*0.5f;
				childBox.size[1]=childSize[1];
				break;
			
			case Alignment::TOP:
				childBox.origin[1]+=childBox.size[1]-childSize[1];
				childBox.size[1]=childSize[1];
				break;
			}
		}
	
	return childBox;
	}

Margin::Margin(const char* sName,Container* sParent,bool sManageChild)
	:SingleChildContainer(sName,sParent,false),
	 marginWidth(0.0f),
	 alignment(Alignment::HFILL,Alignment::VFILL)
	{
	/* Manage me: */
	if(sManageChild)
		manageChild();
	}

void Margin::draw(GLContextData& contextData) const
	{
	/* Draw the grandparent class widget: */
	Container::draw(contextData);
	
	if(child!=0)
		{
		/* Draw the margin around the child widget: */
		glBegin(GL_QUAD_STRIP);
		glColor(backgroundColor);
		glNormal3f(0.0f,0.0f,1.0f);
		glVertex(child->getExterior().getCorner(0));
		glVertex(getInterior().getCorner(0));
		glVertex(child->getExterior().getCorner(1));
		glVertex(getInterior().getCorner(1));
		glVertex(child->getExterior().getCorner(3));
		glVertex(getInterior().getCorner(3));
		glVertex(child->getExterior().getCorner(2));
		glVertex(getInterior().getCorner(2));
		glVertex(child->getExterior().getCorner(0));
		glVertex(getInterior().getCorner(0));
		glEnd();
		
		/* Draw the child widgets: */
		child->draw(contextData);
		}
	}

void Margin::setMarginWidth(GLfloat newMarginWidth)
	{
	/* Change the margin width: */
	marginWidth=newMarginWidth;
	
	if(isManaged)
		{
		/* Try adjusting the widget size to accomodate the new margin width: */
		parent->requestResize(this,calcNaturalSize());
		}
	else
		resize(Box(Vector(0.0f,0.0f,0.0f),calcNaturalSize()));
	}

void Margin::setAlignment(const Alignment& newAlignment)
	{
	/* Set the child alignment: */
	alignment=newAlignment;
	
	/* Resize the widget to the same size to update the child's padding: */
	if(child!=0)
		resize(getExterior());
	
	/* Notify parent widgets that the visual representation has changed: */
	update();
	}

}
