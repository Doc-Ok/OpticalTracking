/***********************************************************************
Doom3MaterialManager - Class to manage access to shared Doom3 material
definitions.
Copyright (c) 2007-2014 Oliver Kreylos

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

#ifndef SCENEGRAPH_INTERNAL_DOOM3MATERIALMANAGER_INCLUDED
#define SCENEGRAPH_INTERNAL_DOOM3MATERIALMANAGER_INCLUDED

#include <vector>
#include <Math/Math.h>
#include <GL/gl.h>
#include <GL/GLObject.h>
#include <GL/Extensions/GLARBShaderObjects.h>
#include <SceneGraph/Internal/Doom3TextureManager.h>
#include <SceneGraph/Internal/Doom3NameTree.h>

/* Forward declarations: */
class GLContextData;
namespace SceneGraph {
class Doom3FileManager;
class Doom3ValueSource;
}

namespace SceneGraph {

class Doom3MaterialManager:public GLObject
	{
	/* Embedded classes: */
	private:
	struct Table // Structure for tables
		{
		/* Elements: */
		public:
		char* name; // Table name
		bool snap; // Flag whether to use nearest-neighbor interpolation
		bool clamp; // Flag whether to clamp parameters to the table range rather than repeat the table
		int numValues; // Number of table values
		float* values; // Array of table values
		
		/* Constructors and destructors: */
		Table(void) // Creates empty table
			:name(0),
			 snap(false),clamp(false),
			 numValues(0),values(0)
			{
			}
		~Table(void) // Destroys a table
			{
			delete[] name;
			delete[] values;
			}
		float operator()(float parameter) const // Performs a table look-up
			{
			/* Calculate the parameter's base index: */
			int index0=int(Math::floor(parameter));
			if(snap)
				{
				/* Limit the base index to the table range: */
				if(clamp)
					{
					if(index0<0)
						index0=0;
					if(index0>numValues-1)
						index0=numValues-1;
					}
				else
					{
					index0=index0%numValues;
					if(index0<0)
						index0+=numValues;
					}
				
				/* Return the uninterpolated value: */
				return values[index0];
				}
			else
				{
				/* Limit the base index to the table range: */
				if(clamp)
					{
					if(index0<0)
						return values[0];
					else if(index0>=numValues-1)
						return values[numValues-1];
					else
						{
						float dx=parameter-float(index0);
						return values[index0]*(1.0f-dx)+values[index0+1]*dx;
						}
					}
				else
					{
					float dx=parameter-float(index0);
					index0=index0%numValues;
					if(index0<0)
						index0+=numValues;
					return values[index0]*(1.0f-dx)+values[(index0+1)%numValues]*dx;
					}
				}
			}
		};
	
	struct ExpressionEnvironment // Structure to hold global values used in evaluating an expression
		{
		/* Elements: */
		public:
		float time;
		float parm[12];
		float global[8];
		float fragmentPrograms;
		float sound;
		};
	
	class Expression // Base class for expressions that can be evaluated for every frame
		{
		/* Constructors and destructors: */
		public:
		virtual ~Expression(void)
			{
			}
		
		/* Methods: */
		virtual float evaluate(const ExpressionEnvironment& env) const =0; // Evaluates an expression
		};
	
	class ConstExpression:public Expression // Class for numeric constants
		{
		/* Elements: */
		private:
		float constant; // The numeric constant
		
		/* Constructors and destructors: */
		public:
		ConstExpression(float sConstant)
			:constant(sConstant)
			{
			}
		
		/* Methods from Expression: */
		float evaluate(const ExpressionEnvironment& env) const
			{
			return constant;
			}
		};
	
	class EnvExpression:public Expression // Class for values from the environment
		{
		/* Elements: */
		private:
		int varIndex; // Index of environment variable
		
		/* Constructors and destructors: */
		public:
		EnvExpression(int sVarIndex)
			:varIndex(sVarIndex)
			{
			}
		
		/* Methods from Expression: */
		float evaluate(const ExpressionEnvironment& env) const
			{
			if(varIndex==0)
				return env.time;
			else if(varIndex-1<12)
				return env.parm[varIndex-1];
			else if(varIndex-13<8)
				return env.global[varIndex-13];
			else if(varIndex==21)
				return env.fragmentPrograms;
			else
				return env.sound;
			}
		};
	
	class TableExpression:public Expression // Class for table look-ups
		{
		/* Elements: */
		private:
		const Table* table; // The table
		Expression* child; // Pointer to child expression
		
		/* Constructors and destructors: */
		public:
		TableExpression(const Table* sTable,Expression* sChild)
			:table(sTable),child(sChild)
			{
			}
		virtual ~TableExpression(void)
			{
			delete child;
			}
		
		/* Methods from Expression: */
		float evaluate(const ExpressionEnvironment& env) const
			{
			return table!=0?(*table)(child->evaluate(env)):0.0f;
			}
		
		/* New methods: */
		void setTable(Table* newTable) // Override table pointer
			{
			table=newTable;
			}
		};
	
	class UnOpExpression:public Expression // Class for unary operators
		{
		/* Elements: */
		private:
		int unOpIndex; // + or -
		Expression* child; // Pointer to child expression
		
		/* Constructors and destructors: */
		public:
		UnOpExpression(int sUnOpIndex,Expression* sChild)
			:unOpIndex(sUnOpIndex),child(sChild)
			{
			}
		virtual ~UnOpExpression(void)
			{
			delete child;
			}
		
		/* Methods from Expression: */
		float evaluate(const ExpressionEnvironment& env) const
			{
			switch(unOpIndex)
				{
				case 0: // + 
					return child->evaluate(env);
				
				case 1: // -
					return -child->evaluate(env);
				
				default:
					return 0.0f; // Never reached
				}
			}
		};
	
	class BinOpExpression:public Expression // Class for binary operators
		{
		/* Elements: */
		private:
		int binOpIndex; // +, -, *, /, %, ==, !=, <, <=, >, >=, &&, ||
		Expression* child0; // Pointer to first child expression
		Expression* child1; // Pointer to second child expression
		
		/* Constructors and destructors: */
		public:
		BinOpExpression(int sBinOpIndex,Expression* sChild0,Expression* sChild1)
			:binOpIndex(sBinOpIndex),child0(sChild0),child1(sChild1)
			{
			}
		virtual ~BinOpExpression(void)
			{
			delete child0;
			delete child1;
			}
		
		/* Methods from Expression: */
		float evaluate(const ExpressionEnvironment& env) const
			{
			switch(binOpIndex)
				{
				case 0: // +
					return child0->evaluate(env)+child1->evaluate(env);
				
				case 1: // -
					return child0->evaluate(env)-child1->evaluate(env);
				
				case 2: // *
					return child0->evaluate(env)*child1->evaluate(env);
				
				case 3: // /
					return child0->evaluate(env)/child1->evaluate(env);
				
				case 4: // %
					return Math::mod(child0->evaluate(env),child1->evaluate(env));
				
				case 5: // ==
					return child0->evaluate(env)==child1->evaluate(env)?1.0f:0.0;
				
				case 6: // !=
					return child0->evaluate(env)!=child1->evaluate(env)?1.0f:0.0;
				
				case 7: // <
					return child0->evaluate(env)<child1->evaluate(env)?1.0f:0.0;
				
				case 8: // <=
					return child0->evaluate(env)<=child1->evaluate(env)?1.0f:0.0;
				
				case 9: // >
					return child0->evaluate(env)>=child1->evaluate(env)?1.0f:0.0;
				
				case 10: // >=
					return child0->evaluate(env)>child1->evaluate(env)?1.0f:0.0;
				
				case 11: // &&
					return (child0->evaluate(env)!=0.0f&&child1->evaluate(env)!=0.0f)?1.0f:0.0f;
				
				case 12: // ||
					return (child0->evaluate(env)!=0.0f||child1->evaluate(env)!=0.0f)?1.0f:0.0f;
				
				default:
					return 0.0f; // Never reached
				}
			}
		};
	
	struct Material // Structure to represent materials
		{
		/* Embedded classes: */
		public:
		struct Stage // Structure representing rendering stages
			{
			/* Embedded classes: */
			public:
			enum BlendType // Enumerated type for special blend types
				{
				Unknown,DiffuseMap,SpecularMap,BumpMap
				};
			
			/* Elements: */
			bool enabled; // Flag whether the stage is enabled or disabled
			BlendType blendType; // Blend type
			GLenum blendSrc,blendDst; // Source and destination blending factors
			GLenum texInterpMode; // Texture interpolation mode
			GLenum texCoordClampMode; // Texture coordinate clamping mode
			Doom3TextureManager::ImageID map; // Pointer to the texture image used by this stage
			bool channelMasks[5]; // Flags whether each of the R, G, B, A, depth channels is masked (write-protected)
			GLfloat alphaTest; // Only write fragments with A value greater than this value
			GLfloat vertexColor[4]; // RGBA vertex color
			};
		
		/* Elements: */
		bool loaded; // Flag if the material has already been loaded
		bool twoSided; // Flag for two-sided rendering
		bool translucent; // Flag for alpha-blended rendering
		int numStages; // Number of rendering stages
		Stage* stages; // Array of rendering stages
		int bumpMapStage; // Index of the rendering stage containing the normal map
		int diffuseMapStage; // Index of the rendering stage containing the diffuse map
		int specularMapStage; // Index of the rendering stage containing the specular map
		int additiveMapStage; // Index of the rendering stage containing the (last) additive map
		int collisionFlags; // Mask of collision flags associated with the material
		int programIndex; // Index of the shader program to use for this material (-1 if invalid)
		
		/* Constructors and destructors: */
		Material(void); // Creates uninitialized material
		
		/* Methods: */
		Stage& addStage(void); // Adds a stage to a material
		};
	
	typedef Doom3NameTree<Material> MaterialTree; // Structure to store named materials
	
	class MaterialDataDeleter // Helper class to delete materials stored in the tree
		{
		public:
		void operator()(std::string name,Material& material) const
			{
			delete[] material.stages;
			}
		};
	
	struct DataItem:public GLObject::DataItem // Structure for per-context data
		{
		/* Elements: */
		public:
		static const int numShaders=6; // Number of shader programs for different material types
		bool haveShaders; // Flag whether the local OpenGL supports GLSL shaders
		GLhandleARB vertexShaderObject; // Handle for the vertex shader object
		GLhandleARB fragmentShaderObjects[numShaders]; // Handle for the fragment shader objects for different material types
		GLhandleARB programObjects[numShaders]; // Handle for the linked shader programs for different material types
		GLint tangentAttributeIndices[2]; // Indices of tanget attributes for s and t in all shader programs (must be identical)
		GLint normalMapUniformIndices[numShaders]; // Indices of normal map sampler uniform variable in shader programs
		GLint diffuseMapUniformIndices[numShaders]; // Indices of diffuse map sampler uniform variable in shader programs
		GLint specularMapUniformIndices[numShaders]; // Indices of specular map sampler uniform variable in shader programs
		GLint additiveMapUniformIndices[numShaders]; // Indices of additive map sampler uniform variable in shader programs
		
		/* Constructors and destructors: */
		DataItem(void);
		virtual ~DataItem(void);
		};
	
	public:
	typedef MaterialTree::LeafID MaterialID; // Type to identify materials to clients
	
	class RenderContext // Class to hold material manager state for the current OpenGL context
		{
		friend class Doom3MaterialManager;
		
		/* Elements: */
		private:
		GLContextData& contextData; // Reference to the GLContextData object for the current context
		DataItem* dataItem; // Pointer to the material manager's data item
		Doom3TextureManager::RenderContext tmRc; // Render context of the texture manager
		bool useDefaultPipeline; // Flag whether to use the fixed-functionality OpenGL pipeline
		int currentProgramIndex; // Index of shader program currently installed in OpenGL context
		
		/* Constructors and destructors: */
		private:
		RenderContext(GLContextData& sContextData,DataItem* sDataItem,Doom3TextureManager& textureManager,bool sUseDefaultPipeline)
			:contextData(sContextData),
			 dataItem(sDataItem),
			 tmRc(textureManager.start(contextData)),
			 useDefaultPipeline(sUseDefaultPipeline||!dataItem->haveShaders),
			 currentProgramIndex(-1)
			{
			}
		
		/* Methods: */
		public:
		void suspend(void); // Temporarily suspends rendering using Doom3 material properties
		};
	
	/* Elements: */
	private:
	Doom3TextureManager& textureManager; // Reference to the texture manager
	std::vector<Table*> tables; // List of tables
	MaterialTree materialTree; // Tree of requested materials
	
	/* Private methods: */
	Expression* parseTerm(Doom3ValueSource& source);
	Expression* parseExp(Doom3ValueSource& source);
	Expression* parseExpression(Doom3ValueSource& source);
	Doom3TextureManager::ImageID parseImageMap(Doom3ValueSource& source);
	
	/* Constructors and destructors: */
	public:
	Doom3MaterialManager(Doom3TextureManager& sTextureManager); // Creates an empty material manager
	~Doom3MaterialManager(void); // Destroys the material manager
	
	/* Methods: */
	virtual void initContext(GLContextData& contextData) const;
	MaterialID loadMaterial(const char* materialName); // Requests a material
	void loadMaterials(Doom3FileManager& fileManager); // Creates all requested materials and loads required texture images
	void parseMaterialFile(Doom3FileManager& fileManager,const char* fileName); // Parses the material file of the given name
	int getCollisionFlags(const MaterialID& materialID) const // Returns the collision flags associated with the given material
		{
		return materialTree.getLeafValue(materialID).collisionFlags;
		}
	RenderContext start(GLContextData& contextData,bool useDefaultPipeline) const; // Prepares the OpenGL context for material rendering; returns a state variable to be handed back in subsequent calls; uses fixed-functionality OpenGL pipeline if flag is true
	GLint getTangentAttributeIndex(RenderContext& renderContext,int tangentIndex) const; // Returns the index of the generic vertex attribute bound to the tangent vector used in the vertex program or -1 if not used
	void setStage(RenderContext& renderContext,const Material::Stage& stage,int textureUnit) const; // Uploads the texture map stored with a material state into the given texture unit
	bool setMaterial(RenderContext& renderContext,const MaterialID& materialID) const; // Installs the given material in the OpenGL context; returns true if the surface needs to be rendered
	void disableMaterial(RenderContext& renderContext) const; // Temporarily disables material rendering in the OpenGL context
	void finish(RenderContext& renderContext) const; // Finishes material rendering into the current OpenGL context
	};

}

#endif
