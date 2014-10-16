/***********************************************************************
Doom3TextureManager - Class to grant Doom3 materials access to shared
texture images.
Copyright (c) 2007-2010 Oliver Kreylos

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

#include <SceneGraph/Internal/Doom3TextureManager.h>

#include <Misc/StringPrintf.h>
#include <Misc/ThrowStdErr.h>
#include <IO/File.h>
#include <Math/Math.h>
#include <Geometry/Vector.h>
#include <GL/gl.h>
#include <GL/GLContextData.h>
#include <Images/TargaImageFileReader.h>
#include <SceneGraph/Internal/Doom3FileManager.h>

namespace SceneGraph {

namespace {

/****************
Helper functions:
****************/

inline Images::RGBAImage::Color encodeNormal(const Geometry::Vector<float,3>& normal)
	{
	Images::RGBAImage::Color result;
	float normalLen=Geometry::mag(normal);
	for(int i=0;i<3;++i)
		{
		float val=normal[i]*128.0f/normalLen+128.0f;
		if(val<=0.0f)
			result[i]=GLubyte(0);
		else if(val>=255.0f)
			result[i]=GLubyte(255);
		else
			result[i]=GLubyte(Math::floor(val));
		}
	result[3]=GLubyte(255);
	return result;
	}

}

/**********************************************
Methods of class Doom3TextureManager::DataItem:
**********************************************/

Doom3TextureManager::DataItem::DataItem(int sNumTextures)
	:numTextures(sNumTextures),
	 textureObjectIds(new GLuint[numTextures])
	{
	/* Allocate the texture objects: */
	glGenTextures(numTextures,textureObjectIds);
	}

Doom3TextureManager::DataItem::~DataItem(void)
	{
	/* Destroy the texture objects: */
	glDeleteTextures(numTextures,textureObjectIds);
	delete[] textureObjectIds;
	}

/*****************************************************
Methods of class Doom3TextureManager::TextureUploader:
*****************************************************/

void Doom3TextureManager::TextureUploader::operator()(std::string name,const Doom3TextureManager::Image& image)
	{
	/* Bind the texture object: */
	glBindTexture(GL_TEXTURE_2D,dataItem->textureObjectIds[image.textureIndex]);
	
	/* Upload the texture image: */
	image.image.glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8);
	
	/* Keep track of the number of textures and pixels: */
	++numTextures;
	totalTextureSize+=size_t(image.image.getWidth())*size_t(image.image.getHeight())*sizeof(Images::RGBAImage::Color);
	}

/************************************
Methods of class Doom3TextureManager:
************************************/

Doom3TextureManager::Doom3TextureManager(Doom3FileManager& sFileManager)
	:fileManager(sFileManager),
	 numTextures(0)
	{
	}

Doom3TextureManager::~Doom3TextureManager(void)
	{
	/* Nothing to do, since RGBAImages take care of themselves... */
	}

void Doom3TextureManager::initContext(GLContextData& contextData) const
	{
	/* Create a context data item and store it in the OpenGL context: */
	DataItem* dataItem=new DataItem(numTextures);
	contextData.addDataItem(this,dataItem);
	
	/* Upload all textures: */
	TextureUploader tu(*this,dataItem);
	imageTree.forEachLeaf(tu);
	
	/* Unbind all texture objects: */
	glBindTexture(GL_TEXTURE_2D,0);
	}

Doom3TextureManager::ImageID Doom3TextureManager::loadTexture(const char* textureName)
	{
	/* Check if the texture name has the .tga extension: */
	const char* extPtr=0;
	for(const char* tnPtr=textureName;*tnPtr!='\0';++tnPtr)
		if(*tnPtr=='.')
			extPtr=tnPtr;
	std::string tempTextureName=textureName;
	if(extPtr==0||strcasecmp(extPtr,".tga")!=0)
		{
		tempTextureName=textureName+std::string(".tga");
		textureName=tempTextureName.c_str();
		}
	
	/* Check if the requested texture is already there: */
	ImageID imageID=imageTree.findLeaf(textureName);
	if(imageID.isValid())
		return imageID;
	
	/* Store a new image structure in the image tree: */
	imageID=imageTree.insertLeaf(textureName,Image());
	
	/* Create a texture image: */
	Image& image=imageTree.getLeafValue(imageID);
	image.textureIndex=numTextures;
	++numTextures;
	
	/* Check for built-in texture names: */
	if(textureName[0]=='_')
		{
		/* Determine the fill color value: */
		Images::RGBAImage::Color imageColor;
		if(strcasecmp(textureName,"_black.tga")==0)
			imageColor=Images::RGBAImage::Color(0,0,0,255);
		else if(strcasecmp(textureName,"_white.tga")==0)
			imageColor=Images::RGBAImage::Color(255,255,255,255);
		else if(strcasecmp(textureName,"_flat.tga")==0)
			imageColor=Images::RGBAImage::Color(128,128,255,255);
		else
			imageColor=Images::RGBAImage::Color(255,0,255,255);
		
		/* Initialize the texture image: */
		image.image=Images::RGBAImage(2,2);
		image.image.clear(imageColor);
		}
	else
		{
		try
			{
			/* Read the texture image file: */
			IO::FilePtr imageFile=fileManager.getFile(textureName);
			Images::TargaImageFileReader<IO::File> targaReader(*imageFile);
			
			/* Initialize the texture image: */
			image.image=targaReader.readImage<Images::RGBAImage>();
			}
		catch(Doom3FileManager::ReadError err)
			{
			/* Initialize the texture image: */
			image.image=Images::RGBAImage(2,2);
			image.image.clear(Images::RGBAImage::Color(255,0,255,255));
			}
		}
	
	/* Return the image ID: */
	return imageID;
	}

Doom3TextureManager::ImageID Doom3TextureManager::computeHeightmap(const Doom3TextureManager::ImageID& source,float bumpiness)
	{
	/* Get a reference to the source image: */
	const Images::RGBAImage& sourceImage=imageTree.getLeafValue(source).image;
	
	/* Store a new image structure in the image tree: */
	ImageID resultId=imageTree.insertLeaf(Misc::stringPrintf("/_computedTextures/tex%06d",numTextures).c_str(),Image());
	Image& result=imageTree.getLeafValue(resultId);
	Images::RGBAImage& resultImage=result.image;
	result.textureIndex=numTextures;
	++numTextures;
	
	/* Compute the result image's pixels: */
	resultImage=Images::RGBAImage(sourceImage.getWidth(),sourceImage.getHeight());
	for(unsigned int y=0;y<resultImage.getHeight();++y)
		{
		const Images::RGBAImage::Color* sourceRow=sourceImage.getPixelRow(y);
		Images::RGBAImage::Color* destRow=resultImage.modifyPixelRow(y);
		for(unsigned int x=0;x<resultImage.getWidth();++x)
			{
			Geometry::Vector<float,3> g;
			if(x==0)
				g[0]=float(sourceRow[x][0])-float(sourceRow[x+1][0]);
			else if(x==resultImage.getWidth()-1)
				g[0]=float(sourceRow[x-1][0])-float(sourceRow[x][0]);
			else
				g[0]=(float(sourceRow[x-1][0])-float(sourceRow[x+1][0]))*0.5f;
			if(y==0)
				g[1]=float(sourceRow[x][0])-float((sourceRow+resultImage.getWidth())[x][0]);
			else if(y==resultImage.getHeight()-1)
				g[1]=float((sourceRow-resultImage.getWidth())[x][0])-float(sourceRow[x][0]);
			else
				g[1]=(float((sourceRow-resultImage.getWidth())[x][0])-float((sourceRow+resultImage.getWidth())[x][0]))*0.5f;
			g[2]=128.0f/bumpiness;
			destRow[x]=encodeNormal(g);
			}
		}
	
	/* Return the result image ID: */
	return resultId;
	}

Doom3TextureManager::ImageID Doom3TextureManager::computeAddNormals(const Doom3TextureManager::ImageID& source1,const Doom3TextureManager::ImageID& source2)
	{
	/* Get a reference to the source images: */
	const Images::RGBAImage& source1Image=imageTree.getLeafValue(source1).image;
	Images::RGBAImage source2Image=imageTree.getLeafValue(source2).image;
	if(source1Image.getWidth()!=source2Image.getWidth()||source1Image.getHeight()!=source2Image.getHeight())
		{
		/* Resample the second image to match the first image's size: */
		source2Image.resize(source1Image.getWidth(),source1Image.getHeight());
		}
	
	/* Store a new image structure in the image tree: */
	ImageID resultId=imageTree.insertLeaf(Misc::stringPrintf("/_computedTextures/tex%06d",numTextures).c_str(),Image());
	Image& result=imageTree.getLeafValue(resultId);
	Images::RGBAImage& resultImage=result.image;
	result.textureIndex=numTextures;
	++numTextures;
	
	/* Compute the result image's pixels: */
	resultImage=Images::RGBAImage(source1Image.getWidth(),source1Image.getHeight());
	for(unsigned int y=0;y<resultImage.getHeight();++y)
		{
		const Images::RGBAImage::Color* source1Row=source1Image.getPixelRow(y);
		const Images::RGBAImage::Color* source2Row=source2Image.getPixelRow(y);
		Images::RGBAImage::Color* destRow=resultImage.modifyPixelRow(y);
		for(unsigned int x=0;x<resultImage.getWidth();++x)
			{
			Geometry::Vector<float,3> g;
			for(int i=0;i<3;++i)
				g[i]=float(source1Row[x][i])+float(source2Row[x][i])-256.0f;
			destRow[x]=encodeNormal(g);
			}
		}
	
	/* Return the result image ID: */
	return resultId;
	}

Doom3TextureManager::ImageID Doom3TextureManager::computeSmoothNormals(const Doom3TextureManager::ImageID& source)
	{
	/* Get a reference to the source image: */
	const Images::RGBAImage& sourceImage=imageTree.getLeafValue(source).image;
	
	/* Store a new image structure in the image tree: */
	ImageID resultId=imageTree.insertLeaf(Misc::stringPrintf("/_computedTextures/tex%06d",numTextures).c_str(),Image());
	Image& result=imageTree.getLeafValue(resultId);
	Images::RGBAImage& resultImage=result.image;
	result.textureIndex=numTextures;
	++numTextures;
	
	/* Compute the result image's pixels: */
	resultImage=Images::RGBAImage(sourceImage.getWidth(),sourceImage.getHeight());
	for(unsigned int y=0;y<resultImage.getHeight();++y)
		{
		const Images::RGBAImage::Color* sourceRow=sourceImage.getPixelRow(y);
		Images::RGBAImage::Color* destRow=resultImage.modifyPixelRow(y);
		for(unsigned int x=0;x<resultImage.getWidth();++x)
			{
			destRow[x]=sourceRow[x];
			}
		}
	
	/* Return the result image ID: */
	return resultId;
	}

Doom3TextureManager::ImageID Doom3TextureManager::computeAdd(const Doom3TextureManager::ImageID& source1,const Doom3TextureManager::ImageID& source2)
	{
	/* Get a reference to the source images: */
	const Images::RGBAImage& source1Image=imageTree.getLeafValue(source1).image;
	Images::RGBAImage source2Image=imageTree.getLeafValue(source2).image;
	if(source1Image.getWidth()!=source2Image.getWidth()||source1Image.getHeight()!=source2Image.getHeight())
		{
		/* Resample the second image to match the first image's size: */
		source2Image.resize(source1Image.getWidth(),source1Image.getHeight());
		}
	
	/* Store a new image structure in the image tree: */
	ImageID resultId=imageTree.insertLeaf(Misc::stringPrintf("/_computedTextures/tex%06d",numTextures).c_str(),Image());
	Image& result=imageTree.getLeafValue(resultId);
	Images::RGBAImage& resultImage=result.image;
	result.textureIndex=numTextures;
	++numTextures;
	
	/* Compute the result image's pixels: */
	resultImage=Images::RGBAImage(source1Image.getWidth(),source1Image.getHeight());
	for(unsigned int y=0;y<resultImage.getHeight();++y)
		{
		const Images::RGBAImage::Color* source1Row=source1Image.getPixelRow(y);
		const Images::RGBAImage::Color* source2Row=source2Image.getPixelRow(y);
		Images::RGBAImage::Color* destRow=resultImage.modifyPixelRow(y);
		for(unsigned int x=0;x<resultImage.getWidth();++x)
			{
			for(int i=0;i<3;++i)
				{
				unsigned int sum=(unsigned int)(source1Row[x][i])+(unsigned int)(source2Row[x][i]);
				if(sum>=255)
					destRow[x][i]=GLubyte(255);
				else
					destRow[x][i]=GLubyte(sum);
				}
			}
		}
	
	/* Return the result image ID: */
	return resultId;
	}

Doom3TextureManager::ImageID Doom3TextureManager::computeScale(const Doom3TextureManager::ImageID& source,const float factors[4])
	{
	/* Get a reference to the source image: */
	const Images::RGBAImage& sourceImage=imageTree.getLeafValue(source).image;
	
	/* Store a new image structure in the image tree: */
	ImageID resultId=imageTree.insertLeaf(Misc::stringPrintf("/_computedTextures/tex%06d",numTextures).c_str(),Image());
	Image& result=imageTree.getLeafValue(resultId);
	Images::RGBAImage& resultImage=result.image;
	result.textureIndex=numTextures;
	++numTextures;
	
	/* Compute the result image's pixels: */
	resultImage=Images::RGBAImage(sourceImage.getWidth(),sourceImage.getHeight());
	for(unsigned int y=0;y<resultImage.getHeight();++y)
		{
		const Images::RGBAImage::Color* sourceRow=sourceImage.getPixelRow(y);
		Images::RGBAImage::Color* destRow=resultImage.modifyPixelRow(y);
		for(unsigned int x=0;x<resultImage.getWidth();++x)
			{
			for(int i=0;i<4;++i)
				{
				float val=float(sourceRow[x][i])*factors[i];
				if(val<0.5f)
					destRow[x][i]=GLubyte(0);
				else if(val>=254.5f)
					destRow[x][i]=GLubyte(255);
				else
					destRow[x][i]=GLubyte(Math::floor(val+0.5f));
				}
			}
		}
	
	/* Return the result image ID: */
	return resultId;
	}

Doom3TextureManager::ImageID Doom3TextureManager::computeInvertAlpha(const Doom3TextureManager::ImageID& source)
	{
	/* Get a reference to the source image: */
	const Images::RGBAImage& sourceImage=imageTree.getLeafValue(source).image;
	
	/* Store a new image structure in the image tree: */
	ImageID resultId=imageTree.insertLeaf(Misc::stringPrintf("/_computedTextures/tex%06d",numTextures).c_str(),Image());
	Image& result=imageTree.getLeafValue(resultId);
	Images::RGBAImage& resultImage=result.image;
	result.textureIndex=numTextures;
	++numTextures;
	
	/* Compute the result image's pixels: */
	resultImage=Images::RGBAImage(sourceImage.getWidth(),sourceImage.getHeight());
	for(unsigned int y=0;y<resultImage.getHeight();++y)
		{
		const Images::RGBAImage::Color* sourceRow=sourceImage.getPixelRow(y);
		Images::RGBAImage::Color* destRow=resultImage.modifyPixelRow(y);
		for(unsigned int x=0;x<resultImage.getWidth();++x)
			{
			for(int i=0;i<3;++i)
				destRow[x][i]=sourceRow[x][i];
			destRow[x][3]=GLubyte(255)-destRow[x][3];
			}
		}
	
	/* Return the result image ID: */
	return resultId;
	}

Doom3TextureManager::ImageID Doom3TextureManager::computeInvertColor(const Doom3TextureManager::ImageID& source)
	{
	/* Get a reference to the source image: */
	const Images::RGBAImage& sourceImage=imageTree.getLeafValue(source).image;
	
	/* Store a new image structure in the image tree: */
	ImageID resultId=imageTree.insertLeaf(Misc::stringPrintf("/_computedTextures/tex%06d",numTextures).c_str(),Image());
	Image& result=imageTree.getLeafValue(resultId);
	Images::RGBAImage& resultImage=result.image;
	result.textureIndex=numTextures;
	++numTextures;
	
	/* Compute the result image's pixels: */
	resultImage=Images::RGBAImage(sourceImage.getWidth(),sourceImage.getHeight());
	for(unsigned int y=0;y<resultImage.getHeight();++y)
		{
		const Images::RGBAImage::Color* sourceRow=sourceImage.getPixelRow(y);
		Images::RGBAImage::Color* destRow=resultImage.modifyPixelRow(y);
		for(unsigned int x=0;x<resultImage.getWidth();++x)
			{
			for(int i=0;i<3;++i)
				destRow[x][i]=GLubyte(255)-sourceRow[x][i];
			destRow[x][3]=destRow[x][3];
			}
		}
	
	/* Return the result image ID: */
	return resultId;
	}

Doom3TextureManager::ImageID Doom3TextureManager::computeMakeIntensity(const Doom3TextureManager::ImageID& source)
	{
	/* Get a reference to the source image: */
	const Images::RGBAImage& sourceImage=imageTree.getLeafValue(source).image;
	
	/* Store a new image structure in the image tree: */
	ImageID resultId=imageTree.insertLeaf(Misc::stringPrintf("/_computedTextures/tex%06d",numTextures).c_str(),Image());
	Image& result=imageTree.getLeafValue(resultId);
	Images::RGBAImage& resultImage=result.image;
	result.textureIndex=numTextures;
	++numTextures;
	
	/* Compute the result image's pixels: */
	resultImage=Images::RGBAImage(sourceImage.getWidth(),sourceImage.getHeight());
	for(unsigned int y=0;y<resultImage.getHeight();++y)
		{
		const Images::RGBAImage::Color* sourceRow=sourceImage.getPixelRow(y);
		Images::RGBAImage::Color* destRow=resultImage.modifyPixelRow(y);
		for(unsigned int x=0;x<resultImage.getWidth();++x)
			{
			for(int i=0;i<4;++i)
				destRow[x][i]=sourceRow[x][0];
			}
		}
	
	/* Return the result image ID: */
	return resultId;
	}

Doom3TextureManager::ImageID Doom3TextureManager::computeMakeAlpha(const Doom3TextureManager::ImageID& source)
	{
	/* Get a reference to the source image: */
	const Images::RGBAImage& sourceImage=imageTree.getLeafValue(source).image;
	
	/* Store a new image structure in the image tree: */
	ImageID resultId=imageTree.insertLeaf(Misc::stringPrintf("/_computedTextures/tex%06d",numTextures).c_str(),Image());
	Image& result=imageTree.getLeafValue(resultId);
	Images::RGBAImage& resultImage=result.image;
	result.textureIndex=numTextures;
	++numTextures;
	
	/* Compute the result image's pixels: */
	resultImage=Images::RGBAImage(sourceImage.getWidth(),sourceImage.getHeight());
	for(unsigned int y=0;y<resultImage.getHeight();++y)
		{
		const Images::RGBAImage::Color* sourceRow=sourceImage.getPixelRow(y);
		Images::RGBAImage::Color* destRow=resultImage.modifyPixelRow(y);
		for(unsigned int x=0;x<resultImage.getWidth();++x)
			{
			unsigned int sum=0U;
			for(int i=0;i<3;++i)
				{
				sum+=sourceRow[x][i];
				destRow[x][i]=GLubyte(255);
				}
			destRow[x][3]=GLubyte((sum+2U)/3U);
			}
		}
	
	/* Return the result image ID: */
	return resultId;
	}

Doom3TextureManager::RenderContext Doom3TextureManager::start(GLContextData& contextData) const
	{
	/* Create a render context: */
	RenderContext result(contextData.retrieveDataItem<DataItem>(this));
	
	return result;
	}

void Doom3TextureManager::bindTexture(Doom3TextureManager::RenderContext& renderContext,const Doom3TextureManager::ImageID& image) const
	{
	glBindTexture(GL_TEXTURE_2D,renderContext.dataItem->textureObjectIds[imageTree.getLeafValue(image).textureIndex]);
	}

void Doom3TextureManager::finish(Doom3TextureManager::RenderContext& renderContext) const
	{
	}

}
