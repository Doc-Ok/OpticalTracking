/***********************************************************************
SeismicPath - Class to represent and render seismic wave propagation
paths in the Earth's interior.
Copyright (c) 2005 Oliver Kreylos

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <stdio.h>
#include <Misc/File.h>
#include <Math/Math.h>
#include <GL/gl.h>
#include <GL/GLVertexTemplates.h>

#include "EarthFunctions.h"

#include "SeismicPath.h"

/****************************
Methods of class SeismicPath:
****************************/

SeismicPath::SeismicPath(const char* pathFileName,double scaleFactor)
	{
	/* Open the path file: */
	Misc::File pathFile(pathFileName,"rt");
	
	/* Read all lines from the path file: */
	char line[256];
	while(!pathFile.eof())
		{
		/* Read the next line from the file: */
		pathFile.gets(line,sizeof(line));
		
		/* Parse the spherical point coordinates from the file: */
		float lat,lng,rad;
		if(sscanf(line,"%f %f %f",&lat,&lng,&rad)==3)
			{
			/* Convert spherical coordinates to Cartesian coordinates: */
			Vertex v;
			calcRadiusPos(Math::rad(lat),Math::rad(lng),rad*1000.0f,scaleFactor,v.position.getXyzw());
			
			/* Append the point to the seismic path: */
			vertices.push_back(v);
			}
		}
	}

SeismicPath::~SeismicPath(void)
	{
	/* Nothing to do, incidentally */
	}

void SeismicPath::glRenderAction(GLContextData& contextData) const
	{
	/* Save and set up OpenGL state: */
	GLboolean lightingEnabled=glIsEnabled(GL_LIGHTING);
	if(lightingEnabled)
		glDisable(GL_LIGHTING);
	
	/* Render the seismic path as a line strip: */
	glBegin(GL_LINE_STRIP);
	for(std::vector<Vertex>::const_iterator vIt=vertices.begin();vIt!=vertices.end();++vIt)
		glVertex(*vIt);
	glEnd();
	
	/* Restore OpenGL state: */
	if(lightingEnabled)
		glEnable(GL_LIGHTING);
	}
