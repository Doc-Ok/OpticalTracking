/***********************************************************************
VideoPane - A GLMotif widget to display video streams in Y'CbCr 4:2:0
pixel format.
Copyright (c) 2010 Oliver Kreylos

This file is part of the Basic Video Library (Video).

The Basic Video Library is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as published
by the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

The Basic Video Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Basic Video Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <Video/VideoPane.h>

#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLTexCoordTemplates.h>
#include <GL/GLVertexTemplates.h>
#include <GLMotif/Container.h>

namespace GLMotif {

/**************************
Methods of class VideoPane:
**************************/

VideoPane::VideoPane(const char* sName,Container* sParent,bool sManageChild)
	:Widget(sName,sParent,false),
	 preferredSize(0.0f,0.0f,0.0f)
	{
	/* Manage me: */
	if(sManageChild)
		manageChild();
	}

Vector VideoPane::calcNaturalSize(void) const
	{
	return calcExteriorSize(preferredSize);
	}

void VideoPane::resize(const Box& newExterior)
	{
	/* Resize the parent class widget: */
	Widget::resize(newExterior);
	
	/* Compare the widget's aspect ratio to the frame's aspect ratio: */
	frame=getInterior();
	if(frame.size[0]*GLfloat(texture.getFrameHeight())>=GLfloat(texture.getFrameWidth())*frame.size[1]) // Widget is wider than frame
		{
		/* Center the frame horizontally: */
		GLfloat newFrameWidth=frame.size[1]*GLfloat(texture.getFrameWidth())/GLfloat(texture.getFrameHeight());
		frame.origin[0]+=(frame.size[0]-newFrameWidth)*0.5f;
		frame.size[0]=newFrameWidth;
		}
	else // Widget is taller than frame
		{
		/* Center the frame vertically: */
		GLfloat newFrameHeight=frame.size[0]*GLfloat(texture.getFrameHeight())/GLfloat(texture.getFrameWidth());
		frame.origin[1]+=(frame.size[1]-newFrameHeight)*0.5f;
		frame.size[1]=newFrameHeight;
		}
	}

void VideoPane::draw(GLContextData& contextData) const
	{
	/* Draw the parent class widget: */
	Widget::draw(contextData);
	
	/* Draw a margin around the frame with the background color: */
	glColor(backgroundColor);
	glBegin(GL_QUAD_STRIP);
	glNormal3f(0.0f,0.0f,1.0f);
	glVertex(frame.getCorner(0));
	glVertex(getInterior().getCorner(0));
	glVertex(frame.getCorner(1));
	glVertex(getInterior().getCorner(1));
	glVertex(frame.getCorner(3));
	glVertex(getInterior().getCorner(3));
	glVertex(frame.getCorner(2));
	glVertex(getInterior().getCorner(2));
	glVertex(frame.getCorner(0));
	glVertex(getInterior().getCorner(0));
	glEnd();
	
	if(texture.haveFrame())
		{
		/* Install the frame texture: */
		GLfloat tc[2];
		texture.install(contextData,tc);
		
		/* Draw the current video frame: */
		glBegin(GL_QUADS);
		glTexCoord(0.0f,tc[1]);
		glVertex(frame.getCorner(0));
		glTexCoord(tc[0],tc[1]);
		glVertex(frame.getCorner(1));
		glTexCoord(tc[0],0.0f);
		glVertex(frame.getCorner(3));
		glTexCoord(0.0f,0.0f);
		glVertex(frame.getCorner(2));
		glEnd();
		
		/* Disable the frame texture: */
		texture.uninstall(contextData);
		}
	else
		{
		/* Fill the frame with the background color: */
		glColor(backgroundColor);
		glBegin(GL_QUADS);
		glNormal3f(0.0f,0.0f,1.0f);
		glVertex(frame.getCorner(0));
		glVertex(frame.getCorner(1));
		glVertex(frame.getCorner(3));
		glVertex(frame.getCorner(2));
		glEnd();
		}
	}

void VideoPane::setPreferredSize(const Vector& newPreferredSize)
	{
	/* Set the new preferred size: */
	preferredSize=newPreferredSize;
	
	if(isManaged)
		{
		/* Try adjusting the widget size to accomodate the new preferred size: */
		parent->requestResize(this,calcNaturalSize());
		}
	else
		resize(Box(Vector(0.0f,0.0f,0.0f),calcNaturalSize()));
	}

}
