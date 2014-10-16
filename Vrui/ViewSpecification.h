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

#ifndef VRUI_VIEWSPECIFICATION_INCLUDED
#define VRUI_VIEWSPECIFICATION_INCLUDED

#include <Math/Math.h>
#include <Geometry/Point.h>
#include <Geometry/Plane.h>
#include <Vrui/Geometry.h>

namespace Vrui {

class ViewSpecification
	{
	/* Elements: */
	private:
	int viewportSize[2]; // Size of window's viewport in pixels
	Plane screenPlane; // Plane containing the screen
	Scalar screenSize[2]; // Size of the screen
	Scalar pixelSize; // (Approximate) size of a pixel
	Point eye; // Position of eye
	Scalar eyeScreenDist; // Distance from eye to screen
	Point frustumVertices[8]; // Position of the eight corner vertices of the view frustum
	Plane frustumPlanes[6]; // Plane equations of the six faces of the view frustum (left, right, bottom, top, front, back)
	
	/* Methods: */
	public:
	void setViewportSize(const int newViewportSize[2]) // Sets viewport size
		{
		for(int i=0;i<2;++i)	
			viewportSize[i]=newViewportSize[i];
		
		/* Recalculate the pixel size: */
		pixelSize=Math::sqrt((Scalar(viewportSize[0])/screenSize[0])*(Scalar(viewportSize[1]/screenSize[1])));
		}
	void setScreenPlane(const Plane& newScreenPlane) // Sets the screen plane
		{
		screenPlane=newScreenPlane;
		}
	void setScreenSize(const Scalar newScreenSize[2]) // Sets screen size
		{
		for(int i=0;i<2;++i)	
			screenSize[i]=newScreenSize[i];
		
		/* Recalculate the pixel size: */
		pixelSize=Math::sqrt((Scalar(viewportSize[0])/screenSize[0])*(Scalar(viewportSize[1]/screenSize[1])));
		}
	void setEye(const Point& newEye) // Sets the eye position
		{
		eye=newEye;
		}
	void setEyeScreenDistance(Scalar newEyeScreenDist) // Sets the eye-screen distance
		{
		eyeScreenDist=newEyeScreenDist;
		}
	void setFrustumVertex(int vertexIndex,const Point& newVertex) // Sets a frustum vertex
		{
		frustumVertices[vertexIndex]=newVertex;
		}
	void setFrustumPlane(int planeIndex,const Plane& newPlane) // Sets a frustum plane
		{
		frustumPlanes[planeIndex]=newPlane;
		}
	void setFromGL(void); // Sets the view specification from the current OpenGL State
	const int* getViewportSize(void) const // Returns viewport size as array
		{
		return viewportSize;
		}
	int getViewportSize(int dimension) const // Returns viewport size along one dimension
		{
		return viewportSize[dimension];
		}
	const Plane& getScreenPlane(void) const // Returns the plane containing the screen
		{
		return screenPlane;
		}
	const Scalar* getScreenSize(void) const // Returns screen size as array
		{
		return screenSize;
		}
	Scalar getScreenSize(int dimension) const // Returns screen size along one dimension
		{
		return screenSize[dimension];
		}
	const Point& getEye(void) const // Returns the eye position
		{
		return eye;
		}
	Scalar getEyeScreenDistance(void) const // Returns the eye-screen distance
		{
		return eyeScreenDist;
		}
	const Point& getFrustumVertex(int vertexIndex) const // Returns one of the frustum corner vertices
		{
		return frustumVertices[vertexIndex];
		}
	const Plane& getFrustumPlane(int planeIndex) const // Returns one of the frustum face planes
		{
		return frustumPlanes[planeIndex];
		}
	Scalar calcProjectedRadius(const Point& sphereCenter,Scalar sphereRadius) const // Returns the (approximate) projected radius of a sphere in pixel units
		{
		/* Calculate distance from screen to sphere center: */
		Scalar sphereScreenDist=screenPlane.calcDistance(sphereCenter);
		
		/* Calculate projection scale: */
		Scalar scale=eyeScreenDist/(eyeScreenDist-sphereScreenDist);
		
		/* Return pixel size of projected sphere: */
		return (sphereRadius*scale)*pixelSize;
		}
	};

}

#endif
