/***********************************************************************
LoadModelFromLWOFile - Function to load static (non-animated) models
from files in LightWave Object (LWO) format.
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

#include <SceneGraph/Internal/LoadModelFromLWOFile.h>

#include <ctype.h>
#include <utility>
#include <string>
#include <vector>
#include <iostream>
#include <Misc/StandardHashFunction.h>
#include <Misc/HashTable.h>
#include <Misc/ThrowStdErr.h>
#include <IO/SeekableFile.h>
#include <SceneGraph/Internal/Doom3FileManager.h>
#include <SceneGraph/Internal/Doom3MaterialManager.h>
#include <SceneGraph/Internal/Doom3Model.h>

namespace SceneGraph {

namespace {

/****************
Helper functions:
****************/

inline bool isTag(const char* tag,const char* value)
	{
	for(int i=0;i<4;++i)
		if(toupper(tag[i])!=toupper(value[i]))
			return false;
	return true;
	}

template <int dataSizeParam>
inline
int
readI(
	char*& dataPtr)
	{
	unsigned char* dp=reinterpret_cast<unsigned char*>(dataPtr);
	int result=int(dp[0]);
	if(dp[0]&(unsigned char)0x80)
		result|=~0xff;
	for(int i=1;i<dataSizeParam;++i)
		result=(result<<8)|int(dp[i]);
	dataPtr+=dataSizeParam;
	return result;
	}

template <int dataSizeParam>
inline
unsigned int
readU(
	char*& dataPtr)
	{
	unsigned char* dp=reinterpret_cast<unsigned char*>(dataPtr);
	unsigned int result=(unsigned int)(dp[0]);
	for(int i=1;i<dataSizeParam;++i)
		result=(result<<8)|(unsigned int)(dp[i]);
	dataPtr+=dataSizeParam;
	return result;
	}

inline
float
readF(
	char*& dataPtr)
	{
	unsigned char* dp=reinterpret_cast<unsigned char*>(dataPtr);
	float result=0.0f;
	unsigned char* fp=reinterpret_cast<unsigned char*>(&result);
	for(int i=0;i<4;++i)
		fp[3-i]=dp[i];
	dataPtr+=4;
	return result;
	}

inline
std::string
readS(
	char*& dataPtr)
	{
	std::string result;
	char* dp;
	for(dp=dataPtr;*dp!='\0';++dp)
		result.push_back(*dp);
	++dp;
	ptrdiff_t len=dp-dataPtr;
	if(len&0x1)
		dataPtr+=len+1;
	else
		dataPtr+=len;
	return result;
	}

inline
unsigned int
readV(
	char*& dataPtr)
	{
	unsigned char* dp=reinterpret_cast<unsigned char*>(dataPtr);
	if(dp[0]==(unsigned char)0xff)
		{
		unsigned int result=(unsigned int)(dp[1]);
		for(int i=2;i<4;++i)
			result=(result<<8)|(unsigned int)(dp[i]);
		dataPtr+=4;
		return result;
		}
	else
		{
		unsigned int result=(unsigned int)(dp[0]);
		result=(result<<8)|(unsigned int)(dp[1]);
		dataPtr+=2;
		return result;
		}
	}

/**************
Helper classes:
**************/

typedef Doom3Model::Scalar Scalar;
typedef Doom3Model::Point Point;
typedef Doom3Model::TexturePoint TexturePoint;
typedef Doom3Model::Vector Vector;
typedef Doom3Model::Vertex Vertex;

struct Surface // Structure representing surface properties
	{
	/* Elements: */
	public:
	unsigned int clipIndex; // Index of the image clip referenced by this surface
	std::string vmapName; // Name of vertex property to use for polygons in this surface
	};

struct Polygon // Structure representing a polygon parsed from a POLS chunk
	{
	/* Elements: */
	public:
	unsigned int firstVertexIndex; // Index of polygon's first vertex index
	unsigned int numVertexIndices; // Number of vertex indices (or vertices) in polygon
	};

typedef std::pair<unsigned int,unsigned int> VmadIndex;

}

}

namespace Misc {

/**************
Helper classes:
**************/

template <>
class StandardHashFunction<std::string>
	{
	/* Static methods: */
	public:
	static size_t hash(const std::string& source,size_t tableSize)
		{
		size_t result=0;
		for(std::string::const_iterator sIt=source.begin();sIt!=source.end();++sIt)
			result=result*17+size_t(*sIt);
		return result%tableSize;
		};
	};

template <>
class StandardHashFunction<SceneGraph::VmadIndex>
	{
	/* Static methods: */
	public:
	static size_t hash(const SceneGraph::VmadIndex& source,size_t tableSize)
		{
		return (size_t(source.first)*17+size_t(source.second)*31)%tableSize;
		};
	};

}

namespace SceneGraph {

namespace {

/**************
Helper classes:
**************/

typedef Misc::HashTable<std::string,Surface> SurfaceHasher;
typedef Misc::HashTable<unsigned int,TexturePoint> VmapTexCoordHasher;
typedef Misc::HashTable<std::string,VmapTexCoordHasher*> VmapHasher;
typedef Misc::HashTable<VmadIndex,TexturePoint> VmadTexCoordHasher;
typedef Misc::HashTable<std::string,VmadTexCoordHasher*> VmadHasher;
typedef Misc::HashTable<unsigned int,unsigned int> VertexIndexHasher;

/****************
Helper functions:
****************/

unsigned int getVertexIndex(unsigned int polygonIndex,unsigned int pntsVertexIndex,const std::vector<Vertex>& pntsVertices,VmapTexCoordHasher* vmapTexCoords,VmadTexCoordHasher* vmadTexCoords,VertexIndexHasher& vertexIndexHasher,Doom3Model& model)
	{
	unsigned int vertexIndex;
	VmadTexCoordHasher::Iterator vtcIt;
	if(vmadTexCoords==0||(vtcIt=vmadTexCoords->findEntry(VmadIndex(pntsVertexIndex,polygonIndex))).isFinished())
		{
		/* Use the original vertex: */
		VertexIndexHasher::Iterator viIt=vertexIndexHasher.findEntry(pntsVertexIndex);
		if(viIt.isFinished())
			{
			vertexIndex=model.vertices.size();
			Vertex newV=pntsVertices[pntsVertexIndex];
			#if 1
			VmapTexCoordHasher::Iterator vtc2It;
			if(vmapTexCoords!=0&&!(vtc2It=vmapTexCoords->findEntry(pntsVertexIndex)).isFinished())
				newV.texCoord=vtc2It->getDest();
			else
				newV.texCoord=TexturePoint::origin;
			#else
			if(vmapTexCoords==0)
				Misc::throwStdErr("loadModelFromLWOFile: Undefined vertex texture coordinate");
			newV.texCoord=vmapTexCoords->getEntry(pntsVertexIndex).getDest();
			#endif
			vertexIndexHasher.setEntry(VertexIndexHasher::Entry(pntsVertexIndex,vertexIndex));
			model.vertices.push_back(newV);
			}
		else
			vertexIndex=viIt->getDest();
		}
	else
		{
		/* Create a new vertex: */
		vertexIndex=model.vertices.size();
		Vertex newV=pntsVertices[pntsVertexIndex];
		newV.texCoord=vtcIt->getDest();
		model.vertices.push_back(newV);
		}
	return vertexIndex;
	}

}

Doom3Model* loadModelFromLWOFile(Doom3FileManager& fileManager,Doom3MaterialManager& materialManager,const char* lwoFileName)
	{
	/* Read the entire LightWave Object file into a memory block: */
	IO::SeekableFilePtr lwoFileReader=fileManager.getSeekableFile(lwoFileName);
	size_t lwoFileSize=size_t(lwoFileReader->getSize());
	char* lwoFile=new char[lwoFileSize];
	lwoFileReader->readRaw(lwoFile,lwoFileSize);
	lwoFileReader=0;
	
	/* Read the main chunk: */
	char* mainChunkPtr=lwoFile;
	if(!isTag(mainChunkPtr,"FORM"))
		Misc::throwStdErr("loadModelFromLWOFile: File %s is not a valid LightWave Object file",lwoFileName);
	mainChunkPtr+=4;
	size_t mainChunkSize=readU<4>(mainChunkPtr);
	char* mainChunkEnd=mainChunkPtr+mainChunkSize;
	
	/* Check the FORM chunk type: */
	if(!isTag(mainChunkPtr,"LWO2"))
		Misc::throwStdErr("loadModelFromLWOFile: File %s is not a valid LightWave Object file",lwoFileName);
	mainChunkPtr+=4;
	
	/* Create the result model: */
	Doom3Model* model=new Doom3Model(materialManager,lwoFileName);
	
	std::vector<std::string> tagsMaterialNames; // List of material names from most recently parsed TAGS chunk
	SurfaceHasher surfaces(17); // Hash table of surface properties
	std::vector<Vertex> pntsVertices; // List of vertices from most recently parsed PNTS chunk
	VmapHasher vmaps(17); // Hash table of vertex texture coordinates by name
	VmapTexCoordHasher* defaultVmap=0;
	VmadHasher vmads(17); // Hash table of per-polygon vertex texture coordinates by name
	VmadTexCoordHasher* defaultVmad=0;
	std::vector<Polygon> polsPolygons; // List of polygons parsed from the most recent POLS chunk
	std::vector<unsigned int> polsVertexIndices; // List of polygon vertex indices from most recently parsed POLS chunk
	std::vector<std::vector<unsigned int> > ptagSurfaceTagPolygonIndices;
	
	/* Read the main chunk's data: */
	while(mainChunkPtr<mainChunkEnd)
		{
		/* Parse the chunk header: */
		char* chunkIdPtr=mainChunkPtr;
		mainChunkPtr+=4;
		size_t chunkSize=readU<4>(mainChunkPtr);
		char* chunkPtr=mainChunkPtr;
		char* chunkEndPtr=mainChunkPtr+chunkSize;
		
		/* Parse the chunk: */
		if(isTag(chunkIdPtr,"TAGS"))
			{
			/* Read the material names: */
			tagsMaterialNames.clear();
			while(chunkPtr<chunkEndPtr)
				tagsMaterialNames.push_back(readS(chunkPtr));
			}
		else if(isTag(chunkIdPtr,"SURF"))
			{
			/* Parse the chunk header: */
			std::string name=readS(chunkPtr);
			std::string source=readS(chunkPtr);
			
			/* Parse the surface subchunks: */
			Surface surface;
			unsigned int surfaceParts=0x0;
			while(chunkPtr<chunkEndPtr)
				{
				/* Read the chunk header: */
				char* subchunkIdPtr=chunkPtr;
				chunkPtr+=4;
				size_t subchunkSize=readU<2>(chunkPtr);
				char* subchunkPtr=chunkPtr;
				char* subchunkEndPtr=chunkPtr+subchunkSize;
				
				if(isTag(subchunkIdPtr,"BLOK"))
					{
					/* Read the header subchunk: */
					char* headerIdPtr=subchunkPtr;
					subchunkPtr+=4;
					size_t headerSize=readU<2>(subchunkPtr);
					// char* headerPtr=subchunkPtr;
					char* headerEndPtr=subchunkPtr+headerSize;
					
					if(isTag(headerIdPtr,"IMAP"))
						{
						/* Go to the attributes subchunks: */
						subchunkPtr=headerEndPtr;
						if(headerSize&0x1)
							++subchunkPtr;
						
						/* Parse all attribute chunks: */
						while(subchunkPtr<subchunkEndPtr)
							{
							/* Read the attribute header: */
							char* attributeIdPtr=subchunkPtr;
							subchunkPtr+=4;
							size_t attributeSize=readU<2>(subchunkPtr);
							char* attributePtr=subchunkPtr;
							char* attributeEndPtr=subchunkPtr+attributeSize;
							
							if(isTag(attributeIdPtr,"IMAG"))
								{
								surface.clipIndex=readV(attributePtr);
								surfaceParts|=0x1;
								}
							else if(isTag(attributeIdPtr,"VMAP"))
								{
								surface.vmapName=readS(attributePtr);
								surfaceParts|=0x2;
								}
							else if(isTag(attributeIdPtr,"TMAP"))
								{
								/* Read all texture map subchunks: */
								while(attributePtr<attributeEndPtr)
									{
									/* Read the subchunk header: */
									// char* tmapIdPtr=attributePtr;
									attributePtr+=4;
									size_t tmapSize=readU<2>(attributePtr);
									// char* tmapPtr=attributePtr;
									char* tmapEndPtr=attributePtr+tmapSize;
									
									/* Go to the next subchunk: */
									attributePtr=tmapEndPtr;
									if(tmapSize&0x1)
										++attributePtr;
									}
								}
							
							/* Go to the next attribute chunk: */
							subchunkPtr=attributeEndPtr;
							if(attributeSize&0x1)
								++subchunkPtr;
							}
						}
					}
				
				/* Move to the end of the subchunk: */
				chunkPtr=subchunkEndPtr;
				if(subchunkSize&0x1)
					++chunkPtr;
				}
			if(surfaceParts==0x3)
				{
				/* Store the surface definition: */
				surfaces.setEntry(SurfaceHasher::Entry(name,surface));
				}
			}
		else if(isTag(chunkIdPtr,"PNTS"))
			{
			/* Read the points: */
			while(chunkPtr<chunkEndPtr)
				{
				Vertex v;
				v.texCoord=TexturePoint::origin;
				v.normal=Vector::zero;
				v.tangents[0]=Vector::zero;
				v.tangents[1]=Vector::zero;
				v.position[0]=Scalar(readF(chunkPtr));
				v.position[2]=Scalar(readF(chunkPtr));
				v.position[1]=Scalar(readF(chunkPtr));
				pntsVertices.push_back(v);
				}
			}
		else if(isTag(chunkIdPtr,"VMAP"))
			{
			/* Parse the chunk header: */
			char* vmapType=chunkPtr;
			chunkPtr+=4;
			unsigned int dimension=readU<2>(chunkPtr);
			std::string name=readS(chunkPtr);
			
			/* Parse the chunk data: */
			if(isTag(vmapType,"TXUV")&&dimension==2)
				{
				/* Create a new VMAP texture coordinate hasher: */
				VmapTexCoordHasher* vmapTexCoords=new VmapTexCoordHasher(17);
				vmaps.setEntry(VmapHasher::Entry(name,vmapTexCoords));
				defaultVmap=vmapTexCoords;
				
				/* Read texture coordinates: */
				while(chunkPtr<chunkEndPtr)
					{
					unsigned int vertexIndex=readV(chunkPtr);
					if(vertexIndex>=pntsVertices.size())
						Misc::throwStdErr("loadModelFromLWOFile: Vertex index out of range in VMAP chunk in file %s",lwoFileName);
					TexturePoint tp;
					for(int i=0;i<2;++i)
						tp[i]=Scalar(readF(chunkPtr));
					tp[1]=Scalar(1)-tp[1];
					vmapTexCoords->setEntry(VmapTexCoordHasher::Entry(vertexIndex,tp));
					}
				}
			else if(strncasecmp(vmapType,"RGB",3)==0&&(dimension==3||dimension==4))
				{
				/* Ignore colors: */
				while(chunkPtr<chunkEndPtr)
					{
					unsigned int vertexIndex=readV(chunkPtr);
					if(vertexIndex>=pntsVertices.size())
						Misc::throwStdErr("loadModelFromLWOFile: Vertex index out of range in VMAP chunk in file %s",lwoFileName);
					for(unsigned int i=0;i<dimension;++i)
						readF(chunkPtr);
					}
				}
			else
				{
				/* Ignore the unknown VMAP chunk type: */
				// std::cout<<"Skipping VMAP chunk of type "<<std::string(vmapType,vmapType+4)<<" and name "<<name<<std::endl;
				}
			}
		else if(isTag(chunkIdPtr,"POLS"))
			{
			/* Parse the chunk header: */
			char* polsType=chunkPtr;
			chunkPtr+=4;
			
			/* Parse the chunk data: */
			if(isTag(polsType,"FACE"))
				{
				/* Read the polygons: */
				polsPolygons.clear();
				polsVertexIndices.clear();
				while(chunkPtr!=chunkEndPtr)
					{
					Polygon polygon;
					polygon.firstVertexIndex=polsVertexIndices.size();
					polygon.numVertexIndices=readU<2>(chunkPtr)&0x3ff;
					polsPolygons.push_back(polygon);
					for(unsigned int vertexIndex=0;vertexIndex<polygon.numVertexIndices;++vertexIndex)
						polsVertexIndices.push_back(readV(chunkPtr));
					}
				}
			else
				{
				/* Ignore the unknown POLS chunk type: */
				// std::cout<<"Skipping POLS chunk of type "<<std::string(polsType,polsType+4)<<std::endl;
				}
			}
		else if(isTag(chunkIdPtr,"VMAD"))
			{
			/* Parse the chunk header: */
			char* vmadType=chunkPtr;
			chunkPtr+=4;
			unsigned int dimension=readU<2>(chunkPtr);
			std::string name=readS(chunkPtr);
			
			/* Parse the chunk data: */
			if(isTag(vmadType,"TXUV")&&dimension==2)
				{
				/* Create a new VMAD texture coordinate hasher: */
				VmadTexCoordHasher* vmadTexCoords=new VmadTexCoordHasher(17);
				vmads.setEntry(VmadHasher::Entry(name,vmadTexCoords));
				defaultVmad=vmadTexCoords;
				
				/* Read texture coordinates: */
				while(chunkPtr<chunkEndPtr)
					{
					unsigned int vertexIndex=readV(chunkPtr);
					if(vertexIndex>=pntsVertices.size())
						Misc::throwStdErr("loadModelFromLWOFile: Vertex index out of range in VMAD chunk in file %s",lwoFileName);
					unsigned int polygonIndex=readV(chunkPtr);
					if(polygonIndex>=polsPolygons.size())
						Misc::throwStdErr("loadModelFromLWOFile: Polygon index out of range in VMAD chunk in file %s",lwoFileName);
					TexturePoint tp;
					for(int i=0;i<2;++i)
						tp[i]=Scalar(readF(chunkPtr));
					tp[1]=Scalar(1)-tp[1];
					vmadTexCoords->setEntry(VmadTexCoordHasher::Entry(VmadIndex(vertexIndex,polygonIndex),tp));
					}
				}
			else if(strncasecmp(vmadType,"RGB",3)==0&&(dimension==3||dimension==4))
				{
				/* Ignore colors: */
				while(chunkPtr<chunkEndPtr)
					{
					/* Skip vertex index and polygon index: */
					readV(chunkPtr);
					readV(chunkPtr);
					
					/* Skip color value: */
					for(unsigned int i=0;i<dimension;++i)
						readF(chunkPtr);
					}
				}
			else
				{
				/* Ignore the unknown VMAD chunk type: */
				// std::cout<<"Skipping VMAD chunk of type "<<std::string(vmadType,vmadType+4)<<" and name "<<name<<std::endl;
				}
			}
		else if(isTag(chunkIdPtr,"PTAG"))
			{
			/* Parse the chunk header: */
			char* ptagType=chunkPtr;
			chunkPtr+=4;
			
			/* Parse the chunk data: */
			if(isTag(ptagType,"SURF"))
				{
				/* Read all surface polygon tags: */
				ptagSurfaceTagPolygonIndices.clear();
				for(unsigned int i=0;i<tagsMaterialNames.size();++i)
					ptagSurfaceTagPolygonIndices.push_back(std::vector<unsigned int>());
				while(chunkPtr<chunkEndPtr)
					{
					unsigned int polygonIndex=readV(chunkPtr);
					unsigned int tagIndex=readU<2>(chunkPtr);
					if(tagIndex>=tagsMaterialNames.size())
						Misc::throwStdErr("loadModelFromLWOFile: tag index out of range in PTAG/SURF chunk in file %s",lwoFileName);
					ptagSurfaceTagPolygonIndices[tagIndex].push_back(polygonIndex);
					}
				}
			}
		else
			{
			/* Ignore the unknown chunk type: */
			// std::cout<<"Skipping chunk of type "<<chunkIdPtr[0]<<chunkIdPtr[1]<<chunkIdPtr[2]<<chunkIdPtr[3]<<" of size "<<chunkSize<<std::endl;
			}
		
		/* Move to the end of the chunk: */
		mainChunkPtr=chunkEndPtr;
		if(chunkSize&0x1)
			++mainChunkPtr;
		}
	
	/* Calculate normal vectors for all vertices from the most recent PNTS chunk based on polygons from the most recent POLS chunk: */
	for(std::vector<Polygon>::const_iterator pIt=polsPolygons.begin();pIt!=polsPolygons.end();++pIt)
		{
		const unsigned int* vis=&polsVertexIndices[pIt->firstVertexIndex];
		
		/* Check the polygon for validity: */
		for(unsigned int vertexIndexIndex=0;vertexIndexIndex<pIt->numVertexIndices;++vertexIndexIndex)
			if(vis[vertexIndexIndex]>=pntsVertices.size())
				Misc::throwStdErr("loadModelFromLWOFile: Polygon vertex index out of range in PTAG/SURF chunk in file %s",lwoFileName);
		
		/* Process each polygon corner: */
		Vertex* v0=&pntsVertices[vis[pIt->numVertexIndices-1]];
		Vector d0=Geometry::normalize(v0->position-pntsVertices[vis[pIt->numVertexIndices-2]].position);
		for(unsigned int vertexIndexIndex=0;vertexIndexIndex<pIt->numVertexIndices;++vertexIndexIndex)
			{
			Vertex* v1=&pntsVertices[vis[vertexIndexIndex]];
			Vector d1=Geometry::normalize(v1->position-v0->position);
			
			/* Calculate a normal vector and accumulation weight for the polygon corner: */
			Vector cornerNormal=d1^d0; // Model faces have clockwise orientation!
			Scalar weight=Math::acos(-(d0*d1))/Geometry::mag(cornerNormal);
			
			/* Accumulate the vertex normal: */
			v0->normal+=cornerNormal*weight;
			
			/* Go to the next corner: */
			v0=v1;
			d0=d1;
			}
		}
	for(std::vector<Vertex>::iterator vIt=pntsVertices.begin();vIt!=pntsVertices.end();++vIt)
		vIt->normal.normalize();
	
	/* Create surfaces from the most recently parsed PNTS and POLS chunks: */
	for(unsigned int tagIndex=0;tagIndex<ptagSurfaceTagPolygonIndices.size();++tagIndex)
		if(ptagSurfaceTagPolygonIndices[tagIndex].size()!=0&&surfaces.isEntry(tagsMaterialNames[tagIndex]))
			{
			/* Get the tag's surface properties: */
			const Surface& surf=surfaces.getEntry(tagsMaterialNames[tagIndex]).getDest();
			VmapHasher::Iterator vmapIt=vmaps.findEntry(surf.vmapName);
			VmapTexCoordHasher* vmapTexCoords=vmapIt.isFinished()?0:vmapIt->getDest();
			if(vmapTexCoords==0)
				{
				/* Try a vertex property of the material's name as fallback: */
				std::string propName=tagsMaterialNames[tagIndex];
				propName.append("_0");
				vmapIt=vmaps.findEntry(propName);
				vmapTexCoords=vmapIt.isFinished()?0:vmapIt->getDest();
				}
			if(vmapTexCoords==0)
				vmapTexCoords=defaultVmap;
			VmadHasher::Iterator vmadIt=vmads.findEntry(surf.vmapName);
			VmadTexCoordHasher* vmadTexCoords=vmadIt.isFinished()?0:vmadIt->getDest();
			if(vmadTexCoords==0)
				{
				/* Try a vertex property of the material's name as fallback: */
				std::string propName=tagsMaterialNames[tagIndex];
				propName.append("_0");
				vmadIt=vmads.findEntry(propName);
				vmadTexCoords=vmadIt.isFinished()?0:vmadIt->getDest();
				}
			if(vmadTexCoords==0)
				vmadTexCoords=defaultVmad;
			
			/* Check if the material name has a file name extension: */
			std::string::iterator extIt=tagsMaterialNames[tagIndex].end();
			for(std::string::iterator tmnIt=tagsMaterialNames[tagIndex].begin();tmnIt!=tagsMaterialNames[tagIndex].end();++tmnIt)
				if(*tmnIt=='.')
					extIt=tmnIt;
			
			/* Create a surface for this tag: */
			Doom3Model::Surface surface;
			if(extIt!=tagsMaterialNames[tagIndex].end())
				surface.material=materialManager.loadMaterial(std::string(tagsMaterialNames[tagIndex].begin(),extIt).c_str());
			else
				surface.material=materialManager.loadMaterial(tagsMaterialNames[tagIndex].c_str());
			surface.firstVertex=model->vertices.size();
			surface.numVertices=0;
			surface.firstVertexIndex=model->vertexIndices.size();
			surface.numVertexIndices=0;
			
			VertexIndexHasher vertexIndexHasher(17); // Keeps track of mapping from PNTS vertex indices to model vertex indices
			
			/* Triangulate the polygons and store the vertices and vertex indices: */
			const std::vector<unsigned int>& polygonIndices=ptagSurfaceTagPolygonIndices[tagIndex];
			for(std::vector<unsigned int>::const_iterator piIt=polygonIndices.begin();piIt!=polygonIndices.end();++piIt)
				{
				const Polygon& polygon=polsPolygons[*piIt];
				const unsigned int* vis=&polsVertexIndices[polygon.firstVertexIndex];
				
				/* Process the polygon's first vertex: */
				unsigned vertexIndex0=getVertexIndex(*piIt,vis[0],pntsVertices,vmapTexCoords,vmadTexCoords,vertexIndexHasher,*model);
				
				/* Triangulate the polygon: */
				unsigned int vertexIndex1=getVertexIndex(*piIt,vis[1],pntsVertices,vmapTexCoords,vmadTexCoords,vertexIndexHasher,*model);
				for(unsigned int vertexIndexIndex=2;vertexIndexIndex<polygon.numVertexIndices;++vertexIndexIndex)
					{
					unsigned int vertexIndex2=getVertexIndex(*piIt,vis[vertexIndexIndex],pntsVertices,vmapTexCoords,vmadTexCoords,vertexIndexHasher,*model);
					
					/* Store this triangle: */
					model->vertexIndices.push_back(vertexIndex0);
					model->vertexIndices.push_back(vertexIndex1);
					model->vertexIndices.push_back(vertexIndex2);
					surface.numVertexIndices+=3;
					
					/* Go to the next triangle: */
					vertexIndex1=vertexIndex2;
					}
				}
			
			/* Store the surface: */
			model->surfaces.push_back(surface);
			}
	
	/* Clean up: */
	for(VmapHasher::Iterator vmIt=vmaps.begin();vmIt!=vmaps.end();++vmIt)
		delete vmIt->getDest();
	for(VmadHasher::Iterator vmIt=vmads.begin();vmIt!=vmads.end();++vmIt)
		delete vmIt->getDest();
	delete[] lwoFile;
	
	/* Finalize and return the model: */
	model->finalizeVertices(false,true);
	return model;
	}

}
