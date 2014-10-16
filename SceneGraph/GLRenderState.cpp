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

#include <SceneGraph/GLRenderState.h>

#include <GL/gl.h>
#include <GL/GLTexEnvTemplates.h>
#include <GL/GLTransformationWrappers.h>

namespace SceneGraph {

/******************************
Methods of class GLRenderState:
******************************/

GLRenderState::GLRenderState(GLContextData& sContextData,const GLRenderState::DOGTransform& initialTransform,const Point& sBaseViewerPos,const Vector& sBaseUpVector)
	:contextData(sContextData),
	 baseViewerPos(sBaseViewerPos),baseUpVector(sBaseUpVector),
	 currentTransform(initialTransform),
	 emissiveColor(0.0f,0.0f,0.0f)
	{
	/* Install the initial transformation: */
	glLoadMatrix(currentTransform);
	
	/* Initialize the view frustum from the current OpenGL context: */
	baseFrustum.setFromGL();
	
	/* Initialize OpenGL state tracking elements: */
	cullingEnabled=glIsEnabled(GL_CULL_FACE);
	GLint tempCulledFace;
	glGetIntegerv(GL_CULL_FACE_MODE,&tempCulledFace);
	culledFace=tempCulledFace;
	lightingEnabled=glIsEnabled(GL_LIGHTING);
	if(lightingEnabled)
		{
		glEnable(GL_NORMALIZE);
		glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,cullingEnabled?GL_FALSE:GL_TRUE);
		}
	else
		glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_FALSE);
	colorMaterialEnabled=glIsEnabled(GL_COLOR_MATERIAL);
	highestTexturePriority=-1;
	if(glIsEnabled(GL_TEXTURE_1D))
		highestTexturePriority=0;
	if(glIsEnabled(GL_TEXTURE_2D))
		highestTexturePriority=1;
	
	GLint lightModelColorControl;
	glGetIntegerv(GL_LIGHT_MODEL_COLOR_CONTROL,&lightModelColorControl);
	separateSpecularColorEnabled=lightModelColorControl==GL_SEPARATE_SPECULAR_COLOR;
	}

GLRenderState::DOGTransform GLRenderState::pushTransform(const OGTransform& deltaTransform)
	{
	/* Update the current transformation: */
	DOGTransform result=currentTransform;
	currentTransform*=deltaTransform;
	currentTransform.renormalize();
	
	/* Set up the new transformation: */
	glLoadMatrix(currentTransform);
	
	return result;
	}

GLRenderState::DOGTransform GLRenderState::pushTransform(const GLRenderState::DOGTransform& deltaTransform)
	{
	/* Update the current transformation: */
	DOGTransform result=currentTransform;
	currentTransform*=deltaTransform;
	currentTransform.renormalize();
	
	/* Set up the new transformation: */
	glLoadMatrix(currentTransform);
	
	return result;
	}

void GLRenderState::popTransform(const GLRenderState::DOGTransform& previousTransform)
	{
	/* Reinstate the current transformation: */
	currentTransform=previousTransform;
	
	/* Set up the new transformation: */
	glLoadMatrix(currentTransform);
	}

bool GLRenderState::doesBoxIntersectFrustum(const Box& box) const
	{
	/* Get the current transformation's direction axes: */
	Vector axis[3];
	for(int i=0;i<3;++i)
		axis[i]=currentTransform.getDirection(i);
	
	/* Check the box against each frustum plane: */
	for(int planeIndex=0;planeIndex<6;++planeIndex)
		{
		/* Get the frustum plane's normal vector: */
		const Vector& normal=baseFrustum.getFrustumPlane(planeIndex).getNormal();
		
		/* Find the point on the bounding box which is closest to the frustum plane: */
		Point p;
		for(int i=0;i<3;++i)
			p[i]=normal*axis[i]>Scalar(0)?box.max[i]:box.min[i];
		
		/* Check if the point is inside the view frustum: */
		if(!baseFrustum.getFrustumPlane(planeIndex).contains(currentTransform.transform(p)))
			return false;
		}
	
	return true;
	}

void GLRenderState::enableCulling(GLenum newCulledFace)
	{
	if(!cullingEnabled)
		{
		glEnable(GL_CULL_FACE);
		if(lightingEnabled)
			glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_FALSE);
		cullingEnabled=true;
		}
	if(culledFace!=newCulledFace)
		{
		glCullFace(newCulledFace);
		culledFace=newCulledFace;
		}
	}

void GLRenderState::disableCulling(void)
	{
	if(cullingEnabled)
		{
		glDisable(GL_CULL_FACE);
		if(lightingEnabled)
			glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);
		cullingEnabled=false;
		}
	}

void GLRenderState::enableMaterials(void)
	{
	if(!lightingEnabled)
		{
		glEnable(GL_LIGHTING);
		glEnable(GL_NORMALIZE);
		if(!cullingEnabled)
			glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);
		if(highestTexturePriority>=0)
			glTexEnvMode(GLTexEnvEnums::TEXTURE_ENV,GLTexEnvEnums::MODULATE);
		lightingEnabled=true;
		}
	if(!colorMaterialEnabled)
		{
		glEnable(GL_COLOR_MATERIAL);
		glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
		colorMaterialEnabled=true;
		}
	if(highestTexturePriority>=0&&!separateSpecularColorEnabled)
		{
		glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL,GL_SEPARATE_SPECULAR_COLOR);
		separateSpecularColorEnabled=true;
		}
	}

void GLRenderState::disableMaterials(void)
	{
	if(lightingEnabled)
		{
		glDisable(GL_LIGHTING);
		glDisable(GL_NORMALIZE);
		if(!cullingEnabled)
			glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_FALSE);
		if(highestTexturePriority>=0)
			glTexEnvMode(GLTexEnvEnums::TEXTURE_ENV,GLTexEnvEnums::REPLACE);
		lightingEnabled=false;
		}
	if(colorMaterialEnabled)
		{
		glDisable(GL_COLOR_MATERIAL);
		colorMaterialEnabled=false;
		}
	if(separateSpecularColorEnabled)
		{
		glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL,GL_SINGLE_COLOR);
		separateSpecularColorEnabled=false;
		}
	}

void GLRenderState::enableTexture1D(void)
	{
	bool textureEnabled=highestTexturePriority>=0;
	if(highestTexturePriority>=1)
		glDisable(GL_TEXTURE_2D);
	if(highestTexturePriority<0)
		glEnable(GL_TEXTURE_1D);
	highestTexturePriority=0;
	
	if(!textureEnabled)
		glTexEnvMode(GLTexEnvEnums::TEXTURE_ENV,lightingEnabled?GLTexEnvEnums::MODULATE:GLTexEnvEnums::REPLACE);
	if(lightingEnabled&&!separateSpecularColorEnabled)
		{
		glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL,GL_SEPARATE_SPECULAR_COLOR);
		separateSpecularColorEnabled=true;
		}
	}

void GLRenderState::enableTexture2D(void)
	{
	bool textureEnabled=highestTexturePriority>=0;
	if(highestTexturePriority<1)
		glEnable(GL_TEXTURE_2D);
	highestTexturePriority=1;
	
	if(!textureEnabled)
		glTexEnvMode(GLTexEnvEnums::TEXTURE_ENV,lightingEnabled?GLTexEnvEnums::MODULATE:GLTexEnvEnums::REPLACE);
	if(lightingEnabled&&!separateSpecularColorEnabled)
		{
		glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL,GL_SEPARATE_SPECULAR_COLOR);
		separateSpecularColorEnabled=true;
		}
	}

void GLRenderState::disableTextures(void)
	{
	if(highestTexturePriority>=1)
		glDisable(GL_TEXTURE_2D);
	if(highestTexturePriority>=0)
		glDisable(GL_TEXTURE_1D);
	highestTexturePriority=-1;
	
	if(separateSpecularColorEnabled)
		{
		glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL,GL_SINGLE_COLOR);
		separateSpecularColorEnabled=false;
		}
	}

}
