/***********************************************************************
VRMLRenderState - Structure describing the current state of the VRML
rendering context.
Copyright (c) 2008 Oliver Kreylos

This file is part of the Virtual Reality VRML viewer (VRMLViewer).

The Virtual Reality VRML viewer is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Virtual Reality VRML viewer is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Virtual Reality VRML viewer; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef VRMLRENDERSTATE_INCLUDED
#define VRMLRENDERSTATE_INCLUDED

#include <vector>
#include <GL/gl.h>
#include <GL/GLFrustum.h>
#include <GL/GLTransformationWrappers.h>

#include "Types.h"

/* Forward declarations: */
class GLContextData;

struct VRMLRenderState
	{
	/* Embedded classes: */
	public:
	typedef GLFrustum<GLfloat> Frustum; // Class describing the rendering context's view frustum
	typedef std::vector<Transformation> TransformationStack; // Type for stacks of transformations
	
	/* Elements: */
	GLContextData& contextData; // The OpenGL context data structure
	Frustum frustum; // The rendering context's view frustum
	Point baseViewerPos; // Viewer position in initial model coordinates
	Vector baseUpVector; // Up vector in initial model coordinates
	TransformationStack transformStack; // A stack of transformations mirroring the OpenGL modelview matrix stack
	Point viewerPos; // Viewer position in current model coordinates
	Vector upVector; // Up vector in current model coordinates
	
	/* Constructors and destructors: */
	VRMLRenderState(GLContextData& sContextData,const Point& sBaseViewerPos,const Vector& sBaseUpVector)
		:contextData(sContextData),
		 baseViewerPos(sBaseViewerPos),
		 baseUpVector(sBaseUpVector)
		{
		/* Read the view frustum from the current OpenGL context: */
		frustum.setFromGL();
		
		/* Initialize the transformation stack: */
		transformStack.push_back(Transformation::identity);
		viewerPos=baseViewerPos;
		upVector=baseUpVector;
		}
	
	/* Methods: */
	void pushTransform(const Transformation& transform) // Pushes a transformation onto the transformation stack
		{
		/* Push the transformation onto the OpenGL matrix stack: */
		glPushMatrix();
		glMultMatrix(transform);
		
		/* Push the transformation onto the mirrored matrix stack: */
		Transformation newTransform=transformStack.back()*transform;
		newTransform.renormalize();
		transformStack.push_back(newTransform);
		
		/* Update the eye position and up vector: */
		viewerPos=newTransform.inverseTransform(baseViewerPos);
		upVector=newTransform.inverseTransform(baseUpVector);
		}
	void popTransform(void) // Pops the top transformation from the transformation stack
		{
		/* Pop the top OpenGL matrix: */
		glPopMatrix();
		
		/* Pop the top transformation: */
		transformStack.pop_back();
		
		/* Update the eye position and up vector: */
		viewerPos=transformStack.back().inverseTransform(baseViewerPos);
		upVector=transformStack.back().inverseTransform(baseUpVector);
		}
	};

#endif
