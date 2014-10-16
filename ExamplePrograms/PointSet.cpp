/***********************************************************************
PointSet - Class to represent and render sets of scattered 3D points.
Copyright (c) 2005-2007 Oliver Kreylos

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <Misc/ThrowStdErr.h>
#include <Misc/File.h>
#include <Math/Math.h>
#include <GL/gl.h>
#include <GL/GLVertexArrayParts.h>
#include <GL/GLContextData.h>
#include <GL/GLExtensionManager.h>
#include <GL/Extensions/GLARBVertexBufferObject.h>

#include "EarthFunctions.h"

#include "PointSet.h"

namespace {

/***********************************************************************
Helper classes and functions to parse spreadsheet files in text format:
***********************************************************************/

int getNextValue(Misc::File& file,int nextChar,char* valueBuffer,size_t valueBufferSize)
	{
	/* Prepare writing to the value buffer: */
	char* vPtr=valueBuffer;
	
	if(nextChar=='"')
		{
		/* Read characters until next quotation mark: */
		while(true)
			{
			/* Read the next character and check for quote: */
			nextChar=file.getc();
			if(nextChar=='"')
				break;
			
			/* Store next character if there is room: */
			if(valueBufferSize>1)
				{
				*vPtr=char(nextChar);
				++vPtr;
				--valueBufferSize;
				}
			}
		
		/* Read the character after the closing quote: */
		nextChar=file.getc();
		}
	else
		{
		while(nextChar!=EOF&&nextChar!=','&&!isspace(nextChar))
			{
			/* Store next character if there is room: */
			if(valueBufferSize>1)
				{
				*vPtr=char(nextChar);
				++vPtr;
				--valueBufferSize;
				}
			
			/* Read the next character: */
			nextChar=file.getc();
			}
		}
	
	/* Terminate the value buffer: */
	*vPtr='\0';
	
	/* Skip whitespace: */
	while(nextChar!=EOF&&nextChar!='\n'&&isspace(nextChar))
		nextChar=file.getc();
	
	/* Check for a separating comma: */
	if(nextChar==',')
		{
		/* Skip the comma: */
		nextChar=file.getc();
		
		/* Skip whitespace: */
		while(nextChar!=EOF&&nextChar!='\n'&&isspace(nextChar))
			nextChar=file.getc();
		}
	
	return nextChar;
	}

}

/***********************************
Methods of class PointSet::DataItem:
***********************************/

PointSet::DataItem::DataItem(void)
	:vertexBufferObjectId(0)
	{
	/* Check if the vertex buffer object extension is supported: */
	if(GLARBVertexBufferObject::isSupported())
		{
		/* Initialize the vertex buffer object extension: */
		GLARBVertexBufferObject::initExtension();
		
		/* Create a vertex buffer object: */
		glGenBuffersARB(1,&vertexBufferObjectId);
		}
	}

PointSet::DataItem::~DataItem(void)
	{
	/* Check if the vertex buffer object extension is supported: */
	if(vertexBufferObjectId>0)
		{
		/* Destroy the vertex buffer object: */
		glDeleteBuffersARB(1,&vertexBufferObjectId);
		}
	}

/*************************
Methods of class PointSet:
*************************/

PointSet::PointSet(const char* pointFileName,double scaleFactor,const float colorMask[3])
	{
	/* Open the point file: */
	Misc::File pointFile(pointFileName,"rt");
	
	/* Skip initial whitespace: */
	int nextChar;
	do
		{
		nextChar=pointFile.getc();
		}
	while(nextChar!=EOF&&isspace(nextChar));
	if(nextChar==EOF)
		Misc::throwStdErr("PointSet::PointSet: Early end of file in input file \"%s\"",pointFileName);
	
	/* Read the file's header line: */
	int cartesianIndex[3]={-1,-1,-1}; // Indices of x, y, z columns
	int topographicIndex[3]={-1,-1,-1}; // Indices of lat, long, radius columns
	enum RadiusMode
		{
		RADIUS,DEPTH,NEG_DEPTH
		} radiusMode=RADIUS;
	int colorIndex[3]={-1,-1,-1}; // Indices of red, green, blue color components
	char valueBuffer[256];
	for(int columnIndex=0;nextChar!=EOF&&nextChar!='\n';++columnIndex)
		{
		/* Read the next column header: */
		nextChar=getNextValue(pointFile,nextChar,valueBuffer,sizeof(valueBuffer));
		
		/* Process the column header: */
		if(strcasecmp(valueBuffer,"x")==0)
			cartesianIndex[0]=columnIndex;
		else if(strcasecmp(valueBuffer,"y")==0)
			cartesianIndex[1]=columnIndex;
		else if(strcasecmp(valueBuffer,"z")==0)
			cartesianIndex[2]=columnIndex;
		else if(strcasecmp(valueBuffer,"lat")==0||strcasecmp(valueBuffer,"latitude")==0)
			topographicIndex[0]=columnIndex;
		else if(strcasecmp(valueBuffer,"lon")==0||strcasecmp(valueBuffer,"long")==0||strcasecmp(valueBuffer,"longitude")==0)
			topographicIndex[1]=columnIndex;
		else if(strcasecmp(valueBuffer,"radius")==0)
			{
			topographicIndex[2]=columnIndex;
			radiusMode=RADIUS;
			}
		else if(strcasecmp(valueBuffer,"depth")==0)
			{
			topographicIndex[2]=columnIndex;
			radiusMode=DEPTH;
			}
		else if(strcasecmp(valueBuffer,"neg depth")==0||strcasecmp(valueBuffer,"negative depth")==0)
			{
			topographicIndex[2]=columnIndex;
			radiusMode=NEG_DEPTH;
			}
		else if(strcasecmp(valueBuffer,"r")==0)
			colorIndex[0]=columnIndex;
		else if(strcasecmp(valueBuffer,"g")==0)
			colorIndex[1]=columnIndex;
		else if(strcasecmp(valueBuffer,"b")==0)
			colorIndex[2]=columnIndex;
		}
	
	/* Check if there is a valid match: */
	bool cartesianValid=true;
	for(int i=0;i<3;++i)
		cartesianValid=cartesianValid&&cartesianIndex[i]>=0;
	bool topographicValid=true;
	for(int i=0;i<3;++i)
		topographicValid=topographicValid&&topographicIndex[i]>=0;
	if(!cartesianValid&&!topographicValid)
		Misc::throwStdErr("PointSet::PointSet: No point coordinates in input file \"%s\"",pointFileName);
	bool colorValid=true;
	for(int i=0;i<3;++i)
		colorValid=colorValid&&colorIndex[i]>=0;
	
	/* Create the column map: */
	int numColumns=-1;
	if(cartesianValid)
		{
		for(int i=0;i<3;++i)
			if(numColumns<cartesianIndex[i])
				numColumns=cartesianIndex[i];
		}
	else if(topographicValid)
		{
		for(int i=0;i<3;++i)
			if(numColumns<topographicIndex[i])
				numColumns=topographicIndex[i];
		}
	if(colorValid)
		{
		for(int i=0;i<3;++i)
			if(numColumns<colorIndex[i])
				numColumns=colorIndex[i];
		}
	++numColumns;
	int* columnMap=new int[numColumns];
	for(int i=0;i<numColumns;++i)
		columnMap[i]=-1;
	if(cartesianValid)
		{
		for(int i=0;i<3;++i)
			columnMap[cartesianIndex[i]]=i;
		}
	else if(topographicValid)
		{
		for(int i=0;i<3;++i)
			columnMap[topographicIndex[i]]=i;
		}
	if(colorValid)
		{
		for(int i=0;i<3;++i)
			columnMap[colorIndex[i]]=3+i;
		}
	int fullComponentMask=0x7;
	if(colorValid)
		fullComponentMask|=0x38;
	
	/* Read all points from the point file: */
	while(nextChar!=EOF)
		{
		/* Skip initial whitespace: */
		do
			{
			nextChar=pointFile.getc();
			}
		while(nextChar!=EOF&&isspace(nextChar));
		
		/* Read the next line from the input file: */
		int index=0;
		double components[6];
		int parsedComponentsMask=0x0;
		while(nextChar!=EOF&&nextChar!='\n')
			{
			char valueBuffer[40];
			nextChar=getNextValue(pointFile,nextChar,valueBuffer,sizeof(valueBuffer));
			if(index<numColumns&&columnMap[index]>=0)
				{
				components[columnMap[index]]=float(atof(valueBuffer));
				parsedComponentsMask|=0x1<<columnMap[index];
				}
			++index;
			}
		
		/* Check if a complete set of coordinates has been parsed: */
		if(parsedComponentsMask==fullComponentMask)
			{
			SourcePoint p;
			if(cartesianValid)
				{
				for(int i=0;i<3;++i)
					p[i]=float(components[i]*scaleFactor);
				}
			else if(topographicValid)
				{
				switch(radiusMode)
					{
					case RADIUS:
						calcRadiusPos<float>(Math::rad(components[0]),Math::rad(components[1]),components[2]*1000.0f,scaleFactor,p.getComponents());
						break;

					case DEPTH:
						calcDepthPos<float>(Math::rad(components[0]),Math::rad(components[1]),components[2]*1000.0f,scaleFactor,p.getComponents());
						break;

					case NEG_DEPTH:
						calcDepthPos<float>(Math::rad(components[0]),Math::rad(components[1]),-components[2]*1000.0f,scaleFactor,p.getComponents());
						break;
					}
				}
			for(int i=0;i<3;++i)
				{
				float v=colorValid?float(components[3+i])*colorMask[i]*255.0f:colorMask[i]*255.0f;
				if(v<0.5f)
					p.value[i]=GLubyte(0);
				else if(v>=254.5f)
					p.value[i]=GLubyte(255);
				else
					p.value[i]=GLubyte(Math::floor(v+0.5f));
				}
			p.value[3]=GLubyte(255);
			
			/* Append the point to the point set: */
			points.push_back(p);
			}
		}
	}

void PointSet::initContext(GLContextData& contextData) const
	{
	/* Create a context data item and store it in the context: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	
	/* Check if the vertex buffer object extension is supported: */
	if(dataItem->vertexBufferObjectId>0)
		{
		/* Create a vertex buffer object to store the points' coordinates: */
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,dataItem->vertexBufferObjectId);
		glBufferDataARB(GL_ARRAY_BUFFER_ARB,points.size()*sizeof(Vertex),0,GL_STATIC_DRAW_ARB);
		
		/* Copy all points: */
		Vertex* vPtr=static_cast<Vertex*>(glMapBufferARB(GL_ARRAY_BUFFER_ARB,GL_WRITE_ONLY_ARB));
		for(std::vector<SourcePoint>::const_iterator pIt=points.begin();pIt!=points.end();++pIt,++vPtr)
			{
			for(int i=0;i<4;++i)
				vPtr->color[i]=pIt->value[i];
			for(int i=0;i<3;++i)
				vPtr->position[i]=GLfloat((*pIt)[i]);
			}
		glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
		
		/* Protect the vertex buffer object: */
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
		}
	}

PointSet::Box PointSet::calcBoundingBox(void) const
	{
	/* Return the box containing all points in the set: */
	Box result=Box::empty;
	for(std::vector<SourcePoint>::const_iterator pIt=points.begin();pIt!=points.end();++pIt)
		result.addPoint(*pIt);
	return result;
	}

void PointSet::glRenderAction(GLContextData& contextData) const
	{
	/* Get a pointer to the data item: */
	DataItem* dataItem=contextData.retrieveDataItem<DataItem>(this);
	
	/* Save and set up OpenGL state: */
	GLboolean lightingEnabled=glIsEnabled(GL_LIGHTING);
	if(lightingEnabled)
		glDisable(GL_LIGHTING);
	GLVertexArrayParts::enable(Vertex::getPartsMask());
	
	/* Check if the vertex buffer object extension is supported: */
	if(dataItem->vertexBufferObjectId>0)
		{
		/* Bind the point set's vertex buffer object: */
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,dataItem->vertexBufferObjectId);
		
		/* Render the point set from the vertex buffer object: */
		glVertexPointer(static_cast<const Vertex*>(0));
		glDrawArrays(GL_POINTS,0,points.size());
		
		/* Protect the vertex buffer object: */
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
		}
	else
		{
		/* Render the point set as a regular vertex array of points: */
		glColorPointer(4,GL_UNSIGNED_BYTE,sizeof(SourcePoint),&(points[0].value));
		glVertexPointer(3,GL_FLOAT,sizeof(SourcePoint),&points[0]);
		glDrawArrays(GL_POINTS,0,points.size());
		}
	
	/* Restore OpenGL state: */
	GLVertexArrayParts::disable(Vertex::getPartsMask());
	if(lightingEnabled)
		glEnable(GL_LIGHTING);
	}
