/***********************************************************************
GLNumberRenderer - Class to render numbers using a HUD-like font.
Copyright (c) 2010-2013 Oliver Kreylos

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

#include <GL/GLNumberRenderer.h>

#include <stdio.h>
#include <Misc/PrintInteger.h>
#include <GL/gl.h>
#include <GL/GLContextData.h>

/*******************************************
Methods of class GLNumberRenderer::DataItem:
*******************************************/

GLNumberRenderer::DataItem::DataItem(void)
	:displayListBase(glGenLists(12))
	{
	}

GLNumberRenderer::DataItem::~DataItem(void)
	{
	glDeleteLists(displayListBase,12);
	}

/*********************************
Methods of class GLNumberRenderer:
*********************************/

GLfloat GLNumberRenderer::calcStringWidth(const char* string) const
	{
	/* Calculate the string's rendering width: */
	GLfloat result=-fontHeight*0.25f; // Take out the superfluous space at the end
	for(const char* sPtr=string;*sPtr!='\0';++sPtr)
		{
		switch(*sPtr)
			{
			case '.':
				result+=charWidths[10];
				break;
			
			case '-':
				result+=charWidths[11];
				break;
			
			default:
				result+=charWidths[int(*sPtr)-'0'];
			}
		}
	
	return result;
	}

void GLNumberRenderer::drawString(const char* string,GLContextData& contextData) const
	{
	/* Retrieve the data item: */
	DataItem* dataItem=contextData.retrieveDataItem<DataItem>(this);
	
	/* Draw all characters: */
	for(const char* sPtr=string;*sPtr!='\0';++sPtr)
		{
		switch(*sPtr)
			{
			case '.':
				glCallList(dataItem->displayListBase+10);
				break;
			
			case '-':
				glCallList(dataItem->displayListBase+11);
				break;
			
			default:
				glCallList(dataItem->displayListBase+int(*sPtr)-'0');
			}
		}
	}

void GLNumberRenderer::drawString(const char* string,const GLNumberRenderer::Vector& pos,int horizAlign,int vertAlign,GLContextData& contextData) const
	{
	/* Align the string: */
	Vector origin=pos;
	if(horizAlign>=0)
		{
		/* Calculate the string width: */
		GLfloat width=calcStringWidth(string);
		origin[0]-=width*GLfloat(horizAlign+1)*0.5f; // Evil hack for centered or right-aligned
		}
	if(vertAlign>=0)
		origin[1]-=fontHeight*GLfloat(vertAlign+1)*0.5f; // Same evil hack for centered or top-aligned
	
	/* Move to the origin: */
	glPushMatrix();
	glTranslatef(origin[0],origin[1],origin[2]);
	
	/* Draw the string: */
	drawString(string,contextData);
	
	/* Return to original coordinates: */
	glPopMatrix();
	}

GLNumberRenderer::GLNumberRenderer(GLfloat sFontHeight,bool sProportional)
	:GLObject(false),
	 fontHeight(sFontHeight),proportional(sProportional)
	{
	/* Set all characters to the same width: */
	GLfloat width=fontHeight*0.75f;
	for(int i=0;i<12;++i)
		charWidths[i]=width;
	
	if(proportional)
		{
		/* Adjust the widths of non-standard characters: */
		charWidths[1]=fontHeight*0.25f; // 1
		charWidths[10]=fontHeight*0.25f; // Decimal point
		}
	
	GLObject::init();
	}

void GLNumberRenderer::initContext(GLContextData& contextData) const
	{
	/* Create and register a data item: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	
	GLfloat s=fontHeight*0.5f;
	
	/* Create all character glyphs: */
	glNewList(dataItem->displayListBase+0,GL_COMPILE);
	glBegin(GL_LINE_LOOP);
	glVertex2f(0.0f*s,0.0f*s);
	glVertex2f(1.0f*s,0.0f*s);
	glVertex2f(1.0f*s,2.0f*s);
	glVertex2f(0.0f*s,2.0f*s);
	glEnd();
	glTranslatef(s*1.5f,0.0f,0.0f);
	glEndList();
	
	glNewList(dataItem->displayListBase+1,GL_COMPILE);
	if(proportional)
		{
		glBegin(GL_LINES);
		glVertex2f(0.0f*s,0.0f*s);
		glVertex2f(0.0f*s,2.0f*s);
		glEnd();
		glTranslatef(s*0.5f,0.0f,0.0f);
		}
	else
		{
		glBegin(GL_LINES);
		glVertex2f(0.5f*s,0.0f*s);
		glVertex2f(0.5f*s,2.0f*s);
		glEnd();
		glTranslatef(s*1.5f,0.0f,0.0f);
		}
	glEndList();
	
	glNewList(dataItem->displayListBase+2,GL_COMPILE);
	glBegin(GL_LINE_STRIP);
	glVertex2f(0.0f*s,2.0f*s);
	glVertex2f(1.0f*s,2.0f*s);
	glVertex2f(1.0f*s,1.0f*s);
	glVertex2f(0.0f*s,1.0f*s);
	glVertex2f(0.0f*s,0.0f*s);
	glVertex2f(1.0f*s,0.0f*s);
	glEnd();
	glTranslatef(s*1.5f,0.0f,0.0f);
	glEndList();
	
	glNewList(dataItem->displayListBase+3,GL_COMPILE);
	glBegin(GL_LINE_STRIP);
	glVertex2f(0.0f*s,2.0f*s);
	glVertex2f(1.0f*s,2.0f*s);
	glVertex2f(1.0f*s,0.0f*s);
	glVertex2f(0.0f*s,0.0f*s);
	glEnd();
	glBegin(GL_LINES);
	glVertex2f(0.0f*s,1.0f*s);
	glVertex2f(1.0f*s,1.0f*s);
	glEnd();
	glTranslatef(s*1.5f,0.0f,0.0f);
	glEndList();
	
	glNewList(dataItem->displayListBase+4,GL_COMPILE);
	glBegin(GL_LINE_STRIP);
	glVertex2f(0.0f*s,2.0f*s);
	glVertex2f(0.0f*s,1.0f*s);
	glVertex2f(1.0f*s,1.0f*s);
	glEnd();
	glBegin(GL_LINES);
	glVertex2f(1.0f*s,2.0f*s);
	glVertex2f(1.0f*s,0.0f*s);
	glEnd();
	glTranslatef(s*1.5f,0.0f,0.0f);
	glEndList();
	
	glNewList(dataItem->displayListBase+5,GL_COMPILE);
	glBegin(GL_LINE_STRIP);
	glVertex2f(1.0f*s,2.0f*s);
	glVertex2f(0.0f*s,2.0f*s);
	glVertex2f(0.0f*s,1.0f*s);
	glVertex2f(1.0f*s,1.0f*s);
	glVertex2f(1.0f*s,0.0f*s);
	glVertex2f(0.0f*s,0.0f*s);
	glEnd();
	glTranslatef(s*1.5f,0.0f,0.0f);
	glEndList();
	
	glNewList(dataItem->displayListBase+6,GL_COMPILE);
	glBegin(GL_LINE_STRIP);
	glVertex2f(0.0f*s,2.0f*s);
	glVertex2f(0.0f*s,0.0f*s);
	glVertex2f(1.0f*s,0.0f*s);
	glVertex2f(1.0f*s,1.0f*s);
	glVertex2f(0.0f*s,1.0f*s);
	glEnd();
	glTranslatef(s*1.5f,0.0f,0.0f);
	glEndList();
	
	glNewList(dataItem->displayListBase+7,GL_COMPILE);
	glBegin(GL_LINE_STRIP);
	glVertex2f(0.0f*s,2.0f*s);
	glVertex2f(1.0f*s,2.0f*s);
	glVertex2f(1.0f*s,0.0f*s);
	glEnd();
	glTranslatef(s*1.5f,0.0f,0.0f);
	glEndList();
	
	glNewList(dataItem->displayListBase+8,GL_COMPILE);
	glBegin(GL_LINE_LOOP);
	glVertex2f(0.0f*s,2.0f*s);
	glVertex2f(0.0f*s,0.0f*s);
	glVertex2f(1.0f*s,0.0f*s);
	glVertex2f(1.0f*s,2.0f*s);
	glEnd();
	glBegin(GL_LINES);
	glVertex2f(0.0f*s,1.0f*s);
	glVertex2f(1.0f*s,1.0f*s);
	glEnd();
	glTranslatef(s*1.5f,0.0f,0.0f);
	glEndList();
	
	glNewList(dataItem->displayListBase+9,GL_COMPILE);
	glBegin(GL_LINE_STRIP);
	glVertex2f(1.0f*s,1.0f*s);
	glVertex2f(0.0f*s,1.0f*s);
	glVertex2f(0.0f*s,2.0f*s);
	glVertex2f(1.0f*s,2.0f*s);
	glVertex2f(1.0f*s,0.0f*s);
	glEnd();
	glTranslatef(s*1.5f,0.0f,0.0f);
	glEndList();
	
	glNewList(dataItem->displayListBase+10,GL_COMPILE);
	if(proportional)
		{
		glBegin(GL_LINES);
		glVertex2f(0.0f*s,0.0f*s);
		glVertex2f(0.0f*s,0.25f*s);
		glEnd();
		glTranslatef(s*0.5f,0.0f,0.0f);
		}
	else
		{
		glBegin(GL_LINES);
		glVertex2f(0.5f*s,0.0f*s);
		glVertex2f(0.5f*s,0.25f*s);
		glEnd();
		glTranslatef(s*1.5f,0.0f,0.0f);
		}
	glEndList();
	
	glNewList(dataItem->displayListBase+11,GL_COMPILE);
	glBegin(GL_LINES);
	glVertex2f(0.0f*s,1.0f*s);
	glVertex2f(1.0f*s,1.0f*s);
	glEnd();
	glTranslatef(s*1.5f,0.0f,0.0f);
	glEndList();
	}

GLfloat GLNumberRenderer::calcNumberWidth(unsigned int number) const
	{
	/* Convert the number to a string and calculate its width: */
	char buffer[11];
	return calcStringWidth(Misc::print(number,buffer+10));
	}

GLfloat GLNumberRenderer::calcNumberWidth(int number) const
	{
	/* Convert the number to a string and calculate its width: */
	char buffer[12];
	return calcStringWidth(Misc::print(number,buffer+11));
	}

GLfloat GLNumberRenderer::calcNumberWidth(double number,int precision) const
	{
	/* Convert the number to a string: */
	char buffer[64];
	snprintf(buffer,sizeof(buffer),"%.*f",precision,number);
	
	return calcStringWidth(buffer);
	}

void GLNumberRenderer::drawNumber(unsigned int number,GLContextData& contextData) const
	{
	/* Convert the number to a string and draw the string: */
	char buffer[11];
	drawString(Misc::print(number,buffer+10),contextData);
	}

void GLNumberRenderer::drawNumber(int number,GLContextData& contextData) const
	{
	/* Convert the number to a string and draw the string: */
	char buffer[12];
	drawString(Misc::print(number,buffer+11),contextData);
	}

void GLNumberRenderer::drawNumber(double number,int precision,GLContextData& contextData) const
	{
	/* Convert the number to a string: */
	char buffer[64];
	snprintf(buffer,sizeof(buffer),"%.*f",precision,number);
	
	/* Draw the string: */
	drawString(buffer,contextData);
	}

void GLNumberRenderer::drawNumber(const GLNumberRenderer::Vector& pos,unsigned int number,GLContextData& contextData,int horizAlign,int vertAlign) const
	{
	/* Convert the number to a string and draw the string: */
	char buffer[11];
	drawString(Misc::print(number,buffer+10),pos,horizAlign,vertAlign,contextData);
	}

void GLNumberRenderer::drawNumber(const GLNumberRenderer::Vector& pos,int number,GLContextData& contextData,int horizAlign,int vertAlign) const
	{
	/* Convert the number to a string and draw the string: */
	char buffer[12];
	drawString(Misc::print(number,buffer+11),pos,horizAlign,vertAlign,contextData);
	}

void GLNumberRenderer::drawNumber(const GLNumberRenderer::Vector& pos,double number,int precision,GLContextData& contextData,int horizAlign,int vertAlign) const
	{
	/* Convert the number to a string: */
	char buffer[64];
	snprintf(buffer,sizeof(buffer),"%.*f",precision,number);
	
	/* Draw the string: */
	drawString(buffer,pos,horizAlign,vertAlign,contextData);
	}
