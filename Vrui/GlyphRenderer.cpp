/***********************************************************************
GlyphRenderer - Class to quickly render several kinds of common glyphs.
Copyright (c) 2004-2013 Oliver Kreylos

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

#include <Vrui/GlyphRenderer.h>

#include <string.h>
#include <Misc/ThrowStdErr.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Geometry/Point.h>
#include <Geometry/Ray.h>
#include <Geometry/OrthonormalTransformation.h>
#include <GL/GLValueCoders.h>
#include <GL/GLGeometryWrappers.h>
#include <GL/GLTransformationWrappers.h>
#include <GL/GLModels.h>
#include <Images/RGBAImage.h>
#include <Images/ReadImageFile.h>
#include <Vrui/Vrui.h>
#include <Vrui/Viewer.h>
#include <Vrui/VRScreen.h>
#include <Vrui/DisplayState.h>

namespace Vrui {

/**********************
Methods of class Glyph:
**********************/

void Glyph::render(int glyphType,GLfloat glyphSize)
	{
	switch(glyphType)
		{
		case CONE:
			glRotatef(-90.0f,1.0f,0.0f,0.0f);
			glTranslatef(0.0f,0.0f,-0.75f*glyphSize);
			glDrawCone(0.25f*glyphSize,glyphSize,16);
			break;
		
		case CUBE:
			glDrawCube(glyphSize);
			break;
		
		case SPHERE:
			glDrawSphereIcosahedron(0.5f*glyphSize,8);
			break;
		
		case CROSSBALL:
			glDrawSphereIcosahedron(0.4f*glyphSize,8);
			glDrawCylinder(0.125f*glyphSize,1.1f*glyphSize,16);
			glRotatef(90.0f,1.0f,0.0f,0.0f);
			glDrawCylinder(0.125f*glyphSize,1.1f*glyphSize,16);
			glRotatef(90.0f,0.0f,1.0f,0.0f);
			glDrawCylinder(0.125f*glyphSize,1.1f*glyphSize,16);
			break;
		
		case BOX:
			{
			#if 1
			glDrawWireframeCube(glyphSize,glyphSize*0.075f,glyphSize*0.15f);
			#else
			GLfloat gs=glyphSize*0.5f;
			glPushAttrib(GL_ENABLE_BIT|GL_POINT_BIT|GL_LINE_BIT);
			glDisable(GL_LIGHTING);
			glPointSize(5.0f);
			glBegin(GL_POINTS);
			glVertex3f(-gs,-gs,-gs);
			glVertex3f( gs,-gs,-gs);
			glVertex3f(-gs, gs,-gs);
			glVertex3f( gs, gs,-gs);
			glVertex3f(-gs,-gs, gs);
			glVertex3f( gs,-gs, gs);
			glVertex3f(-gs, gs, gs);
			glVertex3f( gs, gs, gs);
			glEnd();
			glLineWidth(3.0f);
			glBegin(GL_LINE_STRIP);
			glVertex3f(-gs,-gs,-gs);
			glVertex3f( gs,-gs,-gs);
			glVertex3f( gs, gs,-gs);
			glVertex3f(-gs, gs,-gs);
			glVertex3f(-gs,-gs,-gs);
			glVertex3f(-gs,-gs, gs);
			glVertex3f( gs,-gs, gs);
			glVertex3f( gs, gs, gs);
			glVertex3f(-gs, gs, gs);
			glVertex3f(-gs,-gs, gs);
			glEnd();
			glBegin(GL_LINES);
			glVertex3f( gs,-gs,-gs);
			glVertex3f( gs,-gs, gs);
			glVertex3f( gs, gs,-gs);
			glVertex3f( gs, gs, gs);
			glVertex3f(-gs, gs,-gs);
			glVertex3f(-gs, gs, gs);
			glEnd();
			glPopAttrib();
			#endif
			break;
			}
		}
	}

Glyph::Glyph(void)
	:enabled(false),
	 glyphType(CROSSBALL),
	 glyphMaterial(GLMaterial::Color(0.5f,0.5f,0.5f),GLMaterial::Color(1.0f,1.0f,1.0f),25.0f)
	{
	}

void Glyph::enable(void)
	{
	enabled=true;
	}

void Glyph::enable(Glyph::GlyphType newGlyphType,const GLMaterial& newGlyphMaterial)
	{
	enabled=true;
	glyphType=newGlyphType;
	glyphMaterial=newGlyphMaterial;
	}

void Glyph::disable(void)
	{
	enabled=false;
	}

void Glyph::setGlyphType(Glyph::GlyphType newGlyphType)
	{
	glyphType=newGlyphType;
	}

void Glyph::setGlyphType(const char* newGlyphType)
	{
	if(strcasecmp(newGlyphType,"None")!=0)
		{
		/* Parse the glyph type name: */
		if(strcasecmp(newGlyphType,"Cone")==0)
			glyphType=CONE;
		else if(strcasecmp(newGlyphType,"Cube")==0)
			glyphType=CUBE;
		else if(strcasecmp(newGlyphType,"Sphere")==0)
			glyphType=SPHERE;
		else if(strcasecmp(newGlyphType,"Crossball")==0)
			glyphType=CROSSBALL;
		else if(strcasecmp(newGlyphType,"Box")==0)
			glyphType=BOX;
		else if(strcasecmp(newGlyphType,"Cursor")==0)
			glyphType=CURSOR;
		else
			Misc::throwStdErr("GlyphRenderer::Glyph: Invalid glyph type %s",newGlyphType);
		enabled=true;
		}
	else
		{
		/* Disable the glyph: */
		enabled=false;
		}
	}

void Glyph::setGlyphMaterial(const GLMaterial& newGlyphMaterial)
	{
	glyphMaterial=newGlyphMaterial;
	}

void Glyph::configure(const Misc::ConfigurationFileSection& configFileSection,const char* glyphTypeTagName,const char* glyphMaterialTagName)
	{
	/* Retrieve glyph type as string and set it: */
	setGlyphType(configFileSection.retrieveString(glyphTypeTagName,"None").c_str());
	
	/* Retrieve the glyph material: */
	glyphMaterial=configFileSection.retrieveValue<GLMaterial>(glyphMaterialTagName,glyphMaterial);
	}

/****************************************
Methods of class GlyphRenderer::DataItem:
****************************************/

GlyphRenderer::DataItem::DataItem(GLContextData& sContextData)
	:contextData(sContextData),
	 glyphDisplayLists(glGenLists(Glyph::GLYPHS_END)),
	 cursorTextureObjectId(0)
	{
	glGenTextures(1,&cursorTextureObjectId);
	}

GlyphRenderer::DataItem::~DataItem(void)
	{
	glDeleteLists(glyphDisplayLists,Glyph::GLYPHS_END);
	glDeleteTextures(1,&cursorTextureObjectId);
	}

/******************************
Methods of class GlyphRenderer:
******************************/

GlyphRenderer::GlyphRenderer(GLfloat sGlyphSize,std::string sCursorImageFileName,unsigned int sCursorNominalSize)
	:GLObject(false),
	 glyphSize(sGlyphSize),
	 cursorImageFileName(sCursorImageFileName),
	 cursorNominalSize(sCursorNominalSize)
	{
	GLObject::init();
	}

void GlyphRenderer::initContext(GLContextData& contextData) const
	{
	/* Create a new context data item: */
	DataItem* dataItem=new DataItem(contextData);
	contextData.addDataItem(this,dataItem);
	
	/* Render all glyph types: */
	for(int glyphType=Glyph::CONE;glyphType<Glyph::GLYPHS_END;++glyphType)
		{
		if(glyphType==Glyph::CURSOR)
			{
			/* Load the cursor texture image: */
			unsigned int hotspot[2];
			Images::RGBAImage cursorImage=Images::readCursorFile(cursorImageFileName.c_str(),cursorNominalSize,hotspot);
			
			/* Calculate the cursor texture coordinate box: */
			unsigned int cis[2];
			float tcMin[2];
			float tcMax[2];
			for(int i=0;i<2;++i)
				{
				cis[i]=cursorImage.getSize(i);
				unsigned int texSize;
				for(texSize=1;texSize<cis[i];texSize<<=1)
					;
				tcMin[i]=0.5f/float(texSize);
				tcMax[i]=(float(cis[i])-0.5f)/float(texSize);
				}
			
			/* Calculate the scale factor: */
			float scale=cis[0]>=cis[1]?glyphSize/float(cis[0]):glyphSize/float(cis[1]);
			
			/* Upload the cursor image as a 2D texture: */
			glBindTexture(GL_TEXTURE_2D,dataItem->cursorTextureObjectId);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_BASE_LEVEL,0);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAX_LEVEL,0);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
			cursorImage.glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,true);
			glBindTexture(GL_TEXTURE_2D,0);
			
			/* Render a texture-based glyph: */
			glNewList(dataItem->glyphDisplayLists+glyphType,GL_COMPILE);
			glPushAttrib(GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_VIEWPORT_BIT);
			glDepthRange(0.0,0.0);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D,dataItem->cursorTextureObjectId);
			glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
			glEnable(GL_ALPHA_TEST);
			glAlphaFunc(GL_GEQUAL,0.5f);
			glBegin(GL_QUADS);
			glTexCoord2f(tcMin[0],tcMin[1]);
			glVertex2f(-float(hotspot[0])*scale,-float(cis[1]-1-hotspot[1])*scale);
			glTexCoord2f(tcMax[0],tcMin[1]);
			glVertex2f(float(cis[0]-1-hotspot[0])*scale,-float(cis[1]-1-hotspot[1])*scale);
			glTexCoord2f(tcMax[0],tcMax[1]);
			glVertex2f(float(cis[0]-1-hotspot[0])*scale,float(hotspot[1])*scale);
			glTexCoord2f(tcMin[0],tcMax[1]);
			glVertex2f(-float(hotspot[0])*scale,float(hotspot[1])*scale);
			glEnd();
			glBindTexture(GL_TEXTURE_2D,0);
			glPopAttrib();
			glEndList();
			}
		else
			{
			/* Render a 3D glyph: */
			glNewList(dataItem->glyphDisplayLists+glyphType,GL_COMPILE);
			Glyph::render(glyphType,glyphSize);
			glEndList();
			}
		}
	}

void GlyphRenderer::renderGlyph(const Glyph& glyph,const OGTransform& transformation,const GlyphRenderer::DataItem* contextDataItem) const
	{
	/* Check if the glyph is enabled: */
	if(glyph.enabled)
		{
		if(glyph.glyphType==Glyph::CURSOR)
			{
			/****************************
			Render a texture-based glyph:
			****************************/
			
			/* Align the glyph texture with the current window's current screen: */
			const DisplayState& ds=getDisplayState(contextDataItem->contextData);
			glPushMatrix();
			glTranslate(transformation.getTranslation());
			glRotate(ds.screen->getScreenTransformation().getRotation());
			
			/* Draw the glyph texture: */
			glCallList(contextDataItem->glyphDisplayLists+glyph.glyphType);
			
			glPopMatrix();
			}
		else
			{
			/* Render a 3D glyph: */
			glPushMatrix();
			glMultMatrix(transformation);
			glMaterial(GLMaterialEnums::FRONT,glyph.glyphMaterial);
			glCallList(contextDataItem->glyphDisplayLists+glyph.glyphType);
			glPopMatrix();
			}
		}
	}

}
