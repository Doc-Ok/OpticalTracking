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

#ifndef SCENEGRAPH_INTERNAL_DOOM3TEXTUREMANAGER_INCLUDED
#define SCENEGRAPH_INTERNAL_DOOM3TEXTUREMANAGER_INCLUDED

#include <GL/gl.h>
#include <GL/GLObject.h>
#include <Images/RGBAImage.h>
#include <SceneGraph/Internal/Doom3NameTree.h>

/* Forward declarations: */
namespace SceneGraph {
class Doom3FileManager;
}

namespace SceneGraph {

class Doom3TextureManager:public GLObject
	{
	/* Embedded classes: */
	private:
	struct Image // Structure to describe a texture image
		{
		/* Elements: */
		public:
		Images::RGBAImage image; // The texture image
		int textureIndex; // Index of this texture in the texture object ID array
		};
	
	typedef Doom3NameTree<Image> ImageTree; // Structure to store all requested texture images
	
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		public:
		int numTextures; // Number of requested textures
		GLuint* textureObjectIds; // Array of texture object IDs
		
		/* Constructors and destructors: */
		DataItem(int sNumTextures);
		virtual ~DataItem(void);
		};
	
	struct TextureUploader // Helper class to upload textures into OpenGL
		{
		/* Elements: */
		public:
		const Doom3TextureManager& textureManager;
		DataItem* dataItem;
		unsigned int numTextures; // Number of uploaded textures
		size_t totalTextureSize; // Total number of pixels in all uploaded textures
		
		/* Constructors and destructors: */
		TextureUploader(const Doom3TextureManager& sTextureManager,DataItem* sDataItem)
			:textureManager(sTextureManager),dataItem(sDataItem),
			 numTextures(0),totalTextureSize(0)
			{
			};
		
		/* Methods: */
		void operator()(std::string name,const Image& image);
		};
	
	public:
	typedef ImageTree::LeafID ImageID; // Handle to allow clients to reference texture images
	
	class RenderContext // Class to hold texture manager state for the current OpenGL context
		{
		friend class Doom3TextureManager;
		
		/* Elements: */
		private:
		DataItem* dataItem; // Pointer to the texture manager's data item
		
		/* Constructors and destructors: */
		private:
		RenderContext(DataItem* sDataItem)
			:dataItem(sDataItem)
			{
			}
		};
	
	/* Elements: */
	private:
	Doom3FileManager& fileManager; // Reference to the file manager used to load texture images
	int numTextures; // Number of textures currently in the image tree
	ImageTree imageTree; // The tree containing requested texture images
	
	/* Constructors and destructors: */
	public:
	Doom3TextureManager(Doom3FileManager& sFileManager); // Creates an empty texture manager loading from the given file manager
	virtual ~Doom3TextureManager(void);
	
	/* Methods from GLObject: */
	virtual void initContext(GLContextData& contextData) const; // Uploads all requested textures into texture objects
	
	/* New methods: */
	ImageID loadTexture(const char* textureName); // Loads a texture image and returns its handle
	ImageID computeHeightmap(const ImageID& source,float bumpiness); // Converts a height map into a normalized normal map
	ImageID computeAddNormals(const ImageID& source1,const ImageID& source2); // Adds and renormalizes two normal maps
	ImageID computeSmoothNormals(const ImageID& source); // Smoothes and renormalizes a normal map
	ImageID computeAdd(const ImageID& source1,const ImageID& source2); // Adds two images without renormalizing
	ImageID computeScale(const ImageID& source,const float factors[4]); // Scales the image component-wise
	ImageID computeInvertAlpha(const ImageID& source); // Inverts the image's alpha channel
	ImageID computeInvertColor(const ImageID& source); // Inverts the image's color channels
	ImageID computeMakeIntensity(const ImageID& source); // Copies the red channel to the G, B, and A channels
	ImageID computeMakeAlpha(const ImageID& source); // Sets the alpha channel to the average of the RGB channels and the RGB channels to white
	RenderContext start(GLContextData& contextData) const; // Prepares the OpenGL context for texture binding; returns a state variable to be handed back in subsequent calls
	void bindTexture(RenderContext& renderContext,const ImageID& image) const; // Binds the given texture image into the OpenGL context
	void finish(RenderContext& renderContext) const; // Finishes texture binding into the current OpenGL context
	};

}

#endif
