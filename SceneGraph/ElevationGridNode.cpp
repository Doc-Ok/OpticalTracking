/***********************************************************************
ElevationGridNode - Class for quad-based height fields as renderable
geometry.
Copyright (c) 2009-2013 Oliver Kreylos

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

#define GLGEOMETRY_NONSTANDARD_TEMPLATES

#include <SceneGraph/ElevationGridNode.h>

#include <string.h>
#include <Math/Constants.h>
#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLContextData.h>
#include <GL/GLExtensionManager.h>
#include <GL/Extensions/GLARBVertexBufferObject.h>
#include <GL/GLGeometryWrappers.h>
#include <GL/GLGeometryVertex.h>
#include <SceneGraph/VRMLFile.h>
#include <SceneGraph/GLRenderState.h>

#include <SceneGraph/Internal/LoadElevationGrid.h>

namespace SceneGraph {

/********************************************
Methods of class ElevationGridNode::DataItem:
********************************************/

ElevationGridNode::DataItem::DataItem(void)
	:vertexBufferObjectId(0),indexBufferObjectId(0),
	 numQuads(0),numTriangles(0),
	 version(0)
	{
	if(GLARBVertexBufferObject::isSupported())
		{
		/* Initialize the vertex buffer object extension: */
		GLARBVertexBufferObject::initExtension();
		
		/* Create the vertex buffer object: */
		glGenBuffersARB(1,&vertexBufferObjectId);
		
		/* Create the index buffer object: */
		glGenBuffersARB(1,&indexBufferObjectId);
		}
	}

ElevationGridNode::DataItem::~DataItem(void)
	{
	/* Destroy the vertex buffer object: */
	if(vertexBufferObjectId!=0)
		glDeleteBuffersARB(1,&vertexBufferObjectId);
	
	/* Destroy the index buffer object: */
	if(indexBufferObjectId!=0)
		glDeleteBuffersARB(1,&indexBufferObjectId);
	}

/**********************************
Methods of class ElevationGridNode:
**********************************/

Point* ElevationGridNode::calcVertices(void) const
	{
	/* Allocate the result array: */
	int xDim=xDimension.getValue();
	int zDim=zDimension.getValue();
	Point* vertices=new Point[zDim*xDim];
	
	/* Calculate all vertex positions: */
	Scalar xSp=xSpacing.getValue();
	Scalar zSp=zSpacing.getValue();
	Point* vPtr=vertices;
	const Scalar* hPtr=&height.getValue(0);
	if(heightIsY.getValue())
		{
		Point p;
		p[2]=origin.getValue()[2];
		for(int z=0;z<zDim;++z,p[2]+=zSp)
			{
			p[0]=origin.getValue()[0];
			for(int x=0;x<xDim;++x,++vPtr,++hPtr,p[0]+=xSp)
				{
				p[1]=origin.getValue()[1]+*hPtr*heightScale.getValue();
				*vPtr=p;
				}
			}
		}
	else
		{
		Point p;
		p[1]=origin.getValue()[1];
		for(int z=0;z<zDim;++z,p[1]+=zSp)
			{
			p[0]=origin.getValue()[0];
			for(int x=0;x<xDim;++x,++vPtr,++hPtr,p[0]+=xSp)
				{
				p[2]=origin.getValue()[2]+*hPtr*heightScale.getValue();
				*vPtr=p;
				}
			}
		}
	
	return vertices;
	}

Vector* ElevationGridNode::calcQuadNormals(void) const
	{
	/* Allocate the result array: */
	int xDim=xDimension.getValue();
	int zDim=zDimension.getValue();
	Vector* normals=new Vector[(zDim-1)*(xDim-1)];
	
	/* Calculate the normal scaling factors: */
	Scalar nx=zSpacing.getValue()*heightScale.getValue();
	Scalar ny=xSpacing.getValue()*zSpacing.getValue();
	Scalar nz=xSpacing.getValue()*heightScale.getValue();
	if(!ccw.getValue())
		{
		/* Flip normal vectors if quads are oriented clockwise: */
		nx=-nx;
		ny=-ny;
		nz=-nz;
		}
	
	if(heightIsY.getValue())
		{
		/* Calculate all quad normal vectors: */
		Vector* nPtr=normals;
		for(int z=0;z<zDim-1;++z)
			for(int x=0;x<xDim-1;++x,++nPtr)
				{
				/* Calculate the quad normal as the average of the normals of the quad's two triangles: */
				const Scalar* h=&(height.getValue(z*xDim+x));
				(*nPtr)[0]=(h[0]-h[1]+h[xDim]-h[xDim+1])*nx;
				(*nPtr)[1]=ny*Scalar(2); // To average over sum of two triangle normals
				(*nPtr)[2]=(h[0]+h[1]-h[xDim]-h[xDim+1])*nz;
				}
		}
	else
		{
		/* Flip normal vectors to account for y,z-swap: */
		nx=-nx;
		ny=-ny;
		nz=-nz;
		
		/* Calculate all quad normal vectors: */
		Vector* nPtr=normals;
		for(int z=0;z<zDim-1;++z)
			for(int x=0;x<xDim-1;++x,++nPtr)
				{
				/* Calculate the quad normal as the average of the normals of the quad's two triangles: */
				const Scalar* h=&(height.getValue(z*xDim+x));
				(*nPtr)[0]=(h[0]-h[1]+h[xDim]-h[xDim+1])*nx;
				(*nPtr)[1]=(h[0]+h[1]-h[xDim]-h[xDim+1])*nz;
				(*nPtr)[2]=ny*Scalar(2); // To average over sum of two triangle normals
				}
		}
	
	return normals;
	}

int* ElevationGridNode::calcHoleyQuadCases(GLuint& numQuads,GLuint& numTriangles) const
	{
	/* Allocate the result array: */
	int xDim=xDimension.getValue();
	int zDim=zDimension.getValue();
	int* quadCases=new int[(zDim-1)*(xDim-1)];
	
	/* Calculate the triangulation cases for all grid cells: */
	numQuads=0;
	numTriangles=0;
	int* qcPtr=quadCases;
	for(int z=0;z<zDim-1;++z)
		for(int x=0;x<xDim-1;++x,++qcPtr)
			{
			/* Compare the grid cell's four corner elevations against the invalid value: */
			const Scalar* h=&(height.getValue(z*xDim+x));
			int c=0x0;
			if(h[0]!=invalidHeight.getValue())
				c+=0x1;
			if(h[1]!=invalidHeight.getValue())
				c+=0x2;
			if(h[xDim]!=invalidHeight.getValue())
				c+=0x4;
			if(h[xDim+1]!=invalidHeight.getValue())
				c+=0x8;
			
			/* Accumulate the number of quads or triangles for this grid cell: */
			if(c==0x7||c==0xb||c==0xd||c==0xe)
				++numTriangles;
			if(c==0xf)
				++numQuads;
			
			/* Store the quad case: */
			*qcPtr=c;
			}
	
	return quadCases;
	}

Vector* ElevationGridNode::calcHoleyQuadNormals(const int* quadCases) const
	{
	/* Allocate the result array: */
	int xDim=xDimension.getValue();
	int zDim=zDimension.getValue();
	Vector* normals=new Vector[(zDim-1)*(xDim-1)];
	
	/* Calculate the normal scaling factors: */
	Scalar nx=zSpacing.getValue()*heightScale.getValue();
	Scalar ny=xSpacing.getValue()*zSpacing.getValue();
	Scalar nz=xSpacing.getValue()*heightScale.getValue();
	if(!ccw.getValue())
		{
		/* Flip normal vectors if quads are oriented clockwise: */
		nx=-nx;
		ny=-ny;
		nz=-nz;
		}
	
	if(heightIsY.getValue())
		{
		/* Calculate all quad normal vectors: */
		Vector* nPtr=normals;
		const int* qcPtr=quadCases;
		for(int z=0;z<zDim-1;++z)
			for(int x=0;x<xDim-1;++x,++nPtr,++qcPtr)
				{
				/* Calculate the quad normal depending on the quad's triangulation case: */
				const Scalar* h=&(height.getValue(z*xDim+x));
				switch(*qcPtr)
					{
					case 0x7: // Lower-left triangle
						(*nPtr)[0]=(h[1]-h[0])*nx;
						(*nPtr)[1]=ny;
						(*nPtr)[2]=(h[0]-h[xDim])*nz;
						break;
					
					case 0xb: // Lower-right triangle
						(*nPtr)[0]=(h[0]-h[1])*nx;
						(*nPtr)[1]=ny;
						(*nPtr)[2]=(h[1]-h[xDim+1])*nz;
						break;
					
					case 0xd: // Upper-left triangle
						(*nPtr)[0]=(h[xDim]-h[xDim+1])*nx;
						(*nPtr)[1]=ny;
						(*nPtr)[2]=(h[0]-h[xDim])*nz;
						break;
					
					case 0xe: // Upper-right triangle
						(*nPtr)[0]=(h[xDim]-h[xDim+1])*nx;
						(*nPtr)[1]=ny;
						(*nPtr)[2]=(h[1]-h[xDim+1])*nz;
						break;
					
					case 0xf: // Full quad
						(*nPtr)[0]=(h[0]-h[1]+h[xDim]-h[xDim+1])*nx;
						(*nPtr)[1]=ny*Scalar(2); // To average over sum of two triangle normals;
						(*nPtr)[2]=(h[0]+h[1]-h[xDim]-h[xDim+1])*nz;
						break;
					
					default:
						*nPtr=Vector::zero;
					}
				}
		}
	else
		{
		/* Flip normal vectors to account for y,z-swap: */
		nx=-nx;
		ny=-ny;
		nz=-nz;
		
		/* Calculate all quad normal vectors: */
		Vector* nPtr=normals;
		const int* qcPtr=quadCases;
		for(int z=0;z<zDim-1;++z)
			for(int x=0;x<xDim-1;++x,++nPtr,++qcPtr)
				{
				/* Calculate the quad normal depending on the quad's triangulation case: */
				const Scalar* h=&(height.getValue(z*xDim+x));
				switch(*qcPtr)
					{
					case 0x7: // Lower-left triangle
						(*nPtr)[0]=(h[1]-h[0])*nx;
						(*nPtr)[1]=(h[0]-h[xDim])*nz;
						(*nPtr)[2]=ny;
						break;
					
					case 0xb: // Lower-right triangle
						(*nPtr)[0]=(h[0]-h[1])*nx;
						(*nPtr)[1]=(h[1]-h[xDim+1])*nz;
						(*nPtr)[2]=ny;
						break;
					
					case 0xd: // Upper-left triangle
						(*nPtr)[0]=(h[xDim]-h[xDim+1])*nx;
						(*nPtr)[1]=(h[0]-h[xDim])*nz;
						(*nPtr)[2]=ny;
						break;
					
					case 0xe: // Upper-right triangle
						(*nPtr)[0]=(h[xDim]-h[xDim+1])*nx;
						(*nPtr)[1]=(h[1]-h[xDim+1])*nz;
						(*nPtr)[2]=ny;
						break;
					
					case 0xf: // Full quad
						(*nPtr)[0]=(h[0]-h[1]+h[xDim]-h[xDim+1])*nx;
						(*nPtr)[1]=(h[0]+h[1]-h[xDim]-h[xDim+1])*nz;
						(*nPtr)[2]=ny*Scalar(2); // To average over sum of two triangle normals;
						break;
					
					default:
						*nPtr=Vector::zero;
					}
				}
		}
	
	return normals;
	}

void ElevationGridNode::uploadIndexedQuadStripSet(void) const
	{
	/* Define the vertex type used in the vertex array: */
	typedef GLGeometry::Vertex<Scalar,2,GLubyte,4,Scalar,Scalar,3> Vertex;
	
	/* Retrieve the elevation grid layout: */
	int xDim=xDimension.getValue();
	int zDim=zDimension.getValue();
	Scalar xSp=xSpacing.getValue();
	Scalar zSp=zSpacing.getValue();
	
	/* Calculate all per-quad normal vectors if there are no explicit normals: */
	Vector* quadNormals=0;
	if(normal.getValue()==0)
		quadNormals=calcQuadNormals();
	
	/* Initialize the vertex buffer object: */
	glBufferDataARB(GL_ARRAY_BUFFER_ARB,xDim*zDim*sizeof(Vertex),0,GL_STATIC_DRAW_ARB);
	
	/* Store all vertices: */
	int hComp=2;
	int zComp=1;
	if(heightIsY.getValue())
		std::swap(hComp,zComp);
	Scalar hOffset=origin.getValue()[hComp];
	Scalar zOffset=origin.getValue()[zComp];
	Vertex* vPtr=static_cast<Vertex*>(glMapBufferARB(GL_ARRAY_BUFFER_ARB,GL_WRITE_ONLY_ARB));
	size_t vInd=0;
	for(int z=0;z<zDim;++z)
		for(int x=0;x<xDim;++x,++vPtr,++vInd)
			{
			/* Calculate the raw vertex position: */
			Point p;
			p[0]=origin.getValue()[0]+Scalar(x)*Scalar(xSp);
			p[hComp]=hOffset+height.getValue(vInd)*heightScale.getValue();
			p[zComp]=zOffset+Scalar(z)*Scalar(zSp);
			
			/* Store the vertex' texture coordinate: */
			if(imageProjection.getValue()!=0)
				{
				/* Retrieve texture coordinates from the image projection node: */
				vPtr->texCoord=imageProjection.getValue()->calcTexCoord(p);
				}
			else if(texCoord.getValue()!=0)
				vPtr->texCoord=texCoord.getValue()->point.getValue(vInd);
			else
				{
				/* Generate standard texture coordinates: */
				vPtr->texCoord=Vertex::TexCoord(Scalar(x)/Scalar(xDim-1),Scalar(z)/Scalar(zDim-1));
				}
			
			/* Store the vertex' color: */
			if(color.getValue()!=0)
				vPtr->color=Vertex::Color(color.getValue()->color.getValue(vInd));
			else if(colorMap.getValue()!=0)
				vPtr->color=Vertex::Color(colorMap.getValue()->mapColor(hOffset+height.getValue(vInd)*heightScale.getValue()));
			else
				vPtr->color=Vertex::Color(255,255,255);
			
			/* Calculate the vertex normal: */
			Vector n;
			if(normal.getValue()!=0)
				{
				n=normal.getValue()->vector.getValue(vInd);
				if(!heightIsY.getValue())
					{
					std::swap(n[1],n[2]);
					n=-n;
					}
				}  
			else
				{
				/* Average the quad normals of quads surrounding the vertex: */
				n=Vector::zero;
				Vector* qn=quadNormals+(z*(xDim-1)+x);
				if(x>0)
					{
					if(z>0)
						n+=qn[-(xDim-1)-1];
					if(z<zDim-1)
						n+=qn[-1];
					}
				if(x<xDim-1)
					{
					if(z>0)
						n+=qn[-(xDim-1)];
					if(z<zDim-1)
						n+=qn[0];
					}
				}
			
			/* Store the vertex position and normal: */
			if(pointTransform.getValue()!=0)
				{
				vPtr->normal=Vertex::Normal(pointTransform.getValue()->transformNormal(p,n));
				vPtr->position=Vertex::Position(pointTransform.getValue()->transformPoint(p));
				}
			else
				{
				n.normalize();
				vPtr->normal=Vertex::Normal(n);
				vPtr->position=Vertex::Position(p);
				}
			}
	
	glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
	
	/* Delete the per-quad normals: */
	delete[] quadNormals;
	
	/* Initialize the index buffer object: */
	glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,(zDim-1)*xDim*2*sizeof(GLuint),0,GL_STATIC_DRAW_ARB);
	
	/* Store all vertex indices: */
	GLuint* iPtr=static_cast<GLuint*>(glMapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,GL_WRITE_ONLY_ARB));
	if(ccw.getValue())
		{
		for(int z=0;z<zDim-1;++z)
			for(int x=0;x<xDim;++x,iPtr+=2)
				{
				iPtr[0]=GLuint(z*xDim+x);
				iPtr[1]=GLuint((z+1)*xDim+x);
				}
		}
	else
		{
		for(int z=0;z<zDim-1;++z)
			for(int x=0;x<xDim;++x,iPtr+=2)
				{
				iPtr[0]=GLuint((z+1)*xDim+x);
				iPtr[1]=GLuint(z*xDim+x);
				}
		}
	
	glUnmapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB);
	}

void ElevationGridNode::uploadQuadSet(void) const
	{
	/* Define the vertex type used in the vertex array: */
	typedef GLGeometry::Vertex<Scalar,2,GLubyte,4,Scalar,Scalar,3> Vertex;
	
	/* Retrieve the elevation grid layout: */
	int xDim=xDimension.getValue();
	int zDim=zDimension.getValue();
	
	/* Calculate all untransformed vertex positions: */
	Point* vertices=calcVertices();
	
	/* Calculate all per-quad or per-vertex normal vectors if there are no explicit normals: */
	Vector* quadNormals=0;
	Vector* vertexNormals=0;
	if(normalPerVertex.getValue())
		{
		if(normal.getValue()!=0)
			{
			/* Copy the provided normals: */
			vertexNormals=new Vector[zDim*xDim];
			MF<Vector>::ValueList::const_iterator nIt=normal.getValue()->vector.getValues().begin();
			Vector* vnPtr=vertexNormals;
			if(heightIsY.getValue())
				{
				for(int i=zDim*xDim;i>0;--i,++nIt,++vnPtr)
					*vnPtr=*nIt;
				}
			else
				{
				for(int i=zDim*xDim;i>0;--i,++nIt,++vnPtr)
					{
					(*vnPtr)[0]=-(*nIt)[0];
					(*vnPtr)[1]=-(*nIt)[2];
					(*vnPtr)[2]=-(*nIt)[1];
					}
				}
			}
		else
			{
			/* Calculate the per-quad normals: */
			quadNormals=calcQuadNormals();
			
			/* Convert the per-quad normals to non-normalized per-vertex normals: */
			vertexNormals=new Vector[zDim*xDim];
			Vector* vnPtr=vertexNormals;
			for(int z=0;z<zDim;++z)
				for(int x=0;x<xDim;++x,++vnPtr)
					{
					*vnPtr=Vector::zero;
					Vector* qn=quadNormals+(z*(xDim-1)+x);
					if(x>0)
						{
						if(z>0)
							*vnPtr+=qn[-(xDim-1)-1];
						if(z<zDim-1)
							*vnPtr+=qn[-1];
						}
					if(x<xDim-1)
						{
						if(z>0)
							*vnPtr+=qn[-(xDim-1)];
						if(z<zDim-1)
							*vnPtr+=qn[0];
						}
					}
			
			/* Delete the per-quad normals: */
			delete[] quadNormals;
			quadNormals=0;
			}
		
		if(pointTransform.getValue()!=0)
			{
			/* Transform the per-vertex normals: */
			Point* vPtr=vertices;
			Vector* vnPtr=vertexNormals;
			for(int i=zDim*xDim;i>0;--i,++vPtr,++vnPtr)
				*vnPtr=pointTransform.getValue()->transformNormal(*vPtr,*vnPtr);
			}
		else
			{
			/* Normalize the per-vertex normals: */
			Vector* vnPtr=vertexNormals;
			for(int i=zDim*xDim;i>0;--i,++vnPtr)
				vnPtr->normalize();
			}
		}
	else
		{
		if(normal.getValue()!=0)
			{
			/* Copy the provided normals: */
			quadNormals=new Vector[(zDim-1)*(xDim-1)];
			MF<Vector>::ValueList::const_iterator nIt=normal.getValue()->vector.getValues().begin();
			Vector* qnPtr=quadNormals;
			if(heightIsY.getValue())
				{
				for(int i=(zDim-1)*(xDim-1);i>0;--i,++nIt,++qnPtr)
					*qnPtr=*nIt;
				}
			else
				{
				for(int i=(zDim-1)*(xDim-1);i>0;--i,++nIt,++qnPtr)
					{
					(*qnPtr)[0]=-(*nIt)[0];
					(*qnPtr)[1]=-(*nIt)[2];
					(*qnPtr)[2]=-(*nIt)[1];
					}
				}
			}
		else
			{
			/* Calculate the per-quad normals: */
			quadNormals=calcQuadNormals();
			}
		
		if(pointTransform.getValue()!=0)
			{
			/* Transform the per-quad normals: */
			Vector* qnPtr=quadNormals;
			for(int z=0;z<zDim-1;++z)
				for(int x=0;x<xDim-1;++x,++qnPtr)
					{
					/* Transform the per-quad normal around the quad's midpoint: */
					Point* vPtr=vertices+(z*xDim+x);
					Point mp;
					for(int i=0;i<3;++i)
						mp[i]=(vPtr[0][i]+vPtr[1][i]+vPtr[xDim][i]+vPtr[xDim+1][i])*Scalar(0.25);
					*qnPtr=pointTransform.getValue()->transformNormal(mp,*qnPtr);
					}
			}
		else
			{
			/* Normalize the per-quad normals: */
			Vector* qnPtr=quadNormals;
			for(int i=(zDim-1)*(xDim-1);i>0;--i,++qnPtr)
				qnPtr->normalize();
			}
		}
	
	TexCoord* vertexTexCoords=0;
	if(imageProjection.getValue()!=0)
		{
		/* Calculate per-vertex texture coordinates: */
		vertexTexCoords=new TexCoord[xDim*zDim];
		Point* vPtr=vertices;
		TexCoord* vtcPtr=vertexTexCoords;
		for(int z=0;z<zDim;++z)
			for(int x=0;x<xDim;++x,++vPtr,++vtcPtr)
				*vtcPtr=imageProjection.getValue()->calcTexCoord(*vPtr);
		}
	
	if(pointTransform.getValue()!=0)
		{
		/* Transform all vertex positions: */
		Point* vPtr=vertices;
		for(int i=zDim*xDim;i>0;--i,++vPtr)
			*vPtr=pointTransform.getValue()->transformPoint(*vPtr);
		}
	
	/* Initialize the vertex buffer object: */
	glBufferDataARB(GL_ARRAY_BUFFER_ARB,(xDim-1)*(zDim-1)*4*sizeof(Vertex),0,GL_STATIC_DRAW_ARB);
	
	/* Store all vertices: */
	int hComp=heightIsY.getValue()?1:2;
	Scalar hOffset=origin.getValue()[hComp];
	Vertex* vPtr=static_cast<Vertex*>(glMapBufferARB(GL_ARRAY_BUFFER_ARB,GL_WRITE_ONLY_ARB));
	size_t qInd=0;
	for(int z=0;z<zDim-1;++z)
		for(int x=0;x<xDim-1;++x,vPtr+=4,++qInd)
			{
			size_t vInd=z*xDim+x;
			Vertex v[4];
			
			/* Calculate the corner texture coordinates of the current quad: */
			if(imageProjection.getValue()!=0)
				{
				/* Store the per-vertex texture coordinates: */
				v[0].texCoord=vertexTexCoords[vInd];
				v[1].texCoord=vertexTexCoords[vInd+1];
				v[2].texCoord=vertexTexCoords[vInd+xDim+1];
				v[3].texCoord=vertexTexCoords[vInd+xDim];
				}
			else if(texCoord.getValue()!=0)
				{
				v[0].texCoord=Vertex::TexCoord(texCoord.getValue()->point.getValue(vInd));
				v[1].texCoord=Vertex::TexCoord(texCoord.getValue()->point.getValue(vInd+1));
				v[2].texCoord=Vertex::TexCoord(texCoord.getValue()->point.getValue(vInd+xDim+1));
				v[3].texCoord=Vertex::TexCoord(texCoord.getValue()->point.getValue(vInd+xDim));
				}
			else
				{
				v[0].texCoord=Vertex::TexCoord(Scalar(x)/Scalar(xDim-1),Scalar(z)/Scalar(zDim-1));
				v[1].texCoord=Vertex::TexCoord(Scalar(x+1)/Scalar(xDim-1),Scalar(z)/Scalar(zDim-1));
				v[2].texCoord=Vertex::TexCoord(Scalar(x+1)/Scalar(xDim-1),Scalar(z+1)/Scalar(zDim-1));
				v[3].texCoord=Vertex::TexCoord(Scalar(x)/Scalar(xDim-1),Scalar(z+1)/Scalar(zDim-1));
				}
			
			/* Get the corner colors of the current quad: */
			if(color.getValue()!=0)
				{
				if(colorPerVertex.getValue())
					{
					v[0].color=Vertex::Color(color.getValue()->color.getValue(vInd));
					v[1].color=Vertex::Color(color.getValue()->color.getValue(vInd+1));
					v[2].color=Vertex::Color(color.getValue()->color.getValue(vInd+xDim+1));
					v[3].color=Vertex::Color(color.getValue()->color.getValue(vInd+xDim));
					}
				else
					{
					Vertex::Color c=Vertex::Color(color.getValue()->color.getValue(qInd));
					for(int i=0;i<4;++i)
						v[i].color=c;
					}
				}
			else if(colorMap.getValue()!=0)
				{
				if(colorPerVertex.getValue())
					{
					v[0].color=Vertex::Color(colorMap.getValue()->mapColor(hOffset+height.getValue(vInd)*heightScale.getValue()));
					v[1].color=Vertex::Color(colorMap.getValue()->mapColor(hOffset+height.getValue(vInd+1)*heightScale.getValue()));
					v[2].color=Vertex::Color(colorMap.getValue()->mapColor(hOffset+height.getValue(vInd+xDim+1)*heightScale.getValue()));
					v[3].color=Vertex::Color(colorMap.getValue()->mapColor(hOffset+height.getValue(vInd+xDim)*heightScale.getValue()));
					}
				else
					{
					Scalar h=(height.getValue(vInd)+height.getValue(vInd+1)+height.getValue(vInd+xDim)+height.getValue(vInd+xDim+1))*heightScale.getValue();
					Vertex::Color c=Vertex::Color(colorMap.getValue()->mapColor(hOffset+h*Scalar(0.25)));
					for(int i=0;i<4;++i)
						v[i].color=c;
					}
				}
			else
				{
				for(int i=0;i<4;++i)
					v[i].color=Vertex::Color(255,255,255);
				}
			
			/* Set the corner normal vectors and vertex positions of the current quad: */
			if(normalPerVertex.getValue())
				{
				v[0].normal=Vertex::Normal(vertexNormals[vInd]);
				v[1].normal=Vertex::Normal(vertexNormals[vInd+1]);
				v[2].normal=Vertex::Normal(vertexNormals[vInd+xDim+1]);
				v[3].normal=Vertex::Normal(vertexNormals[vInd+xDim]);
				}
			else
				{
				Vertex::Normal n=Vertex::Normal(quadNormals[qInd]);
				for(int i=0;i<4;++i)
					v[i].normal=n;
				}
			v[0].position=Vertex::Position(vertices[vInd]);
			v[1].position=Vertex::Position(vertices[vInd+1]);
			v[2].position=Vertex::Position(vertices[vInd+xDim+1]);
			v[3].position=Vertex::Position(vertices[vInd+xDim]);
			
			/* Store the corner vertices of the current quad: */
			if(ccw.getValue())
				{
				/* Store the corner vertices in counter-clockwise order: */
				for(int i=0;i<4;++i)
					vPtr[i]=v[3-i];
				}
			else
				{
				/* Store the corner vertices in clockwise order: */
				for(int i=0;i<4;++i)
					vPtr[i]=v[i];
				}
			}
	
	glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
	
	/* Delete the per-quad and per-vertex normals, texture coordinates, and vertex positions: */
	delete[] quadNormals;
	delete[] vertexNormals;
	delete[] vertexTexCoords;
	delete[] vertices;
	}

void ElevationGridNode::uploadHoleyQuadTriangleSet(GLuint& numQuads,GLuint& numTriangles) const
	{
	/* Define the vertex type used in the vertex array: */
	typedef GLGeometry::Vertex<Scalar,2,GLubyte,4,Scalar,Scalar,3> Vertex;
	
	/* Retrieve the elevation grid layout: */
	int xDim=xDimension.getValue();
	int zDim=zDimension.getValue();
	
	/* Calculate all untransformed vertex positions: */
	Point* vertices=calcVertices();
	
	/* Calculate the triangulation cases for all grid quads: */
	int* quadCases=calcHoleyQuadCases(numQuads,numTriangles);
	
	/* Calculate all per-quad or per-vertex normal vectors if there are no explicit normals: */
	Vector* quadNormals=0;
	Vector* vertexNormals=0;
	if(normalPerVertex.getValue())
		{
		if(normal.getValue()!=0)
			{
			/* Copy the provided normals: */
			vertexNormals=new Vector[zDim*xDim];
			MF<Vector>::ValueList::const_iterator nIt=normal.getValue()->vector.getValues().begin();
			Vector* vnPtr=vertexNormals;
			if(heightIsY.getValue())
				{
				for(int i=zDim*xDim;i>0;--i,++nIt,++vnPtr)
					*vnPtr=*nIt;
				}
			else
				{
				for(int i=zDim*xDim;i>0;--i,++nIt,++vnPtr)
					{
					(*vnPtr)[0]=-(*nIt)[0];
					(*vnPtr)[1]=-(*nIt)[2];
					(*vnPtr)[2]=-(*nIt)[1];
					}
				}
			}
		else
			{
			/* Calculate the per-quad normals: */
			quadNormals=calcHoleyQuadNormals(quadCases);
			
			/* Convert the per-quad normals to non-normalized per-vertex normals: */
			vertexNormals=new Vector[zDim*xDim];
			MF<Scalar>::ValueList::const_iterator hIt=height.getValues().begin();
			Vector* vnPtr=vertexNormals;
			for(int z=0;z<zDim;++z)
				for(int x=0;x<xDim;++x,++hIt,++vnPtr)
					if(*hIt!=invalidHeight.getValue())
						{
						*vnPtr=Vector::zero;
						int* qc=quadCases+(z*(xDim-1)+x);
						Vector* qn=quadNormals+(z*(xDim-1)+x);
						
						/* Add each surrounding quad's normal 0, 1, or 2 times depending on the respective quad's triangulation case: */
						if(x>0)
							{
							if(z>0)
								{
								if((qc[-(xDim-1)-1]&0xa)==0xa)
									*vnPtr+=qn[-(xDim-1)-1];
								if((qc[-(xDim-1)-1]&0xc)==0xc)
									*vnPtr+=qn[-(xDim-1)-1];
								}
							if(z<zDim-1)
								{
								if((qc[-1]&0x3)==0x3)
									*vnPtr+=qn[-1];
								if((qc[-1]&0xa)==0xa)
									*vnPtr+=qn[-1];
								}
							}
						if(x<xDim-1)
							{
							if(z>0)
								{
								if((qc[-(xDim-1)]&0x5)==0x5)
									*vnPtr+=qn[-(xDim-1)];
								if((qc[-(xDim-1)]&0xc)==0xc)
									*vnPtr+=qn[-(xDim-1)];
								}
							if(z<zDim-1)
								{
								if((qc[0]&0x3)==0x3)
									*vnPtr+=qn[0];
								if((qc[0]&0x5)==0x5)
									*vnPtr+=qn[0];
								}
							}
						}
			
			/* Delete the per-quad normals: */
			delete[] quadNormals;
			quadNormals=0;
			}
		
		if(pointTransform.getValue()!=0)
			{
			/* Transform the per-vertex normals: */
			MF<Scalar>::ValueList::const_iterator hIt=height.getValues().begin();
			Point* vPtr=vertices;
			Vector* vnPtr=vertexNormals;
			for(int i=zDim*xDim;i>0;--i,++hIt,++vPtr,++vnPtr)
				if(*hIt!=invalidHeight.getValue())
					*vnPtr=pointTransform.getValue()->transformNormal(*vPtr,*vnPtr);
			}
		else
			{
			/* Normalize the per-vertex normals: */
			MF<Scalar>::ValueList::const_iterator hIt=height.getValues().begin();
			Vector* vnPtr=vertexNormals;
			for(int i=zDim*xDim;i>0;--i,++hIt,++vnPtr)
				if(*hIt!=invalidHeight.getValue())
					vnPtr->normalize();
			}
		}
	else
		{
		if(normal.getValue()!=0)
			{
			/* Copy the provided normals: */
			quadNormals=new Vector[(zDim-1)*(xDim-1)];
			MF<Vector>::ValueList::const_iterator nIt=normal.getValue()->vector.getValues().begin();
			Vector* qnPtr=quadNormals;
			if(heightIsY.getValue())
				{
				for(int i=(zDim-1)*(xDim-1);i>0;--i,++nIt,++qnPtr)
					*qnPtr=*nIt;
				}
			else
				{
				for(int i=(zDim-1)*(xDim-1);i>0;--i,++nIt,++qnPtr)
					{
					(*qnPtr)[0]=-(*nIt)[0];
					(*qnPtr)[1]=-(*nIt)[2];
					(*qnPtr)[2]=-(*nIt)[1];
					}
				}
			}
		else
			{
			/* Calculate the per-quad normals: */
			quadNormals=calcHoleyQuadNormals(quadCases);
			}
		
		if(pointTransform.getValue()!=0)
			{
			/* Transform the per-quad normals: */
			int* qcPtr=quadCases;
			Vector* qnPtr=quadNormals;
			for(int z=0;z<zDim-1;++z)
				for(int x=0;x<xDim-1;++x,++qcPtr,++qnPtr)
					{
					/* Transform the per-quad normal around the quad's midpoint: */
					Point* vPtr=vertices+(z*xDim+x);
					Point mp;
					switch(*qcPtr)
						{
						case 0x7:
							for(int i=0;i<3;++i)
								mp[i]=(vPtr[0][i]+vPtr[1][i]+vPtr[xDim][i])/Scalar(3);
							*qnPtr=pointTransform.getValue()->transformNormal(mp,*qnPtr);
							break;
						
						case 0xb:
							for(int i=0;i<3;++i)
								mp[i]=(vPtr[0][i]+vPtr[1][i]+vPtr[xDim+1][i])/Scalar(3);
							*qnPtr=pointTransform.getValue()->transformNormal(mp,*qnPtr);
							break;
						
						case 0xd:
							for(int i=0;i<3;++i)
								mp[i]=(vPtr[0][i]+vPtr[xDim][i]+vPtr[xDim+1][i])/Scalar(3);
							*qnPtr=pointTransform.getValue()->transformNormal(mp,*qnPtr);
							break;
						
						case 0xe:
							for(int i=0;i<3;++i)
								mp[i]=(vPtr[1][i]+vPtr[xDim][i]+vPtr[xDim+1][i])/Scalar(3);
							*qnPtr=pointTransform.getValue()->transformNormal(mp,*qnPtr);
							break;
						
						case 0xf:
							for(int i=0;i<3;++i)
								mp[i]=(vPtr[0][i]+vPtr[1][i]+vPtr[xDim][i]+vPtr[xDim+1][i])*Scalar(0.25);
							*qnPtr=pointTransform.getValue()->transformNormal(mp,*qnPtr);
							break;
						}
					}
			}
		else
			{
			/* Normalize the per-quad normals: */
			Vector* qnPtr=quadNormals;
			for(int i=(zDim-1)*(xDim-1);i>0;--i,++qnPtr)
				qnPtr->normalize();
			}
		}
	
	TexCoord* vertexTexCoords=0;
	if(imageProjection.getValue()!=0)
		{
		/* Calculate per-vertex texture coordinates: */
		vertexTexCoords=new TexCoord[xDim*zDim];
		Point* vPtr=vertices;
		TexCoord* vtcPtr=vertexTexCoords;
		for(int z=0;z<zDim;++z)
			for(int x=0;x<xDim;++x,++vPtr,++vtcPtr)
				*vtcPtr=imageProjection.getValue()->calcTexCoord(*vPtr);
		}
	
	if(pointTransform.getValue()!=0)
		{
		/* Transform all vertex positions: */
		MF<Scalar>::ValueList::const_iterator hIt=height.getValues().begin();
		Point* vPtr=vertices;
		for(int i=zDim*xDim;i>0;--i,++hIt,++vPtr)
			if(*hIt!=invalidHeight.getValue())
				*vPtr=pointTransform.getValue()->transformPoint(*vPtr);
		}
	
	/* Initialize the vertex buffer object: */
	glBufferDataARB(GL_ARRAY_BUFFER_ARB,(numQuads*4+numTriangles*3)*sizeof(Vertex),0,GL_STATIC_DRAW_ARB);
	
	/* Store all vertices: */
	int hComp=heightIsY.getValue()?1:2;
	Scalar hOffset=origin.getValue()[hComp];
	Vertex* qvPtr=static_cast<Vertex*>(glMapBufferARB(GL_ARRAY_BUFFER_ARB,GL_WRITE_ONLY_ARB));
	Vertex* tvPtr=qvPtr+(numQuads*4);
	size_t qInd=0;
	for(int z=0;z<zDim-1;++z)
		for(int x=0;x<xDim-1;++x,++qInd)
			{
			size_t vInd=z*xDim+x;
			Vertex v[4];
			
			/* Calculate the corner texture coordinates of the current quad: */
			if(imageProjection.getValue()!=0)
				{
				/* Store the per-vertex texture coordinates: */
				v[0].texCoord=vertexTexCoords[vInd];
				v[1].texCoord=vertexTexCoords[vInd+1];
				v[2].texCoord=vertexTexCoords[vInd+xDim+1];
				v[3].texCoord=vertexTexCoords[vInd+xDim];
				}
			else if(texCoord.getValue()!=0)
				{
				v[0].texCoord=Vertex::TexCoord(texCoord.getValue()->point.getValue(vInd));
				v[1].texCoord=Vertex::TexCoord(texCoord.getValue()->point.getValue(vInd+1));
				v[2].texCoord=Vertex::TexCoord(texCoord.getValue()->point.getValue(vInd+xDim+1));
				v[3].texCoord=Vertex::TexCoord(texCoord.getValue()->point.getValue(vInd+xDim));
				}
			else
				{
				v[0].texCoord=Vertex::TexCoord(Scalar(x)/Scalar(xDim-1),Scalar(z)/Scalar(zDim-1));
				v[1].texCoord=Vertex::TexCoord(Scalar(x+1)/Scalar(xDim-1),Scalar(z)/Scalar(zDim-1));
				v[2].texCoord=Vertex::TexCoord(Scalar(x+1)/Scalar(xDim-1),Scalar(z+1)/Scalar(zDim-1));
				v[3].texCoord=Vertex::TexCoord(Scalar(x)/Scalar(xDim-1),Scalar(z+1)/Scalar(zDim-1));
				}
			
			/* Get the corner colors of the current quad: */
			if(color.getValue()!=0)
				{
				if(colorPerVertex.getValue())
					{
					v[0].color=Vertex::Color(color.getValue()->color.getValue(vInd));
					v[1].color=Vertex::Color(color.getValue()->color.getValue(vInd+1));
					v[2].color=Vertex::Color(color.getValue()->color.getValue(vInd+xDim+1));
					v[3].color=Vertex::Color(color.getValue()->color.getValue(vInd+xDim));
					}
				else
					{
					Vertex::Color c=Vertex::Color(color.getValue()->color.getValue(qInd));
					for(int i=0;i<4;++i)
						v[i].color=c;
					}
				}
			else if(colorMap.getValue()!=0)
				{
				if(colorPerVertex.getValue())
					{
					v[0].color=Vertex::Color(colorMap.getValue()->mapColor(hOffset+height.getValue(vInd)*heightScale.getValue()));
					v[1].color=Vertex::Color(colorMap.getValue()->mapColor(hOffset+height.getValue(vInd+1)*heightScale.getValue()));
					v[2].color=Vertex::Color(colorMap.getValue()->mapColor(hOffset+height.getValue(vInd+xDim+1)*heightScale.getValue()));
					v[3].color=Vertex::Color(colorMap.getValue()->mapColor(hOffset+height.getValue(vInd+xDim)*heightScale.getValue()));
					}
				else
					{
					Scalar h=Scalar(0);
					Scalar w=Scalar(0);
					if(quadCases[qInd]&0x1)
						{
						h+=height.getValue(vInd);
						w+=Scalar(1);
						}
					if(quadCases[qInd]&0x2)
						{
						h+=height.getValue(vInd+1);
						w+=Scalar(1);
						}
					if(quadCases[qInd]&0x4)
						{
						h+=height.getValue(vInd+xDim);
						w+=Scalar(1);
						}
					if(quadCases[qInd]&0x8)
						{
						h+=height.getValue(vInd+xDim+1);
						w+=Scalar(1);
						}
					Vertex::Color c=Vertex::Color(colorMap.getValue()->mapColor(hOffset+h*heightScale.getValue()/w));
					for(int i=0;i<4;++i)
						v[i].color=c;
					}
				}
			else
				{
				for(int i=0;i<4;++i)
					v[i].color=Vertex::Color(255,255,255);
				}
			
			/* Set the corner normal vectors and vertex positions of the current quad: */
			if(normalPerVertex.getValue())
				{
				v[0].normal=Vertex::Normal(vertexNormals[vInd]);
				v[1].normal=Vertex::Normal(vertexNormals[vInd+1]);
				v[2].normal=Vertex::Normal(vertexNormals[vInd+xDim+1]);
				v[3].normal=Vertex::Normal(vertexNormals[vInd+xDim]);
				}
			else
				{
				Vertex::Normal n=Vertex::Normal(quadNormals[qInd]);
				for(int i=0;i<4;++i)
					v[i].normal=n;
				}
			v[0].position=Vertex::Position(vertices[vInd]);
			v[1].position=Vertex::Position(vertices[vInd+1]);
			v[2].position=Vertex::Position(vertices[vInd+xDim+1]);
			v[3].position=Vertex::Position(vertices[vInd+xDim]);
			
			/* Store the corner vertices of the current grid cell depending on the cell's triangulation case: */
			if(ccw.getValue())
				{
				/* Store the corner vertices in counter-clockwise order: */
				switch(quadCases[qInd])
					{
					case 0x7:
						tvPtr[0]=v[3];
						tvPtr[1]=v[1];
						tvPtr[2]=v[0];
						tvPtr+=3;
						break;
					
					case 0xb:
						tvPtr[0]=v[2];
						tvPtr[1]=v[1];
						tvPtr[2]=v[0];
						tvPtr+=3;
						break;
					
					case 0xd:
						tvPtr[0]=v[3];
						tvPtr[1]=v[2];
						tvPtr[2]=v[0];
						tvPtr+=3;
						break;
					
					case 0xe:
						tvPtr[0]=v[3];
						tvPtr[1]=v[2];
						tvPtr[2]=v[1];
						tvPtr+=3;
						break;
					
					case 0xf:
						for(int i=0;i<4;++i)
							qvPtr[i]=v[3-i];
						qvPtr+=4;
					}
				}
			else
				{
				/* Store the corner vertices in clockwise order: */
				switch(quadCases[qInd])
					{
					case 0x7:
						tvPtr[0]=v[0];
						tvPtr[1]=v[1];
						tvPtr[2]=v[3];
						tvPtr+=3;
						break;
					
					case 0xb:
						tvPtr[0]=v[0];
						tvPtr[1]=v[1];
						tvPtr[2]=v[2];
						tvPtr+=3;
						break;
					
					case 0xd:
						tvPtr[0]=v[0];
						tvPtr[1]=v[2];
						tvPtr[2]=v[3];
						tvPtr+=3;
						break;
					
					case 0xe:
						tvPtr[0]=v[1];
						tvPtr[1]=v[2];
						tvPtr[2]=v[3];
						tvPtr+=3;
						break;
					
					case 0xf:
						for(int i=0;i<4;++i)
							qvPtr[i]=v[i];
						qvPtr+=4;
					}
				}
			}
	
	glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
	
	/* Delete the per-quad and per-vertex normals, the triangulation cases, texture coordinates, and the vertex positions: */
	delete[] quadNormals;
	delete[] vertexNormals;
	delete[] quadCases;
	delete[] vertexTexCoords;
	delete[] vertices;
	}

ElevationGridNode::ElevationGridNode(void)
	:colorPerVertex(true),normalPerVertex(true),
	 creaseAngle(0),
	 origin(Point::origin),
	 xDimension(0),xSpacing(0),
	 zDimension(0),zSpacing(0),
	 heightScale(1),
	 heightIsY(true),
	 removeInvalids(false),invalidHeight(0),
	 ccw(true),solid(true),
	 multiplexer(0),valid(false),indexed(false),version(0)
	{
	}

const char* ElevationGridNode::getStaticClassName(void)
	{
	return "ElevationGrid";
	}

const char* ElevationGridNode::getClassName(void) const
	{
	return "ElevationGrid";
	}

void ElevationGridNode::parseField(const char* fieldName,VRMLFile& vrmlFile)
	{
	if(strcmp(fieldName,"texCoord")==0)
		vrmlFile.parseSFNode(texCoord);
	else if(strcmp(fieldName,"color")==0)
		vrmlFile.parseSFNode(color);
	else if(strcmp(fieldName,"colorMap")==0)
		vrmlFile.parseSFNode(colorMap);
	else if(strcmp(fieldName,"imageProjection")==0)
		vrmlFile.parseSFNode(imageProjection);
	else if(strcmp(fieldName,"colorPerVertex")==0)
		vrmlFile.parseField(colorPerVertex);
	else if(strcmp(fieldName,"normal")==0)
		vrmlFile.parseSFNode(normal);
	else if(strcmp(fieldName,"normalPerVertex")==0)
		vrmlFile.parseField(normalPerVertex);
	else if(strcmp(fieldName,"creaseAngle")==0)
		vrmlFile.parseField(creaseAngle);
	else if(strcmp(fieldName,"origin")==0)
		vrmlFile.parseField(origin);
	else if(strcmp(fieldName,"xDimension")==0)
		vrmlFile.parseField(xDimension);
	else if(strcmp(fieldName,"xSpacing")==0)
		vrmlFile.parseField(xSpacing);
	else if(strcmp(fieldName,"zDimension")==0)
		vrmlFile.parseField(zDimension);
	else if(strcmp(fieldName,"zSpacing")==0)
		vrmlFile.parseField(zSpacing);
	else if(strcmp(fieldName,"height")==0)
		vrmlFile.parseField(height);
	else if(strcmp(fieldName,"heightUrl")==0)
		{
		vrmlFile.parseField(heightUrl);
		
		/* Fully qualify all URLs: */
		for(size_t i=0;i<heightUrl.getNumValues();++i)
			heightUrl.setValue(i,vrmlFile.getFullUrl(heightUrl.getValue(i)));
		
		/* Store the VRML file's multicast pipe multiplexer: */
		multiplexer=vrmlFile.getMultiplexer();
		}
	else if(strcmp(fieldName,"heightUrlFormat")==0)
		vrmlFile.parseField(heightUrlFormat);
	else if(strcmp(fieldName,"heightScale")==0)
		vrmlFile.parseField(heightScale);
	else if(strcmp(fieldName,"heightIsY")==0)
		vrmlFile.parseField(heightIsY);
	else if(strcmp(fieldName,"removeInvalids")==0)
		vrmlFile.parseField(removeInvalids);
	else if(strcmp(fieldName,"invalidHeight")==0)
		vrmlFile.parseField(invalidHeight);
	else if(strcmp(fieldName,"ccw")==0)
		vrmlFile.parseField(ccw);
	else if(strcmp(fieldName,"solid")==0)
		vrmlFile.parseField(solid);
	else
		GeometryNode::parseField(fieldName,vrmlFile);
	}

void ElevationGridNode::update(void)
	{
	/* Check whether the height field should be loaded from a file: */
	if(heightUrl.getNumValues()>0)
		{
		try
			{
			/* Load the elevation grid's height values: */
			loadElevationGrid(*this,multiplexer);
			}
		catch(std::runtime_error err)
			{
			/* Carry on... */
			}
		}
	
	/* Check whether the elevation grid is valid: */
	valid=xDimension.getValue()>0&&zDimension.getValue()>0&&height.getNumValues()>=size_t(xDimension.getValue())*size_t(zDimension.getValue());
	
	/* Check whether the elevation grid can be represented by a set of indexed triangle strips: */
	indexed=(colorPerVertex.getValue()||(color.getValue()==0&&colorMap.getValue()==0))&&normalPerVertex.getValue();
	
	/* Check if there are invalid heights to be removed: */
	haveInvalids=false;
	if(removeInvalids.getValue())
		{
		/* Test all heights against the invalid value: */
		for(MFFloat::ValueList::iterator hIt=height.getValues().begin();hIt!=height.getValues().end();++hIt)
			if(*hIt==invalidHeight.getValue())
				{
				haveInvalids=true;
				break;
				}
		}
	
	/* Can't use indexed quad strip if there are holes: */
	if(haveInvalids)
		indexed=false;
	
	/* Bump up the elevation grid's version number: */
	++version;
	}

Box ElevationGridNode::calcBoundingBox(void) const
	{
	Box result=Box::empty;
	
	if(valid)
		{
		if(pointTransform.getValue()!=0)
			{
			/* Return the bounding box of the transformed point coordinates: */
			if(heightIsY.getValue())
				{
				if(haveInvalids)
					{
					MFFloat::ValueList::const_iterator hIt=height.getValues().begin();
					Point p;
					p[2]=origin.getValue()[2];
					for(int z=0;z<zDimension.getValue();++z,p[2]+=zSpacing.getValue())
						{
						p[0]=origin.getValue()[0];
						for(int x=0;x<xDimension.getValue();++x,p[0]+=xSpacing.getValue(),++hIt)
							if(*hIt!=invalidHeight.getValue())
								{
								p[1]=origin.getValue()[1]+*hIt*heightScale.getValue();
								result.addPoint(pointTransform.getValue()->transformPoint(p));
								}
						}
					}
				else
					{
					MFFloat::ValueList::const_iterator hIt=height.getValues().begin();
					Point p;
					p[2]=origin.getValue()[2];
					for(int z=0;z<zDimension.getValue();++z,p[2]+=zSpacing.getValue())
						{
						p[0]=origin.getValue()[0];
						for(int x=0;x<xDimension.getValue();++x,p[0]+=xSpacing.getValue(),++hIt)
							{
							p[1]=origin.getValue()[1]+*hIt*heightScale.getValue();
							result.addPoint(pointTransform.getValue()->transformPoint(p));
							}
						}
					}
				}
			else
				{
				if(haveInvalids)
					{
					MFFloat::ValueList::const_iterator hIt=height.getValues().begin();
					Point p;
					p[1]=origin.getValue()[1];
					for(int z=0;z<zDimension.getValue();++z,p[1]+=zSpacing.getValue())
						{
						p[0]=origin.getValue()[0];
						for(int x=0;x<xDimension.getValue();++x,p[0]+=xSpacing.getValue(),++hIt)
							if(*hIt!=invalidHeight.getValue())
								{
								p[2]=origin.getValue()[2]+*hIt*heightScale.getValue();
								result.addPoint(pointTransform.getValue()->transformPoint(p));
								}
						}
					}
				else
					{
					MFFloat::ValueList::const_iterator hIt=height.getValues().begin();
					Point p;
					p[1]=origin.getValue()[1];
					for(int z=0;z<zDimension.getValue();++z,p[1]+=zSpacing.getValue())
						{
						p[0]=origin.getValue()[0];
						for(int x=0;x<xDimension.getValue();++x,p[0]+=xSpacing.getValue(),++hIt)
							{
							p[2]=origin.getValue()[2]+*hIt*heightScale.getValue();
							result.addPoint(pointTransform.getValue()->transformPoint(p));
							}
						}
					}
				}
			}
		else
			{
			/* Return the bounding box of the untransformed point coordinates: */
			if(haveInvalids)
				{
				Scalar yMin=Math::Constants<Scalar>::max;
				Scalar yMax=Math::Constants<Scalar>::min;
				bool empty=true;
				for(MFFloat::ValueList::const_iterator hIt=height.getValues().begin();hIt!=height.getValues().end();++hIt)
					if(*hIt!=invalidHeight.getValue())
						{
						Scalar h=*hIt*heightScale.getValue();
						if(yMin>h)
							yMin=h;
						if(yMax<h)
							yMax=h;
						empty=false;
						}
				if(empty)
					result=Box::empty;
				else if(heightIsY.getValue())
					result=Box(origin.getValue()+Vector(0,yMin,0),origin.getValue()+Vector(Scalar(xDimension.getValue()-1)*xSpacing.getValue(),yMax,Scalar(zDimension.getValue()-1)*zSpacing.getValue()));
				else
					result=Box(origin.getValue()+Vector(0,0,yMin),origin.getValue()+Vector(Scalar(xDimension.getValue()-1)*xSpacing.getValue(),Scalar(zDimension.getValue()-1)*zSpacing.getValue(),yMax));
				}
			else
				{
				MFFloat::ValueList::const_iterator hIt=height.getValues().begin();
				Scalar yMin,yMax;
				yMin=yMax=*hIt*heightScale.getValue();
				for(++hIt;hIt!=height.getValues().end();++hIt)
					{
					Scalar h=*hIt*heightScale.getValue();
					if(yMin>h)
						yMin=h;
					if(yMax<h)
						yMax=h;
					}
				if(heightIsY.getValue())
					result=Box(origin.getValue()+Vector(0,yMin,0),origin.getValue()+Vector(Scalar(xDimension.getValue()-1)*xSpacing.getValue(),yMax,Scalar(zDimension.getValue()-1)*zSpacing.getValue()));
				else
					result=Box(origin.getValue()+Vector(0,0,yMin),origin.getValue()+Vector(Scalar(xDimension.getValue()-1)*xSpacing.getValue(),Scalar(zDimension.getValue()-1)*zSpacing.getValue(),yMax));
				}
			}
		}
	
	return result;
	}

void ElevationGridNode::glRenderAction(GLRenderState& renderState) const
	{
	/* Bail out if the elevation grid is invalid: */
	if(!valid)
		return;
	
	/* Set up OpenGL state: */
	if(solid.getValue())
		renderState.enableCulling(GL_BACK);
	else
		renderState.disableCulling();
	
	/* Get the context data item: */
	DataItem* dataItem=renderState.contextData.retrieveDataItem<DataItem>(this);
	
	typedef GLGeometry::Vertex<Scalar,2,GLubyte,4,Scalar,Scalar,3> Vertex;
	
	/* Bind the vertex buffer object: */
	glBindBufferARB(GL_ARRAY_BUFFER_ARB,dataItem->vertexBufferObjectId);
	
	/* Set up the vertex arrays: */
	int vertexArrayParts=Vertex::getPartsMask();
	if(color.getValue()==0&&colorMap.getValue()==0)
		{
		/* Disable the color vertex array: */
		vertexArrayParts&=~GLVertexArrayParts::Color;
		}
	GLVertexArrayParts::enable(vertexArrayParts);
	glVertexPointer(static_cast<Vertex*>(0));
	
	if(indexed)
		{
		/* Bind the index buffer object: */
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,dataItem->indexBufferObjectId);
		
		/* Check if the buffers are current: */
		if(dataItem->version!=version)
			{
			/* Upload the set of indexed quad strips: */
			uploadIndexedQuadStripSet();
			
			/* Mark the buffers as up-to-date: */
			dataItem->version=version;
			}
		
		/* Draw the elevation grid as a set of indexed quad strips: */
		const GLuint* iPtr=0;
		for(int z=0;z<zDimension.getValue()-1;++z,iPtr+=xDimension.getValue()*2)
			glDrawElements(GL_QUAD_STRIP,xDimension.getValue()*2,GL_UNSIGNED_INT,iPtr);
		
		/* Protect the index buffer object: */
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,0);
		}
	else
		{
		/* Check if the buffer is current: */
		if(dataItem->version!=version)
			{
			if(haveInvalids)
				{
				/* Upload the set of quads and triangles: */
				uploadHoleyQuadTriangleSet(dataItem->numQuads,dataItem->numTriangles);
				}
			else
				{
				/* Upload the set of quads: */
				uploadQuadSet();
				
				/* Store the number of uploaded quads: */
				dataItem->numQuads=(xDimension.getValue()-1)*(zDimension.getValue()-1);
				}
			
			/* Mark the buffers as up-to-date: */
			dataItem->version=version;
			}
		
		/* Draw the elevation grid as a set of quads and/or triangles: */
		if(dataItem->numQuads!=0)
			glDrawArrays(GL_QUADS,0,dataItem->numQuads*4);
		if(dataItem->numTriangles!=0)
			glDrawArrays(GL_TRIANGLES,dataItem->numQuads*4,dataItem->numTriangles*3);
		}
	
	/* Reset the vertex arrays: */
	GLVertexArrayParts::disable(vertexArrayParts);
	
	/* Protect the vertex buffer object: */
	glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
	}

void ElevationGridNode::initContext(GLContextData& contextData) const
	{
	/* Create a data item and store it in the context: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	}

}
