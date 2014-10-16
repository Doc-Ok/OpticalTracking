/***********************************************************************
IndexedFaceSetNode - Class for shapes represented as sets of faces.
Copyright (c) 2006-2008 Oliver Kreylos

This file is part of the Virtual Reality VRML viewer (VRMLViewer).

The Virtual Reality VRML viewer is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Virtual Reality VRML viewer is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Virtual Reality VRML viewer; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <utility>
#include <vector>
#include <iostream>
#include <Misc/OrderedTuple.h>
#include <Misc/HashTable.h>
#include <Math/Math.h>
#include <GL/gl.h>
#include <GL/GLTexCoordTemplates.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLNormalTemplates.h>
#include <GL/GLVertexTemplates.h>
#define GLVERTEX_NONSTANDARD_TEMPLATES
#include <GL/GLVertex.h>
#include <GL/GLContextData.h>
#include <GL/GLExtensionManager.h>
#include <GL/Extensions/GLARBVertexBufferObject.h>
#include <GL/GLGeometryWrappers.h>

#include "Types.h"
#include "Fields/SFBool.h"
#include "Fields/SFFloat.h"
#include "Fields/MFInt32.h"

#include "VRMLParser.h"
#include "VRMLRenderState.h"
#include "TextureCoordinateNode.h"
#include "ColorNode.h"
#include "NormalNode.h"
#include "CoordinateNode.h"

#include "IndexedFaceSetNode.h"

namespace {

/**************
Helper classes:
**************/

void calculateFaceNormals(const CoordinateNode* coordNode,const std::vector<Int32>& coordIndices,NormalNode* normalNode)
	{
	/* Calculate normal vectors for each face: */
	const CoordinateNode::PointList& points=coordNode->getPoints();
	NormalNode::VectorList& vectors=normalNode->getVectors();
	std::vector<Int32>::const_iterator ciIt=coordIndices.begin();
	while(ciIt!=coordIndices.end())
		{
		/* Get the face's first edge: */
		std::vector<Int32>::const_iterator faceCiIt=ciIt;
		int vi0=*ciIt;
		++ciIt;
		int vi1=*ciIt;
		++ciIt;
		Vector d1=points[vi1]-points[vi0];
		
		/* Initialize the face's normal vector: */
		Vector normal=Vector::zero;
		
		/* Process the face's edges: */
		while(*ciIt>=0)
			{
			/* Get the face's next edge: */
			int vi2=*ciIt;
			++ciIt;
			Vector d2=points[vi2]-points[vi0];
			
			/* Calculate the vertex triple's normal vector: */
			normal+=Geometry::cross(d1,d2);
			
			/* Go to the next edge: */
			vi1=vi2;
			d1=d2;
			}
		++ciIt;
		
		/* Store the face's normal vector: */
		normal.normalize();
		vectors.push_back(normal.getComponents());
		}
	}

void calculateVertexNormals(const CoordinateNode* coordNode,const std::vector<Int32>& coordIndices,Float creaseAngleCos,NormalNode* normalNode,std::vector<Int32>& normalIndices)
	{
	typedef Misc::OrderedTuple<int,2> HalfEdge; // Class for directed edges as hash table keys
	typedef Misc::HashTable<HalfEdge,std::pair<int,int>,HalfEdge> HalfEdgeHasher; // Class to map directed edges to face indices and previous vertex indices
	typedef Misc::HashTable<int,void> VertexIndexHasher; // Class for sets of vertex indices
	typedef Misc::OrderedTuple<int,2> VertexFace; // Class for vertex/face index pairs as hash table keys
	typedef Misc::HashTable<VertexFace,int,VertexFace> VertexFaceHasher; // Class to map vertex/face index pairs to normal vector indices
	
	/* Calculate normal vectors for each face, create a hash table of directed edges, and accumulate vertex normals: */
	const CoordinateNode::PointList& points=coordNode->getPoints();
	int faceIndex=0;
	std::vector<Vector> faceNormals;
	HalfEdgeHasher halfEdges(101);
	
	/* Initialize the array of averaged vertex normals for non-crease vertices: */
	std::vector<Vector>& vertexNormals=normalNode->getVectors();
	vertexNormals.reserve(points.size());
	for(size_t i=0;i<points.size();++i)
		vertexNormals.push_back(Vector::zero);
	
	std::vector<Int32>::const_iterator ciIt=coordIndices.begin();
	while(ciIt!=coordIndices.end())
		{
		/* Get the face's first edge: */
		std::vector<Int32>::const_iterator faceCiIt=ciIt;
		int vi0=*ciIt;
		++ciIt;
		int vi1=*ciIt;
		++ciIt;
		Vector d1=points[vi1]-points[vi0];
		
		/* Initialize the face's normal vector: */
		Vector normal=Vector::zero;
		
		/* Process the face's edges: */
		while(*ciIt>=0)
			{
			/* Get the face's next edge: */
			int vi2=*ciIt;
			++ciIt;
			Vector d2=points[vi2]-points[vi0];
			if(halfEdges.setEntry(HalfEdgeHasher::Entry(HalfEdge(vi1,vi2),std::make_pair(faceIndex,ciIt[-3]))))
				std::cerr<<"Non-manifold edge for vertex indices "<<vi1<<" and "<<vi2<<std::endl;
			
			/* Calculate the vertex triple's normal vector: */
			normal+=Geometry::cross(d1,d2);
			
			/* Go to the next edge: */
			vi1=vi2;
			d1=d2;
			}
		
		/* Get the face's last and first edges: */
		++ciIt;
		if(halfEdges.setEntry(HalfEdgeHasher::Entry(HalfEdge(vi1,vi0),std::make_pair(faceIndex,ciIt[-3]))))
			std::cerr<<"Non-manifold edge for vertex indices "<<vi1<<" and "<<vi0<<std::endl;
		if(halfEdges.setEntry(HalfEdgeHasher::Entry(HalfEdge(vi0,faceCiIt[1]),std::make_pair(faceIndex,vi1))))
			std::cerr<<"Non-manifold edge for vertex indices "<<vi0<<" and "<<faceCiIt[1]<<std::endl;
		
		/* Store the face's normal vector: */
		normal.normalize();
		faceNormals.push_back(normal);
		
		/* Accumulate the face's normal vector to all the face's vertices: */
		while(*faceCiIt>=0)
			{
			vertexNormals[*faceCiIt]+=normal;
			++faceCiIt;
			}
		
		/* Go to the next face: */
		++faceIndex;
		}
	
	std::cout<<"Have "<<halfEdges.getNumEntries()<<" half edges"<<std::endl;
	
	/* Find all crease edges and process their vertices and faces: */
	VertexIndexHasher creaseVertices(17);
	VertexFaceHasher vertexFaceNormalIndices(17);
	for(HalfEdgeHasher::Iterator heIt=halfEdges.begin();!heIt.isFinished();++heIt)
		{
		/* Find the edge's opposite: */
		HalfEdgeHasher::Iterator oppIt=halfEdges.findEntry(HalfEdge(heIt->getSource()[1],heIt->getSource()[0]));
		
		/* Check if the edge is a crease edge: */
		if(oppIt.isFinished()||faceNormals[heIt->getDest().first]*faceNormals[oppIt->getDest().first]<creaseAngleCos)
			{
			/* Get the edge's start vertex index: */
			int vertexIndex=heIt->getSource()[0];
			
			/* Mark the edge's start vertex as a crease vertex and get the normal index for this platelet segment: */
			int normalIndex;
			if(creaseVertices.setEntry(VertexIndexHasher::Entry(vertexIndex)))
				{
				/* Vertex was already marked; add a new normal to the end of the array: */
				normalIndex=vertexNormals.size();
				vertexNormals.push_back(Vector::zero);
				}
			else
				{
				/* Vertex is marked for first time; use vertex index as normal index: */
				normalIndex=vertexIndex;
				vertexNormals[normalIndex]=Vector::zero;
				}
			
			/* Calculate the averaged normal vector for this platelet segment: */
			HalfEdgeHasher::Iterator peIt=heIt;
			int firstFaceIndex=peIt->getDest().first;
			while(true)
				{
				/* Accumulate the current face's normal vector: */
				int faceIndex=peIt->getDest().first;
				vertexNormals[normalIndex]+=faceNormals[faceIndex];
				
				/* Store the normal index for the vertex/face index pair: */
				vertexFaceNormalIndices.setEntry(VertexFaceHasher::Entry(VertexFace(vertexIndex,faceIndex),normalIndex));
				
				std::cout<<"Assigning normal "<<normalIndex<<" to vertex "<<vertexIndex<<", face "<<faceIndex<<std::endl;
				
				/* Get the next edge around the vertex, in counter-clockwise order: */
				peIt=halfEdges.findEntry(HalfEdge(vertexIndex,peIt->getDest().second));
				
				/* Terminate the current platelet segment at a crease edge: */
				if(peIt.isFinished()||peIt->getDest().first==firstFaceIndex||faceNormals[faceIndex]*faceNormals[peIt->getDest().first]<creaseAngleCos)
					break;
				}
			}
		}
	
	/* Normalize all accumulated vertex normals: */
	for(std::vector<Vector>::iterator vnIt=vertexNormals.begin();vnIt!=vertexNormals.end();++vnIt)
		vnIt->normalize();
	
	/* Create array of vertex normal indices: */
	faceIndex=0;
	ciIt=coordIndices.begin();
	while(ciIt!=coordIndices.end())
		{
		/* Process this face: */
		while(*ciIt>=0)
			{
			/* Check if the vertex is a crease vertex: */
			if(creaseVertices.isEntry(*ciIt))
				{
				/* Use a per-face vertex normal: */
				try
					{
					normalIndices.push_back(vertexFaceNormalIndices.getEntry(VertexFace(*ciIt,faceIndex)).getDest());
					}
				catch(VertexFaceHasher::EntryNotFoundError err)
					{
					std::cerr<<"Missing vertex/face hash entry for vertex "<<*ciIt<<", face "<<faceIndex<<std::endl;
					normalIndices.push_back(*ciIt);
					}
				}
			else
				{
				/* Use the averaged face normal: */
				normalIndices.push_back(*ciIt);
				}
			++ciIt;
			}
		
		/* Go to the next face: */
		normalIndices.push_back(*ciIt);
		++faceIndex;
		++ciIt;
		}
	}

struct VertexIndices // Structure to store the indices of vertex components
	{
	/* Elements: */
	public:
	int texCoord,color,normal,coord;
	
	/* Constructors and destructors: */
	VertexIndices(int sTexCoord,int sColor,int sNormal,int sCoord)
		:texCoord(sTexCoord),color(sColor),normal(sNormal),coord(sCoord)
		{
		};
	
	/* Methods: */
	friend bool operator==(const VertexIndices& vi1,const VertexIndices& vi2)
		{
		return vi1.texCoord==vi2.texCoord&&vi1.color==vi2.color&&vi1.normal==vi2.normal&&vi1.coord==vi2.coord;
		};
	friend bool operator!=(const VertexIndices& vi1,const VertexIndices& vi2)
		{
		return vi1.texCoord!=vi2.texCoord||vi1.color!=vi2.color||vi1.normal!=vi2.normal||vi1.coord!=vi2.coord;
		};
	static size_t hash(const VertexIndices& value,size_t tableSize)
		{
		return (((size_t(value.texCoord)*7+size_t(value.color))*5+size_t(value.normal)*3)+size_t(value.coord))%tableSize;
		};
	};

}

/*********************************************
Methods of class IndexedFaceSetNode::DataItem:
*********************************************/

IndexedFaceSetNode::DataItem::DataItem(void)
	:vertexBufferObjectId(0),
	 indexBufferObjectId(0)
	{
	if(GLARBVertexBufferObject::isSupported())
		{
		/* Initialize the vertex buffer object extension: */
		GLARBVertexBufferObject::initExtension();
		
		/* Create vertex and index buffer objects: */
		glGenBuffersARB(1,&vertexBufferObjectId);
		glGenBuffersARB(1,&indexBufferObjectId);
		}
	}

IndexedFaceSetNode::DataItem::~DataItem(void)
	{
	if(vertexBufferObjectId!=0||indexBufferObjectId!=0)
		{
		/* Destroy the buffer objects: */
		glDeleteBuffersARB(1,&vertexBufferObjectId);
		glDeleteBuffersARB(1,&indexBufferObjectId);
		}
	}

/***********************************
Methods of class IndexedFaceSetNode:
***********************************/

IndexedFaceSetNode::IndexedFaceSetNode(VRMLParser& parser)
	:ccw(true),solid(true),convex(true),
	 colorPerVertex(true),
	 normalPerVertex(true),creaseAngle(0.0f)
	{
	/* Check for the opening brace: */
	if(!parser.isToken("{"))
		Misc::throwStdErr("IndexedFaceSetNode::IndexedFaceSetNode: Missing opening brace in node definition");
	parser.getNextToken();
	
	/* Process attributes until closing brace: */
	while(!parser.isToken("}"))
		{
		if(parser.isToken("ccw"))
			{
			parser.getNextToken();
			ccw=SFBool::parse(parser);
			}
		else if(parser.isToken("solid"))
			{
			parser.getNextToken();
			solid=SFBool::parse(parser);
			}
		else if(parser.isToken("convex"))
			{
			parser.getNextToken();
			convex=SFBool::parse(parser);
			}
		else if(parser.isToken("colorPerVertex"))
			{
			parser.getNextToken();
			colorPerVertex=SFBool::parse(parser);
			}
		else if(parser.isToken("normalPerVertex"))
			{
			parser.getNextToken();
			normalPerVertex=SFBool::parse(parser);
			}
		else if(parser.isToken("creaseAngle"))
			{
			parser.getNextToken();
			creaseAngle=SFFloat::parse(parser);
			}
		else if(parser.isToken("texCoord"))
			{
			/* Parse the texture coordinate node: */
			parser.getNextToken();
			texCoord=parser.getNextNode();
			}
		else if(parser.isToken("color"))
			{
			/* Parse the color node: */
			parser.getNextToken();
			color=parser.getNextNode();
			}
		else if(parser.isToken("normal"))
			{
			/* Parse the normal node: */
			parser.getNextToken();
			normal=parser.getNextNode();
			}
		else if(parser.isToken("coord"))
			{
			/* Parse the coordinate node: */
			parser.getNextToken();
			coord=parser.getNextNode();
			}
		else if(parser.isToken("texCoordIndex"))
			{
			/* Parse the texture coordinate index array: */
			parser.getNextToken();
			texCoordIndices=MFInt32::parse(parser);
			}
		else if(parser.isToken("colorIndex"))
			{
			/* Parse the color index array: */
			parser.getNextToken();
			colorIndices=MFInt32::parse(parser);
			}
		else if(parser.isToken("normalIndex"))
			{
			/* Parse the normal vector index array: */
			parser.getNextToken();
			normalIndices=MFInt32::parse(parser);
			}
		else if(parser.isToken("coordIndex"))
			{
			/* Parse the coordinate index array: */
			parser.getNextToken();
			coordIndices=MFInt32::parse(parser);
			
			/* Terminate the coordinate index array: */
			if(coordIndices.back()>=0)
				coordIndices.push_back(-1);
			}
		else
			Misc::throwStdErr("IndexedFaceSetNode::IndexedFaceSetNode: unknown attribute \"%s\" in node definition",parser.getToken());
		}
	
	/* Skip the closing brace: */
	parser.getNextToken();
	
	/* Create normal vectors if necessary: */
	CoordinateNode* c=dynamic_cast<CoordinateNode*>(coord.getPointer());
	NormalNode* n=dynamic_cast<NormalNode*>(normal.getPointer());
	if(coord!=0&&n==0)
		{
		/* Create a new normal node and clear the normal index array: */
		n=new NormalNode;
		normal=n;
		normalIndices.clear();
		
		/* Create normal vectors and normal indices: */
		if(normalPerVertex)
			calculateVertexNormals(c,coordIndices,Math::cos(creaseAngle),n,normalIndices);
		else
			calculateFaceNormals(c,coordIndices,n);
		}
	}

IndexedFaceSetNode::~IndexedFaceSetNode(void)
	{
	}

void IndexedFaceSetNode::initContext(GLContextData& contextData) const
	{
	/* Create a data item and store it in the context: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	
	/* Do nothing if the vertex buffer object extension is not supported: */
	if(dataItem->vertexBufferObjectId==0||dataItem->indexBufferObjectId==0)
		return;
	
	const TextureCoordinateNode* texCoordNode=dynamic_cast<const TextureCoordinateNode*>(texCoord.getPointer());
	const ColorNode* colorNode=dynamic_cast<const ColorNode*>(color.getPointer());
	const NormalNode* normalNode=dynamic_cast<const NormalNode*>(normal.getPointer());
	const CoordinateNode* coordNode=dynamic_cast<const CoordinateNode*>(coord.getPointer());
	
	/*********************************************************************
	The problem with indexed face sets in VRML is that the format supports
	component-wise vertex indices, i.e., a vertex used in a face can have
	different indices for texture coordinate, color, normal, and position.
	OpenGL, on the other hand, only supports a single index for all vertex
	components. This method tries to reuse vertices as much as possible,
	by mapping tuples of per-component vertex indices to complete OpenGL
	vertex indices using a hash table.
	*********************************************************************/
	
	/* Create a hash table to map compound vertex indices to complete vertices: */
	typedef Misc::HashTable<VertexIndices,GLuint,VertexIndices> VertexHasher;
	VertexHasher vertexHasher(101);
	
	/* Count the number of vertices that need to be created and store their compound indices: */
	std::vector<int>::const_iterator texCoordIt=texCoordIndices.empty()?coordIndices.begin():texCoordIndices.begin();
	std::vector<int>::const_iterator colorIt=colorIndices.empty()?coordIndices.begin():colorIndices.begin();
	int colorCounter=0;
	std::vector<int>::const_iterator normalIt=normalIndices.empty()?coordIndices.begin():normalIndices.begin();
	int normalCounter=0;
	std::vector<int>::const_iterator coordIt=coordIndices.begin();
	VertexIndices currentVertex(0,0,0,0);
	std::vector<VertexIndices> vertexIndices;
	std::vector<GLuint> triangleVertexIndices;
	dataItem->numTriangles=0;
	while(coordIt!=coordIndices.end())
		{
		/* Process the vertices of this face: */
		std::vector<GLuint> faceVertexIndices;
		while(*coordIt>=0)
			{
			/* Create the current compound vertex: */
			if(texCoordNode!=0)
				currentVertex.texCoord=*texCoordIt;
			if(colorNode!=0)
				{
				if(!colorPerVertex&&colorIndices.empty())
					currentVertex.color=colorCounter;
				else
					currentVertex.color=*colorIt;
				}
			if(normalNode!=0)
				{
				if(!normalPerVertex&&normalIndices.empty())
					currentVertex.normal=normalCounter;
				else
					currentVertex.normal=*normalIt;
				}
			currentVertex.coord=*coordIt;
			
			if(currentVertex.texCoord<0||currentVertex.color<0||currentVertex.normal<0||currentVertex.coord<0)
				Misc::throwStdErr("Bad index in vertex!");
			
			/* Find the index of the complete vertex: */
			int vertexIndex;
			VertexHasher::Iterator vhIt=vertexHasher.findEntry(currentVertex);
			if(vhIt.isFinished())
				{
				/* Create a new vertex and store its index: */
				faceVertexIndices.push_back(vertexIndices.size());
				vertexHasher.setEntry(VertexHasher::Entry(currentVertex,vertexIndices.size()));
				vertexIndices.push_back(currentVertex);
				}
			else
				{
				/* Store the existing vertex index: */
				faceVertexIndices.push_back(vhIt->getDest());
				}
			
			/* Go to the next vertex in the same face: */
			++texCoordIt;
			if(colorPerVertex)
				++colorIt;
			if(normalPerVertex)
				++normalIt;
			++coordIt;
			}
		
		/* Create triangles for this face: */
		for(int i=2;i<faceVertexIndices.size();++i)
			{
			triangleVertexIndices.push_back(faceVertexIndices[0]);
			triangleVertexIndices.push_back(faceVertexIndices[i-1]);
			triangleVertexIndices.push_back(faceVertexIndices[i]);
			++dataItem->numTriangles;
			}
		
		/* Go to the next face: */
		++texCoordIt;
		if(!colorPerVertex&&colorIndices.empty())
			++colorCounter;
		else
			++colorIt;
		if(!normalPerVertex&&normalIndices.empty())
			++normalCounter;
		else
			++normalIt;
		++coordIt;
		}
	
	/* Upload all vertices into the vertex buffer: */
	glBindBufferARB(GL_ARRAY_BUFFER_ARB,dataItem->vertexBufferObjectId);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB,vertexIndices.size()*sizeof(Vertex),0,GL_STATIC_DRAW_ARB);
	Vertex* vertices=static_cast<Vertex*>(glMapBufferARB(GL_ARRAY_BUFFER_ARB,GL_WRITE_ONLY_ARB));
	for(std::vector<VertexIndices>::const_iterator viIt=vertexIndices.begin();viIt!=vertexIndices.end();++viIt,++vertices)
		{
		/* Assemble the vertex from its components: */
		if(texCoordNode!=0)
			vertices->texCoord=Vertex::TexCoord(texCoordNode->getPoint(viIt->texCoord).getComponents());
		if(colorNode!=0)
			vertices->color=colorNode->getColor(viIt->color);
		if(normalNode!=0)
			vertices->normal=Vertex::Normal(normalNode->getVector(viIt->normal).getComponents());
		vertices->position=Vertex::Position(coordNode->getPoint(viIt->coord).getComponents());
		}
	
	/* Unmap and protect the vertex buffer: */
	glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
	
	/* Upload all vertex indices into the index buffers: */
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,dataItem->indexBufferObjectId);
	glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,triangleVertexIndices.size()*sizeof(GLuint),&triangleVertexIndices[0],GL_STATIC_DRAW_ARB);
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,0);
	}

VRMLNode::Box IndexedFaceSetNode::calcBoundingBox(void) const
	{
	/* Get a pointer to the coord node: */
	CoordinateNode* coordNode=dynamic_cast<CoordinateNode*>(coord.getPointer());
	
	/* Calculate the bounding box of all used vertex coordinates: */
	Box result=Box::empty;
	for(std::vector<int>::const_iterator ciIt=coordIndices.begin();ciIt!=coordIndices.end();++ciIt)
		if(*ciIt>=0)
			result.addPoint(coordNode->getPoint(*ciIt));
	return result;
	}

void IndexedFaceSetNode::glRenderAction(VRMLRenderState& renderState) const
	{
	/* Retrieve the data item from the context: */
	DataItem* dataItem=renderState.contextData.retrieveDataItem<DataItem>(this);
	
	const TextureCoordinateNode* texCoordNode=dynamic_cast<const TextureCoordinateNode*>(texCoord.getPointer());
	const ColorNode* colorNode=dynamic_cast<const ColorNode*>(color.getPointer());
	const NormalNode* normalNode=dynamic_cast<const NormalNode*>(normal.getPointer());
	const CoordinateNode* coordNode=dynamic_cast<const CoordinateNode*>(coord.getPointer());
	
	/* Set up OpenGL: */
	if(ccw)
		glFrontFace(GL_CCW);
	else
		glFrontFace(GL_CW);
	if(solid)
		{
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_FALSE);
		}
	else
		{
		glDisable(GL_CULL_FACE);
		glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);
		}
	
	if(dataItem->vertexBufferObjectId!=0&&dataItem->indexBufferObjectId!=0)
		{
		/* Determine which parts of the vertex array to enable: */
		int vertexPartsMask=0;
		if(texCoordNode!=0)
			vertexPartsMask|=GLVertexArrayParts::TexCoord;
		if(colorNode!=0)
			vertexPartsMask|=GLVertexArrayParts::Color;
		if(normalNode!=0)
			vertexPartsMask|=GLVertexArrayParts::Normal;
		vertexPartsMask|=GLVertexArrayParts::Position;
		
		/* Draw the indexed triangle set: */
		GLVertexArrayParts::enable(vertexPartsMask);
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,dataItem->vertexBufferObjectId);
		glVertexPointer(vertexPartsMask,static_cast<const Vertex*>(0));
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,dataItem->indexBufferObjectId);
		glDrawElements(GL_TRIANGLES,dataItem->numTriangles*3,GL_UNSIGNED_INT,static_cast<const GLuint*>(0));
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,0);
		GLVertexArrayParts::disable(vertexPartsMask);
		}
	else
		{
		/* Process all faces: */
		std::vector<int>::const_iterator texCoordIt=texCoordIndices.empty()?coordIndices.begin():texCoordIndices.begin();
		std::vector<int>::const_iterator colorIt=colorIndices.empty()?coordIndices.begin():colorIndices.begin();
		int colorCounter=0;
		std::vector<int>::const_iterator normalIt=normalIndices.empty()?coordIndices.begin():normalIndices.begin();
		int normalCounter=0;
		std::vector<int>::const_iterator coordIt=coordIndices.begin();
		while(coordIt!=coordIndices.end())
			{
			glBegin(GL_POLYGON);
			while(*coordIt>=0)
				{
				if(texCoordNode!=0)
					glTexCoord(texCoordNode->getPoint(*texCoordIt));
				if(colorNode!=0)
					{
					if(!colorPerVertex&&colorIndices.empty())
						glColor(colorNode->getColor(colorCounter));
					else
						glColor(colorNode->getColor(*colorIt));
					}
				if(normalNode!=0)
					{
					if(!normalPerVertex&&normalIndices.empty())
						glNormal(normalNode->getVector(normalCounter));
					else
						glNormal(normalNode->getVector(*normalIt));
					}
				glVertex(coordNode->getPoint(*coordIt));
				++texCoordIt;
				if(colorPerVertex)
					++colorIt;
				if(normalPerVertex)
					++normalIt;
				++coordIt;
				}
			glEnd();
			
			++texCoordIt;
			if(!colorPerVertex&&colorIndices.empty())
				++colorCounter;
			else
				++colorIt;
			if(!normalPerVertex&&normalIndices.empty())
				++normalCounter;
			else
				++normalIt;
			++coordIt;
			}
		}
	
	/* Reset OpenGL state: */
	if(!ccw)
		glFrontFace(GL_CCW);
	if(!solid)
		{
		glEnable(GL_CULL_FACE);
		glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_FALSE);
		}
	}
