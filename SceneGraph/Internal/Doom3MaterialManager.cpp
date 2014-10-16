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

#include <SceneGraph/Internal/Doom3MaterialManager.h>

// DEBUGGING
#include <iostream>

#include <ctype.h>
#include <string>
#include <Misc/ThrowStdErr.h>
#include <GL/gl.h>
#include <GL/GLContextData.h>
#include <GL/GLExtensionManager.h>
#include <GL/Extensions/GLARBMultitexture.h>
#include <GL/Extensions/GLARBVertexShader.h>
#include <GL/Extensions/GLARBFragmentShader.h>
#include <SceneGraph/Internal/Doom3FileManager.h>
#include <SceneGraph/Internal/Doom3CollisionFlags.h>
#include <SceneGraph/Internal/Doom3ValueSource.h>
#include <SceneGraph/Internal/Config.h>

namespace SceneGraph {

namespace {

/**************
Helper classes:
**************/

class MaterialFileParser // Helper class to parse material files
	{
	/* Elements: */
	private:
	Doom3MaterialManager& materialManager; // Reference back to the manager
	Doom3FileManager& fileManager; // Reference to the file manager
	
	/* Constructors and destructors: */
	public:
	MaterialFileParser(Doom3MaterialManager& sMaterialManager,Doom3FileManager& sFileManager)
		:materialManager(sMaterialManager),fileManager(sFileManager)
		{
		};
	
	/* Methods: */
	void operator()(const char* pathName) // Processes the material file of the given name
		{
		/* Just refer back to the material manager: */
		materialManager.parseMaterialFile(fileManager,pathName);
		};
	};

/****************
Helper functions:
****************/

bool equal(const std::string& s1,const char* s2)
	{
	std::string::const_iterator s1It;
	const char* s2Ptr;
	for(s1It=s1.begin(),s2Ptr=s2;s1It!=s1.end()&&*s2Ptr!='\0';++s1It,++s2Ptr)
		if(tolower(*s1It)!=tolower(*s2Ptr))
			break;
	return s1It==s1.end()&&*s2Ptr=='\0';
	}

}

/***********************************************
Methods of class Doom3MaterialManager::Material:
***********************************************/

Doom3MaterialManager::Material::Material(void)
	:loaded(false),
	 twoSided(false),translucent(false),
	 numStages(0),stages(0),
	 bumpMapStage(-1),diffuseMapStage(-1),specularMapStage(-1),additiveMapStage(-1),
	 collisionFlags(CF_SOLID),
	 programIndex(-1)
	{
	}

Doom3MaterialManager::Material::Stage& Doom3MaterialManager::Material::addStage(void)
	{
	/* Allocate a new stage array: */
	Stage* newStages=new Stage[numStages+1];
	
	/* Copy the old stage array: */
	for(int i=0;i<numStages;++i)
		newStages[i]=stages[i];
	delete[] stages;
	
	/* Set the new stage array: */
	++numStages;
	stages=newStages;
	
	/* Initialize the stage to the default settings: */
	Stage& stage=stages[numStages-1];
	stage.enabled=true;
	stage.blendType=Stage::Unknown;
	stage.blendSrc=GL_ONE;
	stage.blendDst=GL_ZERO;
	stage.texInterpMode=GL_LINEAR;
	stage.texCoordClampMode=GL_REPEAT;
	for(int i=0;i<5;++i)
		stage.channelMasks[i]=true;
	stage.alphaTest=0.0f;
	for(int i=0;i<4;++i)
		stage.vertexColor[i]=1.0f;
	
	/* Return the new stage: */
	return stage;
	}

/***********************************************
Methods of class Doom3MaterialManager::DataItem:
***********************************************/

Doom3MaterialManager::DataItem::DataItem(void)
	:haveShaders(GLARBMultitexture::isSupported()&&GLARBShaderObjects::isSupported()&&GLARBVertexShader::isSupported()&&GLARBFragmentShader::isSupported())
	{
	/* Check for the required OpenGL extensions: */
	if(haveShaders)
		{
		/* Initialize the required extensions: */
		GLARBMultitexture::initExtension();
		GLARBShaderObjects::initExtension();
		GLARBVertexShader::initExtension();
		GLARBFragmentShader::initExtension();
		
		/* Initialize shader states: */
		vertexShaderObject=0;
		for(int i=0;i<2;++i)
			tangentAttributeIndices[i]=-1;
		for(int i=0;i<numShaders;++i)
			{
			fragmentShaderObjects[i]=0;
			programObjects[i]=0;
			normalMapUniformIndices[i]=-1;
			diffuseMapUniformIndices[i]=-1;
			specularMapUniformIndices[i]=-1;
			additiveMapUniformIndices[i]=-1;
			}
		
		/* Create the shader and program objects: */
		vertexShaderObject=glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
		for(int i=0;i<numShaders;++i)
			{
			fragmentShaderObjects[i]=glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
			programObjects[i]=glCreateProgramObjectARB();
			}
		
		/* Attach the shaders to the programs: */
		for(int i=0;i<numShaders;++i)
			{
			glAttachObjectARB(programObjects[i],vertexShaderObject);
			glAttachObjectARB(programObjects[i],fragmentShaderObjects[i]);
			}
		}
	}

Doom3MaterialManager::DataItem::~DataItem(void)
	{
	if(haveShaders)
		{
		/* Detach the shaders from the programs: */
		for(int i=0;i<numShaders;++i)
			{
			glDetachObjectARB(programObjects[i],vertexShaderObject);
			glDetachObjectARB(programObjects[i],fragmentShaderObjects[i]);
			}
		
		/* Delete the shader and program objects: */
		glDeleteObjectARB(vertexShaderObject);
		for(int i=0;i<numShaders;++i)
			{
			glDeleteObjectARB(programObjects[i]);
			glDeleteObjectARB(fragmentShaderObjects[i]);
			}
		}
	}

/****************************************************
Methods of class Doom3MaterialManager::RenderContext:
****************************************************/

void Doom3MaterialManager::RenderContext::suspend(void)
	{
	/* Disable the currently-used shader program: */
	if(currentProgramIndex!=-1)
		{
		currentProgramIndex=-1;
		glUseProgramObjectARB(0);
		}
	
	/* Return to default OpenGL state: */
	glEnable(GL_CULL_FACE);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_FALSE);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glActiveTextureARB(GL_TEXTURE0_ARB);
	}

namespace {

/***************
Helper function:
***************/

inline void checkImageMapSyntax(Doom3ValueSource& source,char expectedCharacter)
	{
	if(source.readChar()!=expectedCharacter)
		Misc::throwStdErr("Doom3MaterialManager::parseImageMap: Malformed image map at %s",source.where().c_str());
	}

}

/*************************************
Methods of class Doom3MaterialManager:
*************************************/

Doom3MaterialManager::Expression* Doom3MaterialManager::parseTerm(Doom3ValueSource& source)
	{
	/* Check the next token: */
	if(source.peekc()=='(')
		{
		source.readChar(); // Skip the opening parenthesis
		
		/* Parse an expression: */
		Expression* result=parseExp(source);
		
		/* Check the closing parenthesis: */
		if(source.readChar()!=')')
			{
			delete result;
			Misc::throwStdErr("Doom3MaterialManager::parseTerm: Missing closing parenthesis at %s",source.where().c_str());
			}
		
		return result;
		}
	else if(source.peekc()=='+'||source.peekc()=='-')
		{
		/* Check if it's a numeric constant or a unary operator: */
		int sign=source.getChar();
		if(source.peekc()=='.'||(source.peekc()>='0'&&source.peekc()<='9'))
			{
			/* Read and return the constant: */
			source.ungetChar(sign);
			return new ConstExpression(float(source.readNumber()));
			}
		else
			{
			/* Return a unary operator: */
			source.skipWs();
			return new UnOpExpression(sign=='-'?1:0,parseExp(source));
			}
		}
	else if(source.peekc()=='.'||(source.peekc()>='0'&&source.peekc()<='9'))
		{
		/* Return a numeric constant: */
		return new ConstExpression(float(source.readNumber()));
		}
	else
		{
		std::string term=source.readString();
		if(equal(term,"time"))
			return new EnvExpression(0);
		else if(strncasecmp(term.c_str(),"parm",4)==0)
			{
			int parmIndex=atoi(term.c_str()+4);
			if(parmIndex<0||parmIndex>=12)
				Misc::throwStdErr("Doom3MaterialManager::parseTerm: Unknown variable %s at %s",term.c_str(),source.where().c_str());
			return new EnvExpression(parmIndex+1);
			}
		else if(strncasecmp(term.c_str(),"global",6)==0)
			{
			int globalIndex=atoi(term.c_str()+6);
			if(globalIndex<0||globalIndex>=8)
				Misc::throwStdErr("Doom3MaterialManager::parseTerm: Unknown variable %s at %s",term.c_str(),source.where().c_str());
			return new EnvExpression(globalIndex+13);
			}
		else if(equal(term,"fragmentPrograms"))
			return new EnvExpression(21);
		else if(equal(term,"sound"))
			return new EnvExpression(22);
		else
			{
			/* Find a table matching the term name: */
			Table* table=0;
			for(std::vector<Table*>::iterator tIt=tables.begin();tIt!=tables.end()&&table==0;++tIt)
				if(term==(*tIt)->name)
					table=*tIt;
			#if 0
			if(table==0)
				Misc::throwStdErr("Doom3MaterialManager::parseTerm: Unknown table %s at %s",term.c_str(),source.where().c_str());
			#else
			// DEBUGGING
			std::cout<<"Ignoring unknown table "<<term<<" at "<<source.where()<<std::endl;
			#endif
			
			if(source.readChar()!='[')
				Misc::throwStdErr("Doom3MaterialManager::parseTerm: Malformed table expression at %s",source.where().c_str());
			
			Expression* result=new TableExpression(table,parseExp(source));
			
			if(source.readChar()!=']')
				{
				delete result;
				Misc::throwStdErr("Doom3MaterialManager::parseTerm: Malformed table expression at %s",source.where().c_str());
				}
			
			return result;
			}
		}
	}

Doom3MaterialManager::Expression* Doom3MaterialManager::parseExp(Doom3ValueSource& source)
	{
	/* Parse the first term: */
	Expression* firstTerm=parseTerm(source);
	
	/* Check if the next token is a binary operator: */
	if(source.peekc()=='+'||source.peekc()=='-'||source.peekc()=='*'||source.peekc()=='/'||source.peekc()=='%')
		{
		switch(source.readChar())
			{
			case '+':
				return new BinOpExpression(0,firstTerm,parseExp(source));
			
			case '-':
				return new BinOpExpression(1,firstTerm,parseExp(source));
			
			case '*':
				return new BinOpExpression(2,firstTerm,parseExp(source));
			
			case '/':
				return new BinOpExpression(3,firstTerm,parseExp(source));
			
			case '%':
				return new BinOpExpression(4,firstTerm,parseExp(source));
			
			default:
				return 0; // Never reached
			}
		}
	else if(source.peekc()=='='||source.peekc()=='!')
		{
		int first=source.getChar();
		if(source.peekc()=='=')
			{
			source.readChar();
			return new BinOpExpression(first=='='?5:6,firstTerm,parseExp(source));
			}
		else
			{
			source.ungetChar(first);
			return firstTerm;
			}
		}
	else if(source.peekc()=='<'||source.peekc()=='>')
		{
		int first=source.getChar();
		if(source.peekc()=='=')
			{
			source.readChar();
			return new BinOpExpression(first=='<'?8:10,firstTerm,parseExp(source));
			}
		else
			{
			source.skipWs();
			return new BinOpExpression(first=='<'?7:9,firstTerm,parseExp(source));
			}
		}
	else if(source.peekc()=='&'||source.peekc()=='|')
		{
		int first=source.getChar();
		if(source.peekc()==first)
			{
			source.readChar();
			return new BinOpExpression(first=='&'?11:12,firstTerm,parseExp(source));
			}
		else
			{
			source.ungetChar(first);
			return firstTerm;
			}
		}
	else
		return firstTerm;
	}

Doom3MaterialManager::Expression* Doom3MaterialManager::parseExpression(Doom3ValueSource& source)
	{
	/* Temporarily enable math operators as punctuation: */
	source.setPunctuation('+',true);
	source.setPunctuation('-',true);
	source.setPunctuation('*',true);
	source.setPunctuation('/',true);
	source.setPunctuation('%',true);
	
	/* Parse the expression: */
	Expression* result=parseExp(source);
	
	/* Disable math operators as punctuation: */
	source.setPunctuation('+',false);
	source.setPunctuation('-',false);
	source.setPunctuation('*',false);
	source.setPunctuation('/',false);
	source.setPunctuation('%',false);
	
	return result;
	}

Doom3TextureManager::ImageID Doom3MaterialManager::parseImageMap(Doom3ValueSource& source)
	{
	/* Read the map name sourceen: */
	std::string mapName=source.readString();
	
	/* Check for image functions: */
	if(equal(mapName,"heightmap"))
		{
		checkImageMapSyntax(source,'(');
		Doom3TextureManager::ImageID baseImage=parseImageMap(source);
		checkImageMapSyntax(source,',');
		float bumpiness=float(source.readNumber());
		checkImageMapSyntax(source,')');
		
		return textureManager.computeHeightmap(baseImage,bumpiness);
		}
	else if(equal(mapName,"addnormals"))
		{
		checkImageMapSyntax(source,'(');
		Doom3TextureManager::ImageID baseImage1=parseImageMap(source);
		checkImageMapSyntax(source,',');
		Doom3TextureManager::ImageID baseImage2=parseImageMap(source);
		checkImageMapSyntax(source,')');
		
		return textureManager.computeAddNormals(baseImage1,baseImage2);
		}
	else if(equal(mapName,"smoothnormals"))
		{
		checkImageMapSyntax(source,'(');
		Doom3TextureManager::ImageID baseImage=parseImageMap(source);
		checkImageMapSyntax(source,')');
		
		return textureManager.computeSmoothNormals(baseImage);
		}
	else if(equal(mapName,"add"))
		{
		checkImageMapSyntax(source,'(');
		Doom3TextureManager::ImageID baseImage1=parseImageMap(source);
		checkImageMapSyntax(source,',');
		Doom3TextureManager::ImageID baseImage2=parseImageMap(source);
		checkImageMapSyntax(source,')');
		
		return textureManager.computeAdd(baseImage1,baseImage2);
		}
	else if(equal(mapName,"scale"))
		{
		checkImageMapSyntax(source,'(');
		Doom3TextureManager::ImageID baseImage=parseImageMap(source);
		checkImageMapSyntax(source,',');
		
		/* Read the first scale factor: */
		float factors[4]={0.0f,0.0f,0.0f,0.0f};
		factors[0]=float(source.readNumber());
		
		/* Read up to three more scale factors: */
		for(int i=1;i<4&&source.peekc()==',';++i)
			{
			source.readChar(); // Skip the comma
			
			/* Read the scale factor: */
			factors[i]=float(source.readNumber());
			}
		
		checkImageMapSyntax(source,')');
		
		return textureManager.computeScale(baseImage,factors);
		}
	else if(equal(mapName,"invertAlpha"))
		{
		checkImageMapSyntax(source,'(');
		Doom3TextureManager::ImageID baseImage=parseImageMap(source);
		checkImageMapSyntax(source,')');
		
		return textureManager.computeInvertAlpha(baseImage);
		}
	else if(equal(mapName,"invertColor"))
		{
		checkImageMapSyntax(source,'(');
		Doom3TextureManager::ImageID baseImage=parseImageMap(source);
		checkImageMapSyntax(source,')');
		
		return textureManager.computeInvertColor(baseImage);
		}
	else if(equal(mapName,"makeIntensity"))
		{
		checkImageMapSyntax(source,'(');
		Doom3TextureManager::ImageID baseImage=parseImageMap(source);
		checkImageMapSyntax(source,')');
		
		return textureManager.computeMakeIntensity(baseImage);
		}
	else if(equal(mapName,"makeAlpha"))
		{
		checkImageMapSyntax(source,'(');
		Doom3TextureManager::ImageID baseImage=parseImageMap(source);
		checkImageMapSyntax(source,')');
		
		return textureManager.computeMakeAlpha(baseImage);
		}
	else if(equal(mapName,"downsize")) // A Quake 4 thing, actually, but we can parse it
		{
		checkImageMapSyntax(source,'(');
		Doom3TextureManager::ImageID baseImage=parseImageMap(source);
		checkImageMapSyntax(source,',');
		
		/* Ignore the scale factor: */
		source.readNumber();
		checkImageMapSyntax(source,')');
		
		return baseImage;
		}
	else
		return textureManager.loadTexture(mapName.c_str());
	}

Doom3MaterialManager::Doom3MaterialManager(Doom3TextureManager& sTextureManager)
	:textureManager(sTextureManager)
	{
	}

Doom3MaterialManager::~Doom3MaterialManager(void)
	{
	/* Destroy all stored materials: */
	MaterialDataDeleter mdd;
	materialTree.forEachLeaf(mdd);
	
	/* Destroy all tables: */
	for(std::vector<Table*>::iterator tIt=tables.begin();tIt!=tables.end();++tIt)
		delete *tIt;
	}

void Doom3MaterialManager::initContext(GLContextData& contextData) const
	{
	/* Create a new data item: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	
	if(dataItem->haveShaders)
		{
		std::string shaderDir=SCENEGRAPH_CONFIG_DOOM3MATERIALMANAGER_SHADERDIR;
		shaderDir.push_back('/');
		
		/* Load the shared vertex program: */
		glCompileShaderFromFile(dataItem->vertexShaderObject,(shaderDir+"BumpMapping.vs").c_str());
		
		/* Load and compile the fragment shaders and link the shader programs: */
		static const char* fragmentShaderNames[DataItem::numShaders]=
			{
			"BumpDiffuse.fs","BumpDiffuseAdd.fs",
			"BumpSpecular.fs","BumpSpecularAdd.fs",
			"BumpDiffuseSpecular.fs","BumpDiffuseSpecularAdd.fs"
			};
		for(int i=0;i<DataItem::numShaders;++i)
			{
			/* Load and compile the fragment shader: */
			glCompileShaderFromFile(dataItem->fragmentShaderObjects[i],(shaderDir+fragmentShaderNames[i]).c_str());
			
			/* Link the program: */
			glLinkProgramARB(dataItem->programObjects[i]);
			
			/* Check if the program linked successfully: */
			GLint linkStatus;
			glGetObjectParameterivARB(dataItem->programObjects[i],GL_OBJECT_LINK_STATUS_ARB,&linkStatus);
			if(!linkStatus)
				{
				/* Get some more detailed information: */
				GLcharARB linkLogBuffer[2048];
				GLsizei linkLogSize;
				glGetInfoLogARB(dataItem->programObjects[i],sizeof(linkLogBuffer),&linkLogSize,linkLogBuffer);
				
				/* Signal an error: */
				Misc::throwStdErr("Doom3MaterialManager::initContext: Error \"%s\" while linking shader",linkLogBuffer);
				}
			
			#if 0
			/* Query all active attributes: */
			GLint numAttributes;
			glGetObjectParameterivARB(dataItem->programObjects[i],GL_OBJECT_ACTIVE_ATTRIBUTES_ARB,&numAttributes);
			GLint maxAttributeLength;
			glGetObjectParameterivARB(dataItem->programObjects[i],GL_OBJECT_ACTIVE_ATTRIBUTE_MAX_LENGTH_ARB,&maxAttributeLength);
			GLchar* attributeName=new GLchar[maxAttributeLength];
			for(int j=0;j<numAttributes;++j)
				{
				GLsizei length;
				GLint size;
				GLenum type;
				glGetActiveAttribARB(dataItem->programObjects[i],j,maxAttributeLength,&length,&size,&type,attributeName);
				printf("%d %d %d %s\n",int(length),int(size),int(type),attributeName);
				}
			delete[] attributeName;
			
			/* Query all active uniforms: */
			GLint numUniforms;
			glGetObjectParameterivARB(dataItem->programObjects[i],GL_OBJECT_ACTIVE_UNIFORMS_ARB,&numUniforms);
			GLint maxUniformLength;
			glGetObjectParameterivARB(dataItem->programObjects[i],GL_OBJECT_ACTIVE_UNIFORM_MAX_LENGTH_ARB,&maxUniformLength);
			GLchar* uniformName=new GLchar[maxUniformLength];
			for(int j=0;j<numUniforms;++j)
				{
				GLsizei length;
				GLint size;
				GLenum type;
				glGetActiveUniformARB(dataItem->programObjects[i],j,maxUniformLength,&length,&size,&type,uniformName);
				printf("%d %d %d %s\n",int(length),int(size),int(type),uniformName);
				}
			delete[] uniformName;
			#endif
			
			/* Get the tangent attribute indices: */
			int tangentAttributeIndexS=glGetAttribLocationARB(dataItem->programObjects[i],"tangentS");
			if(i==0)
				dataItem->tangentAttributeIndices[0]=tangentAttributeIndexS;
			else if(dataItem->tangentAttributeIndices[0]!=tangentAttributeIndexS)
				Misc::throwStdErr("Doom3MaterialManager::initContext: Mismatching tangent attribute indices in shader programs");
			int tangentAttributeIndexT=glGetAttribLocationARB(dataItem->programObjects[i],"tangentT");
			if(i==0)
				dataItem->tangentAttributeIndices[1]=tangentAttributeIndexT;
			else if(dataItem->tangentAttributeIndices[1]!=tangentAttributeIndexT)
				Misc::throwStdErr("Doom3MaterialManager::initContext: Mismatching tangent attribute indices in shader programs");
			
			/* Get the texture map sampler uniform indices: */
			dataItem->normalMapUniformIndices[i]=glGetUniformLocationARB(dataItem->programObjects[i],"normalMap");
			dataItem->diffuseMapUniformIndices[i]=glGetUniformLocationARB(dataItem->programObjects[i],"diffuseMap");
			dataItem->specularMapUniformIndices[i]=glGetUniformLocationARB(dataItem->programObjects[i],"specularMap");
			dataItem->additiveMapUniformIndices[i]=glGetUniformLocationARB(dataItem->programObjects[i],"additiveMap");
			}
		}
	}

Doom3MaterialManager::MaterialID Doom3MaterialManager::loadMaterial(const char* materialName)
	{
	/* Check if the requested material is already there: */
	MaterialID materialID=materialTree.findLeaf(materialName);
	if(!materialID.isValid())
		{
		/* Store a new material structure in the image tree: */
		materialID=materialTree.insertLeaf(materialName,Material());
		}
	
	/* Return the new material: */
	return materialID;
	}

void Doom3MaterialManager::loadMaterials(Doom3FileManager& fileManager)
	{
	/* Process all files with ending .mtr in the file manager: */
	MaterialFileParser mtfp(*this,fileManager);
	fileManager.searchFileTree(mtfp,"mtr");
	}

void Doom3MaterialManager::parseMaterialFile(Doom3FileManager& fileManager,const char* fileName)
	{
	/* Create a default environment: */
	ExpressionEnvironment currentEnv;
	currentEnv.time=0.0f;
	for(int i=0;i<12;++i)
		currentEnv.parm[i]=0.0f;
	for(int i=0;i<8;++i)
		currentEnv.global[i]=0.0f;
	currentEnv.fragmentPrograms=1.0f;
	currentEnv.sound=0.0f;
	
	/* Read the material file and create a value source: */
	Doom3ValueSource source(fileManager.getFile(fileName),fileName);
	
	/* Parse all material definitions in the file: */
	while(!source.eof())
		{
		std::string materialName=source.readString();
		if(materialName=="particle"||materialName=="skin")
			{
			/* Read, but otherwise ignore the skin/particle/whatever: */
			source.skipString(); // Skip the thing name
			if(source.readChar()!='{')
				Misc::throwStdErr("Doom3MaterialManager::parseMaterialFile: Malformed particle/skin/etc. definition at %s",source.where().c_str());
			
			/* Just skip all tokens until the matching closing brace: */
			int braceLevel=1;
			while(!source.eof()&&braceLevel>0)
				{
				if(source.peekc()=='{')
					++braceLevel;
				else if(source.peekc()=='}')
					--braceLevel;
				source.skipString();
				}
			}
		else if(equal(materialName,"guide"))
			{
			/* A guide is a nifty Quake 4 shorthand for material definitions; ignore it for now: */
			source.skipString(); // Ignore the target material name
			source.skipString(); // Ignore the guide name
			if(source.readChar()!='(')
				Misc::throwStdErr("Doom3MaterialManager::parseMaterialFile: Malformed guided material definition at %s",source.where().c_str());
			
			/* Skip the first parameter: */
			source.skipString();
			
			/* Skip all other parameters: */
			while(source.peekc()==',')
				{
				source.readChar(); // Skip the comma
				source.skipString(); // Skip the guide argument
				}
			if(source.readChar()!=')')
				Misc::throwStdErr("Doom3MaterialManager::parseMaterialFile: Malformed guided material definition at %s",source.where().c_str());
			}
		else if(materialName=="table"||materialName=="Table")
			{
			/* Create a new table: */
			Table* table=new Table;
			tables.push_back(table);
			
			/* Read the table name: */
			std::string tableName=source.readString();
			table->name=new char[tableName.size()+1];
			memcpy(table->name,tableName.c_str(),tableName.size()+1);
			
			if(source.readChar()!='{')
				Misc::throwStdErr("Doom3MaterialManager::parseMaterialFile: Malformed table definition at %s",source.where().c_str());
			
			/* Check the optional keywords: */
			bool haveExtraBrace=false;
			while(!source.eof()&&!(source.peekc()=='.'||(source.peekc()>='0'&&source.peekc()<='9')))
				{
				std::string keyword=source.readString();
				if(equal(keyword,"snap"))
					table->snap=true;
				else if(equal(keyword,"clamp"))
					table->clamp=true;
				else if(keyword=="{")
					{
					haveExtraBrace=true;
					break;
					}
				else
					Misc::throwStdErr("Doom3MaterialManager::parseMaterialFile: Unrecognized table keyword %s at %s",keyword.c_str(),source.where().c_str());
				}
			
			/* Read the table values: */
			std::vector<float> tableValues;
			while(!source.eof()&&source.peekc()!='}')
				{
				/* Store the table value: */
				tableValues.push_back(float(source.readNumber()));
				
				/* Skip an optional comma: */
				if(source.peekc()==',')
					source.readChar();
				}
			if(haveExtraBrace&&source.readChar()!='}')
				Misc::throwStdErr("Doom3MaterialManager::parseMaterialFile: Malformed table definition at %s",source.where().c_str());
			
			/* Store the table values: */
			table->numValues=int(tableValues.size());
			table->values=new float[table->numValues];
			for(int i=0;i<table->numValues;++i)
				table->values[i]=tableValues[i];
			
			if(source.readChar()!='}')
				Misc::throwStdErr("Doom3MaterialManager::parseMaterialFile: Malformed table definition at %s",source.where().c_str());
			}
		else
			{
			/* Skip the optional material keyword: */
			if(equal(materialName,"material"))
				materialName=source.readString();
			
			/* Check if a material of this name has been requested, and is not yet loaded: */
			MaterialID materialID=materialTree.findLeaf(materialName.c_str());
			if(materialID.isValid()&&!materialTree.getLeafValue(materialID).loaded)
				{
				/* Get a handle to the material object: */
				Material& material=materialTree.getLeafValue(materialID);
				
				if(source.readChar()!='{')
					Misc::throwStdErr("Doom3MaterialManager::parseMaterialFile: Malformed material definition at %s",source.where().c_str());
				
				/* Read and parse all tokens from the material section: */
				while(!source.eof()&&source.peekc()!='}')
					{
					/* Read the next token: */
					std::string token=source.readString();
					
					if(token=="{")
						{
						/* Create a new rendering stage: */
						Material::Stage& stage=material.addStage();
						
						/* Parse the rendering stage: */
						while(!source.eof()&&source.peekc()!='}')
							{
							/* Read the next setting: */
							std::string setting=source.readString();
							
							if(equal(setting,"if"))
								{
								/* Parse the expression: */
								Expression* exp=parseExpression(source);
								stage.enabled=exp->evaluate(currentEnv)!=0.0f;
								delete exp;
								}
							else if(equal(setting,"blend"))
								{
								/* Parse the blend type: */
								std::string blendType=source.readString();
								if(equal(blendType,"blend"))
									{
									stage.blendSrc=GL_SRC_ALPHA;
									stage.blendDst=GL_ONE_MINUS_SRC_ALPHA;
									}
								else if(equal(blendType,"add"))
									{
									stage.blendSrc=GL_ONE;
									stage.blendDst=GL_ONE;
									material.additiveMapStage=material.numStages-1;
									}
								else if(equal(blendType,"filter"))
									{
									stage.blendSrc=GL_DST_COLOR;
									stage.blendDst=GL_ZERO;
									}
								else if(equal(blendType,"modulate"))
									{
									stage.blendSrc=GL_DST_COLOR;
									stage.blendDst=GL_ZERO;
									}
								else if(equal(blendType,"none"))
									{
									stage.blendSrc=GL_ZERO;
									stage.blendDst=GL_ONE;
									}
								else if(equal(blendType,"bumpmap"))
									{
									stage.blendType=Material::Stage::BumpMap;
									stage.blendSrc=GL_ZERO;
									stage.blendDst=GL_ONE;
									material.bumpMapStage=material.numStages-1;
									}
								else if(equal(blendType,"diffusemap"))
									{
									stage.blendType=Material::Stage::DiffuseMap;
									stage.blendSrc=GL_ONE;
									stage.blendDst=GL_ZERO;
									material.diffuseMapStage=material.numStages-1;
									}
								else if(equal(blendType,"specularmap"))
									{
									stage.blendType=Material::Stage::SpecularMap;
									stage.blendSrc=GL_ZERO;
									stage.blendDst=GL_ONE;
									material.specularMapStage=material.numStages-1;
									}
								else if(equal(blendType,"shader"))
									{
									/* Ignore this extension from Prey */
									}
								else
									{
									/* Read explicit source/destination blending factors: */
									for(int blendIndex=0;blendIndex<2;++blendIndex)
										{
										GLenum blendMode=GL_ZERO;
										if(equal(blendType,"gl_zero"))
											blendMode=GL_ZERO;
										else if(equal(blendType,"gl_one"))
											blendMode=GL_ONE;
										else if(equal(blendType,"gl_src_color"))
											blendMode=GL_SRC_COLOR;
										else if(equal(blendType,"gl_one_minus_src_color"))
											blendMode=GL_ONE_MINUS_SRC_COLOR;
										else if(equal(blendType,"gl_dst_color"))
											blendMode=GL_DST_COLOR;
										else if(equal(blendType,"gl_one_minus_dst_color"))
											blendMode=GL_ONE_MINUS_DST_COLOR;
										else if(equal(blendType,"gl_src_alpha"))
											blendMode=GL_SRC_ALPHA;
										else if(equal(blendType,"gl_one_minus_src_alpha"))
											blendMode=GL_ONE_MINUS_SRC_ALPHA;
										else if(equal(blendType,"gl_dst_alpha"))
											blendMode=GL_DST_ALPHA;
										else if(equal(blendType,"gl_one_minus_dst_alpha"))
											blendMode=GL_ONE_MINUS_DST_ALPHA;
										else if(equal(blendType,"gl_src_alpha_saturate"))
											blendMode=GL_SRC_ALPHA_SATURATE;
										else
											Misc::throwStdErr("Doom3MaterialManager::parseMaterialFile: unrecognized blending mode %s at %s",blendType.c_str(),source.where().c_str());
										
										if(blendIndex==0)
											{
											stage.blendSrc=blendMode;
											if(source.readChar()!=',')
												Misc::throwStdErr("Doom3MaterialManager::parseMaterialFile: malformed blending function at %s",source.where().c_str());
											blendType=source.readString();
											}
										else
											stage.blendDst=blendMode;
										}
									
									/* Test for additive stage: */
									if(stage.blendSrc==GL_ONE&&stage.blendDst==GL_ONE)
										material.additiveMapStage=material.numStages-1;
									}
								}
							else if(equal(setting,"map"))
								{
								/* Recursively parse the image map: */
								stage.map=parseImageMap(source);
								}
							else if(equal(setting,"remoteRenderMap"))
								{
								source.readInteger(); // Skip render map width
								source.readInteger(); // Skip render map height
								}
							else if(equal(setting,"mirrorRenderMap"))
								{
								source.readInteger(); // Skip render map width
								source.readInteger(); // Skip render map height
								}
							else if(equal(setting,"videomap"))
								{
								if(source.readString()=="loop")
									source.skipString(); // Skip the video map name
								}
							else if(equal(setting,"soundmap"))
								{
								/* Ignore this completely and parse optional waveform keyword on next pass */
								}
							else if(equal(setting,"waveform"))
								{
								/* This is actually an optional continuation of "soundmap" */
								}
							else if(equal(setting,"cubeMap"))
								{
								source.skipString(); // Skip the cube map name
								}
							else if(equal(setting,"cameraCubeMap"))
								{
								source.skipString(); // Skip the cube map name
								}
							else if(equal(setting,"ignoreAlphaTest"))
								{
								/* Ignore this completely */
								}
							else if(equal(setting,"nearest"))
								{
								stage.texInterpMode=GL_NEAREST;
								}
							else if(equal(setting,"linear"))
								{
								stage.texInterpMode=GL_LINEAR;
								}
							else if(equal(setting,"clamp"))
								{
								stage.texCoordClampMode=GL_CLAMP;
								}
							else if(equal(setting,"zeroclamp"))
								{
								/* Can't do this for now */
								}
							else if(equal(setting,"alphazeroclamp"))
								{
								/* Can't do this for now */
								}
							else if(equal(setting,"noclamp"))
								{
								stage.texCoordClampMode=GL_REPEAT;
								}
							else if(equal(setting,"uncompressed"))
								{
								/* Ignore this completely */
								}
							else if(equal(setting,"highquality"))
								{
								/* Ignore this completely */
								}
							else if(equal(setting,"forceHighQuality"))
								{
								/* Ignore this completely */
								}
							else if(equal(setting,"nopicmip"))
								{
								/* Ignore this completely */
								}
							else if(equal(setting,"vertexColor"))
								{
								/* Ignore this completely */
								}
							else if(equal(setting,"inverseVertexColor"))
								{
								/* Ignore this completely */
								}
							else if(equal(setting,"privatePolygonOffset"))
								{
								source.readNumber(); // Skip polygon offset
								}
							else if(equal(setting,"texGen"))
								{
								std::string texGenType=source.readString();
								if(equal(texGenType,"normal"))
									{
									/* Ignore this completely */
									}
								else if(equal(texGenType,"reflect"))
									{
									/* Ignore this completely */
									}
								else if(equal(texGenType,"skybox"))
									{
									/* Ignore this completely */
									}
								else if(equal(texGenType,"wobbleSky"))
									{
									/* Parse and forget three expressions: */
									for(int i=0;i<3;++i)
										delete parseExpression(source);
									}
								}
							else if(equal(setting,"scroll"))
								{
								/* Parse and forget two expressions: */
								for(int i=0;i<2;++i)
									{
									if(i>0&&source.readChar()!=',')
										Misc::throwStdErr("Doom3MaterialManager::parseMaterialFile: malformed scroll keyword at %s",source.where().c_str());
									delete parseExpression(source);
									}
								}
							else if(equal(setting,"translate"))
								{
								/* Parse and forget two expressions: */
								for(int i=0;i<2;++i)
									{
									if(i>0&&source.readChar()!=',')
										Misc::throwStdErr("Doom3MaterialManager::parseMaterialFile: malformed translate keyword at %s",source.where().c_str());
									delete parseExpression(source);
									}
								}
							else if(equal(setting,"scale"))
								{
								/* Parse and forget two expressions: */
								for(int i=0;i<2;++i)
									{
									if(i>0&&source.readChar()!=',')
										Misc::throwStdErr("Doom3MaterialManager::parseMaterialFile: malformed scale keyword at %s",source.where().c_str());
									delete parseExpression(source);
									}
								}
							else if(equal(setting,"centerScale"))
								{
								/* Parse and forget two expressions: */
								for(int i=0;i<2;++i)
									{
									if(i>0&&source.readChar()!=',')
										Misc::throwStdErr("Doom3MaterialManager::parseMaterialFile: malformed centerScale keyword at %s",source.where().c_str());
									delete parseExpression(source);
									}
								}
							else if(equal(setting,"shear"))
								{
								/* Parse and forget two expressions: */
								for(int i=0;i<2;++i)
									{
									if(i>0&&source.readChar()!=',')
										Misc::throwStdErr("Doom3MaterialManager::parseMaterialFile: malformed shear keyword at %s",source.where().c_str());
									delete parseExpression(source);
									}
								}
							else if(equal(setting,"rotate"))
								{
								/* Parse and forget an expression: */
								delete parseExpression(source);
								}
							else if(equal(setting,"maskRed"))
								{
								stage.channelMasks[0]=false;
								}
							else if(equal(setting,"maskGreen"))
								{
								stage.channelMasks[1]=false;
								}
							else if(equal(setting,"maskBlue"))
								{
								stage.channelMasks[2]=false;
								}
							else if(equal(setting,"maskAlpha"))
								{
								stage.channelMasks[3]=false;
								}
							else if(equal(setting,"maskColor"))
								{
								for(int i=0;i<3;++i)
									stage.channelMasks[i]=false;
								}
							else if(equal(setting,"maskDepth"))
								{
								stage.channelMasks[4]=false;
								}
							else if(equal(setting,"alphaTest"))
								{
								/* Parse the alpha test expression: */
								Expression* exp=parseExpression(source);
								
								/* Evaluate it using the current environment: */
								stage.alphaTest=GLfloat(exp->evaluate(currentEnv));
								
								/* Delete the expression: */
								delete exp;
								}
							else if(equal(setting,"red"))
								{
								/* Parse the red expression: */
								Expression* exp=parseExpression(source);
								
								/* Evaluate it using the current environment: */
								stage.vertexColor[0]=GLfloat(exp->evaluate(currentEnv));
								
								/* Delete the expression: */
								delete exp;
								}
							else if(equal(setting,"green"))
								{
								/* Parse the green expression: */
								Expression* exp=parseExpression(source);
								
								/* Evaluate it using the current environment: */
								stage.vertexColor[1]=GLfloat(exp->evaluate(currentEnv));
								
								/* Delete the expression: */
								delete exp;
								}
							else if(equal(setting,"blue"))
								{
								/* Parse the blue expression: */
								Expression* exp=parseExpression(source);
								
								/* Evaluate it using the current environment: */
								stage.vertexColor[2]=GLfloat(exp->evaluate(currentEnv));
								
								/* Delete the expression: */
								delete exp;
								}
							else if(equal(setting,"alpha"))
								{
								/* Parse the alpha expression: */
								Expression* exp=parseExpression(source);
								
								/* Evaluate it using the current environment: */
								stage.vertexColor[3]=GLfloat(exp->evaluate(currentEnv));
								
								/* Delete the expression: */
								delete exp;
								}
							else if(equal(setting,"rgb"))
								{
								/* Parse the rgb expression: */
								Expression* exp=parseExpression(source);
								
								/* Evaluate it using the current environment: */
								GLfloat rgb=GLfloat(exp->evaluate(currentEnv));
								for(int i=0;i<3;++i)
									stage.vertexColor[i]=rgb;
								
								/* Delete the expression: */
								delete exp;
								}
							else if(equal(setting,"rgba"))
								{
								/* Parse the rgba expression: */
								Expression* exp=parseExpression(source);
								
								/* Evaluate it using the current environment: */
								GLfloat rgb=GLfloat(exp->evaluate(currentEnv));
								for(int i=0;i<4;++i)
									stage.vertexColor[i]=rgb;
								
								/* Delete the expression: */
								delete exp;
								}
							else if(equal(setting,"color"))
								{
								/* Parse four expressions: */
								for(int i=0;i<4;++i)
									{
									if(i>0&&source.readChar()!=',')
										Misc::throwStdErr("Doom3MaterialManager::parseMaterialFile: malformed color keyword at %s",source.where().c_str());
									Expression* exp=parseExpression(source);
									stage.vertexColor[i]=exp->evaluate(currentEnv);
									delete exp;
									}
								}
							else if(equal(setting,"colored"))
								{
								/* Set vertex color from the current environment: */
								for(int i=0;i<4;++i)
									stage.vertexColor[i]=currentEnv.parm[i];
								}
							else if(equal(setting,"fragmentProgram"))
								{
								source.skipString(); // Skip program name
								}
							else if(equal(setting,"vertexProgram"))
								{
								source.skipString(); // Skip program name
								}
							else if(equal(setting,"program"))
								{
								source.skipString(); // Skip program name
								}
							else if(equal(setting,"vertexparm"))
								{
								source.readInteger(); // Skip texture unit index
								delete parseExpression(source); // Skip x expression
								for(int i=1;i<4&&source.peekc()==',';++i)
									{
									source.readChar(); // Skip comma
									delete parseExpression(source); // Skip y, z, w expressions
									}
								}
							else if(equal(setting,"fragmentMap"))
								{
								source.readInteger(); // Skip texture unit index
								
								/* Read options and image map: */
								while(true)
									{
									std::string option=source.readString();
									if(equal(option,"cubeMap"))
										{
										}
									else if(equal(option,"cameraCubeMap"))
										{
										}
									else if(equal(option,"nearest"))
										{
										}
									else if(equal(option,"linear"))
										{
										}
									else if(equal(option,"clamp"))
										{
										}
									else if(equal(option,"noclamp"))
										{
										}
									else if(equal(option,"zeroclamp"))
										{
										}
									else if(equal(option,"alphazeroclamp"))
										{
										}
									else if(equal(option,"forceHighQuality"))
										{
										}
									else if(equal(option,"uncompressed"))
										{
										}
									else if(equal(option,"highquality"))
										{
										}
									else if(equal(option,"nopicmip"))
										{
										}
									else
										{
										/* Parse and ignore the image map: */
										parseImageMap(source);
										break;
										}
									}
								}
							else
								{
								// DEBUGGING
								std::cout<<"Ignoring unknown token "<<setting<<" in material stage definition at "<<source.where()<<std::endl;
								}
							}
						source.readChar(); // Skip the closing brace
						}
					else
						{
						/* Parse a global token: */
						if(equal(token,"qer_editorimage"))
							{
							source.skipString(); // Skip the image name
							}
						else if(equal(token,"description"))
							{
							source.skipString(); // Skip the description
							}
						else if(equal(token,"polygonOffset"))
							{
							source.skipString(); // Skip the polygon offset
							}
						else if(equal(token,"noShadows"))
							{
							}
						else if(equal(token,"noSelfShadow"))
							{
							}
						else if(equal(token,"forceShadows"))
							{
							}
						else if(equal(token,"noOverlay"))
							{
							}
						else if(equal(token,"forceOverlays"))
							{
							}
						else if(equal(token,"translucent"))
							{
							material.translucent=true;
							}
						else if(equal(token,"clamp"))
							{
							}
						else if(equal(token,"zeroclamp"))
							{
							}
						else if(equal(token,"alphazeroclamp"))
							{
							}
						else if(equal(token,"forceOpaque"))
							{
							}
						else if(equal(token,"twoSided"))
							{
							material.twoSided=true;
							}
						else if(equal(token,"backSided"))
							{
							}
						else if(equal(token,"mirror"))
							{
							}
						else if(equal(token,"noFog"))
							{
							}
						else if(equal(token,"unsmoothedTangents"))
							{
							}
						else if(equal(token,"guisurf"))
							{
							source.skipString(); // Skip the GUI surface name
							}
						else if(equal(token,"sort"))
							{
							source.skipString(); // Skip the sorting order
							}
						else if(equal(token,"spectrum"))
							{
							source.skipString();
							}
						else if(equal(token,"deform"))
							{
							std::string deformType=source.readString();
							if(equal(deformType,"sprite"))
								{
								}
							else if(equal(deformType,"tube"))
								{
								}
							else if(equal(deformType,"flare"))
								{
								source.skipString();
								}
							else if(equal(deformType,"expand"))
								{
								source.skipString();
								}
							else if(equal(deformType,"move"))
								{
								source.skipString();
								}
							else if(equal(deformType,"turbulent"))
								{
								source.skipString();
								source.skipString();
								source.skipString();
								source.skipString();
								}
							else if(equal(deformType,"eyeBall"))
								{
								}
							else if(equal(deformType,"particle"))
								{
								source.skipString();
								}
							else if(equal(deformType,"particle2"))
								{
								source.skipString();
								}
							}
						else if(equal(token,"decalInfo"))
							{
							source.skipString();
							source.skipString();
							
							/* Skip two optional RGBA values: */
							for(int i=0;i<2&&source.peekc()=='(';++i)
								{
								source.readChar(); // Skip the opening parenthesis
								for(int j=0;j<4;++j)
									source.skipString(); // Skip the color component
								if(source.readChar()!=')')
									Misc::throwStdErr("Doom3MaterialManager::parseMaterialFile: Malformed RGBA color at %s",source.where().c_str());
								}
							}
						else if(equal(token,"renderbump"))
							{
							/* Skip optional command line parameters: */
							while(source.peekc()=='-')
								{
								/* Read the command line parameter: */
								std::string parameter=source.readString();
								if(equal(parameter,"-size"))
									{
									/* Skip the map size: */
									for(int i=0;i<2;++i)
										source.readInteger();
									}
								else if(equal(parameter,"-aa"))
									{
									/* Skip the anti-aliasing setting: */
									source.readInteger();
									}
								else if(equal(parameter,"-trace"))
									{
									/* Skip the trace weight: */
									source.readNumber();
									}
								else if(equal(parameter,"-colorMap"))
									{
									}
								else
									{
									// DEBUGGING
									std::cout<<"Ignoring unknown command line parameter "<<parameter<<" in renderbump setting at "<<source.where()<<std::endl;
									}
								}
							
							/* Skip the map and model names: */
							source.skipString();
							source.skipString();
							}
						else if(equal(token,"diffusemap"))
							{
							/* Add a diffuse stage to the material: */
							Material::Stage& stage=material.addStage();
							stage.blendType=Material::Stage::DiffuseMap;
							stage.map=parseImageMap(source);
							
							/* Remember the diffuse map stage: */
							material.diffuseMapStage=material.numStages-1;
							}
						else if(equal(token,"specularmap"))
							{
							/* Add a specular stage to the material: */
							Material::Stage& stage=material.addStage();
							stage.blendType=Material::Stage::SpecularMap;
							stage.map=parseImageMap(source);
							
							/* Remember the specular map stage: */
							material.specularMapStage=material.numStages-1;
							}
						else if(equal(token,"bumpmap"))
							{
							/* Add a bump stage to the material: */
							Material::Stage& stage=material.addStage();
							stage.blendType=Material::Stage::BumpMap;
							stage.map=parseImageMap(source);
							
							/* Remember the normal map stage: */
							material.bumpMapStage=material.numStages-1;
							}
						else if(equal(token,"DECAL_MACRO"))
							{
							}
						else if(equal(token,"noPortalFog"))
							{
							}
						else if(equal(token,"fogLight"))
							{
							}
						else if(equal(token,"blendLight"))
							{
							}
						else if(equal(token,"ambientLight"))
							{
							}
						else if(equal(token,"lightFalloffImage"))
							{
							source.skipString();
							}
						else if(equal(token,"solid"))
							{
							material.collisionFlags|=CF_SOLID;
							}
						else if(equal(token,"water"))
							{
							}
						else if(equal(token,"playerclip"))
							{
							material.collisionFlags|=CF_PLAYERCLIP;
							}
						else if(equal(token,"monsterclip"))
							{
							material.collisionFlags|=CF_MONSTERCLIP;
							}
						else if(equal(token,"moveableclip"))
							{
							material.collisionFlags|=CF_MOVEABLECLIP;
							}
						else if(equal(token,"ikclip"))
							{
							material.collisionFlags|=CF_IKCLIP;
							}
						else if(equal(token,"blood"))
							{
							}
						else if(equal(token,"trigger"))
							{
							}
						else if(equal(token,"aassolid"))
							{
							}
						else if(equal(token,"aasobstacle"))
							{
							}
						else if(equal(token,"flashlight_trigger"))
							{
							}
						else if(equal(token,"nonsolid"))
							{
							material.collisionFlags&=~CF_SOLID;
							}
						else if(equal(token,"nullNormal"))
							{
							}
						else if(equal(token,"areaportal"))
							{
							}
						else if(equal(token,"qer_nocarve"))
							{
							}
						else if(equal(token,"discrete"))
							{
							}
						else if(equal(token,"noFragment"))
							{
							}
						else if(equal(token,"slick"))
							{
							}
						else if(equal(token,"collision"))
							{
							}
						else if(equal(token,"noimpact"))
							{
							}
						else if(equal(token,"nodamage"))
							{
							}
						else if(equal(token,"ladder"))
							{
							}
						else if(equal(token,"nosteps"))
							{
							}
						else if(equal(token,"metal"))
							{
							}
						else if(equal(token,"stone"))
							{
							}
						else if(equal(token,"flesh"))
							{
							}
						else if(equal(token,"wood"))
							{
							}
						else if(equal(token,"cardboard"))
							{
							}
						else if(equal(token,"liquid"))
							{
							}
						else if(equal(token,"glass"))
							{
							}
						else if(equal(token,"plastic"))
							{
							}
						else if(equal(token,"ricochet"))
							{
							}
						else if(equal(token,"surftype10"))
							{
							}
						else if(equal(token,"surftype11"))
							{
							}
						else if(equal(token,"surftype12"))
							{
							}
						else if(equal(token,"surftype13"))
							{
							}
						else if(equal(token,"surftype14"))
							{
							}
						else if(equal(token,"surftype15"))
							{
							}
						else
							{
							// DEBUGGING
							std::cout<<"Ignoring unknown token "<<token<<" in material definition at "<<source.where()<<std::endl;
							}
						}
					}
				source.readChar(); // Skip the closing brace
				
				/* Check the validity of all stages: */
				if(material.diffuseMapStage>=0&&(!material.stages[material.diffuseMapStage].map.isValid()||!material.stages[material.diffuseMapStage].enabled))
					material.diffuseMapStage=-1;
				if(material.specularMapStage>=0&&(!material.stages[material.specularMapStage].map.isValid()||!material.stages[material.specularMapStage].enabled))
					material.specularMapStage=-1;
				if(material.additiveMapStage>=0&&(!material.stages[material.additiveMapStage].map.isValid()||!material.stages[material.additiveMapStage].enabled))
					material.additiveMapStage=-1;
				
				/* Check if the material needs a default bump map stage: */
				if(material.bumpMapStage==-1)
					{
					/* Add a bump stage to the material: */
					Material::Stage& stage=material.addStage();
					stage.blendType=Material::Stage::BumpMap;
					stage.map=textureManager.loadTexture("_flat.tga");
					
					/* Remember the bump map stage: */
					material.bumpMapStage=material.numStages-1;
					}
				
				#if 0
				/* Check if the material needs a default diffuse map stage: */
				if(material.diffuseMapStage==-1)
					{
					/* Add a diffuse stage to the material: */
					Material::Stage& stage=material.addStage();
					stage.blendType=Material::Stage::DiffuseMap;
					stage.map=textureManager.loadTexture("_white.tga");
					
					/* Remember the diffuse map stage: */
					material.diffuseMapStage=material.numStages-1;
					}
				#endif
				
				/* Determine which shader program to use for the material: */
				material.programIndex=0;
				if(material.additiveMapStage>=0)
					material.programIndex+=1;
				if(material.diffuseMapStage>=0)
					material.programIndex+=2;
				if(material.specularMapStage>=0)
					material.programIndex+=4;
				material.programIndex-=2; // Account for missing bump-only programs
				
				/* Mark the material as loaded: */
				material.loaded=true;
				}
			else
				{
				/* Read, but otherwise ignore the material: */
				if(source.readChar()!='{')
					Misc::throwStdErr("Doom3MaterialManager::parseMaterialFile: Malformed skipped material definition at %s",source.where().c_str());
				
				/* Just skip all tokens until the matching closing brace: */
				int braceLevel=1;
				while(!source.eof()&&braceLevel>0)
					{
					if(source.peekc()=='{')
						++braceLevel;
					if(source.peekc()=='}')
						--braceLevel;
					source.skipString();
					}
				}
			}
		}
	}

Doom3MaterialManager::RenderContext Doom3MaterialManager::start(GLContextData& contextData,bool useDefaultPipeline) const
	{
	/* Save OpenGL state: */
	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_LIGHTING_BIT|GL_POLYGON_BIT|GL_TEXTURE_BIT);
	
	/* Create a render context: */
	RenderContext result(contextData,contextData.retrieveDataItem<DataItem>(this),textureManager,useDefaultPipeline);
	
	if(result.useDefaultPipeline)
		{
		/* Set up the standard OpenGL pipeline: */
		glEnable(GL_LIGHTING);
		glEnable(GL_COLOR_MATERIAL);
		glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
		glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL,GL_SEPARATE_SPECULAR_COLOR);
		glActiveTextureARB(GL_TEXTURE0_ARB);
		glEnable(GL_TEXTURE_2D);
		glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
		}
	
	return result;
	}

GLint Doom3MaterialManager::getTangentAttributeIndex(Doom3MaterialManager::RenderContext& renderContext,int tangentIndex) const
	{
	return renderContext.useDefaultPipeline?-1:renderContext.dataItem->tangentAttributeIndices[tangentIndex];
	}

void Doom3MaterialManager::setStage(Doom3MaterialManager::RenderContext& renderContext,const Doom3MaterialManager::Material::Stage& stage,int textureUnit) const
	{
	/* Enable texture mapping: */
	glActiveTextureARB(GL_TEXTURE0_ARB+textureUnit);
	textureManager.bindTexture(renderContext.tmRc,stage.map);
	
	/* Set the texture parameters: */
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,stage.texCoordClampMode);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,stage.texCoordClampMode);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,stage.texInterpMode);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,stage.texInterpMode);
	}

bool Doom3MaterialManager::setMaterial(Doom3MaterialManager::RenderContext& renderContext,const Doom3MaterialManager::MaterialID& materialID) const
	{
	/* Retrieve the material structure by its ID: */
	const Material& material=materialTree.getLeafValue(materialID);
	
	/* Set global settings: */
	if(material.twoSided)
		{
		glDisable(GL_CULL_FACE);
		glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);
		}
	else
		{
		glEnable(GL_CULL_FACE);
		glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_FALSE);
		}
	
	/* Set up alpha blending: */
	if(material.translucent)
		{
		glEnable(GL_BLEND);
		// glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glBlendFunc(GL_ONE,GL_ONE);
		glDepthMask(GL_FALSE);
		}
	else
		{
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
		}
	
	/* Only use valid materials: */
	if(material.programIndex>=0)
		{
		int parameterStage=-1;
		if(renderContext.useDefaultPipeline)
			{
			if(material.diffuseMapStage>=0)
				{
				/* Bind the diffuse map to texture unit 0: */
				setStage(renderContext,material.stages[material.diffuseMapStage],0);
				parameterStage=material.diffuseMapStage;
				}
			}
		else
			{
			/* Install the shader program: */
			if(renderContext.currentProgramIndex!=material.programIndex)
				{
				renderContext.currentProgramIndex=material.programIndex;
				
				glUseProgramObjectARB(renderContext.dataItem->programObjects[renderContext.currentProgramIndex]);
				glUniformARB(renderContext.dataItem->normalMapUniformIndices[renderContext.currentProgramIndex],0);
				if(material.diffuseMapStage>=0)
					glUniformARB(renderContext.dataItem->diffuseMapUniformIndices[renderContext.currentProgramIndex],1);
				if(material.specularMapStage>=0)
					glUniformARB(renderContext.dataItem->specularMapUniformIndices[renderContext.currentProgramIndex],2);
				if(material.additiveMapStage>=0)
					glUniformARB(renderContext.dataItem->additiveMapUniformIndices[renderContext.currentProgramIndex],3);
				}
			
			/* Bind all stage maps to texture units and determine which stage to use for vertex colors and alpha tests: */
			int textureUnit=0;
			if(material.bumpMapStage>=0)
				{
				setStage(renderContext,material.stages[material.bumpMapStage],textureUnit);
				++textureUnit;
				}
			if(material.diffuseMapStage>=0)
				{
				setStage(renderContext,material.stages[material.diffuseMapStage],textureUnit);
				++textureUnit;
				if(parameterStage==-1)
					parameterStage=material.diffuseMapStage;
				}
			if(material.specularMapStage>=0)
				{
				setStage(renderContext,material.stages[material.specularMapStage],textureUnit);
				++textureUnit;
				if(parameterStage==-1)
					parameterStage=material.specularMapStage;
				}
			if(material.additiveMapStage>=0)
				{
				setStage(renderContext,material.stages[material.additiveMapStage],textureUnit);
				++textureUnit;
				if(parameterStage==-1)
					parameterStage=material.additiveMapStage;
				}
			}
		
		if(parameterStage>=0)
			{
			const Material::Stage& pStage=material.stages[parameterStage];
			
			/* Set up vertex color: */
			glColor4fv(pStage.vertexColor);
			
			/* Set up channel masks: */
			// glColorMask(pStage.channelMasks[0],pStage.channelMasks[1],pStage.channelMasks[2],pStage.channelMasks[3]);
			// glDepthMask(pStage.channelMasks[4]);
			
			/* Set up alpha test: */
			if(pStage.alphaTest>0.0f)
				{
				glEnable(GL_ALPHA_TEST);
				glAlphaFunc(GL_GREATER,pStage.alphaTest);
				}
			else
				glDisable(GL_ALPHA_TEST);
			}
		else
			glDisable(GL_ALPHA_TEST);
		
		return true;
		}
	else
		return false;
	}

void Doom3MaterialManager::disableMaterial(Doom3MaterialManager::RenderContext& renderContext) const
	{
	/* Disable the currently-used shader program: */
	if(renderContext.currentProgramIndex!=-1)
		{
		renderContext.currentProgramIndex=-1;
		glUseProgramObjectARB(0);
		}
	
	/* Return to default OpenGL state: */
	glEnable(GL_CULL_FACE);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_FALSE);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glActiveTextureARB(GL_TEXTURE0_ARB);
	}

void Doom3MaterialManager::finish(Doom3MaterialManager::RenderContext& renderContext) const
	{
	if(!renderContext.useDefaultPipeline)
		{
		/* Uninstall the OpenGL program: */
		glUseProgramObjectARB(0);
		}
	
	/* Shut down the texture manager: */
	textureManager.finish(renderContext.tmRc);
	
	/* Restore OpenGL state: */
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glPopAttrib();
	}

}
