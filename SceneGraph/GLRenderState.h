/***********************************************************************
GLRenderState - Class encapsulating the traversal state of a scene graph
during OpenGL rendering.
Copyright (c) 2009-2013 Oliver Kreylos

This file is part of the Simple Scene Graph Renderer (SceneGraph).

The Simple Scene Graph Renderer is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Simple Scene Graph Renderer is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Simple Scene Graph Renderer; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef SCENEGRAPH_GLRENDERSTATE_INCLUDED
#define SCENEGRAPH_GLRENDERSTATE_INCLUDED

#include <Geometry/OrthogonalTransformation.h>
#include <GL/gl.h>
#include <GL/GLColor.h>
#include <GL/GLFrustum.h>
#include <SceneGraph/Geometry.h>

/* Forward declarations: */
class GLContextData;

namespace SceneGraph {

class GLRenderState
	{
	/* Embedded classes: */
	public:
	typedef GLColor<GLfloat,3> Color; // Type for RGBA colors
	typedef Geometry::OrthogonalTransformation<double,3> DOGTransform; // Double-precision orthogonal transformations as internal representations
	typedef GLFrustum<Scalar> Frustum; // Class describing the rendering context's view frustum
	
	/* Elements: */
	GLContextData& contextData; // Context data of the current OpenGL context
	private:
	Frustum baseFrustum; // The rendering context's view frustum in initial model coordinates
	Point baseViewerPos; // Viewer position in initial model coordinates
	Vector baseUpVector; // Up vector in initial model coordinates
	DOGTransform currentTransform; // Transformation from initial model coordinates to current model coordinates
	
	/* Elements shadowing current OpenGL state: */
	public:
	bool cullingEnabled;
	GLenum culledFace;
	bool lightingEnabled;
	Color emissiveColor;
	bool colorMaterialEnabled;
	int highestTexturePriority; // Priority level of highest enabled texture unit (None=-1, 1D=0, 2D, 3D, cube map)
	bool separateSpecularColorEnabled;
	
	/* Constructors and destructors: */
	GLRenderState(GLContextData& sContextData,const DOGTransform& initialTransform,const Point& sBaseViewerPos,const Vector& sBaseUpVector); // Creates a render state object
	
	/* Methods: */
	Point getViewerPos(void) const // Returns the viewer position in current model coordinates
		{
		return Point(currentTransform.inverseTransform(baseViewerPos));
		}
	Vector getUpVector(void) const // Returns the up direction in current model coordinates
		{
		return Vector(currentTransform.inverseTransform(baseUpVector));
		}
	DOGTransform pushTransform(const OGTransform& deltaTransform); // Pushes the given transformation onto the matrix stack and returns the previous transformation
	DOGTransform pushTransform(const DOGTransform& deltaTransform); // Ditto, with a double-precision transformation
	void popTransform(const DOGTransform& previousTransform); // Resets the matrix stack to the given transformation; must be result from previous pushTransform call
	bool doesBoxIntersectFrustum(const Box& box) const; // Returns true if the given box in current model coordinates intersects the view frustum
	
	/* OpenGL state management methods: */
	void enableCulling(GLenum newCulledFace); // Enables OpenGL face culling
	void disableCulling(void); // Disables OpenGL face culling
	void enableMaterials(void); // Enables OpenGL material rendering
	void disableMaterials(void); // Disables OpenGL material rendering
	void enableTexture1D(void); // Enables OpenGL 1D texture mapping
	void enableTexture2D(void); // Enables OpenGL 2D texture mapping
	void disableTextures(void); // Disables OpenGL texture mapping
	};

}

#endif
