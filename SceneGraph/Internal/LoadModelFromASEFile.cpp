/***********************************************************************
LoadModelFromASEFile - Function to load static (non-animated) models
from files in ASE format.
Copyright (c) 2007-2013 Oliver Kreylos

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

#include <SceneGraph/Internal/LoadModelFromASEFile.h>

#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <utility>
#include <iostream>
#include <Misc/ThrowStdErr.h>
#include <IO/TokenSource.h>
#include <SceneGraph/Internal/Doom3FileManager.h>
#include <SceneGraph/Internal/Doom3MaterialManager.h>
#include <SceneGraph/Internal/Doom3Model.h>

namespace SceneGraph {

namespace {

/**************
Helper classes:
**************/

class Tokenizer:public IO::TokenSource
	{
	/* Embedded classes: */
	private:
	typedef IO::TokenSource Base;
	
	/* Elements: */
	int lastChar; // The last character read from the input stream (mirrored from IO::TokenSource to support unreadToken)
	bool unread; // Flag if there is an "unread" token already in the buffer
	int previousLastChar; // Saves old lastchar value when a token is unread
	
	/* Constructors and destructors: */
	public:
	Tokenizer(IO::FilePtr sSource) // Creates a tokenizer for the given data source
		:Base(sSource),
		 lastChar(Base::peekc()),
		 unread(false)
		{
		}
	
	/* Methods: */
	bool eof(void) const
		{
		return lastChar<0;
		}
	void skipWs(void)
		{
		Base::skipWs();
		lastChar=Base::peekc();
		}
	void skipLine(void)
		{
		Base::skipLine();
		lastChar=Base::peekc();
		}
	int peekc(void) const
		{
		return lastChar;
		}
	const char* readNextToken(void)
		{
		if(unread)
			{
			/* Pretend to read the unread token again: */
			unread=false;
			std::swap(lastChar,previousLastChar);
			return getToken();
			}
		else
			{
			previousLastChar=lastChar;
			Base::readNextToken();
			lastChar=Base::peekc();
			return getToken();
			}
		}
	void unreadToken(void) // "Unreads" the last read token
		{
		if(!unread)
			{
			unread=true;
			std::swap(lastChar,previousLastChar);
			}
		}
	};

typedef Doom3Model::Scalar Scalar;
typedef Doom3Model::Point Point;
typedef Doom3Model::TexturePoint TexturePoint;
typedef Doom3Model::Vector Vector;

struct MeshFace
	{
	/* Elements: */
	public:
	int firstVertex;
	int numVertices;
	Vector normal;
	int firstTextureVertex;
	int numTextureVertices;
	int firstVertexNormal;
	int numVertexNormals;
	};

enum Group
	{
	MATERIAL_LIST,
	MATERIAL,
	SUBMATERIAL,
	MAP_DIFFUSE,
	GEOMOBJECT,
	NODE_TM,
	MESH,
	MESH_VERTEX_LIST,
	MESH_FACE_LIST,
	MESH_TVERTLIST,
	MESH_TFACELIST,
	MESH_NORMALS,
	UNKNOWN
	};

}

Doom3Model* loadModelFromASEFile(Doom3FileManager& fileManager,Doom3MaterialManager& materialManager,const char* aseFileName)
	{
	/* Open the ASE file and create a tokenizer for it: */
	Tokenizer aseTok(fileManager.getFile(aseFileName));
	aseTok.setPunctuation("{}");
	aseTok.setQuotes("\"");
	aseTok.skipWs();
	
	/* Create the result model: */
	Doom3Model* model=new Doom3Model(materialManager,aseFileName);
	
	std::vector<std::string> materialNames; // List of names of materials referenced by the model
	int currentMaterialIndex=-1; // Index of currently parsed material
	int numMeshVertices=0; // Expected number of vertices in mesh
	std::vector<Point> meshVertices; // List of vertices
	int numMeshTextureVertices=0; // Expected number of texture vertices in mesh
	std::vector<TexturePoint> meshTextureVertices; // List of texture vertices
	int numMeshFaces=0; // Expected number of faces in mesh
	std::vector<MeshFace> meshFaces; // List of faces
	std::vector<int> meshFaceVertexIndices; // List of polygon vertex indices
	std::vector<int> meshFaceTextureVertexIndices; // List of polygon texture vertex indices
	std::vector<Vector> meshFaceVertexNormals; // List of polygon vertex normal vectors
	bool meshHaveNormals=false; // Flag whether the current mesh has explicit normal vectors
	int currentNormalFace=-1; // Index of current face in mesh normal group
	int geomobjectMaterialIndex=-1; // Material index of current geometry object
	
	/* Process tag/value pairs until end of file: */
	std::vector<Group> groupStack;
	while(!aseTok.eof())
		{
		const char* tag=aseTok.readNextToken();
		if(strcasecmp(tag,"}")==0)
			{
			if(groupStack.empty())
				Misc::throwStdErr("loadModelFromASEFile: Extra closing brace in file %s",aseFileName);
			
			if(groupStack.back()==GEOMOBJECT&&geomobjectMaterialIndex>=0&&materialNames[geomobjectMaterialIndex]!="")
				{
				/* Create a surface for the most recently read vertices and faces: */
				Doom3Model::Surface surface;
				surface.material=materialManager.loadMaterial(materialNames[geomobjectMaterialIndex].c_str());
				surface.firstVertex=model->vertices.size();
				surface.numVertices=0;
				surface.firstVertexIndex=model->vertexIndices.size();
				surface.numVertexIndices=0;
				
				#if 0
				std::cout<<"Creating surface for material "<<materialNames[geomobjectMaterialIndex];
				if(meshHaveNormals)
					std::cout<<" with explicit normal vectors";
				std::cout<<std::endl;
				#endif
				meshHaveNormals=false;
				
				/* Create per-vertex normal vectors if none are specified in the ASE file: */
				std::vector<Vector> vertexNormals;
				if(!meshHaveNormals)
					{
					/* Initialize per-vertex normals: */
					vertexNormals.resize(meshVertices.size(),Vector::zero);
					
					/* Accumulate per-face normals for all vertices: */
					for(std::vector<MeshFace>::const_iterator fIt=meshFaces.begin();fIt!=meshFaces.end();++fIt)
						{
						const int* vis=&meshFaceVertexIndices[fIt->firstVertex];
						
						/* Process each face corner: */
						int index0=vis[fIt->numVertices-1];
						Point* v0=&meshVertices[index0];
						Vector d0=Geometry::normalize(*v0-meshVertices[vis[fIt->numVertices-2]]);
						for(int vertexIndexIndex=0;vertexIndexIndex<fIt->numVertices;++vertexIndexIndex)
							{
							int index1=vis[vertexIndexIndex];
							Point* v1=&meshVertices[index1];
							Vector d1=Geometry::normalize(*v1-*v0);
							
							/* Calculate a normal vector and accumulation weight for the polygon corner: */
							Vector cornerNormal=d0^d1;
							Scalar weight=Math::acos(-(d0*d1))/Geometry::mag(cornerNormal);
							
							/* Accumulate the vertex normal: */
							vertexNormals[index0]+=cornerNormal*weight;
							
							/* Go to the next corner: */
							index0=index1;
							v0=v1;
							d0=d1;
							}
						}
					for(std::vector<Vector>::iterator nIt=vertexNormals.begin();nIt!=vertexNormals.end();++nIt)
						nIt->normalize();
					}
				
				/* Triangulate the polygons and store the vertices and vertex indices: */
				for(std::vector<MeshFace>::const_iterator fIt=meshFaces.begin();fIt!=meshFaces.end();++fIt)
					{
					/* Store the vertices for this face: */
					for(int i=0;i<fIt->numVertices;++i)
						{
						Doom3Model::Vertex v;
						int vertexIndex=meshFaceVertexIndices[fIt->firstVertex+i];
						if(fIt->numTextureVertices==fIt->numVertices)
							v.texCoord=meshTextureVertices[meshFaceTextureVertexIndices[fIt->firstTextureVertex+i]];
						if(!meshHaveNormals)
							v.normal=vertexNormals[vertexIndex];
						else if(fIt->numVertexNormals==fIt->numVertices)
							v.normal=meshFaceVertexNormals[fIt->firstVertexNormal+i];
						else
							v.normal=fIt->normal;
						v.position=meshVertices[vertexIndex];
						model->vertices.push_back(v);
						++surface.numVertices;
						}
					
					/* Triangulate the face: */
					for(int i=2;i<fIt->numVertices;++i)
						{
						/* Flip triangle orientation from counter-clockwise to clockwise: */
						model->vertexIndices.push_back(surface.firstVertex+fIt->firstVertex);
						model->vertexIndices.push_back(surface.firstVertex+fIt->firstVertex+i);
						model->vertexIndices.push_back(surface.firstVertex+fIt->firstVertex+i-1);
						surface.numVertexIndices+=3;
						}
					}
				
				model->surfaces.push_back(surface);
				}
			
			groupStack.pop_back();
			}
		else if(tag[0]!='*')
			{
			Misc::throwStdErr("loadModelFromASEFile: Missing tag in file %s",aseFileName);
			}
		else if(strcasecmp(tag,"*3DSMAX_ASCIIEXPORT")==0)
			{
			/* Skip the file version number: */
			aseTok.readNextToken();
			}
		else if(strcasecmp(tag,"*COMMENT")==0)
			{
			/* Skip the comment: */
			aseTok.readNextToken();
			}
		else if(strcasecmp(tag,"*MATERIAL_LIST")==0)
			{
			/* Check if at top level: */
			if(!groupStack.empty())
				Misc::throwStdErr("loadModelFromASEFile: Non-global MATERIAL_LIST group in file %s",aseFileName);
			
			/* Enter a material list group: */
			if(strcmp(aseTok.readNextToken(),"{")!=0)
				Misc::throwStdErr("loadModelFromASEFile: Missing opening brace in MATERIAL_LIST group in file %s",aseFileName);
			groupStack.push_back(MATERIAL_LIST);
			
			/* Initialize the material parser: */
			currentMaterialIndex=-1;
			materialNames.clear();
			}
		else if(strcasecmp(tag,"*MATERIAL_COUNT")==0)
			{
			/* Check if inside MATERIAL_LIST group: */
			if(groupStack.back()!=MATERIAL_LIST)
				Misc::throwStdErr("loadModelFromASEFile: MATERIAL_COUNT outside of MATERIAL_LIST in file %s",aseFileName);
			
			/* Read the number of materials: */
			int numMaterials=atoi(aseTok.readNextToken());
			
			/* Initialize the material name list: */
			for(int i=0;i<numMaterials;++i)
				materialNames.push_back("");
			}
		else if(strcasecmp(tag,"*MATERIAL")==0)
			{
			/* Check if inside MATERIAL_LIST group: */
			if(groupStack.back()!=MATERIAL_LIST)
				Misc::throwStdErr("loadModelFromASEFile: MATERIAL group outside of MATERIAL_LIST in file %s",aseFileName);
			
			/* Read the the material index: */
			currentMaterialIndex=atoi(aseTok.readNextToken());
			if(currentMaterialIndex>int(materialNames.size()))
				Misc::throwStdErr("loadModelFromASEFile: Material index out of bounds in file %s",aseFileName);
			else if(currentMaterialIndex==int(materialNames.size()))
				materialNames.push_back("");
			
			/* Enter a material group: */
			if(strcmp(aseTok.readNextToken(),"{")!=0)
				Misc::throwStdErr("loadModelFromASEFile: Missing opening brace in MATERIAL group in file %s",aseFileName);
			groupStack.push_back(MATERIAL);
			}
		else if(strcasecmp(tag,"*SUBMATERIAL")==0)
			{
			/* Check if inside MATERIAL group: */
			if(groupStack.back()!=MATERIAL)
				Misc::throwStdErr("loadModelFromASEFile: SUBMATERIAL group outside of MATERIAL in file %s",aseFileName);
			
			/* Skip the submaterial index: */
			aseTok.readNextToken();
			
			/* Enter a submaterial group: */
			if(strcmp(aseTok.readNextToken(),"{")!=0)
				Misc::throwStdErr("loadModelFromASEFile: Missing opening brace in SUBMATERIAL group in file %s",aseFileName);
			groupStack.push_back(SUBMATERIAL);
			}
		else if(strcasecmp(tag,"*MAP_DIFFUSE")==0)
			{
			/* Check if inside MATERIAL or SUBMATERIAL group: */
			if(groupStack.back()!=MATERIAL&&groupStack.back()!=SUBMATERIAL)
				Misc::throwStdErr("loadModelFromASEFile: MAP_DIFFUSE group outside of MATERIAL or SUBMATERIAL in file %s",aseFileName);
			
			/* Enter a diffuse map group: */
			if(strcmp(aseTok.readNextToken(),"{")!=0)
				Misc::throwStdErr("loadModelFromASEFile: Missing opening brace in MAP_DIFFUSE group in file %s",aseFileName);
			groupStack.push_back(MAP_DIFFUSE);
			}
		else if(strcasecmp(tag,"*BITMAP")==0)
			{
			/* Check if inside MAP_DIFFUSE group: */
			if(groupStack.back()!=MAP_DIFFUSE)
				Misc::throwStdErr("loadModelFromASEFile: BITMAP outside of MAP_DIFFUSE in file %s",aseFileName);
			
			/* Read the bitmap name: */
			const char* bmPtr=aseTok.readNextToken();
			
			/* Find the base/ prefix: */
			const char* materialStart=0;
			while(*bmPtr!='\0'&&(*bmPtr=='/'||*bmPtr=='\\'))
				++bmPtr;
			while(*bmPtr!='\0')
				{
				/* Find the next slash: */
				const char* slashPtr;
				for(slashPtr=bmPtr;*slashPtr!='\0'&&*slashPtr!='/'&&*slashPtr!='\\';++slashPtr)
					;
				const char* nextPtr;
				for(nextPtr=slashPtr;*nextPtr!='\0'&&(*nextPtr=='/'||*nextPtr=='\\');++nextPtr)
					;
				if(slashPtr-bmPtr==4&&strncasecmp(bmPtr,"base",4)==0)
					{
					materialStart=nextPtr;
					break;
					}
				
				bmPtr=nextPtr;
				}
			
			if(materialStart!=0&&*materialStart!='\0')
				{
				/* Find the bitmap name extension: */
				const char* materialEnd=0;
				const char* sPtr;
				for(sPtr=materialStart;*sPtr!='\0';++sPtr)
					if(*sPtr=='.')
						materialEnd=sPtr;
				if(materialEnd==0)
					materialEnd=sPtr;
				
				/* Store the bitmap name: */
				materialNames[currentMaterialIndex]=std::string(materialStart,materialEnd);
				
				/* Change all backslashes to forward slashes: */
				for(std::string::iterator mnIt=materialNames[currentMaterialIndex].begin();mnIt!=materialNames[currentMaterialIndex].end();++mnIt)
					if(*mnIt=='\\')
						*mnIt='/';
				}
			}
		else if(strcasecmp(tag,"*GEOMOBJECT")==0||strcasecmp(tag,"*SHAPEOBJECT")==0)
			{
			/* Check if at top level: */
			if(!groupStack.empty())
				Misc::throwStdErr("loadModelFromASEFile: Non-global GEOMOBJECT group in file %s",aseFileName);
			
			/* Enter a geometry object group: */
			if(strcmp(aseTok.readNextToken(),"{")!=0)
				Misc::throwStdErr("loadModelFromASEFile: Missing opening brace in GEOMOBJECT group in file %s",aseFileName);
			groupStack.push_back(GEOMOBJECT);
			
			/* Initialize geometry object state: */
			geomobjectMaterialIndex=-1;
			}
		else if(strcasecmp(tag,"*NODE_TM")==0)
			{
			/* Check if inside GEOMOBJECT group: */
			if(groupStack.back()!=GEOMOBJECT)
				Misc::throwStdErr("loadModelFromASEFile: NODE_TM group outside of GEOMOBJECT in file %s",aseFileName);
			
			/* Enter a node transformation group: */
			if(strcmp(aseTok.readNextToken(),"{")!=0)
				Misc::throwStdErr("loadModelFromASEFile: Missing opening brace in NODE_TM group in file %s",aseFileName);
			groupStack.push_back(NODE_TM);
			}
		else if(strcasecmp(tag,"*MESH")==0)
			{
			/* Check if inside GEOMOBJECT group: */
			if(groupStack.back()!=GEOMOBJECT)
				Misc::throwStdErr("loadModelFromASEFile: MESH group outside of GEOMOBJECT in file %s",aseFileName);
			
			/* Enter a mesh group: */
			if(strcmp(aseTok.readNextToken(),"{")!=0)
				Misc::throwStdErr("loadModelFromASEFile: Missing opening brace in MESH group in file %s",aseFileName);
			groupStack.push_back(MESH);
			
			/* Initialize the mesh structures: */
			numMeshVertices=0;
			meshVertices.clear();
			numMeshTextureVertices=0;
			meshTextureVertices.clear();
			numMeshFaces=0;
			meshFaces.clear();
			meshFaceVertexIndices.clear();
			meshFaceTextureVertexIndices.clear();
			meshFaceVertexNormals.clear();
			meshHaveNormals=false;
			}
		else if(strcasecmp(tag,"*MESH_NUMVERTEX")==0)
			{
			/* Check if inside MESH group: */
			if(groupStack.back()!=MESH)
				Misc::throwStdErr("loadModelFromASEFile: MESH_NUMVERTEX group outside of MESH in file %s",aseFileName);
			
			/* Initialize the vertex list: */
			numMeshVertices=atoi(aseTok.readNextToken());
			meshVertices.resize(numMeshVertices,Point::origin);
			}
		else if(strcasecmp(tag,"*MESH_VERTEX_LIST")==0)
			{
			/* Check if inside MESH group: */
			if(groupStack.back()!=MESH)
				Misc::throwStdErr("loadModelFromASEFile: MESH_VERTEX_LIST group outside of MESH in file %s",aseFileName);
			
			/* Enter a mesh vertex list group: */
			if(strcmp(aseTok.readNextToken(),"{")!=0)
				Misc::throwStdErr("loadModelFromASEFile: Missing opening brace in MESH_VERTEX_LIST group in file %s",aseFileName);
			groupStack.push_back(MESH_VERTEX_LIST);
			}
		else if(strcasecmp(tag,"*MESH_VERTEX")==0)
			{
			/* Check if inside MESH_VERTEX_LIST group: */
			if(groupStack.back()!=MESH_VERTEX_LIST)
				Misc::throwStdErr("loadModelFromASEFile: MESH_VERTEX outside of MESH_VERTEX_LIST in file %s",aseFileName);
			
			/* Read the vertex: */
			int vertexIndex=atoi(aseTok.readNextToken());
			if(vertexIndex<0||vertexIndex>=numMeshVertices)
				Misc::throwStdErr("loadModelFromASEFile: Vertex index out of range in file %s",aseFileName);
			for(int i=0;i<3;++i)
				meshVertices[vertexIndex][i]=Scalar(atof(aseTok.readNextToken()));
			}
		else if(strcasecmp(tag,"*MESH_NUMFACES")==0)
			{
			/* Check if inside MESH group: */
			if(groupStack.back()!=MESH)
				Misc::throwStdErr("loadModelFromASEFile: MESH_NUMFACES group outside of MESH in file %s",aseFileName);
			
			/* Initialize the face list: */
			numMeshFaces=atoi(aseTok.readNextToken());
			MeshFace dummyFace;
			dummyFace.firstVertex=0;
			dummyFace.numVertices=0;
			dummyFace.normal=Vector::zero;
			dummyFace.firstTextureVertex=0;
			dummyFace.numTextureVertices=0;
			dummyFace.firstVertexNormal=0;
			dummyFace.numVertexNormals=0;
			meshFaces.resize(numMeshFaces,dummyFace);
			}
		else if(strcasecmp(tag,"*MESH_FACE_LIST")==0)
			{
			/* Check if inside MESH group: */
			if(groupStack.back()!=MESH)
				Misc::throwStdErr("loadModelFromASEFile: MESH_FACE_LIST group outside of MESH in file %s",aseFileName);
			
			/* Enter a mesh face list group: */
			if(strcmp(aseTok.readNextToken(),"{")!=0)
				Misc::throwStdErr("loadModelFromASEFile: Missing opening brace in MESH_FACE_LIST group in file %s",aseFileName);
			groupStack.push_back(MESH_FACE_LIST);
			}
		else if(strcasecmp(tag,"*MESH_FACE")==0)
			{
			/* Check if inside MESH_FACE_LIST group: */
			if(groupStack.back()!=MESH_FACE_LIST)
				Misc::throwStdErr("loadModelFromASEFile: MESH_FACE outside of MESH_FACE_LIST in file %s",aseFileName);
			
			/* Read the face: */
			int faceIndex=atoi(aseTok.readNextToken());
			if(faceIndex<0||faceIndex>=numMeshFaces)
				Misc::throwStdErr("loadModelFromASEFile: Face index out of range in file %s",aseFileName);
			MeshFace& face=meshFaces[faceIndex];
			face.firstVertex=meshFaceVertexIndices.size();
			face.numVertices=0;
			
			/* Read the face vertex indices: */
			while(true)
				{
				const char* label=aseTok.readNextToken();
				if(label[1]==':')
					{
					/* Vertex indices are denoted by uppercase letters: */
					int faceVertexIndex=toupper(label[0])-'A';
					if(faceVertexIndex!=face.numVertices)
						Misc::throwStdErr("loadModelFromASEFile: Face vertex index out of order in file %s",aseFileName);
					meshFaceVertexIndices.push_back(atoi(aseTok.readNextToken()));
					++face.numVertices;
					}
				else
					{
					aseTok.unreadToken();
					break;
					}
				}
			
			/* Skip the edge flags: */
			for(int i=0;i<face.numVertices;++i)
				{
				if(aseTok.readNextToken()[2]!=':')
					Misc::throwStdErr("loadModelFromASEFile: Missing face edge flag in file %s",aseFileName);
				aseTok.readNextToken();
				}
			
			/* Ignore the smoothing flag: */
			if(strcasecmp(aseTok.readNextToken(),"*MESH_SMOOTHING")==0)
				{
				/* Read all smoothing flags: */
				aseTok.readNextToken();
				while(!aseTok.eof()&&aseTok.getToken()[0]!='*'&&strcmp(aseTok.getToken(),"}")!=0)
					aseTok.readNextToken();
				aseTok.unreadToken();
				}
			else
				aseTok.unreadToken();
			
			/* Ignore the material identifier: */
			if(strcasecmp(aseTok.readNextToken(),"*MESH_MTLID")==0)
				{
				/* Read all material IDs: */
				aseTok.readNextToken();
				while(!aseTok.eof()&&aseTok.getToken()[0]!='*'&&strcmp(aseTok.getToken(),"}")!=0)
					aseTok.readNextToken();
				aseTok.unreadToken();
				}
			else
				aseTok.unreadToken();
			}
		else if(strcasecmp(tag,"*MESH_NUMTVERTEX")==0)
			{
			/* Check if inside MESH group: */
			if(groupStack.back()!=MESH)
				Misc::throwStdErr("loadModelFromASEFile: MESH_NUMTVERTEX group outside of MESH in file %s",aseFileName);
			
			/* Initialize the texture vertex list: */
			numMeshTextureVertices=atoi(aseTok.readNextToken());
			meshTextureVertices.resize(numMeshTextureVertices,TexturePoint::origin);
			}
		else if(strcasecmp(tag,"*MESH_TVERTLIST")==0)
			{
			/* Check if inside MESH group: */
			if(groupStack.back()!=MESH)
				Misc::throwStdErr("loadModelFromASEFile: MESH_TVERTLIST group outside of MESH in file %s",aseFileName);
			
			/* Enter a mesh texture vertex list group: */
			if(strcmp(aseTok.readNextToken(),"{")!=0)
				Misc::throwStdErr("loadModelFromASEFile: Missing opening brace in MESH_TVERTLIST group in file %s",aseFileName);
			groupStack.push_back(MESH_TVERTLIST);
			}
		else if(strcasecmp(tag,"*MESH_TVERT")==0)
			{
			/* Check if inside MESH_TVERTLIST group: */
			if(groupStack.back()!=MESH_TVERTLIST)
				Misc::throwStdErr("loadModelFromASEFile: MESH_TVERT outside of MESH_TVERTLIST in file %s",aseFileName);
			
			/* Read the texture vertex: */
			int textureVertexIndex=atoi(aseTok.readNextToken());
			if(textureVertexIndex<0||textureVertexIndex>=numMeshTextureVertices)
				Misc::throwStdErr("loadModelFromASEFile: Texture vertex index out of range in file %s",aseFileName);
			for(int i=0;i<2;++i)
				meshTextureVertices[textureVertexIndex][i]=Scalar(atof(aseTok.readNextToken()));
			meshTextureVertices[textureVertexIndex][1]=Scalar(1)-meshTextureVertices[textureVertexIndex][1];
			
			/* Ignore the third texture coordinate: */
			aseTok.readNextToken();
			}
		else if(strcasecmp(tag,"*MESH_NUMTVFACES")==0)
			{
			/* Check if inside MESH group: */
			if(groupStack.back()!=MESH)
				Misc::throwStdErr("loadModelFromASEFile: MESH_NUMTVFACES group outside of MESH in file %s",aseFileName);
			
			/* Read the number of texture faces: */
			int numTextureFaces=atoi(aseTok.readNextToken());
			if(numTextureFaces!=numMeshFaces)
				Misc::throwStdErr("loadModelFromASEFile: Mismatching number of faces and texture faces in file %s",aseFileName);
			}
		else if(strcasecmp(tag,"*MESH_TFACELIST")==0)
			{
			/* Check if inside MESH group: */
			if(groupStack.back()!=MESH)
				Misc::throwStdErr("loadModelFromASEFile: MESH_TFACELIST group outside of MESH in file %s",aseFileName);
			
			/* Enter a mesh texture face list group: */
			if(strcmp(aseTok.readNextToken(),"{")!=0)
				Misc::throwStdErr("loadModelFromASEFile: Missing opening brace in MESH_TFACELIST group in file %s",aseFileName);
			groupStack.push_back(MESH_TFACELIST);
			}
		else if(strcasecmp(tag,"*MESH_TFACE")==0)
			{
			/* Check if inside MESH_TFACELIST group: */
			if(groupStack.back()!=MESH_TFACELIST)
				Misc::throwStdErr("loadModelFromASEFile: MESH_TFACE outside of MESH_TFACELIST in file %s",aseFileName);
			
			/* Read the texture face index: */
			int textureFaceIndex=atoi(aseTok.readNextToken());
			if(textureFaceIndex<0||textureFaceIndex>=numMeshFaces)
				Misc::throwStdErr("loadModelFromASEFile: Texture face index out of range in file %s",aseFileName);
			MeshFace& face=meshFaces[textureFaceIndex];
			face.firstTextureVertex=meshFaceTextureVertexIndices.size();
			face.numTextureVertices=0;
			
			/* Read the face texture vertex indices: */
			while(!aseTok.eof())
				{
				aseTok.readNextToken();
				if(aseTok.getToken()[0]=='*'||strcmp(aseTok.getToken(),"}")==0)
					break;
				meshFaceTextureVertexIndices.push_back(atoi(aseTok.getToken()));
				++face.numTextureVertices;
				}
			aseTok.unreadToken();
			}
		else if(strcasecmp(tag,"*MESH_NORMALS")==0)
			{
			/* Check if inside MESH group: */
			if(groupStack.back()!=MESH)
				Misc::throwStdErr("loadModelFromASEFile: MESH_NORMALS group outside of MESH in file %s",aseFileName);
			
			/* Enter a mesh normals group: */
			if(strcmp(aseTok.readNextToken(),"{")!=0)
				Misc::throwStdErr("loadModelFromASEFile: Missing opening brace in MESH_NORMALS group in file %s",aseFileName);
			groupStack.push_back(MESH_NORMALS);
			
			/* Initialize the mesh normal parsing state: */
			meshHaveNormals=true;
			currentNormalFace=-1;
			}
		else if(strcasecmp(tag,"*MESH_FACENORMAL")==0)
			{
			/* Check if inside MESH_NORMALS group: */
			if(groupStack.back()!=MESH_NORMALS)
				Misc::throwStdErr("loadModelFromASEFile: MESH_FACENORMAL outside of MESH_NORMALS in file %s",aseFileName);
			
			/* Read the face: */
			int faceIndex=atoi(aseTok.readNextToken());
			if(faceIndex<0||faceIndex>=numMeshFaces)
				Misc::throwStdErr("loadModelFromASEFile: Face index out of range in file %s",aseFileName);
			MeshFace& face=meshFaces[faceIndex];
			
			/* Read the normal vector: */
			for(int i=0;i<3;++i)
				face.normal[i]=Scalar(atof(aseTok.readNextToken()));
			
			/* Prepare to parse the following face vertex normals: */
			face.firstVertexNormal=meshFaceVertexNormals.size();
			currentNormalFace=faceIndex;
			}
		else if(strcasecmp(tag,"*MESH_VERTEXNORMAL")==0)
			{
			/* Check if inside MESH_NORMALS group: */
			if(groupStack.back()!=MESH_NORMALS)
				Misc::throwStdErr("loadModelFromASEFile: MESH_VERTEXNORMAL outside of MESH_NORMALS in file %s",aseFileName);
			
			if(currentNormalFace==-1)
				Misc::throwStdErr("loadModelFromASEFile: MESH_VERTEXNORMAL without current face in file %s",aseFileName);
			MeshFace& face=meshFaces[currentNormalFace];
			
			/* Read the vertex index: */
			int vertexIndex=atoi(aseTok.readNextToken());
			if(vertexIndex<0||vertexIndex>=numMeshVertices)
				Misc::throwStdErr("loadModelFromASEFile: Vertex index out of range in MESH_VERTEXNORMAL in file %s",aseFileName);
			
			/* Find the vertex in the current face's list: */
			int faceVertexIndex;
			for(faceVertexIndex=0;faceVertexIndex<face.numVertices&&meshFaceVertexIndices[face.firstVertex+faceVertexIndex]!=vertexIndex;++faceVertexIndex)
				;
			if(faceVertexIndex>=face.numVertices)
				Misc::throwStdErr("loadModelFromASEFile: Vertex not found in current face in MESH_VERTEXNORMAL in file %s",aseFileName);
			if(faceVertexIndex!=face.numVertexNormals)
				Misc::throwStdErr("loadModelFromASEFile: Face vertex index out of order in MESH_VERTEXNORMAL in file %s",aseFileName);
			
			/* Read the normal vector: */
			Vector normal;
			for(int i=0;i<3;++i)
				normal[i]=Scalar(atof(aseTok.readNextToken()));
			meshFaceVertexNormals.push_back(normal);
			++face.numVertexNormals;
			}
		else if(strcasecmp(tag,"*MATERIAL_REF")==0)
			{
			/* Check if inside GEOMOBJECT group: */
			if(groupStack.back()!=GEOMOBJECT)
				Misc::throwStdErr("loadModelFromASEFile: MATERIAL_REF group outside of GEOMOBJECT in file %s",aseFileName);
			
			/* Read the material index: */
			geomobjectMaterialIndex=atoi(aseTok.readNextToken());
			}
		else // Ignore unrecognized tags
			{
			/* Read all tag values and check for braces: */
			aseTok.readNextToken();
			while(!aseTok.eof()&&aseTok.getToken()[0]!='*'&&strcmp(aseTok.getToken(),"}")!=0)
				{
				if(strcmp(aseTok.getToken(),"{")==0)
					{
					/* Enter an unknown group: */
					groupStack.push_back(UNKNOWN);
					}
				aseTok.readNextToken();
				}
			aseTok.unreadToken();
			}
		}
	if(!groupStack.empty())
		Misc::throwStdErr("loadModelFromASEFile: Missing closing brace at end of file in file %s",aseFileName);
	
	/* Finalize and return the model: */
	model->finalizeVertices(false,true);
	return model;
	}

}
