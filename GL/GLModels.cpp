/***********************************************************************
GLModels - Helper functions to render simple models using OpenGL.
Copyright (c) 2004-2013 Oliver Kreylos

This file is part of the OpenGL Support Library (GLSupport).

The OpenGL Support Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The OpenGL Support Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the OpenGL Support Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <math.h>

#include <GL/GLModels.h>

void glDrawCube(GLfloat size)
	{
	GLfloat s=0.5f*size;
	
	glBegin(GL_QUADS);
	glNormal3f(-1.0f,0.0f,0.0f);
	glVertex3f(-s,-s,-s);
	glVertex3f(-s,-s, s);
	glVertex3f(-s, s, s);
	glVertex3f(-s, s,-s);
	glNormal3f(1.0f,0.0f,0.0f);
	glVertex3f( s,-s,-s);
	glVertex3f( s, s,-s);
	glVertex3f( s, s, s);
	glVertex3f( s,-s, s);
	glNormal3f(0.0f,-1.0f,0.0f);
	glVertex3f(-s,-s,-s);
	glVertex3f( s,-s,-s);
	glVertex3f( s,-s, s);
	glVertex3f(-s,-s, s);
	glNormal3f(0.0f,1.0f,0.0f);
	glVertex3f(-s, s,-s);
	glVertex3f(-s, s, s);
	glVertex3f( s, s, s);
	glVertex3f( s, s,-s);
	glNormal3f(0.0f,0.0f,-1.0f);
	glVertex3f(-s,-s,-s);
	glVertex3f(-s, s,-s);
	glVertex3f( s, s,-s);
	glVertex3f( s,-s,-s);
	glNormal3f(0.0f,0.0f,1.0f);
	glVertex3f(-s,-s, s);
	glVertex3f( s,-s, s);
	glVertex3f( s, s, s);
	glVertex3f(-s, s, s);
	glEnd();
	}

void glDrawBox(const GLfloat min[3],const GLfloat max[3])
	{
	glBegin(GL_QUADS);
	glNormal3f(-1.0f,0.0f,0.0f);
	glVertex3f(min[0],min[1],min[2]);
	glVertex3f(min[0],min[1],max[2]);
	glVertex3f(min[0],max[1],max[2]);
	glVertex3f(min[0],max[1],min[2]);
	glNormal3f(1.0f,0.0f,0.0f);
	glVertex3f(max[0],min[1],min[2]);
	glVertex3f(max[0],max[1],min[2]);
	glVertex3f(max[0],max[1],max[2]);
	glVertex3f(max[0],min[1],max[2]);
	glNormal3f(0.0f,-1.0f,0.0f);
	glVertex3f(min[0],min[1],min[2]);
	glVertex3f(max[0],min[1],min[2]);
	glVertex3f(max[0],min[1],max[2]);
	glVertex3f(min[0],min[1],max[2]);
	glNormal3f(0.0f,1.0f,0.0f);
	glVertex3f(min[0],max[1],min[2]);
	glVertex3f(min[0],max[1],max[2]);
	glVertex3f(max[0],max[1],max[2]);
	glVertex3f(max[0],max[1],min[2]);
	glNormal3f(0.0f,0.0f,-1.0f);
	glVertex3f(min[0],min[1],min[2]);
	glVertex3f(min[0],max[1],min[2]);
	glVertex3f(max[0],max[1],min[2]);
	glVertex3f(max[0],min[1],min[2]);
	glNormal3f(0.0f,0.0f,1.0f);
	glVertex3f(min[0],min[1],max[2]);
	glVertex3f(max[0],min[1],max[2]);
	glVertex3f(max[0],max[1],max[2]);
	glVertex3f(min[0],max[1],max[2]);
	glEnd();
	}

void glDrawSphereMercator(GLfloat radius,GLsizei numStrips,GLsizei numQuads)
	{
	const GLfloat pi=GLfloat(M_PI);
	
	GLfloat lat1=1.0f*pi/GLfloat(numStrips)-0.5f*pi;
	GLfloat r1=cosf(lat1);
	GLfloat z1=sinf(lat1);
	
	/* Draw "southern polar cap": */
	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(0.0f,0.0f,-1.0f);
	glVertex3f(0.0f,0.0f,-radius);
	for(int j=numQuads;j>=0;--j)
		{
		GLfloat lng=GLfloat(j)*(2.0f*pi)/GLfloat(numQuads);
		GLfloat x1=cosf(lng)*r1;
		GLfloat y1=sinf(lng)*r1;
		glNormal3f(x1,y1,z1);
		glVertex3f(x1*radius,y1*radius,z1*radius);
		}
	glEnd();
	
	/* Draw quad strips: */
	for(int i=2;i<numStrips;++i)
		{
		GLfloat r0=r1;
		GLfloat z0=z1;
		lat1=GLfloat(i)*pi/GLfloat(numStrips)-0.5f*pi;
		r1=cosf(lat1);
		z1=sinf(lat1);
		
		glBegin(GL_QUAD_STRIP);
		for(int j=0;j<=numQuads;++j)
			{
			GLfloat lng=GLfloat(j)*(2.0f*pi)/GLfloat(numQuads);
			GLfloat x1=cosf(lng)*r1;
			GLfloat y1=sinf(lng)*r1;
			glNormal3f(x1,y1,z1);
			glVertex3f(x1*radius,y1*radius,z1*radius);
			GLfloat x0=cosf(lng)*r0;
			GLfloat y0=sinf(lng)*r0;
			glNormal3f(x0,y0,z0);
			glVertex3f(x0*radius,y0*radius,z0*radius);
			}
		glEnd();
		}
	
	/* Draw "northern polar cap": */
	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(0.0f,0.0f,1.0f);
	glVertex3f(0.0f,0.0f,radius);
	for(int j=0;j<=numQuads;++j)
		{
		GLfloat lng=GLfloat(j)*(2.0f*pi)/GLfloat(numQuads);
		GLfloat x1=cosf(lng)*r1;
		GLfloat y1=sinf(lng)*r1;
		glNormal3f(x1,y1,z1);
		glVertex3f(x1*radius,y1*radius,z1*radius);
		}
	glEnd();
	}

void glDrawSphereMercatorWithTexture(GLfloat radius,GLsizei numStrips,GLsizei numQuads)
	{
	const GLfloat pi=GLfloat(M_PI);
	
	GLfloat texY1=1.0f/GLfloat(numStrips);
	GLfloat lat1=1.0f*pi/GLfloat(numStrips)-0.5f*pi;
	GLfloat r1=cosf(lat1);
	GLfloat z1=sinf(lat1);
	
	/* Draw "southern polar cap": */
	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(0.0f,0.0f,-1.0f);
	glTexCoord2f(0.5f,0.0f);
	glVertex3f(0.0f,0.0f,-radius);
	for(int j=numQuads;j>=0;--j)
		{
		GLfloat texX=GLfloat(j)/GLfloat(numQuads);
		GLfloat lng=GLfloat(j)*(2.0f*pi)/GLfloat(numQuads);
		GLfloat x1=cosf(lng)*r1;
		GLfloat y1=sinf(lng)*r1;
		glNormal3f(x1,y1,z1);
		glTexCoord2f(texX,texY1);
		glVertex3f(x1*radius,y1*radius,z1*radius);
		}
	glEnd();
	
	/* Draw quad strips: */
	for(int i=2;i<numStrips;++i)
		{
		GLfloat r0=r1;
		GLfloat z0=z1;
		GLfloat texY0=texY1;
		texY1=GLfloat(i)/GLfloat(numStrips);
		lat1=GLfloat(i)*pi/GLfloat(numStrips)-0.5f*pi;
		r1=cosf(lat1);
		z1=sinf(lat1);
		
		glBegin(GL_QUAD_STRIP);
		for(int j=0;j<=numQuads;++j)
			{
			GLfloat texX=GLfloat(j)/GLfloat(numQuads);
			GLfloat lng=GLfloat(j)*(2.0f*pi)/GLfloat(numQuads);
			GLfloat x1=cosf(lng)*r1;
			GLfloat y1=sinf(lng)*r1;
			glNormal3f(x1,y1,z1);
			glTexCoord2f(texX,texY1);
			glVertex3f(x1*radius,y1*radius,z1*radius);
			GLfloat x0=cosf(lng)*r0;
			GLfloat y0=sinf(lng)*r0;
			glNormal3f(x0,y0,z0);
			glTexCoord2f(texX,texY0);
			glVertex3f(x0*radius,y0*radius,z0*radius);
			}
		glEnd();
		}
	
	/* Draw "northern polar cap": */
	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(0.0f,0.0f,1.0f);
	glTexCoord2f(0.5f,1.0f);
	glVertex3f(0.0f,0.0f,radius);
	for(int j=0;j<=numQuads;++j)
		{
		GLfloat texX=GLfloat(j)/GLfloat(numQuads);
		GLfloat lng=GLfloat(j)*(2.0f*pi)/GLfloat(numQuads);
		GLfloat x1=cosf(lng)*r1;
		GLfloat y1=sinf(lng)*r1;
		glNormal3f(x1,y1,z1);
		glTexCoord2f(texX,texY1);
		glVertex3f(x1*radius,y1*radius,z1*radius);
		}
	glEnd();
	}

inline void combine(const GLfloat p100[3],const GLfloat p010[3],const GLfloat p001[3],GLfloat w0,GLfloat w1,GLfloat radius)
	{
	GLfloat w2=1.0f-w0-w1;
	GLfloat result[3];
	GLfloat resultLen=0.0f;
	for(int i=0;i<3;++i)
		{
		result[i]=p100[i]*w0+p010[i]*w1+p001[i]*w2;
		resultLen+=result[i]*result[i];
		}
	resultLen=sqrtf(resultLen);
	for(int i=0;i<3;++i)
		result[i]/=resultLen;
	glNormal3fv(result);
	for(int i=0;i<3;++i)
		result[i]*=radius;
	glVertex3fv(result);
	}

inline void combine(const GLfloat p00[3],const GLfloat p10[3],const GLfloat p01[3],const GLfloat p11[3],GLfloat wx,GLfloat wy,GLfloat radius)
	{
	GLfloat result[3];
	GLfloat resultLen=0.0f;
	if(wx>wy)
		{
		for(int i=0;i<3;++i)
			{
			result[i]=p00[i]*(1.0f-wx)+p11[i]*wy+p10[i]*(wx-wy);
			resultLen+=result[i]*result[i];
			}
		}
	else
		{
		for(int i=0;i<3;++i)
			{
			result[i]=p11[i]*wx+p00[i]*(1.0f-wy)+p01[i]*(wy-wx);
			resultLen+=result[i]*result[i];
			}
		}
	resultLen=sqrtf(resultLen);
	for(int i=0;i<3;++i)
		result[i]/=resultLen;
	glNormal3fv(result);
	for(int i=0;i<3;++i)
		result[i]*=radius;
	glVertex3fv(result);
	}

void glDrawSphereIcosahedron(GLfloat radius,GLsizei numStrips)
	{
	/* Construct static icosahedron model: */
	const GLfloat b0=0.525731112119133606f; // b0=sqrt((5.0-sqrt(5.0))/10);
	const GLfloat b1=0.850650808352039932f; // b1=sqrt((5.0+sqrt(5.0))/10);
	static const GLfloat vUnit[12][3]={{-b0,0.0f, b1},{ b0,0.0f, b1},{-b0,0.0f,-b1},{ b0,0.0f,-b1},
	                                   {0.0f, b1, b0},{0.0f, b1,-b0},{0.0f,-b1, b0},{0.0f,-b1,-b0},
	                                   { b1, b0,0.0f},{-b1, b0,0.0f},{ b1,-b0,0.0f},{-b1,-b0,0.0f}};
	static const int stripIndices[12]={0,1,4,8,5,3,2,7,11,6,0,1};
	static const int fanIndices[2][7]={{9,0,4,5,2,11,0},{10,1,6,7,3,8,1}};
	
	/* Render the central triangle strips: */
	for(int strip=0;strip<numStrips;++strip)
		{
		GLfloat botW=GLfloat(strip)/GLfloat(numStrips);
		GLfloat topW=GLfloat(strip+1)/GLfloat(numStrips);
		glBegin(GL_TRIANGLE_STRIP);
		for(int i=0;i<10;i+=2)
			{
			const GLfloat* p00=vUnit[stripIndices[i+1]];
			const GLfloat* p10=vUnit[stripIndices[i+3]];
			const GLfloat* p01=vUnit[stripIndices[i+0]];
			const GLfloat* p11=vUnit[stripIndices[i+2]];
			for(int j=0;j<numStrips;++j)
				{
				GLfloat leftW=GLfloat(j)/GLfloat(numStrips);
				// GLfloat rightW=GLfloat(j+1)/GLfloat(numStrips);
				combine(p00,p10,p01,p11,leftW,topW,radius);
				combine(p00,p10,p01,p11,leftW,botW,radius);
				}
			combine(p00,p10,p01,p11,1.0f,topW,radius);
			combine(p00,p10,p01,p11,1.0f,botW,radius);
			}
		glEnd();
		}
	
	for(int cap=0;cap<2;++cap)
		{
		/* Render the cap triangle strips: */
		for(int strip=0;strip<numStrips-1;++strip)
			{
			GLfloat botW=GLfloat(strip)/GLfloat(numStrips);
			GLfloat topW=GLfloat(strip+1)/GLfloat(numStrips);
			glBegin(GL_TRIANGLE_STRIP);
			combine(vUnit[fanIndices[cap][0]],vUnit[fanIndices[cap][2]],vUnit[fanIndices[cap][1]],topW,0.0f,radius);
			for(int i=1;i<6;++i)
				{
				const GLfloat* p100=vUnit[fanIndices[cap][0]];
				const GLfloat* p010=vUnit[fanIndices[cap][i]];
				const GLfloat* p001=vUnit[fanIndices[cap][i+1]];
				for(int j=0;j<numStrips-strip;++j)
					{
					GLfloat leftW=GLfloat(j)/GLfloat(numStrips);
					combine(p100,p001,p010,botW,leftW,radius);
					combine(p100,p001,p010,topW,leftW,radius);
					}
				}
			combine(vUnit[fanIndices[cap][0]],vUnit[fanIndices[cap][2]],vUnit[fanIndices[cap][1]],botW,0.0f,radius);
			glEnd();
			}
		
		/* Render the cap triangle fan: */
		glBegin(GL_TRIANGLE_FAN);
		combine(vUnit[fanIndices[cap][0]],vUnit[fanIndices[cap][2]],vUnit[fanIndices[cap][1]],1.0f,0.0f,radius);
		GLfloat botW=GLfloat(numStrips-1)/GLfloat(numStrips);
		for(int i=1;i<6;++i)
			combine(vUnit[fanIndices[cap][0]],vUnit[fanIndices[cap][i+1]],vUnit[fanIndices[cap][i]],botW,0.0f,radius);
		combine(vUnit[fanIndices[cap][0]],vUnit[fanIndices[cap][2]],vUnit[fanIndices[cap][1]],botW,0.0f,radius);
		glEnd();
		}
	}

void glDrawCylinder(GLfloat radius,GLfloat height,GLsizei numStrips)
	{
	const GLfloat pi=GLfloat(M_PI);
	
	GLfloat h=0.5f*height;
	
	/* Draw bottom circle: */
	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(0.0f,0.0f,-1.0f);
	glVertex3f(0.0f,0.0f,-h);
	for(int j=numStrips;j>=0;--j)
		{
		GLfloat lng=GLfloat(j)*(2.0f*pi)/GLfloat(numStrips);
		GLfloat x=cosf(lng);
		GLfloat y=sinf(lng);
		glVertex3f(x*radius,y*radius,-h);
		}
	glEnd();
	
	/* Draw mantle: */
	glBegin(GL_QUAD_STRIP);
	for(int j=0;j<=numStrips;++j)
		{
		GLfloat lng=GLfloat(j)*(2.0f*pi)/GLfloat(numStrips);
		GLfloat x=cosf(lng);
		GLfloat y=sinf(lng);
		glNormal3f(x,y,0.0f);
		glVertex3f(x*radius,y*radius,h);
		glVertex3f(x*radius,y*radius,-h);
		}
	glEnd();
	
	/* Draw top circle: */
	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(0.0f,0.0f,1.0f);
	glVertex3f(0.0f,0.0f,h);
	for(int j=0;j<=numStrips;++j)
		{
		GLfloat lng=GLfloat(j)*(2.0f*pi)/GLfloat(numStrips);
		GLfloat x=cosf(lng);
		GLfloat y=sinf(lng);
		glVertex3f(x*radius,y*radius,h);
		}
	glEnd();
	}

void glDrawCone(GLfloat radius,GLfloat height,GLsizei numStrips)
	{
	const GLfloat pi=GLfloat(M_PI);
	
	GLfloat z0=-0.25f*height;
	GLfloat z1=0.75f*height;
	GLfloat zn=radius/height;
	GLfloat nl=sqrtf(1.0f+zn*zn);
	GLfloat rn=1.0f/nl;
	zn*=rn;
	
	/* Draw bottom circle: */
	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(0.0f,0.0f,-1.0f);
	glVertex3f(0.0f,0.0f,z0);
	for(int j=numStrips;j>=0;--j)
		{
		GLfloat lng=GLfloat(j)*(2.0f*pi)/GLfloat(numStrips);
		GLfloat x=cosf(lng);
		GLfloat y=sinf(lng);
		glVertex3f(x*radius,y*radius,z0);
		}
	glEnd();
	
	/* Draw mantle: */
	glBegin(GL_QUAD_STRIP);
	for(int j=0;j<=numStrips;++j)
		{
		GLfloat lng=GLfloat(j)*(2.0f*pi)/GLfloat(numStrips);
		GLfloat x=cosf(lng);
		GLfloat y=sinf(lng);
		glNormal3f(x*rn,y*rn,zn);
		glVertex3f(0.0f,0.0f,z1);
		glVertex3f(x*radius,y*radius,z0);
		}
	glEnd();
	}

static void glDrawBox(const GLfloat center[3],const GLfloat halfSize[3],int sideMask)
	{
	static const GLfloat vertices[8][3]={{-1.0f,-1.0f,-1.0f},{ 1.0f,-1.0f,-1.0f},{-1.0f, 1.0f,-1.0f},{ 1.0f, 1.0f,-1.0f},
	                                     {-1.0f,-1.0f, 1.0f},{ 1.0f,-1.0f, 1.0f},{-1.0f, 1.0f, 1.0f},{ 1.0f, 1.0f, 1.0f}};
	static const int sides[6][4]={{0,4,6,2},{1,3,7,5},{0,1,5,4},{2,6,7,3},{0,2,3,1},{4,5,7,6}};
	static const GLfloat normals[6][3]={{-1.0f, 0.0f, 0.0f},{ 1.0f, 0.0f, 0.0f},{ 0.0f,-1.0f, 0.0f},
	                                    { 0.0f, 1.0f, 0.0f},{ 0.0f, 0.0f,-1.0f},{ 0.0f, 0.0f, 1.0f}};
	
	for(int side=0;side<6;++side)
		if(sideMask&(1<<side))
			{
			glNormal3fv(normals[side]);
			for(int i=0;i<4;++i)
				{
				const GLfloat* v=vertices[sides[side][i]];
				glVertex3f(center[0]+v[0]*halfSize[0],center[1]+v[1]*halfSize[1],center[2]+v[2]*halfSize[2]);
				}
			}
	}

void glDrawWireframeCube(GLfloat cubeSize,GLfloat edgeSize,GLfloat vertexSize)
	{
	GLfloat cs=cubeSize*0.5f;
	GLfloat es=edgeSize*0.5f;
	GLfloat vs=vertexSize*0.5f;
	GLfloat halfSize[3];
	GLfloat center[3];
	
	glBegin(GL_QUADS);
	
	/* Render box vertices: */
	halfSize[0]=halfSize[1]=halfSize[2]=vs;
	for(int vertex=0;vertex<8;++vertex)
		{
		for(int i=0;i<3;++i)
			center[i]=vertex&(1<<i)?cs:-cs;
		glDrawBox(center,halfSize,0x3f);
		}
	
	/* Render box edges: */
	for(int dim=0;dim<3;++dim)
		{
		halfSize[0]=halfSize[1]=halfSize[2]=es;
		halfSize[dim]=cs-vs;
		for(int edge=0;edge<4;++edge)
			{
			center[dim]=0.0f;
			for(int i=0;i<2;++i)
				center[(i+dim+1)%3]=edge&(1<<i)?cs:-cs;
			glDrawBox(center,halfSize,0x3f&~(0x3<<(dim*2)));
			}
		}
	
	glEnd();
	}

void glDrawArrow(GLfloat shaftRadius,GLfloat tipRadius,GLfloat tipHeight,GLfloat totalHeight,GLsizei numStrips)
	{
	const GLfloat toRad=GLfloat(2.0*M_PI)/GLfloat(numStrips);
	
	GLfloat z0=-0.5f*totalHeight;
	GLfloat z1=0.5f*totalHeight-tipHeight;
	GLfloat z2=0.5f*totalHeight;
	
	/* Draw bottom circle: */
	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(0.0f,0.0f,-1.0f);
	glVertex3f(0.0f,0.0f,z0);
	for(int j=numStrips;j>=0;--j)
		{
		GLfloat lng=GLfloat(j)*toRad;
		GLfloat x=cosf(lng);
		GLfloat y=sinf(lng);
		glVertex3f(x*shaftRadius,y*shaftRadius,z0);
		}
	glEnd();
	
	/* Draw shaft: */
	glBegin(GL_QUAD_STRIP);
	for(int j=0;j<=numStrips;++j)
		{
		GLfloat lng=GLfloat(j)*toRad;
		GLfloat x=cosf(lng);
		GLfloat y=sinf(lng);
		glNormal3f(x,y,0.0f);
		glVertex3f(x*shaftRadius,y*shaftRadius,z1);
		glVertex3f(x*shaftRadius,y*shaftRadius,z0);
		}
	glEnd();
	
	/* Draw tip bottom: */
	glBegin(GL_QUAD_STRIP);
	glNormal3f(0.0f,0.0f,-1.0f);
	for(int j=0;j<=numStrips;++j)
		{
		GLfloat lng=GLfloat(j)*toRad;
		GLfloat x=cosf(lng);
		GLfloat y=sinf(lng);
		glVertex3f(x*tipRadius,y*tipRadius,z1);
		glVertex3f(x*shaftRadius,y*shaftRadius,z1);
		}
	glEnd();
	
	/* Draw tip: */
	GLfloat zn=tipRadius/tipHeight;
	GLfloat nl=sqrtf(1.0f+zn*zn);
	GLfloat rn=1.0f/nl;
	zn*=rn;
	glBegin(GL_QUAD_STRIP);
	for(int j=0;j<=numStrips;++j)
		{
		GLfloat lng=GLfloat(j)*toRad;
		GLfloat x=cosf(lng);
		GLfloat y=sinf(lng);
		glNormal3f(x*rn,y*rn,zn);
		glVertex3f(0.0f,0.0f,z2);
		glVertex3f(x*tipRadius,y*tipRadius,z1);
		}
	glEnd();
	}
