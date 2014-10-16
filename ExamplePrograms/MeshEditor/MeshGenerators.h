/*****************************
* MeshGenerators - Functions *
* to create meshes for       *
* several basic polyhedra    *
* (c)2001 Oliver Kreylos     *
*****************************/

#ifndef MESHGENERATORS_INCLUDED
#define MESHGENERATORS_INCLUDED

#include "PolygonMesh.h"

PolygonMesh* loadMeshfile(const char* meshfileName);
PolygonMesh* loadObjMeshfile(const char* objMeshfileName);
PolygonMesh* loadGtsMeshfile(const char* gtsMeshfileName);
PolygonMesh* loadPlyMeshfile(const char* plyMeshfileName);
PolygonMesh* loadTsurfMeshfile(const char* tsurfMeshfileName);
void saveMeshfile(const char* meshfileName,const PolygonMesh& mesh);
void savePlyMeshfile(const char* meshfileName,const PolygonMesh& mesh);

#endif
