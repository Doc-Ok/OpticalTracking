/***********************************************************************
CAVERenderer - Vislet class to render the default KeckCAVES backround
image seamlessly inside a VR application.
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

#include <string.h>
#include <stdlib.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLVertex.h>
#include <GL/GLMatrixTemplates.h>
#include <GL/GLLight.h>
#include <GL/GLValueCoders.h>
#include <GL/GLClipPlaneTracker.h>
#include <GL/GLContextData.h>
#include <GL/GLTransformationWrappers.h>
#include <Images/ReadImageFile.h>
#include <Vrui/LightsourceManager.h>
#include <Vrui/Viewer.h>
#include <Vrui/VisletManager.h>
#include <Vrui/Vrui.h>

#include <Vrui/Vislets/CAVERenderer.h>

namespace Vrui {

namespace Vislets {

namespace {

/****************
Helper functions:
****************/

inline double clampAngle(double angle)
	{
	if(angle<0.0)
		return 0.0;
	else if(angle>180.0)
		return 180.0;
	else
		return angle;
	}

}

/************************************
Methods of class CAVERendererFactory:
************************************/

CAVERendererFactory::CAVERendererFactory(VisletManager& visletManager)
	:VisletFactory("CAVERenderer",visletManager),
	 surfaceMaterial(GLMaterial::Color(1.0f,1.0f,1.0f),GLMaterial::Color(0.0f,0.0f,0.0f),0.0f),
	 tilesPerFoot(12),
	 wallTextureFileName("KeckCAVESWall.png"),
	 floorTextureFileName("KeckCAVESFloor.png")
	{
	#if 0
	/* Insert class into class hierarchy: */
	VisletFactory* visletFactory=visletManager.loadClass("Vislet");
	visletFactory->addChildClass(this);
	addParentClass(visletFactory);
	#endif
	
	/* Load class settings: */
	Misc::ConfigurationFileSection cfs=visletManager.getVisletClassSection(getClassName());
	surfaceMaterial=cfs.retrieveValue<GLMaterial>("./surfaceMaterial",surfaceMaterial);
	tilesPerFoot=cfs.retrieveValue<int>("./tilesPerFoot",tilesPerFoot);
	wallTextureFileName=cfs.retrieveValue<std::string>("./wallTextureFileName",wallTextureFileName);
	floorTextureFileName=cfs.retrieveValue<std::string>("./floorTextureFileName",floorTextureFileName);
	
	/* Set tool class' factory pointer: */
	CAVERenderer::factory=this;
	}

CAVERendererFactory::~CAVERendererFactory(void)
	{
	/* Reset tool class' factory pointer: */
	CAVERenderer::factory=0;
	}

Vislet* CAVERendererFactory::createVislet(int numArguments,const char* const arguments[]) const
	{
	return new CAVERenderer(numArguments,arguments);
	}

void CAVERendererFactory::destroyVislet(Vislet* vislet) const
	{
	delete vislet;
	}

extern "C" void resolveCAVERendererDependencies(Plugins::FactoryManager<VisletFactory>& manager)
	{
	#if 0
	/* Load base classes: */
	manager.loadClass("Vislet");
	#endif
	}

extern "C" VisletFactory* createCAVERendererFactory(Plugins::FactoryManager<VisletFactory>& manager)
	{
	/* Get pointer to vislet manager: */
	VisletManager* visletManager=static_cast<VisletManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	CAVERendererFactory* caveRendererFactory=new CAVERendererFactory(*visletManager);
	
	/* Return factory object: */
	return caveRendererFactory;
	}

extern "C" void destroyCAVERendererFactory(VisletFactory* factory)
	{
	delete factory;
	}

/***************************************
Methods of class CAVERenderer::DataItem:
***************************************/

CAVERenderer::DataItem::DataItem(void)
	:wallTextureObjectId(0),
	 floorTextureObjectId(0),
	 wallDisplayListId(glGenLists(1)),
	 floorDisplayListId(glGenLists(1))
	{
	glGenTextures(1,&wallTextureObjectId);
	glGenTextures(1,&floorTextureObjectId);
	}

CAVERenderer::DataItem::~DataItem(void)
	{
	glDeleteTextures(1,&wallTextureObjectId);
	glDeleteTextures(1,&floorTextureObjectId);
	glDeleteLists(wallDisplayListId,1);
	glDeleteLists(floorDisplayListId,1);
	}

/*************************************
Static elements of class CAVERenderer:
*************************************/

CAVERendererFactory* CAVERenderer::factory=0;

/*****************************
Methods of class CAVERenderer:
*****************************/

int CAVERenderer::createMipmap(const Images::RGBImage& baseImage) const
	{
	Images::RGBImage level=baseImage;
	int levelIndex=0;
	while(true)
		{
		/* Upload the current level texture image: */
		level.glTexImage2D(GL_TEXTURE_2D,levelIndex,GL_RGB);
		++levelIndex;
		
		/* Check if we can go down another level: */
		unsigned int nextWidth=level.getWidth();
		unsigned int nextHeight=level.getHeight();
		if((nextWidth&0x1)==0&&(nextHeight&0x1)==0&&nextWidth>=32&&nextHeight>=32)
			{
			/* Downsample the current level image: */
			nextWidth/=2;
			nextHeight/=2;
			Images::RGBImage nextLevel(nextWidth,nextHeight);
			for(unsigned int y=0;y<nextHeight;++y)
				{
				Images::RGBImage::Color* row=nextLevel.modifyPixelRow(y);
				for(unsigned int x=0;x<nextWidth;++x)
					{
					/* Sample the current level image: */
					for(int i=0;i<3;++i)
						{
						unsigned int accColor=0;
						accColor+=level.getPixel(2*x+0,2*y+0)[i];
						accColor+=level.getPixel(2*x+1,2*y+0)[i];
						accColor+=level.getPixel(2*x+0,2*y+1)[i];
						accColor+=level.getPixel(2*x+1,2*y+1)[i];
						row[x][i]=Images::RGBImage::Scalar((accColor+2)/4);
						}
					}
				}
			
			level=nextLevel;
			}
		else
			break;
		}
	
	return levelIndex;
	}

void CAVERenderer::renderWall(CAVERenderer::DataItem* dataItem) const
	{
	typedef GLVertex<GLfloat,2,void,0,GLfloat,GLfloat,3> Vertex;
	
	/* Set up the wall rendering parameters: */
	float tileSize=12.0f/float(tilesPerFoot);
	int numTilesX=10*tilesPerFoot;
	int numTilesY=8*tilesPerFoot;
	
	/* Render the wall as a series of textured quad strips: */
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,dataItem->wallTextureObjectId);
	glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
	Vertex v1,v2;
	v1.normal=v2.normal=Vertex::Normal(0.0f,0.0f,1.0f);
	v1.position[2]=v2.position[2]=0.0f;
	for(int y=0;y<numTilesY;++y)
		{
		v1.texCoord[1]=float(y)/float(numTilesY);
		v1.position[1]=float(y)*tileSize;
		v2.texCoord[1]=float(y+1)/float(numTilesY);
		v2.position[1]=float(y+1)*tileSize;
		glBegin(GL_QUAD_STRIP);
		for(int x=0;x<=numTilesX;++x)
			{
			v1.texCoord[0]=v2.texCoord[0]=float(x)/float(numTilesX);
			v1.position[0]=v2.position[0]=float(x)*tileSize;
			glVertex(v2);
			glVertex(v1);
			}
		glEnd();
		}
	glBindTexture(GL_TEXTURE_2D,0);
	glDisable(GL_TEXTURE_2D);
	}

void CAVERenderer::renderFloor(CAVERenderer::DataItem* dataItem) const
	{
	typedef GLVertex<GLfloat,2,void,0,GLfloat,GLfloat,3> Vertex;
	
	/* Set up the floor rendering parameters: */
	float tileSize=12.0f/float(tilesPerFoot);
	int numTilesX=10*tilesPerFoot;
	int numTilesY=8*tilesPerFoot;
	
	/* Render the floor as a series of textured quad strips: */
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,dataItem->floorTextureObjectId);
	glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
	Vertex v1,v2;
	v1.normal=v2.normal=Vertex::Normal(0.0f,0.0f,1.0f);
	v1.position[2]=v2.position[2]=0.0f;
	for(int y=0;y<numTilesY;++y)
		{
		v1.texCoord[1]=float(y)/float(numTilesY);
		v1.position[1]=float(y)*tileSize;
		v2.texCoord[1]=float(y+1)/float(numTilesY);
		v2.position[1]=float(y+1)*tileSize;
		glBegin(GL_QUAD_STRIP);
		for(int x=0;x<=numTilesX;++x)
			{
			v1.texCoord[0]=v2.texCoord[0]=float(x)/float(numTilesX);
			v1.position[0]=v2.position[0]=float(x)*tileSize;
			glVertex(v2);
			glVertex(v1);
			}
		glEnd();
		}
	glBindTexture(GL_TEXTURE_2D,0);
	glDisable(GL_TEXTURE_2D);
	}

CAVERenderer::CAVERenderer(int numArguments,const char* const arguments[])
	:GLObject(false),
	 surfaceMaterial(factory->surfaceMaterial),
	 tilesPerFoot(factory->tilesPerFoot),
	 numViewers(getNumViewers()),
	 viewerHeadlightStates(0),
	 angle(720.0),angleAnimStep(0.0),lastFrame(0.0)
	{
	/* Parse the command line: */
	std::string wallTextureFileName=factory->wallTextureFileName;
	std::string floorTextureFileName=factory->floorTextureFileName;
	for(int i=0;i<numArguments;++i)
		{
		if(arguments[i][0]=='-')
			{
			if(strcasecmp(arguments[i]+1,"wall")==0)
				{
				++i;
				wallTextureFileName=arguments[i];
				}
			else if(strcasecmp(arguments[i]+1,"floor")==0)
				{
				++i;
				floorTextureFileName=arguments[i];
				}
			else if(strcasecmp(arguments[i]+1,"tilesPerFoot")==0)
				{
				++i;
				tilesPerFoot=atoi(arguments[i]);
				}
			}
		}
	
	/* Load the texture images: */
	wallTextureImage=Images::readImageFile(wallTextureFileName.c_str());
	floorTextureImage=Images::readImageFile(floorTextureFileName.c_str());
	
	/* Create static ceiling light sources in the CAVE room: */
	GLLight::Color lightColor(0.25f,0.25f,0.25f);
	for(int i=0;i<4;++i)
		{
		GLLight::Position pos(30.0f,30.0f,96.0f,1.0f);
		for(int j=0;j<2;++j)
			if(i&(0x1<<j))
				pos[j]=-pos[j];
		lightsources[i]=getLightsourceManager()->createLightsource(true,GLLight(lightColor,pos));
		}
	
	GLObject::init();
	}

CAVERenderer::~CAVERenderer(void)
	{
	delete[] viewerHeadlightStates;
	
	/* Destroy static ceiling light sources: */
	for(int i=0;i<4;++i)
		getLightsourceManager()->destroyLightsource(lightsources[i]);
	}

VisletFactory* CAVERenderer::getFactory(void) const
	{
	return factory;
	}

void CAVERenderer::disable(void)
	{
	/* Trigger the folding animation: */
	angleAnimStep=-90.0;
	lastFrame=getApplicationTime();
		
	/* Request another frame: */
	scheduleUpdate(getApplicationTime()+1.0/125.0);
	
	/* Frame function will disable vislet when animation is done */
	}

void CAVERenderer::enable(void)
	{
	/* Enable the static ceiling light sources: */
	for(int i=0;i<4;++i)
		lightsources[i]->enable();
	
	/* Save all viewers' head light states and then turn them off: */
	viewerHeadlightStates=new bool[getNumViewers()];
	for(int i=0;i<getNumViewers()&&i<numViewers;++i)
		{
		viewerHeadlightStates[i]=getViewer(i)->getHeadlight().isEnabled();
		getViewer(i)->setHeadlightState(false);
		}
	
	/* Enable the vislet as far as the vislet manager is concerned: */
	active=true;
	
	/* Trigger the unfolding animation: */
	angleAnimStep=90.0;
	lastFrame=getApplicationTime();
	
	/* Request another frame: */
	scheduleUpdate(getApplicationTime()+1.0/125.0);
	}

void CAVERenderer::initContext(GLContextData& contextData) const
	{
	/* Create the context data item: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	
	/* Upload the wall texture image: */
	glBindTexture(GL_TEXTURE_2D,dataItem->wallTextureObjectId);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_BASE_LEVEL,0);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
	int wallNumLevels=createMipmap(wallTextureImage);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAX_LEVEL,wallNumLevels-1);
	glBindTexture(GL_TEXTURE_2D,0);
	
	/* Upload the floor texture image: */
	glBindTexture(GL_TEXTURE_2D,dataItem->floorTextureObjectId);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_BASE_LEVEL,0);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
	int floorNumLevels=createMipmap(floorTextureImage);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAX_LEVEL,floorNumLevels-1);
	glBindTexture(GL_TEXTURE_2D,0);
	
	/* Create the wall display list: */
	glNewList(dataItem->wallDisplayListId,GL_COMPILE);
	renderWall(dataItem);
	glEndList();
	
	/* Create the floor display list: */
	glNewList(dataItem->floorDisplayListId,GL_COMPILE);
	renderFloor(dataItem);
	glEndList();
	}

void CAVERenderer::frame(void)
	{
	if(angleAnimStep!=0.0)
		{
		/* Get the time since the last frame: */
		double delta=getApplicationTime()-lastFrame;
		lastFrame=getApplicationTime();
		
		/* Update the angle value: */
		angle+=angleAnimStep*delta;
		if(angle<0.0)
			{
			angle=0.0;
			angleAnimStep=0.0;
			
			/* Disable the static ceiling light sources: */
			for(int i=0;i<4;++i)
				lightsources[i]->disable();
			
			/* Set all viewers' head lights to the saved state: */
			for(int i=0;i<getNumViewers()&&i<numViewers;++i)
				getViewer(i)->setHeadlightState(viewerHeadlightStates[i]);
			delete[] viewerHeadlightStates;
			viewerHeadlightStates=0;
			
			/* Disable the vislet: */
			active=false;
			}
		else if(angle>720.0)
			{
			angle=720.0;
			angleAnimStep=0.0;
			}
		else
			{
			/* Request another frame: */
			scheduleUpdate(getApplicationTime()+1.0/125.0);
			}
		}
	}

void CAVERenderer::display(GLContextData& contextData) const
	{
	/* Get a pointer to the data item: */
	DataItem* dataItem=contextData.retrieveDataItem<DataItem>(this);
	
	/* Temporarily disable all clipping planes: */
	contextData.getClipPlaneTracker()->pause();
	
	/* Set up the rendering mode for the CAVE room: */
	glMaterial(GLMaterialEnums::FRONT,surfaceMaterial);
	
	/* Save the modelview matrix: */
	glPushMatrix();
	
	/* Render the floor: */
	glTranslate(-60.0,-36.0,0.0);
	glRotate(clampAngle(angle)-180.0,1.0,0.0,0.0);
	glCallList(dataItem->floorDisplayListId);
	
	/* Render the left wall: */
	glTranslate(0.0,-24.0,0.0);
	glRotate(90.0,0.0,0.0,1.0);
	glRotate(clampAngle(angle-180.0)-90.0,1.0,0.0,0.0);
	glCallList(dataItem->wallDisplayListId);
	
	/* Render the back wall: */
	glTranslate(120.0,0.0,0.0);
	glRotate(90.0-clampAngle(angle-360.0),0.0,1.0,0.0);
	glCallList(dataItem->wallDisplayListId);
	
	/* Render the right wall: */
	glTranslate(120.0,0.0,0.0);
	glRotate(90.0-clampAngle(angle-540.0),0.0,1.0,0.0);
	glCallList(dataItem->wallDisplayListId);
	
	/* Restore the modelview matrix: */
	glPopMatrix();
	
	/* Re-enable clipping: */
	contextData.getClipPlaneTracker()->resume();
	}

}

}
