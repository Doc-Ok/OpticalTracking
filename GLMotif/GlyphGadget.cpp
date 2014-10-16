/***********************************************************************
GlyphGadget - Helper class to render assorted glyphs as part of other
widgets.
Copyright (c) 2010 Oliver Kreylos

This file is part of the GLMotif Widget Library (GLMotif).

The GLMotif Widget Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GLMotif Widget Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the GLMotif Widget Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <GLMotif/GlyphGadget.h>

#include <GL/GLColorTemplates.h>
#include <GL/GLNormalTemplates.h>
#include <GL/GLVertexTemplates.h>

namespace GLMotif {

/****************************
Methods of class GlyphGadget:
****************************/

void GlyphGadget::layoutGlyph(void)
	{
	for(int i=0;i<4;++i)
		glyphVertices[i]=glyphBox.getCorner(i);
	Vector center;
	for(int i=0;i<2;++i)
		center[i]=glyphBox.origin[i]+glyphBox.size[i]*0.5f;
	center[2]=glyphBox.origin[2];
	for(int i=4;i<numVertices;++i)
		glyphVertices[i]=center;
	
	int rotate=0;
	switch(glyphType)
		{
		case NONE:
			break;
		
		case SQUARE:
			{
			glyphVertices[4][0]-=glyphSize+bevelSize;
			glyphVertices[4][1]-=glyphSize+bevelSize;
			glyphVertices[5][0]+=glyphSize+bevelSize;
			glyphVertices[5][1]-=glyphSize+bevelSize;
			glyphVertices[6][0]+=glyphSize+bevelSize;
			glyphVertices[6][1]+=glyphSize+bevelSize;
			glyphVertices[7][0]-=glyphSize+bevelSize;
			glyphVertices[7][1]+=glyphSize+bevelSize;
			
			glyphVertices[8][0]-=glyphSize;
			glyphVertices[8][1]-=glyphSize;
			glyphVertices[9][0]+=glyphSize;
			glyphVertices[9][1]-=glyphSize;
			glyphVertices[10][0]+=glyphSize;
			glyphVertices[10][1]+=glyphSize;
			glyphVertices[11][0]-=glyphSize;
			glyphVertices[11][1]+=glyphSize;
			
			glyphNormals[0]=Vector( 0.0000f, 0.7071f, 0.7071f);
			glyphNormals[1]=Vector(-0.7071f, 0.0000f, 0.7071f);
			glyphNormals[2]=Vector( 0.0000f,-0.7071f, 0.7071f);
			glyphNormals[3]=Vector( 0.7071f, 0.0000f, 0.7071f);
			
			break;
			}
		
		case DIAMOND:
			{
			GLfloat bs=bevelSize*1.414214f;
			
			glyphVertices[4][1]-=glyphSize+bs;
			glyphVertices[5][0]+=glyphSize+bs;
			glyphVertices[6][1]+=glyphSize+bs;
			glyphVertices[7][0]-=glyphSize+bs;
			
			glyphVertices[8][1]-=glyphSize;
			glyphVertices[9][0]+=glyphSize;
			glyphVertices[10][1]+=glyphSize;
			glyphVertices[11][0]-=glyphSize;
			
			glyphNormals[0]=Vector(-0.5000f, 0.5000f, 0.7071f);
			glyphNormals[1]=Vector(-0.5000f,-0.5000f, 0.7071f);
			glyphNormals[2]=Vector( 0.5000f,-0.5000f, 0.7071f);
			glyphNormals[3]=Vector( 0.5000f, 0.5000f, 0.7071f);
			
			break;
			}
		
		case LOW_BAR:
			{
			glyphVertices[4][0]-=glyphSize*2.0f+bevelSize;
			glyphVertices[4][1]-=glyphSize*2.0f+bevelSize;
			glyphVertices[5][0]+=glyphSize*2.0f+bevelSize;
			glyphVertices[5][1]-=glyphSize*2.0f+bevelSize;
			glyphVertices[6][0]+=glyphSize*2.0f+bevelSize;
			glyphVertices[6][1]-=glyphSize-bevelSize;
			glyphVertices[7][0]-=glyphSize*2.0f+bevelSize;
			glyphVertices[7][1]-=glyphSize-bevelSize;
			
			glyphVertices[8][0]-=glyphSize*2.0f;
			glyphVertices[8][1]-=glyphSize*2.0f;
			glyphVertices[9][0]+=glyphSize*2.0f;
			glyphVertices[9][1]-=glyphSize*2.0f;
			glyphVertices[10][0]+=glyphSize*2.0f;
			glyphVertices[10][1]-=glyphSize;
			glyphVertices[11][0]-=glyphSize*2.0f;
			glyphVertices[11][1]-=glyphSize;
			
			glyphNormals[0]=Vector( 0.0000f, 0.7071f, 0.7071f);
			glyphNormals[1]=Vector(-0.7071f, 0.0000f, 0.7071f);
			glyphNormals[2]=Vector( 0.0000f,-0.7071f, 0.7071f);
			glyphNormals[3]=Vector( 0.7071f, 0.0000f, 0.7071f);
			
			break;
			}
		
		case CROSS:
			{
			GLfloat gs=glyphSize*0.707107f;
			GLfloat bs=bevelSize*1.414214f;
			
			glyphVertices[4][0]-=gs*1.5f;
			glyphVertices[4][1]-=gs*2.5f+bs;
			glyphVertices[5][1]-=gs+bs;
			glyphVertices[6][0]+=gs*1.5f;
			glyphVertices[6][1]-=gs*2.5f+bs;
			glyphVertices[7][0]+=gs*2.5f+bs;
			glyphVertices[7][1]-=gs*1.5f;
			glyphVertices[8][0]+=gs+bs;
			glyphVertices[9][0]+=gs*2.5f+bs;
			glyphVertices[9][1]+=gs*1.5f;
			glyphVertices[10][0]+=gs*1.5f;
			glyphVertices[10][1]+=gs*2.5f+bs;
			glyphVertices[11][1]+=gs+bs;
			glyphVertices[12][0]-=gs*1.5f;
			glyphVertices[12][1]+=gs*2.5f+bs;
			glyphVertices[13][0]-=gs*2.5f+bs;
			glyphVertices[13][1]+=gs*1.5f;
			glyphVertices[14][0]-=gs+bs;
			glyphVertices[15][0]-=gs*2.5f+bs;
			glyphVertices[15][1]-=gs*1.5f;
			
			glyphVertices[16][0]-=gs*1.5f;
			glyphVertices[16][1]-=gs*2.5f;
			glyphVertices[17][1]-=gs;
			glyphVertices[18][0]+=gs*1.5f;
			glyphVertices[18][1]-=gs*2.5f;
			glyphVertices[19][0]+=gs*2.5f;
			glyphVertices[19][1]-=gs*1.5f;
			glyphVertices[20][0]+=gs;
			glyphVertices[21][0]+=gs*2.5f;
			glyphVertices[21][1]+=gs*1.5f;
			glyphVertices[22][0]+=gs*1.5f;
			glyphVertices[22][1]+=gs*2.5f;
			glyphVertices[23][1]+=gs;
			glyphVertices[24][0]-=gs*1.5f;
			glyphVertices[24][1]+=gs*2.5f;
			glyphVertices[25][0]-=gs*2.5f;
			glyphVertices[25][1]+=gs*1.5f;
			glyphVertices[26][0]-=gs;
			glyphVertices[27][0]-=gs*2.5f;
			glyphVertices[27][1]-=gs*1.5f;
			
			glyphNormals[0]=Vector(-0.5000f, 0.5000f, 0.7071f);
			glyphNormals[1]=Vector( 0.5000f, 0.5000f, 0.7071f);
			glyphNormals[2]=glyphNormals[0];
			glyphNormals[3]=Vector(-0.5000f,-0.5000f, 0.7071f);
			glyphNormals[4]=glyphNormals[2];
			glyphNormals[5]=glyphNormals[3];
			glyphNormals[6]=Vector( 0.5000f,-0.5000f, 0.7071f);
			glyphNormals[7]=glyphNormals[5];
			glyphNormals[8]=glyphNormals[6];
			glyphNormals[9]=glyphNormals[1];
			glyphNormals[10]=glyphNormals[8];
			glyphNormals[11]=glyphNormals[9];
			
			break;
			}
		
		case SIMPLE_ARROW_LEFT:
		case SIMPLE_ARROW_DOWN:
		case SIMPLE_ARROW_RIGHT:
		case SIMPLE_ARROW_UP:
			{
			/* Create a left arrow: */
			glyphVertices[4][0]-=glyphSize+bevelSize*1.736068f;
			glyphVertices[5][0]+=glyphSize+bevelSize*1.5f;
			glyphVertices[5][1]-=glyphSize+bevelSize*1.618034f;
			glyphVertices[6][0]+=glyphSize+bevelSize*1.5f;
			glyphVertices[6][1]+=glyphSize+bevelSize*1.618034f;
			
			glyphVertices[7][0]-=glyphSize-bevelSize*0.5f;
			glyphVertices[8][0]+=glyphSize+bevelSize*0.5f;
			glyphVertices[8][1]-=glyphSize;
			glyphVertices[9][0]+=glyphSize+bevelSize*0.5f;
			glyphVertices[9][1]+=glyphSize;
			
			glyphNormals[0]=Vector( 0.2236f, 0.4472f, 0.7071f);
			glyphNormals[1]=Vector(-0.7071f, 0.0000f, 0.7071f);
			glyphNormals[2]=Vector( 0.2236f,-0.4472f, 0.7071f);
			
			/* Set the glyph rotation: */
			rotate=int(glyphType)-int(SIMPLE_ARROW_LEFT);
			
			break;
			}
		
		case FANCY_ARROW_LEFT:
		case FANCY_ARROW_DOWN:
		case FANCY_ARROW_RIGHT:
		case FANCY_ARROW_UP:
			{
			/* Create a left arrow: */
			glyphVertices[ 4][0]-=glyphSize*2.0f+bevelSize*1.414214f;
			glyphVertices[ 5][0]+=bevelSize;
			glyphVertices[ 5][1]-=glyphSize*2.0f+bevelSize*2.414214f;
			glyphVertices[ 6][0]+=bevelSize;
			glyphVertices[ 6][1]-=glyphSize+bevelSize;
			glyphVertices[ 7][0]+=glyphSize*2.0f+bevelSize;
			glyphVertices[ 7][1]-=glyphSize+bevelSize;
			glyphVertices[ 8][0]+=glyphSize*2.0f+bevelSize;
			glyphVertices[ 8][1]+=glyphSize+bevelSize;
			glyphVertices[ 9][0]+=bevelSize;
			glyphVertices[ 9][1]+=glyphSize+bevelSize;
			glyphVertices[10][0]+=bevelSize;
			glyphVertices[10][1]+=glyphSize*2.0f+bevelSize*2.414214f;
			
			glyphVertices[11][0]-=glyphSize*2.0f;
			glyphVertices[12][1]-=glyphSize*2.0f;
			glyphVertices[13][1]-=glyphSize;
			glyphVertices[14][0]+=glyphSize*2.0f;
			glyphVertices[14][1]-=glyphSize;
			glyphVertices[15][0]+=glyphSize*2.0f;
			glyphVertices[15][1]+=glyphSize;
			glyphVertices[16][1]+=glyphSize;
			glyphVertices[17][1]+=glyphSize*2.0f;
			
			glyphNormals[0]=Vector( 0.5000f, 0.5000f, 0.7071f);
			glyphNormals[1]=Vector(-0.7071f, 0.0000f, 0.7071f);
			glyphNormals[2]=Vector( 0.0000f, 0.7071f, 0.7071f);
			glyphNormals[3]=glyphNormals[1];
			glyphNormals[4]=Vector( 0.0000f,-0.7071f, 0.7071f);
			glyphNormals[5]=glyphNormals[3];
			glyphNormals[6]=Vector( 0.5000f,-0.5000f, 0.7071f);
			
			/* Set the glyph rotation: */
			rotate=int(glyphType)-int(FANCY_ARROW_LEFT);
			
			break;
			}
		}
	
	/* Rotate the glyph: */
	switch(rotate)
		{
		case 1:
			{
			Vector t=glyphVertices[0];
			glyphVertices[0]=glyphVertices[1];
			glyphVertices[1]=glyphVertices[3];
			glyphVertices[3]=glyphVertices[2];
			glyphVertices[2]=t;
			
			for(int i=4;i<numVertices;++i)
				{
				GLfloat dx=glyphVertices[i][0]-center[0];
				GLfloat dy=glyphVertices[i][1]-center[1];
				glyphVertices[i][0]=center[0]-dy;
				glyphVertices[i][1]=center[1]+dx;
				}
			
			for(int i=0;i<numNormals;++i)
				{
				GLfloat t=glyphNormals[i][0];
				glyphNormals[i][0]=-glyphNormals[i][1];
				glyphNormals[i][1]=t;
				}
			
			break;
			}
		
		case 2:
			{
			Vector t=glyphVertices[0];
			glyphVertices[0]=glyphVertices[3];
			glyphVertices[3]=t;
			t=glyphVertices[1];
			glyphVertices[1]=glyphVertices[2];
			glyphVertices[2]=t;
			
			for(int i=4;i<numVertices;++i)
				{
				GLfloat dx=glyphVertices[i][0]-center[0];
				GLfloat dy=glyphVertices[i][1]-center[1];
				glyphVertices[i][0]=center[0]-dx;
				glyphVertices[i][1]=center[1]-dy;
				}
			
			for(int i=0;i<numNormals;++i)
				{
				glyphNormals[i][0]=-glyphNormals[i][0];
				glyphNormals[i][1]=-glyphNormals[i][1];
				}
			
			break;
			}
		
		case 3:
			{
			Vector t=glyphVertices[0];
			glyphVertices[0]=glyphVertices[2];
			glyphVertices[2]=glyphVertices[3];
			glyphVertices[3]=glyphVertices[1];
			glyphVertices[1]=t;
			
			for(int i=4;i<numVertices;++i)
				{
				GLfloat dx=glyphVertices[i][0]-center[0];
				GLfloat dy=glyphVertices[i][1]-center[1];
				glyphVertices[i][0]=center[0]+dy;
				glyphVertices[i][1]=center[1]-dx;
				}
			
			for(int i=0;i<numNormals;++i)
				{
				GLfloat t=glyphNormals[i][0];
				glyphNormals[i][0]=glyphNormals[i][1];
				glyphNormals[i][1]=-t;
				}
			
			break;
			}
		}
	
	/* Adjust the inner part of the glyph based on the glyph's depth: */
	switch(depth)
		{
		case IN:
			{
			for(int i=innerStart;i<numVertices;++i)
				glyphVertices[i][2]-=bevelSize;
			break;
			}
		
		case OUT:
			{
			for(int i=innerStart;i<numVertices;++i)
				glyphVertices[i][2]+=bevelSize;
			
			/* Flip normal vectors if the glyph is an outie: */
			for(int i=0;i<numNormals;++i)
				{
				glyphNormals[i][0]=-glyphNormals[i][0];
				glyphNormals[i][1]=-glyphNormals[i][1];
				}
			break;
			}
		}
	}

GlyphGadget::GlyphGadget(void)
	:glyphType(NONE),depth(IN),
	 glyphSize(0.0f),bevelSize(0.0f),
	 glyphBox(Box(Vector(0.0f,0.0f,0.0f),Vector(glyphSize*4.0f+bevelSize*4.0f,glyphSize*4.0f+bevelSize*4.0f,0.0f))),
	 glyphVertices(0),glyphNormals(0)
	{
	setGlyphType(glyphType);
	}

GlyphGadget::GlyphGadget(GlyphGadget::GlyphType sGlyphType,GlyphGadget::Depth sDepth,GLfloat sGlyphSize)
	:glyphType(sGlyphType),depth(sDepth),
	 glyphSize(sGlyphSize),bevelSize(glyphSize*0.5f),
	 glyphBox(Box(Vector(0.0f,0.0f,0.0f),Vector(glyphSize*4.0f+bevelSize*4.0f,glyphSize*4.0f+bevelSize*4.0f,0.0f))),
	 glyphVertices(0),glyphNormals(0)
	{
	setGlyphType(glyphType);
	}

GlyphGadget::~GlyphGadget(void)
	{
	delete[] glyphVertices;
	delete[] glyphNormals;
	}

GLfloat GlyphGadget::getPreferredBoxSize(void) const
	{
	GLfloat result=0.0f;
	
	switch(glyphType)
		{
		case NONE:
			break;
		
		case SQUARE:
		case DIAMOND:
			result=glyphSize*2.0f+bevelSize*4.0f;
			break;
		
		case LOW_BAR:
		case CROSS:
			result=glyphSize*4.0f+bevelSize*4.0f;
			break;
		
		case SIMPLE_ARROW_LEFT:
		case SIMPLE_ARROW_DOWN:
		case SIMPLE_ARROW_RIGHT:
		case SIMPLE_ARROW_UP:
			result=glyphSize*2.0f+bevelSize*4.0f;
			break;
		
		case FANCY_ARROW_LEFT:
		case FANCY_ARROW_DOWN:
		case FANCY_ARROW_RIGHT:
		case FANCY_ARROW_UP:
			result=glyphSize*4.0f+bevelSize*4.0f;
			break;
		}
	
	return result;
	}

ZRange GlyphGadget::calcZRange(void) const
	{
	ZRange result=ZRange(glyphBox.origin[2],glyphBox.origin[2]);
	if(depth==IN)
		result.first-=bevelSize;
	else
		result.second+=bevelSize;
	return result;
	}

void GlyphGadget::setGlyphType(GlyphGadget::GlyphType newGlyphType)
	{
	/* Release the old glyph cache: */
	delete[] glyphVertices;
	delete[] glyphNormals;
	
	/* Set the glyph type: */
	glyphType=newGlyphType;
	
	/* Allocate the new glyph cache: */
	switch(glyphType)
		{
		case NONE:
			innerStart=4;
			numVertices=4;
			numNormals=0;
			break;
		
		case SQUARE:
		case DIAMOND:
		case LOW_BAR:
			innerStart=8;
			numVertices=12;
			numNormals=4;
			break;
		
		case CROSS:
			innerStart=16;
			numVertices=28;
			numNormals=12;
			break;
		
		case SIMPLE_ARROW_LEFT:
		case SIMPLE_ARROW_DOWN:
		case SIMPLE_ARROW_RIGHT:
		case SIMPLE_ARROW_UP:
			innerStart=7;
			numVertices=10;
			numNormals=3;
			break;
		
		case FANCY_ARROW_LEFT:
		case FANCY_ARROW_DOWN:
		case FANCY_ARROW_RIGHT:
		case FANCY_ARROW_UP:
			innerStart=11;
			numVertices=18;
			numNormals=7;
			break;
		}
	glyphVertices=numVertices>0?new Vector[numVertices]:0;
	glyphNormals=numNormals>0?new Vector[numNormals]:0;
	
	/* Update the glyph cache: */
	layoutGlyph();
	}

void GlyphGadget::setDepth(GlyphGadget::Depth newDepth)
	{
	/* Set the new depth: */
	depth=newDepth;
	
	/* Update the glyph cache: */
	layoutGlyph();
	}

void GlyphGadget::setGlyphSize(GLfloat newGlyphSize)
	{
	/* Set the new glyph size: */
	glyphSize=newGlyphSize;
	
	/* Update the glyph cache: */
	layoutGlyph();
	}

void GlyphGadget::setBevelSize(GLfloat newBevelSize)
	{
	/* Set the new bevel size: */
	bevelSize=newBevelSize;
	
	/* Update the glyph cache: */
	layoutGlyph();
	}

void GlyphGadget::setGlyphBox(const Box& newGlyphBox)
	{
	/* Set the new glyph box: */
	glyphBox=newGlyphBox;
	
	/* Update the glyph cache: */
	layoutGlyph();
	}

void GlyphGadget::setGlyphColor(const Color& newGlyphColor)
	{
	/* Set the new glyph color: */
	glyphColor=newGlyphColor;
	}

void GlyphGadget::draw(GLContextData& contextData) const
	{
	/* Draw the margin around the glyph: */
	switch(glyphType)
		{
		case NONE:
			glBegin(GL_QUADS);
			glNormal3f(0.0f,0.0f,1.0f);
			glVertex(glyphVertices[0]);
			glVertex(glyphVertices[1]);
			glVertex(glyphVertices[3]);
			glVertex(glyphVertices[2]);
			glEnd();
			break;
		
		case SQUARE:
		case LOW_BAR:
			glBegin(GL_QUAD_STRIP);
			glNormal3f(0.0f,0.0f,1.0f);
			glVertex(glyphVertices[4]);
			glVertex(glyphVertices[0]);
			glVertex(glyphVertices[5]);
			glVertex(glyphVertices[1]);
			glVertex(glyphVertices[6]);
			glVertex(glyphVertices[3]);
			glVertex(glyphVertices[7]);
			glVertex(glyphVertices[2]);
			glVertex(glyphVertices[4]);
			glVertex(glyphVertices[0]);
			glEnd();
			break;
		
		case DIAMOND:
			glBegin(GL_TRIANGLE_FAN);
			glNormal3f(0.0f,0.0f,1.0f);
			glVertex(glyphVertices[0]);
			glVertex(glyphVertices[1]);
			glVertex(glyphVertices[4]);
			glVertex(glyphVertices[7]);
			glVertex(glyphVertices[2]);
			glEnd();
			glBegin(GL_TRIANGLE_FAN);
			glVertex(glyphVertices[3]);
			glVertex(glyphVertices[2]);
			glVertex(glyphVertices[6]);
			glVertex(glyphVertices[5]);
			glVertex(glyphVertices[1]);
			glEnd();
			glBegin(GL_TRIANGLES);
			glVertex(glyphVertices[1]);
			glVertex(glyphVertices[5]);
			glVertex(glyphVertices[4]);
			glVertex(glyphVertices[2]);
			glVertex(glyphVertices[7]);
			glVertex(glyphVertices[6]);
			glEnd();
			break;
		
		case CROSS:
			glBegin(GL_TRIANGLE_FAN);
			glNormal3f(0.0f,0.0f,1.0f);
			glVertex(glyphVertices[0]);
			glVertex(glyphVertices[1]);
			glVertex(glyphVertices[6]);
			glVertex(glyphVertices[4]);
			glVertex(glyphVertices[15]);
			glVertex(glyphVertices[13]);
			glVertex(glyphVertices[2]);
			glEnd();
			glBegin(GL_TRIANGLE_FAN);
			glVertex(glyphVertices[3]);
			glVertex(glyphVertices[2]);
			glVertex(glyphVertices[12]);
			glVertex(glyphVertices[10]);
			glVertex(glyphVertices[9]);
			glVertex(glyphVertices[7]);
			glVertex(glyphVertices[1]);
			glEnd();
			glBegin(GL_TRIANGLES);
			glVertex(glyphVertices[4]);
			glVertex(glyphVertices[6]);
			glVertex(glyphVertices[5]);
			glVertex(glyphVertices[6]);
			glVertex(glyphVertices[1]);
			glVertex(glyphVertices[7]);
			glVertex(glyphVertices[7]);
			glVertex(glyphVertices[9]);
			glVertex(glyphVertices[8]);
			glVertex(glyphVertices[10]);
			glVertex(glyphVertices[12]);
			glVertex(glyphVertices[11]);
			glVertex(glyphVertices[12]);
			glVertex(glyphVertices[2]);
			glVertex(glyphVertices[13]);
			glVertex(glyphVertices[13]);
			glVertex(glyphVertices[15]);
			glVertex(glyphVertices[14]);
			glEnd();
			break;
		
		case SIMPLE_ARROW_LEFT:
		case SIMPLE_ARROW_DOWN:
		case SIMPLE_ARROW_RIGHT:
		case SIMPLE_ARROW_UP:
			glBegin(GL_TRIANGLE_FAN);
			glNormal3f(0.0f,0.0f,1.0f);
			glVertex(glyphVertices[4]);
			glVertex(glyphVertices[6]);
			glVertex(glyphVertices[2]);
			glVertex(glyphVertices[0]);
			glVertex(glyphVertices[5]);
			glEnd();
			glBegin(GL_TRIANGLES);
			glVertex(glyphVertices[0]);
			glVertex(glyphVertices[1]);
			glVertex(glyphVertices[5]);
			glVertex(glyphVertices[3]);
			glVertex(glyphVertices[2]);
			glVertex(glyphVertices[6]);
			glEnd();
			glBegin(GL_QUADS);
			glVertex(glyphVertices[1]);
			glVertex(glyphVertices[3]);
			glVertex(glyphVertices[6]);
			glVertex(glyphVertices[5]);
			glEnd();
			break;
		
		case FANCY_ARROW_LEFT:
		case FANCY_ARROW_DOWN:
		case FANCY_ARROW_RIGHT:
		case FANCY_ARROW_UP:
			glBegin(GL_TRIANGLE_FAN);
			glNormal3f(0.0f,0.0f,1.0f);
			glVertex(glyphVertices[4]);
			glVertex(glyphVertices[10]);
			glVertex(glyphVertices[2]);
			glVertex(glyphVertices[0]);
			glVertex(glyphVertices[5]);
			glEnd();
			glBegin(GL_TRIANGLE_FAN);
			glVertex(glyphVertices[1]);
			glVertex(glyphVertices[8]);
			glVertex(glyphVertices[7]);
			glVertex(glyphVertices[6]);
			glVertex(glyphVertices[5]);
			glVertex(glyphVertices[0]);
			glEnd();
			glBegin(GL_TRIANGLE_FAN);
			glVertex(glyphVertices[3]);
			glVertex(glyphVertices[2]);
			glVertex(glyphVertices[10]);
			glVertex(glyphVertices[9]);
			glVertex(glyphVertices[8]);
			glVertex(glyphVertices[1]);
			glEnd();
			break;
		}
	
	/* Draw the glyph bevel: */
	glBegin(GL_QUADS);
	glColor(glyphColor);
	for(int i=0;i<numNormals;++i)
		{
		glNormal(glyphNormals[i]);
		int i1=(i+1)%numNormals;
		glVertex(glyphVertices[4+i]);
		glVertex(glyphVertices[4+i1]);
		glVertex(glyphVertices[innerStart+i1]);
		glVertex(glyphVertices[innerStart+i]);
		}
	glEnd();
	
	/* Draw the glyph face: */
	switch(glyphType)
		{
		case NONE:
			break;
		
		case SQUARE:
		case DIAMOND:
		case LOW_BAR:
			glBegin(GL_QUADS);
			glNormal3f(0.0f,0.0f,1.0f);
			glVertex(glyphVertices[8]);
			glVertex(glyphVertices[9]);
			glVertex(glyphVertices[10]);
			glVertex(glyphVertices[11]);
			glEnd();
			break;
		
		case CROSS:
			glBegin(GL_QUADS);
			glNormal3f(0.0f,0.0f,1.0f);
			glVertex(glyphVertices[16]);
			glVertex(glyphVertices[17]);
			glVertex(glyphVertices[26]);
			glVertex(glyphVertices[27]);
			glVertex(glyphVertices[17]);
			glVertex(glyphVertices[18]);
			glVertex(glyphVertices[19]);
			glVertex(glyphVertices[20]);
			glVertex(glyphVertices[20]);
			glVertex(glyphVertices[21]);
			glVertex(glyphVertices[22]);
			glVertex(glyphVertices[23]);
			glVertex(glyphVertices[23]);
			glVertex(glyphVertices[24]);
			glVertex(glyphVertices[25]);
			glVertex(glyphVertices[26]);
			glVertex(glyphVertices[17]);
			glVertex(glyphVertices[20]);
			glVertex(glyphVertices[23]);
			glVertex(glyphVertices[26]);
			glEnd();
			break;
		
		case SIMPLE_ARROW_LEFT:
		case SIMPLE_ARROW_DOWN:
		case SIMPLE_ARROW_RIGHT:
		case SIMPLE_ARROW_UP:
			glBegin(GL_TRIANGLES);
			glNormal3f(0.0f,0.0f,1.0f);
			glVertex(glyphVertices[7]);
			glVertex(glyphVertices[8]);
			glVertex(glyphVertices[9]);
			glEnd();
			break;
		
		case FANCY_ARROW_LEFT:
		case FANCY_ARROW_DOWN:
		case FANCY_ARROW_RIGHT:
		case FANCY_ARROW_UP:
			glBegin(GL_TRIANGLE_FAN);
			glNormal3f(0.0f,0.0f,1.0f);
			for(int i=11;i<18;++i)
				glVertex(glyphVertices[i]);
			glEnd();
			break;
		}
	}

}
