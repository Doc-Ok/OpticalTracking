/***********************************************************************
GLLineIlluminator - Class to render illuminated lines using a 2D texture
encoding Phong's lighting model.
Copyright (c) 2001-2005 Oliver Kreylos

This file is part of the OpenGL Support Library (GLSupport).

The OpenGL Support Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The OpenGL Support Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the OpenGL Support Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef GLLINEILLUMINATOR_INCLUDED
#define GLLINEILLUMINATOR_INCLUDED

#include <GL/gl.h>
#include <GL/GLColor.h>
#include <GL/GLVector.h>
#include <GL/GLMaterial.h>
#include <GL/GLObject.h>

class GLLineIlluminator:public GLObject
	{
	/* Embedded classes: */
	public:
	typedef GLVector<GLfloat,3> Vector;
	typedef GLColor<GLfloat,4> Color;
	
	private:
	enum MaterialType
		{
		NONE,INTENSITY,RGBA
		};
	
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		public:
		unsigned int materialVersion; // Version of material currently encoded in material texture
		MaterialType materialType; // Type of current material
		GLuint materialTextureId; // ID of material texture object
		
		/* Constructors and destructors: */
		DataItem(void)
			:materialVersion(0),
			 materialType(NONE)
			{
			glGenTextures(1,&materialTextureId);
			}
		~DataItem(void)
			{
			glDeleteTextures(1,&materialTextureId);
			}
		};
	
	/* Elements: */
	private:
	unsigned int materialVersion; // Version of material currently stored in object
	MaterialType materialType; // Current material type
	GLMaterial material; // Current material properties
	Vector sceneCenter; // Central point used to convert origin points to directions
	bool autoViewDirection; // If true, calculate view direction from GL matrices
	Vector viewDirection; // Caller-provided view direction
	bool autoLightDirection; // If true, calculate light direction from GL light source
	GLsizei autoLightIndex; // Index of GL light source used for illumination
	Vector lightDirection; // Caller-provided light direction
	
	/* Private methods: */
	void updateMaterial(DataItem* dataItem) const;
	
	/* Constructors and destructors: */
	public:
	GLLineIlluminator(void);
	
	/* Methods: */
	virtual void initContext(GLContextData& contextData) const;
	
	/* Methods to set the current line material: */
	void setMaterial(GLfloat ambient,GLfloat diffuse,GLfloat specular,GLfloat shininess);
	void setMaterial(const Color& ambient,const Color& diffuse,const Color& specular,GLfloat shininess);
	void setMaterial(const GLMaterial& newMaterial);
	
	void setSceneCenter(const Vector& newSceneCenter) // Sets the scene center used to convert points to vectors (in current model coordinates)
		{
		sceneCenter=newSceneCenter;
		}
	void enableAutoView(void) // Enables automatic calculation of view direction
		{
		autoViewDirection=true;
		}
	void disableAutoView(void) // Disables automatic calculation of view direction
		{
		autoViewDirection=false;
		}
	void setViewDirection(const Vector& newViewDirection); // Sets a view direction (in current model coordinates) and disables auto view direction
	void enableAutoLight(GLenum lightIndex) // Enables automatic calculation of light direction
		{
		autoLightDirection=true;
		autoLightIndex=lightIndex-GL_LIGHT0;
		}
	void disableAutoLight(void) // Disables automatic calculation of light direction
		{
		autoLightDirection=false;
		}
	void setLightDirection(const Vector& newLightDirection); // Sets a light direction (in current model coordinates) and disables auto light direction
	void enableLighting(GLContextData& contextData) const; // Sets up GL state to render illuminated lines
	void disableLighting(GLContextData& contextData) const; // Turns off illuminated line rendering
	};

#endif
