/***********************************************************************
IndexedLineSetNode - Class for sets of lines or polylines as renderable
geometry.
Copyright (c) 2009 Oliver Kreylos

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

#include <SceneGraph/IndexedLineSetNode.h>

#include <string.h>
#include <Geometry/Box.h>
#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLVertexTemplates.h>
#include <GL/GLContextData.h>
#include <GL/GLExtensionManager.h>
#include <GL/Extensions/GLARBVertexBufferObject.h>
#include <GL/GLGeometryWrappers.h>
#include <GL/GLGeometryVertex.h>
#include <SceneGraph/VRMLFile.h>
#include <SceneGraph/GLRenderState.h>

namespace SceneGraph {

/*********************************************
Methods of class IndexedLineSetNode::DataItem:
*********************************************/

IndexedLineSetNode::DataItem::DataItem(void)
	:vertexBufferObjectId(0),
	 version(0)
	{
	if(GLARBVertexBufferObject::isSupported())
		{
		/* Initialize the vertex buffer object extension: */
		GLARBVertexBufferObject::initExtension();
		
		/* Create the vertex buffer object: */
		glGenBuffersARB(1,&vertexBufferObjectId);
		}
	}

IndexedLineSetNode::DataItem::~DataItem(void)
	{
	/* Destroy the vertex buffer object: */
	if(vertexBufferObjectId!=0)
		glDeleteBuffersARB(1,&vertexBufferObjectId);
	}

/***********************************
Methods of class IndexedLineSetNode:
***********************************/

void IndexedLineSetNode::uploadColoredLineSet(DataItem* dataItem) const
	{
	/* Define the vertex type used in the vertex array: */
	typedef GLGeometry::Vertex<void,0,GLubyte,4,void,Scalar,3> Vertex;
	
	/* Allocate the vertex buffer and the line size array: */
	glBufferDataARB(GL_ARRAY_BUFFER_ARB,totalNumVertices*sizeof(Vertex),0,GL_STATIC_DRAW_ARB);
	Vertex* vPtr=static_cast<Vertex*>(glMapBufferARB(GL_ARRAY_BUFFER_ARB,GL_WRITE_ONLY_ARB));
	
	/* Copy vertices into the vertex buffer: */
	const std::vector<Point>& points=coord.getValue()->point.getValues();
	const MFInt::ValueList& coordIndices=coordIndex.getValues();
	const std::vector<Color>& colors=color.getValue()->color.getValues();
	const MFInt::ValueList& colorIndices=colorIndex.getValues();
	if(colorPerVertex.getValue())
		{
		if(colorIndices.empty())
			{
			int colorIndex=0;
			for(MFInt::ValueList::const_iterator coordIt=coordIndices.begin();coordIt!=coordIndices.end();++coordIt)
				{
				if(*coordIt>=0)
					{
					vPtr->color=colors[colorIndex];
					if(pointTransform.getValue()!=0)
						vPtr->position=pointTransform.getValue()->transformPoint(points[*coordIt]);
					else
						vPtr->position=points[*coordIt];
					++vPtr;
					++colorIndex;
					}
				}
			}
		else
			{
			MFInt::ValueList::const_iterator colorIt=colorIndices.begin();
			for(MFInt::ValueList::const_iterator coordIt=coordIndices.begin();coordIt!=coordIndices.end();++coordIt)
				{
				if(*coordIt>=0)
					{
					vPtr->color=colors[*colorIt];
					if(pointTransform.getValue()!=0)
						vPtr->position=pointTransform.getValue()->transformPoint(points[*coordIt]);
					else
						vPtr->position=points[*coordIt];
					++vPtr;
					}
				++colorIt;
				}
			}
		}
	else
		{
		if(colorIndices.empty())
			{
			int colorIndex=0;
			for(MFInt::ValueList::const_iterator coordIt=coordIndices.begin();coordIt!=coordIndices.end();++coordIt)
				{
				if(*coordIt>=0)
					{
					vPtr->color=colors[colorIndex];
					if(pointTransform.getValue()!=0)
						vPtr->position=pointTransform.getValue()->transformPoint(points[*coordIt]);
					else
						vPtr->position=points[*coordIt];
					++vPtr;
					}
				else
					++colorIndex;
				}
			}
		else
			{
			MFInt::ValueList::const_iterator colorIt=colorIndices.begin();
			for(MFInt::ValueList::const_iterator coordIt=coordIndices.begin();coordIt!=coordIndices.end();++coordIt)
				{
				if(*coordIt>=0)
					{
					vPtr->color=colors[*colorIt];
					if(pointTransform.getValue()!=0)
						vPtr->position=pointTransform.getValue()->transformPoint(points[*coordIt]);
					else
						vPtr->position=points[*coordIt];
					++vPtr;
					}
				else
					++colorIt;
				}
			}
		}
	
	glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
	}

void IndexedLineSetNode::uploadLineSet(DataItem* dataItem) const
	{
	/* Define the vertex type used in the vertex array: */
	typedef GLGeometry::Vertex<void,0,void,0,void,Scalar,3> Vertex;
	
	/* Allocate the vertex buffer and the line size array: */
	glBufferDataARB(GL_ARRAY_BUFFER_ARB,totalNumVertices*sizeof(Vertex),0,GL_STATIC_DRAW_ARB);
	Vertex* vPtr=static_cast<Vertex*>(glMapBufferARB(GL_ARRAY_BUFFER_ARB,GL_WRITE_ONLY_ARB));
	
	/* Copy vertices into the vertex buffer: */
	const std::vector<Point>& points=coord.getValue()->point.getValues();
	const MFInt::ValueList& coordIndices=coordIndex.getValues();
	for(MFInt::ValueList::const_iterator coordIt=coordIndices.begin();coordIt!=coordIndices.end();++coordIt)
		{
		if(*coordIt>=0)
			{
			if(pointTransform.getValue()!=0)
				vPtr->position=pointTransform.getValue()->transformPoint(points[*coordIt]);
			else
				vPtr->position=points[*coordIt];
			++vPtr;
			}
		}
	
	glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
	}

IndexedLineSetNode::IndexedLineSetNode(void)
	:colorPerVertex(true),
	 lineWidth(Scalar(1)),
	 totalNumVertices(0),
	 version(0)
	{
	}

const char* IndexedLineSetNode::getStaticClassName(void)
	{
	return "IndexedLineSet";
	}

const char* IndexedLineSetNode::getClassName(void) const
	{
	return "IndexedLineSet";
	}

void IndexedLineSetNode::parseField(const char* fieldName,VRMLFile& vrmlFile)
	{
	if(strcmp(fieldName,"color")==0)
		{
		vrmlFile.parseSFNode(color);
		}
	else if(strcmp(fieldName,"coord")==0)
		{
		vrmlFile.parseSFNode(coord);
		}
	else if(strcmp(fieldName,"colorIndex")==0)
		{
		vrmlFile.parseField(colorIndex);
		}
	else if(strcmp(fieldName,"colorPerVertex")==0)
		{
		vrmlFile.parseField(colorPerVertex);
		}
	else if(strcmp(fieldName,"coordIndex")==0)
		{
		vrmlFile.parseField(coordIndex);
		}
	else if(strcmp(fieldName,"lineWidth")==0)
		{
		vrmlFile.parseField(lineWidth);
		}
	else
		GeometryNode::parseField(fieldName,vrmlFile);
	}

void IndexedLineSetNode::update(void)
	{
	/* Iterate over the coordinate index array to count the number of vertices for each line and the total number of vertices: */
	const MFInt::ValueList& coordIndices=coordIndex.getValues();
	numVertices.clear();
	totalNumVertices=0;
	GLsizei currentNumVertices=0;
	for(MFInt::ValueList::const_iterator coordIt=coordIndices.begin();coordIt!=coordIndices.end();++coordIt)
		{
		if(*coordIt>=0)
			{
			++totalNumVertices;
			++currentNumVertices;
			}
		else
			{
			if(currentNumVertices>0)
				numVertices.push_back(currentNumVertices);
			currentNumVertices=0;
			}
		}
	if(currentNumVertices>0)
		numVertices.push_back(currentNumVertices);
	
	/* Bump up the indexed line set's version number: */
	++version;
	}

Box IndexedLineSetNode::calcBoundingBox(void) const
	{
	if(coord.getValue()!=0)
		{
		if(pointTransform.getValue()!=0)
			{
			/* Return the bounding box of the transformed point coordinates: */
			return pointTransform.getValue()->calcBoundingBox(coord.getValue()->point.getValues());
			}
		else
			{
			/* Return the bounding box of the untransformed point coordinates: */
			return coord.getValue()->calcBoundingBox();
			}
		}
	else
		return Box::empty;
	}

void IndexedLineSetNode::glRenderAction(GLRenderState& renderState) const
	{
	/* Set up OpenGL state: */
	renderState.disableMaterials();
	renderState.disableTextures();
	glLineWidth(lineWidth.getValue());
	
	/* Get the context data item: */
	DataItem* dataItem=renderState.contextData.retrieveDataItem<DataItem>(this);
	
	if(dataItem->vertexBufferObjectId!=0)
		{
		/*******************************************************************
		Render the indexed line set from the vertex buffer:
		*******************************************************************/
		
		typedef GLGeometry::Vertex<void,0,GLubyte,4,void,Scalar,3> ColorVertex;
		typedef GLGeometry::Vertex<void,0,void,0,void,Scalar,3> Vertex;
		
		/* Bind the line set's vertex buffer object: */
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,dataItem->vertexBufferObjectId);
		
		if(dataItem->version!=version)
			{
			/* Upload the new line set: */
			if(color.getValue()!=0)
				uploadColoredLineSet(dataItem);
			else
				uploadLineSet(dataItem);
			
			/* Mark the vertex and index buffer objects as up-to-date: */
			dataItem->version=version;
			}
		
		/* Set up the vertex array: */
		if(color.getValue()!=0)
			{
			GLVertexArrayParts::enable(ColorVertex::getPartsMask());
			glVertexPointer(static_cast<ColorVertex*>(0));
			}
		else
			{
			GLVertexArrayParts::enable(Vertex::getPartsMask());
			glVertexPointer(static_cast<Vertex*>(0));
			
			/* Use the current emissive color: */
			glColor(renderState.emissiveColor);
			}
		
		/* Draw the indexed line set: */
		GLint baseVertexIndex=0;
		for(std::vector<GLsizei>::const_iterator nvIt=numVertices.begin();nvIt!=numVertices.end();++nvIt)
			{
			if(*nvIt>=2)
				{
				/* Render the line: */
				glDrawArrays(GL_LINE_STRIP,baseVertexIndex,*nvIt);
				}
			
			/* Go to the next line: */
			baseVertexIndex+=*nvIt;
			}
		
		/* Disable the vertex array: */
		if(color.getValue()!=0)
			GLVertexArrayParts::disable(ColorVertex::getPartsMask());
		else
			GLVertexArrayParts::disable(Vertex::getPartsMask());
		
		/* Protect the buffer: */
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
		}
	else
		{
		/*******************************************************************
		Render the indexed line set directly:
		*******************************************************************/
		
		if(coord.getValue()!=0)
			{
			/* Draw the line set: */
			const std::vector<Point>& points=coord.getValue()->point.getValues();
			const MFInt::ValueList& coordIndices=coordIndex.getValues();
			if(color.getValue()!=0)
				{
				const std::vector<Color>& colors=color.getValue()->color.getValues();
				const MFInt::ValueList& colorIndices=colorIndex.getValues();
				MFInt::ValueList::const_iterator colorIt=colorIndices.empty()?coordIndices.begin():colorIndices.begin();
				bool countColors=!colorPerVertex.getValue()&&colorIndices.empty();
				int colorCounter=0;
				MFInt::ValueList::const_iterator coordIt=coordIndices.begin();
				while(coordIt!=coordIndices.end())
					{
					glBegin(GL_LINE_STRIP);
					while(coordIt!=coordIndices.end()&&*coordIt>=0)
						{
						if(countColors)
							glColor(colors[colorCounter]);
						else
							glColor(colors[*colorIt]);
						if(pointTransform.getValue()!=0)
							glVertex(pointTransform.getValue()->transformPoint(points[*coordIt]));
						else
							glVertex(points[*coordIt]);
						if(colorPerVertex.getValue())
							++colorIt;
						++coordIt;
						}
					glEnd();
					
					if(countColors)
						++colorCounter;
					else
						++colorIt;
					if(coordIt!=coordIndices.end())
						++coordIt;
					}
				}
			else
				{
				/* Use the current emissive color: */
				glColor(renderState.emissiveColor);
				
				MFInt::ValueList::const_iterator coordIt=coordIndices.begin();
				while(coordIt!=coordIndices.end())
					{
					glBegin(GL_LINE_STRIP);
					while(*coordIt>=0)
						{
						if(pointTransform.getValue()!=0)
							glVertex(pointTransform.getValue()->transformPoint(points[*coordIt]));
						else
							glVertex(points[*coordIt]);
						++coordIt;
						}
					glEnd();
					
					if(coordIt!=coordIndices.end())
						++coordIt;
					}
				}
			}
		}
	}

void IndexedLineSetNode::initContext(GLContextData& contextData) const
	{
	/* Create a data item and store it in the context: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	}

}
