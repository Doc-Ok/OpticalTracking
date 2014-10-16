/***********************************************************************
ViewSpecification - Class to represent the viewing specification of a VR
display window.
Copyright (c) 2005-2013 Oliver Kreylos

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

#include <Geometry/HVector.h>
#include <Geometry/ProjectiveTransformation.h>
#include <GL/gl.h>
#include <GL/GLGeometryWrappers.h>
#include <GL/GLTransformationWrappers.h>

#include <Vrui/ViewSpecification.h>

namespace Vrui {

/**********************************
Methods of class ViewSpecification:
**********************************/

void ViewSpecification::setFromGL(void)
	{
	typedef PTransform::HVector HVector;
	
	/* Get viewport size from OpenGL: */
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT,viewport);
	for(int i=0;i<2;++i)
		viewportSize[i]=viewport[i+2];
	
	/* Read projection and modelview matrices from OpenGL: */
	PTransform pmv=glGetProjectionMatrix<Scalar>();
	pmv*=glGetModelviewMatrix<Scalar>();
	
	/* Use the frustum near plane as the screen plane for lack of more information: */
	Vector viewPlaneX=pmv.inverseTransform(HVector(1,0,0,0)).toVector();
	Vector viewPlaneY=pmv.inverseTransform(HVector(0,1,0,0)).toVector();
	Point viewPlaneO=pmv.inverseTransform(HVector(0,0,-1,1)).toPoint();
	screenPlane=Plane(viewPlaneX^viewPlaneY,viewPlaneO);
	screenPlane.normalize();
	
	/* Calculate the eye point: */
	eye=pmv.inverseTransform(HVector(0,0,1,0)).toPoint(); // This only works for perspective transformations!
	
	/* Calculate eye-screen distance: */
	eyeScreenDist=screenPlane.calcDistance(eye);
	
	/* Calculate the eight frustum vertices: */
	frustumVertices[0]=pmv.inverseTransform(HVector(-1,-1,-1,1)).toPoint();
	frustumVertices[1]=pmv.inverseTransform(HVector( 1,-1,-1,1)).toPoint();
	frustumVertices[2]=pmv.inverseTransform(HVector(-1, 1,-1,1)).toPoint();
	frustumVertices[3]=pmv.inverseTransform(HVector( 1, 1,-1,1)).toPoint();
	frustumVertices[4]=pmv.inverseTransform(HVector(-1,-1, 1,1)).toPoint();
	frustumVertices[5]=pmv.inverseTransform(HVector( 1,-1, 1,1)).toPoint();
	frustumVertices[6]=pmv.inverseTransform(HVector(-1, 1, 1,1)).toPoint();
	frustumVertices[7]=pmv.inverseTransform(HVector( 1, 1, 1,1)).toPoint();
	
	/* Calculate the pixel size: */
	screenSize[0]=Geometry::dist(frustumVertices[0],frustumVertices[1]);
	screenSize[1]=Geometry::dist(frustumVertices[0],frustumVertices[2]);
	pixelSize=Math::sqrt((Scalar(viewportSize[0])/screenSize[0])*(Scalar(viewportSize[1]/screenSize[1])));
	
	/* Calculate the six frustum face planes: */
	frustumPlanes[0]=Plane((frustumVertices[4]-frustumVertices[0])^(frustumVertices[2]-frustumVertices[0]),frustumVertices[0]);
	frustumPlanes[1]=Plane((frustumVertices[3]-frustumVertices[1])^(frustumVertices[5]-frustumVertices[1]),frustumVertices[1]);
	frustumPlanes[2]=Plane((frustumVertices[1]-frustumVertices[0])^(frustumVertices[4]-frustumVertices[0]),frustumVertices[0]);
	frustumPlanes[3]=Plane((frustumVertices[6]-frustumVertices[2])^(frustumVertices[3]-frustumVertices[2]),frustumVertices[2]);
	frustumPlanes[4]=Plane((frustumVertices[2]-frustumVertices[0])^(frustumVertices[1]-frustumVertices[0]),frustumVertices[0]);
	frustumPlanes[5]=Plane((frustumVertices[5]-frustumVertices[4])^(frustumVertices[6]-frustumVertices[4]),frustumVertices[4]);
	for(int i=0;i<6;++i)
		frustumPlanes[i].normalize();
	}

}
