/***********************************************************************
JelloRenderer - Class render Jell-O crystals as translucent blocks.
Copyright (c) 2007 Oliver Kreylos

This file is part of the Virtual Jell-O interactive VR demonstration.

Virtual Jell-O is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

Virtual Jell-O is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with Virtual Jell-O; if not, write to the Free Software Foundation,
Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef JELLORENDERER_INCLUDED
#define JELLORENDERER_INCLUDED

#include <Geometry/SplinePatch.h>
#include <GL/gl.h>
#include <GL/GLColor.h>
#include <GL/GLVertex.h>
#include <GL/GLMaterial.h>
#include <GL/GLObject.h>
#include <Vrui/TransparentObject.h>

#include "JelloCrystal.h"

class JelloRenderer:public GLObject,public Vrui::TransparentObject
	{
	/* Embedded classes: */
	public:
	typedef JelloCrystal::Scalar Scalar;
	typedef JelloCrystal::Point Point;
	typedef JelloCrystal::Vector Vector;
	typedef JelloCrystal::Box Box;
	typedef JelloCrystal::Index Index;
	private:
	typedef Geometry::SplinePatch<Scalar,3> SplinePatch; // Type for spline patches to render the crystal's faces
	typedef GLVertex<void,0,void,0,GLfloat,GLfloat,3> Vertex; // Type for render vertices
	
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		public:
		bool hasVertexBufferObjectExtension; // Flag if the local OpenGL supports the ARB vertex buffer object extension
		GLuint faceVertexBufferObjectIDs[6]; // Array of vertex buffer object IDs for the Jell-O faces
		GLuint faceIndexBufferObjectIDs[6]; // Array of index buffer object IDs for the Jell-O faces
		unsigned int splineParameterVersion; // Version number of state dependent on spline patch parameters
		unsigned int vertexDataVersion; // Version number of the face data in the vertex buffers
		
		/* Constructors and destructors: */
		DataItem(void);
		virtual ~DataItem(void);
		};
	
	/* Elements: */
	const JelloCrystal* crystal; // Pointer to the rendered Jell-O crystal
	bool renderDomainBox; // Flag whether to render the Jell-O domain box's outline
	GLColor<GLfloat,3> domainBoxColor; // Color to draw the domain box
	int surfaceDegree; // Polynomial degree of surface spline patches
	SplinePatch* faces[6]; // The six spline patches for the crystal's faces
	SplinePatch::EvaluationCache* evcs[6]; // Evaluation caches for the face spline patches
	int numVerticesPerSegment; // Number of vertices per spline segment
	SplinePatch::Size numVertices[6]; // The number of evaluation vertices for each face
	unsigned int splineParameterVersion; // Version number of spline patch parameters (spline degree, subdivision level)
	GLMaterial faceMaterial; // Material for rendering the face spline patches
	bool active; // Flag if this Jell-O renderer will render its transparent pass
	GLuint* indices[6]; // Arrays of quad strip indices for each face
	Vertex* vertices[6]; // 2D arrays of vertices for each face
	unsigned int vertexDataVersion; // Version number of the face data in the vertex buffers
	
	/* Private methods: */
	void createFacePatches(void); // Creates the six spline patches to render a block of Jell-O
	
	/* Constructors and destructors: */
	public:
	JelloRenderer(const JelloCrystal& sCrystal); // Creates a renderer for the given Jell-O crystal
	virtual ~JelloRenderer(void); // Destroys the renderer
	
	/* Methods: */
	void setRenderDomainBox(bool newRenderDomainBox); // Sets whether to draw the domain box outline
	void setDomainBoxColor(const GLColor<GLfloat,3>& newDomainBoxColor); // Updates the domain box color
	void setSurfaceDegree(int newSurfaceDegree); // Sets the polynomial degree of the face spline patches
	void setTesselation(int newNumVerticesPerSegment); // Sets the tesselation level of the face spline patches
	void setFaceMaterial(const GLMaterial& newFaceMaterial); // Sets the material properties of the Jell-O block's face splines
	virtual void initContext(GLContextData& contextData) const;
	void setCrystal(const JelloCrystal* newCrystal); // Associates the renderer with a new Jello crystal of the same dimensions; requires calling update() before the next rendering
	void setActive(bool newActive); // Activates or deactivates the renderer
	void update(void); // Updates the face splines to represent the new state of the Jell-O crystal; must be called at least once before the first rendering
	void glRenderAction(GLContextData& contextData) const; // Renders the opaque parts of the most recently updated state of the Jell-O crystal
	void glRenderActionTransparent(GLContextData& contextData) const; // Renders the transparent part of the most recently updated state of the Jell-O crystal
	};

#endif
