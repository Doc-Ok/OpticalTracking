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

/***********************************************************************
Abuse the existing non-standard template instantiation mechanism to link
the required classes and functions from the templatized geometry library
statically into this module.
***********************************************************************/

#define GEOMETRY_NONSTANDARD_TEMPLATES

#include <Math/Math.h>
#include <Geometry/HVector.h>
#include <Geometry/Matrix.h>
#include <GL/GLColorOperations.h>
#include <GL/GLGetTemplates.h>
#include <GL/GLGetLightTemplates.h>
#include <GL/GLContextData.h>

#include <GL/GLLineIlluminator.h>

/**********************************
Methods of class GLLineIlluminator:
**********************************/

void GLLineIlluminator::updateMaterial(GLLineIlluminator::DataItem* dataItem) const
	{
	/* Update the material version: */
	dataItem->materialVersion=materialVersion;
	
	/* Upload the material texture: */
	dataItem->materialType=materialType;
	if(materialType==INTENSITY)
		{
		/* Create a 2D texture map encoding Phong's lighting model: */
		static GLfloat texture[32][32];
		for(int x=0;x<32;++x)
			{
			GLfloat s=2.0f*(GLfloat(x)+0.5f)/32.0f-1.0f;
			GLfloat oneMinusS2=1.0f-s*s;
			GLfloat ambientDiffuse=material.diffuse[0];
			ambientDiffuse*=Math::pow(Math::sqrt(oneMinusS2),2.0f);
			ambientDiffuse+=material.ambient[0];
			for(int y=0;y<32;++y)
				{
				GLfloat t=2.0f*(GLfloat(y)+0.5f)/32.0f-1.0f;
				GLfloat oneMinusT2=1.0f-t*t;
				GLfloat color=material.specular[0];
				color*=Math::pow(Math::abs(Math::sqrt(oneMinusS2*oneMinusT2)-s*t),material.shininess);
				color+=ambientDiffuse;
				texture[y][x]=color;
				}
			}
		
		/* Upload the created texture: */
		glBindTexture(GL_TEXTURE_2D,dataItem->materialTextureId);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_BASE_LEVEL,0);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAX_LEVEL,0);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glPixelStorei(GL_UNPACK_ROW_LENGTH,0);
		glPixelStorei(GL_UNPACK_SKIP_ROWS,0);
		glPixelStorei(GL_UNPACK_SKIP_PIXELS,0);
		glPixelStorei(GL_UNPACK_ALIGNMENT,1);
		glTexImage2D(GL_TEXTURE_2D,0,GL_INTENSITY,32,32,0,GL_LUMINANCE,GL_FLOAT,texture);
		glBindTexture(GL_TEXTURE_2D,0);
		}
	else if(materialType==RGBA)
		{
		/* Create a 2D texture map encoding Phong's lighting model: */
		static Color texture[32][32];
		for(int x=0;x<32;++x)
			{
			GLfloat s=2.0f*(GLfloat(x)+0.5f)/32.0f-1.0f;
			GLfloat oneMinusS2=1.0f-s*s;
			Color ambientDiffuse=material.diffuse;
			ambientDiffuse*=Math::pow(Math::sqrt(oneMinusS2),2.0f);
			ambientDiffuse+=material.ambient;
			for(int y=0;y<32;++y)
				{
				GLfloat t=2.0f*(GLfloat(y)+0.5f)/32.0f-1.0f;
				GLfloat oneMinusT2=1.0f-t*t;
				Color color=material.specular;
				color*=Math::pow(Math::abs(Math::sqrt(oneMinusS2*oneMinusT2)-s*t),material.shininess);
				color+=ambientDiffuse;
				texture[y][x]=color;
				}
			}
		
		/* Upload the created texture: */
		glBindTexture(GL_TEXTURE_2D,dataItem->materialTextureId);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_BASE_LEVEL,0);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAX_LEVEL,0);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glPixelStorei(GL_UNPACK_ROW_LENGTH,0);
		glPixelStorei(GL_UNPACK_SKIP_ROWS,0);
		glPixelStorei(GL_UNPACK_SKIP_PIXELS,0);
		glPixelStorei(GL_UNPACK_ALIGNMENT,1);
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,32,32,0,GL_RGBA,GL_FLOAT,texture);
		glBindTexture(GL_TEXTURE_2D,0);
		}
	}

GLLineIlluminator::GLLineIlluminator(void)
	:materialVersion(0),
	 materialType(NONE),
	 sceneCenter(0.0,0.0,0.0),
	 autoViewDirection(true),
	 autoLightDirection(true),autoLightIndex(0)
	{
	}

void GLLineIlluminator::initContext(GLContextData& contextData) const
	{
	/* Create a new data item: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	}

void GLLineIlluminator::setMaterial(GLfloat ambient,GLfloat diffuse,GLfloat specular,GLfloat shininess)
	{
	/* Update the material version: */
	++materialVersion;
	
	/* Store the material properties: */
	materialType=INTENSITY;
	material.ambient[0]=ambient;
	material.diffuse[0]=diffuse;
	material.specular[0]=specular;
	material.shininess=shininess;
	}

void GLLineIlluminator::setMaterial(const GLLineIlluminator::Color& ambient,const GLLineIlluminator::Color& diffuse,const GLLineIlluminator::Color& specular,GLfloat shininess)
	{
	/* Update the material version: */
	++materialVersion;
	
	/* Store the material properties: */
	materialType=RGBA;
	material.ambient=ambient;
	material.diffuse=diffuse;
	material.specular=specular;
	material.shininess=shininess;
	}

void GLLineIlluminator::setMaterial(const GLMaterial& newMaterial)
	{
	/* Update the material version: */
	++materialVersion;
	
	/* Store the material properties: */
	materialType=RGBA;
	material=newMaterial;
	}

void GLLineIlluminator::setViewDirection(const GLLineIlluminator::Vector& newViewDirection)
	{
	autoViewDirection=false;
	viewDirection=newViewDirection;
	GLfloat viewDirectionLen=Math::sqrt(Math::sqr(viewDirection[0])+Math::sqr(viewDirection[1])+Math::sqr(viewDirection[2]));
	viewDirection[0]/=viewDirectionLen;
	viewDirection[1]/=viewDirectionLen;
	viewDirection[2]/=viewDirectionLen;
	}

void GLLineIlluminator::setLightDirection(const GLLineIlluminator::Vector& newLightDirection)
	{
	autoLightDirection=false;
	lightDirection=newLightDirection;
	GLfloat lightDirectionLen=Math::sqrt(Math::sqr(lightDirection[0])+Math::sqr(lightDirection[1])+Math::sqr(lightDirection[2]));
	lightDirection[0]/=lightDirectionLen;
	lightDirection[1]/=lightDirectionLen;
	lightDirection[2]/=lightDirectionLen;
	}

void GLLineIlluminator::enableLighting(GLContextData& contextData) const
	{
	/* Get a pointer to the context data item: */
	DataItem* dataItem=contextData.retrieveDataItem<DataItem>(this);
	
	/* Update the material texture if it is outdated: */
	if(dataItem->materialVersion!=materialVersion)
		updateMaterial(dataItem);
	
	GLenum previousMatrixMode=glGet<GLint>(GL_MATRIX_MODE);
	
	Geometry::Matrix<GLfloat,4,4> modelView;
	if(autoViewDirection||autoLightDirection)
		{
		/* Get the modelview matrix from OpenGL: */
		GLfloat matrixArray[16];
		glGetFloatv(GL_MODELVIEW_MATRIX,matrixArray);
		modelView=Geometry::Matrix<GLfloat,4,4>::fromColumnMajor(matrixArray);
		}
	
	/* Determine the view direction: */
	Geometry::ComponentArray<GLfloat,3> viewDir(viewDirection.getXyzw());
	if(autoViewDirection)
		{
		/* Get the projection matrix from OpenGL: */
		GLfloat matrixArray[16];
		glGetFloatv(GL_PROJECTION_MATRIX,matrixArray);
		Geometry::Matrix<GLfloat,4,4> projection=Geometry::Matrix<GLfloat,4,4>::fromColumnMajor(matrixArray);
		
		/* Calculate the view direction from the OpenGL projection and modelview matrices: */
		Geometry::ComponentArray<GLfloat,4> viewPos(0.0f,0.0f,1.0f,0.0f);
		viewPos=viewPos/projection;
		viewPos=viewPos/modelView;
		
		/* Check if it's an orthogonal or perspective projection: */
		if(Math::abs(viewPos[3])<1.0e-8f)
			{
			/* Just copy the view direction: */
			viewDir=viewPos;
			}
		else
			{
			/* Calculate the direction from the view point to the scene center: */
			for(int i=0;i<3;++i)
				viewDir[i]=viewPos[i]/viewPos[3]-sceneCenter[i];
			}
		GLfloat viewDirLen=GLfloat(Geometry::mag(viewDir));
		for(int i=0;i<3;++i)
			viewDir[i]/=viewDirLen;
		}
	
	/* Determine the light direction: */
	Geometry::ComponentArray<GLfloat,3> lightDir(lightDirection.getXyzw());
	if(autoLightDirection)
		{
		/* Query the light direction from OpenGL and transform it to model coordinates: */
		Geometry::ComponentArray<GLfloat,4> lightPos;
		glGetLightPosition(autoLightIndex,lightPos.getComponents());
		lightPos=lightPos/modelView;
		
		/* Check if it's a directional or point light: */
		if(Math::abs(lightPos[3])<1.0e-8f)
			{
			/* Just copy the light direction: */
			lightDir=lightPos;
			}
		else
			{
			/* Calculate the direction from the light source to the scene center: */
			for(int i=0;i<3;++i)
				lightDir[i]=lightPos[i]/lightPos[3]-sceneCenter[i];
			}
		GLfloat lightDirLen=GLfloat(Geometry::mag(lightDir));
		for(int i=0;i<3;++i)
			lightDir[i]/=lightDirLen;
		}
	
	/* Set up the OpenGL texture matrix: */
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	GLfloat matrix[4][4];
	for(int j=0;j<3;++j)
		{
		matrix[j][0]=lightDir[j];
		matrix[j][1]=viewDir[j];
		matrix[j][2]=0.0f;
		matrix[j][3]=0.0f;
		}
	matrix[3][0]=1.0f;
	matrix[3][1]=1.0f;
	matrix[3][2]=0.0f;
	matrix[3][3]=2.0f;
	glLoadMatrixf((const GLfloat*)matrix);
	
	/* Set the OpenGL rendering mode: */
	glPushAttrib(GL_TEXTURE_BIT);
	glBindTexture(GL_TEXTURE_2D,dataItem->materialTextureId);
	glEnable(GL_TEXTURE_2D);
	if(dataItem->materialType==INTENSITY)
		glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
	else
		glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
	
	/* Clean up: */
	glMatrixMode(previousMatrixMode);
	}

void GLLineIlluminator::disableLighting(GLContextData&) const
	{
	GLenum previousMatrixMode=glGet<GLint>(GL_MATRIX_MODE);
	
	/* Reset the texture matrix: */
	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
	
	/* Reset the OpenGL rendering mode: */
	glPopAttrib();
	
	/* Clean up: */
	glMatrixMode(previousMatrixMode);
	}
