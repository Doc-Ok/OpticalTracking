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

#ifndef SEISMICPATH_INCLUDED
#define SEISMICPATH_INCLUDED

#include <vector>
#include <GL/gl.h>
#include <GL/GLVertex.h>

/* Forward declarations: */
class GLContextData;

class SeismicPath // Class for seismic paths rendered as (illuminated?) line strips
	{
	/* Embedded classes: */
	private:
	typedef GLVertex<void,0,void,0,void,GLfloat,3> Vertex; // Type for seismic path vertices
	
	/* Elements: */
	std::vector<Vertex> vertices; // Array of vertices of the seismic path
	
	/* Constructors and destructors: */
	public:
	SeismicPath(const char* pathFileName,double scaleFactor); // Reads seismic path from ASCII file; applies scale factor to Cartesian coordinates
	~SeismicPath(void);
	
	/* Methods: */
	void glRenderAction(GLContextData& contextData) const; // Renders the seismic path
	};

#endif
