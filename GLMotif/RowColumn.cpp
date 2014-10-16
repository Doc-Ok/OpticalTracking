/***********************************************************************
RowColumn - Container class to arrange children on a two-dimensional
grid.
Copyright (c) 2001-2014 Oliver Kreylos

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

#include <GLMotif/RowColumn.h>

#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLVertexTemplates.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/Event.h>

namespace GLMotif {

/**************************
Methods of class RowColumn:
**************************/

Vector RowColumn::calcGrid(std::vector<GLfloat>& columnWidths,std::vector<GLfloat>& rowHeights) const
	{
	/* Initialize the field size arrays: */
	rowHeights.clear();
	columnWidths.clear();
	if(orientation==VERTICAL)
		{
		/* Initialize the column widths for all minor widgets: */
		columnWidths.insert(columnWidths.end(),numMinorWidgets,0.0f);
		}
	else
		{
		/* Initialize the row heights for all minor widgets: */
		rowHeights.insert(rowHeights.end(),numMinorWidgets,0.0f);
		}
	
	/* Calculate the natural size of the grid: */
	GLfloat maxWidth=0.0f;
	GLfloat maxHeight=0.0f;
	int rowIndex=0;
	int columnIndex=0;
	for(WidgetList::const_iterator chIt=children.begin();chIt!=children.end();++chIt)
		{
		/* Fit the current child into its grid cell: */
		if(rowIndex==int(rowHeights.size()))
			rowHeights.push_back(0.0f);
		if(columnIndex==int(columnWidths.size()))
			columnWidths.push_back(0.0f);
		Vector childSize=(*chIt)->calcNaturalSize();
		if(rowHeights[rowIndex]<childSize[1])
			{
			rowHeights[rowIndex]=childSize[1];
			if(maxHeight<childSize[1])
				maxHeight=childSize[1];
			}
		if(columnWidths[columnIndex]<childSize[0])
			{
			columnWidths[columnIndex]=childSize[0];
			if(maxWidth<childSize[0])
				maxWidth=childSize[0];
			}
		
		/* Go to the next child: */
		if(orientation==VERTICAL)
			{
			if(++columnIndex==numMinorWidgets)
				{
				columnIndex=0;
				++rowIndex;
				}
			}
		else
			{
			if(++rowIndex==numMinorWidgets)
				{
				rowIndex=0;
				++columnIndex;
				}
			}
		}
	
	if(packing==PACK_GRID)
		{
		/* Set all sizes to the maximum: */
		for(std::vector<GLfloat>::iterator rIt=rowHeights.begin();rIt!=rowHeights.end();++rIt)
			*rIt=maxHeight;
		for(std::vector<GLfloat>::iterator cIt=columnWidths.begin();cIt!=columnWidths.end();++cIt)
			*cIt=maxWidth;
		}
	
	/* Calculate the overall size: */
	Vector result(0.0f,0.0f,0.0f);
	if(!children.empty())
		{
		for(std::vector<GLfloat>::iterator cIt=columnWidths.begin();cIt!=columnWidths.end();++cIt)
			result[0]+=*cIt;
		for(std::vector<GLfloat>::iterator rIt=rowHeights.begin();rIt!=rowHeights.end();++rIt)
			result[1]+=*rIt;
		result[0]+=GLfloat(columnWidths.size()-1)*spacing;
		result[1]+=GLfloat(rowHeights.size()-1)*spacing;
		}
	
	return result;
	}

RowColumn::RowColumn(const char* sName,Container* sParent,bool sManageChild)
	:Container(sName,sParent,false),
	 orientation(VERTICAL),
	 packing(PACK_TIGHT),
	 alignment(Alignment::HFILL,Alignment::VFILL),
	 numMinorWidgets(1),
	 nextChildIndex(0)
	{
	/* Get the style sheet: */
	const StyleSheet* ss=getStyleSheet();
	
	/* Set the children margin and spacing: */
	marginWidth=ss->containerMarginWidth;
	spacing=ss->containerSpacing;
	
	/* Initialize the number of rows and columns: */
	columnWeights.push_back(0.0f);
	
	/* Manage me: */
	if(sManageChild)
		manageChild();
	}

RowColumn::~RowColumn(void)
	{
	/* Delete all children: */
	for(WidgetList::iterator chIt=children.begin();chIt!=children.end();++chIt)
		deleteChild(*chIt);
	}

Vector RowColumn::calcNaturalSize(void) const
	{
	/* Calculate the natural grid size: */
	std::vector<GLfloat> columnWidths,rowHeights;
	Vector result=calcGrid(columnWidths,rowHeights);
	result[0]+=2.0f*marginWidth;
	result[1]+=2.0f*marginWidth;
	
	return calcExteriorSize(result);
	}

ZRange RowColumn::calcZRange(void) const
	{
	/* Calculate the parent class widget's z range: */
	ZRange myZRange=Container::calcZRange();
	
	/* Calculate the combined z range of all children: */
	for(WidgetList::const_iterator chIt=children.begin();chIt!=children.end();++chIt)
		myZRange+=(*chIt)->calcZRange();
	
	return myZRange;
	}

void RowColumn::resize(const Box& newExterior)
	{
	/* Resize the parent class widget: */
	Container::resize(newExterior);
	
	if(!children.empty())
		{
		/* Calculate the natural grid size: */
		std::vector<GLfloat> columnWidths,rowHeights;
		Vector gridSize=calcGrid(columnWidths,rowHeights);

		/* Calculate the size available to the grid: */
		Box box=getInterior().inset(Vector(marginWidth,marginWidth,0.0f));
		
		/* Align the grid inside its surrounding box: */
		if(gridSize[0]<box.size[0])
			{
			switch(alignment.hAlignment)
				{
				case Alignment::HFILL:
					if(packing==PACK_GRID)
						{
						/* Equally enlarge all columns to fit the margin box: */
						GLfloat columnWidth=(box.size[0]-GLfloat(columnWidths.size()-1)*spacing)/GLfloat(columnWidths.size());
						for(std::vector<GLfloat>::iterator cIt=columnWidths.begin();cIt!=columnWidths.end();++cIt)
							*cIt=columnWidth;
						}
					else
						{
						/* Calculate the total expansion weight: */
						GLfloat totalWeight=0.0f;
						for(std::vector<GLfloat>::const_iterator cwIt=columnWeights.begin();cwIt!=columnWeights.end();++cwIt)
							totalWeight+=*cwIt;
						
						if(totalWeight>0.0f)
							{
							/* Adjust the width of all columns: */
							for(unsigned int i=0;i<columnWeights.size();++i)
								columnWidths[i]+=(box.size[0]-gridSize[0])*columnWeights[i]/totalWeight;
							}
						else
							{
							/* Last guy eats it: */
							columnWidths[columnWidths.size()-1]+=box.size[0]-gridSize[0];
							}
						}
					break;
				
				case Alignment::LEFT:
					box.size[0]=gridSize[0];
					break;
				
				case Alignment::HCENTER:
					box.origin[0]+=(box.size[0]-gridSize[0])*0.5f;
					box.size[0]=gridSize[0];
					break;
				
				case Alignment::RIGHT:
					box.origin[0]+=box.size[0]-gridSize[0];
					box.size[0]=gridSize[0];
					break;
				}
			}
		if(gridSize[1]<box.size[1])
			{
			switch(alignment.vAlignment)
				{
				case Alignment::VFILL:
					if(packing==PACK_GRID)
						{
						/* Equally enlarge all rows to fit the margin box: */
						GLfloat rowHeight=(box.size[1]-GLfloat(rowHeights.size()-1)*spacing)/GLfloat(rowHeights.size());
						for(std::vector<GLfloat>::iterator cIt=rowHeights.begin();cIt!=rowHeights.end();++cIt)
							*cIt=rowHeight;
						}
					else
						{
						/* Calculate the total expansion weight: */
						GLfloat totalWeight=0.0f;
						for(std::vector<GLfloat>::const_iterator rwIt=rowWeights.begin();rwIt!=rowWeights.end();++rwIt)
							totalWeight+=*rwIt;
						
						if(totalWeight>0.0f)
							{
							/* Adjust the height of all rows: */
							for(unsigned int i=0;i<rowWeights.size();++i)
								rowHeights[i]+=(box.size[1]-gridSize[1])*rowWeights[i]/totalWeight;
							}
						else
							{
							/* Last guy eats it: */
							rowHeights[rowHeights.size()-1]+=box.size[1]-gridSize[1];
							}
						}
					break;
				
				case Alignment::BOTTOM:
					box.size[1]=gridSize[1];
					break;
				
				case Alignment::VCENTER:
					box.origin[1]+=(box.size[1]-gridSize[1])*0.5f;
					box.size[1]=gridSize[1];
					break;
				
				case Alignment::TOP:
					box.origin[1]+=box.size[1]-gridSize[1];
					box.size[1]=gridSize[1];
					break;
				}
			}
		
		/* Fill the grid descriptor arrays: */
		Vector origin=box.origin;
		origin[1]+=box.size[1]+spacing;
		rows.clear();
		for(unsigned int i=0;i<rowHeights.size();++i)
			{
			origin[1]-=rowHeights[i]+spacing;
			rows.push_back(GridCell(origin[1],rowHeights[i]));
			}
		columns.clear();
		for(unsigned int i=0;i<columnWidths.size();++i)
			{
			columns.push_back(GridCell(origin[0],columnWidths[i]));
			origin[0]+=columnWidths[i]+spacing;
			}
		
		/* Position and resize all children: */
		int rowIndex=0;
		int columnIndex=0;
		for(WidgetList::iterator chIt=children.begin();chIt!=children.end();++chIt)
			{
			/* Resize the current child: */
			(*chIt)->resize(Box(Vector(columns[columnIndex].origin,rows[rowIndex].origin,origin[2]),Vector(columnWidths[columnIndex],rowHeights[rowIndex],0.0f)));
			
			/* Go to the next child: */
			if(orientation==VERTICAL)
				{
				if(++columnIndex==numMinorWidgets)
					{
					columnIndex=0;
					++rowIndex;
					}
				}
			else
				{
				if(++rowIndex==numMinorWidgets)
					{
					rowIndex=0;
					++columnIndex;
					}
				}
			}
		}
	}

void RowColumn::draw(GLContextData& contextData) const
	{
	/* Draw the parent class widget: */
	Container::draw(contextData);
	
	/* Bail out if there are no children: */
	if(children.empty())
		return;
	
	/* Draw the margin and separators: */
	glColor(backgroundColor);
	Vector p=getInterior().origin;
	
	/* Draw the top left margin part: */
	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(0.0f,0.0f,1.0f);
	glVertex(getInterior().getCorner(2));
	glVertex(getInterior().getCorner(0));
	p[0]=columns[0].origin;
	for(int i=rows.size()-1;i>=0;--i)
		{
		p[1]=rows[i].origin;
		glVertex(p);
		p[1]+=rows[i].size;
		glVertex(p);
		}
	p[0]+=columns[0].size;
	glVertex(p);
	for(unsigned int i=1;i<columns.size();++i)
		{
		p[0]=columns[i].origin;
		glVertex(p);
		p[0]+=columns[i].size;
		glVertex(p);
		}
	glVertex(getInterior().getCorner(3));
	glEnd();
	
	/* Draw the bottom right margin part: */
	glBegin(GL_TRIANGLE_FAN);
	glVertex(getInterior().getCorner(1));
	glVertex(getInterior().getCorner(3));
	for(unsigned int i=0;i<rows.size();++i)
		{
		p[1]=rows[i].origin+rows[i].size;
		glVertex(p);
		p[1]=rows[i].origin;
		glVertex(p);
		}
	p[0]=columns[columns.size()-1].origin;
	glVertex(p);
	for(int i=columns.size()-2;i>=0;--i)
		{
		p[0]=columns[i].origin+columns[i].size;
		glVertex(p);
		p[0]=columns[i].origin;
		glVertex(p);
		}
	glVertex(getInterior().getCorner(0));
	glEnd();
	
	/* Draw the horizontal separators: */
	for(unsigned int i=1;i<rows.size();++i)
		{
		GLfloat y0=rows[i-1].origin;
		GLfloat y1=rows[i].origin+rows[i].size;
		glBegin(GL_QUAD_STRIP);
		for(unsigned int j=0;j<columns.size();++j)
			{
			GLfloat x=columns[j].origin;
			glVertex3f(x,y0,p[2]);
			glVertex3f(x,y1,p[2]);
			x+=columns[j].size;
			glVertex3f(x,y0,p[2]);
			glVertex3f(x,y1,p[2]);
			}
		glEnd();
		}
	
	/* Draw the vertical separators: */
	glBegin(GL_QUADS);
	for(unsigned int i=0;i<rows.size();++i)
		{
		GLfloat y0=rows[i].origin;
		GLfloat y1=y0+rows[i].size;
		for(unsigned int j=1;j<columns.size();++j)
			{
			GLfloat x0=columns[j-1].origin+columns[j-1].size;
			GLfloat x1=columns[j].origin;
			glVertex3f(x0,y0,p[2]);
			glVertex3f(x1,y0,p[2]);
			glVertex3f(x1,y1,p[2]);
			glVertex3f(x0,y1,p[2]);
			}
		}
	glEnd();
	
	/* Draw the children: */
	int numChildren=0;
	for(WidgetList::const_iterator chIt=children.begin();chIt!=children.end();++chIt,++numChildren)
		(*chIt)->draw(contextData);
	
	/* Are there leftover unfilled cells? */
	int minorIndex=numChildren%numMinorWidgets;
	if(minorIndex!=0)
		{
		int majorIndex=numChildren/numMinorWidgets;
		const GridCell* row;
		const GridCell* col;
		if(orientation==VERTICAL)
			{
			col=&columns[minorIndex];
			row=&rows[majorIndex];
			}
		else
			{
			col=&columns[majorIndex];
			row=&rows[minorIndex];
			}
		for(;minorIndex<numMinorWidgets;++minorIndex)
			{
			/* Fill the hole: */
			glColor(backgroundColor);
			glBegin(GL_QUADS);
			glNormal3f(0.0f,0.0f,1.0f);
			glVertex3f(col->origin,row->origin,p[2]);
			glVertex3f(col->origin+col->size,row->origin,p[2]);
			glVertex3f(col->origin+col->size,row->origin+row->size,p[2]);
			glVertex3f(col->origin,row->origin+row->size,p[2]);
			glEnd();
			if(orientation==VERTICAL)
				++col;
			else
				++row;
			}
		}
	}

bool RowColumn::findRecipient(Event& event)
	{
	/* Distribute the question to the child widgets: */
	bool childFound=false;
	for(WidgetList::iterator chIt=children.begin();!childFound&&chIt!=children.end();++chIt)
		childFound=(*chIt)->findRecipient(event);
	
	/* If no child was found, return ourselves (and ignore any incoming events): */
	if(childFound)
		return true;
	else
		{
		/* Check ourselves: */
		Event::WidgetPoint wp=event.calcWidgetPoint(this);
		if(isInside(wp.getPoint()))
			return event.setTargetWidget(this,wp);
		else
			return false;
		}
	}

void RowColumn::addChild(Widget* newChild)
	{
	/* Add the child to the list: */
	children.insert(children.begin()+nextChildIndex,newChild);
	nextChildIndex=GLint(children.size());
	
	/* Update the number of rows and columns: */
	size_t numMajors=(children.size()+numMinorWidgets-1)/numMinorWidgets;
	switch(orientation)
		{
		case VERTICAL:
			if(numMajors>rowWeights.size())
				rowWeights.push_back(0.0f);
			break;
		
		case HORIZONTAL:
			if(numMajors>columnWeights.size())
				columnWeights.push_back(0.0f);
			break;
		}
	
	if(isManaged)
		{
		/* Try to resize the widget to accomodate the new child: */
		parent->requestResize(this,calcNaturalSize());
		}
	}

void RowColumn::removeChild(Widget* removeChild)
	{
	/* Find the given widget in the list of children: */
	WidgetList::iterator chIt;
	GLsizei childIndex=0;
	for(chIt=children.begin();chIt!=children.end()&&*chIt!=removeChild;++chIt,++childIndex)
		;
	if(chIt!=children.end())
		{
		/* Remove the child from the list: */
		children.erase(chIt);
		
		/* Update the number of rows and columns: */
		GLint majorPos=GLint(childIndex%numMinorWidgets);
		size_t numMajors=(children.size()+numMinorWidgets-1)/numMinorWidgets;
		switch(orientation)
			{
			case VERTICAL:
				if(numMajors<rowWeights.size())
					rowWeights.erase(rowWeights.begin()+majorPos);
				break;
			
			case HORIZONTAL:
				if(numMajors<columnWeights.size())
					columnWeights.erase(columnWeights.begin()+majorPos);
				break;
			}
		
		/* Update the child insertion position: */
		if(nextChildIndex>GLint(children.size()))
			nextChildIndex=GLint(children.size());
		
		if(isManaged)
			{
			/* Try to resize the widget to calculate the new layout: */
			parent->requestResize(this,calcNaturalSize());
			}
		}
	}

void RowColumn::requestResize(Widget* child,const Vector& newExteriorSize)
	{
	/* Just grant the request if nothing really changed: */
	if(!isManaged)
		{
		/* Just resize the child: */
		child->resize(Box(child->getExterior().origin,newExteriorSize));
		}
	else if(newExteriorSize[0]==child->getExterior().size[0]&&newExteriorSize[1]==child->getExterior().size[1])
		{
		/* Resize the child in its previous box: */
		child->resize(child->getExterior());
		
		/* Invalidate the visual representation: */
		update();
		}
	else
		{
		/* Calculate the natural grid size: */
		std::vector<GLfloat> columnWidths,rowHeights;
		calcGrid(columnWidths,rowHeights);

		/* Adjust the grid size for the new child size: */
		if(packing==PACK_GRID)
			{
			if(rowHeights[0]<newExteriorSize[1])
				{
				/* Adjust all rows: */
				for(std::vector<GLfloat>::iterator rIt=rowHeights.begin();rIt!=rowHeights.end();++rIt)
					*rIt=newExteriorSize[1];
				}
			if(columnWidths[0]<newExteriorSize[0])
				{
				/* Adjust all columns: */
				for(std::vector<GLfloat>::iterator cIt=columnWidths.begin();cIt!=columnWidths.end();++cIt)
					*cIt=newExteriorSize[0];
				}
			}
		else
			{
			/* Find the child in the children list: */
			int childIndex=0;
			for(WidgetList::iterator chIt=children.begin();*chIt!=child;++chIt,++childIndex)
				;
			int rowIndex,columnIndex;
			if(orientation==VERTICAL)
				{
				columnIndex=childIndex%numMinorWidgets;
				rowIndex=childIndex/numMinorWidgets;
				}
			else
				{
				rowIndex=childIndex%numMinorWidgets;
				columnIndex=childIndex/numMinorWidgets;
				}

			/* Adjust the cell's size, if necessary: */
			if(rowHeights[rowIndex]<newExteriorSize[1])
				rowHeights[rowIndex]=newExteriorSize[1];
			if(columnWidths[columnIndex]<newExteriorSize[0])
				columnWidths[columnIndex]=newExteriorSize[0];
			}

		/* Calculate the new overall size: */
		Vector newSize(0.0f,0.0f,0.0f);
		for(std::vector<GLfloat>::iterator cIt=columnWidths.begin();cIt!=columnWidths.end();++cIt)
			newSize[0]+=*cIt;
		for(std::vector<GLfloat>::iterator rIt=rowHeights.begin();rIt!=rowHeights.end();++rIt)
			newSize[1]+=*rIt;
		newSize[0]+=2.0f*marginWidth+GLfloat(columnWidths.size()-1)*spacing;
		newSize[1]+=2.0f*marginWidth+GLfloat(rowHeights.size()-1)*spacing;
		
		/* Try to resize the widget: */
		parent->requestResize(this,calcExteriorSize(newSize));
		}
	}

Widget* RowColumn::getFirstChild(void)
	{
	if(children.empty())
		return 0;
	else
		return children.front();
	}

Widget* RowColumn::getNextChild(Widget* child)
	{
	/* Search the given widget in the children list: */
	WidgetList::iterator childIt;
	for(childIt=children.begin();childIt!=children.end();++childIt)
		if(*childIt==child)
			{
			/* Return the child after the found one: */
			++childIt;
			break;
			}
	
	/* If the iterator is valid, return its content; otherwise, return null: */
	if(childIt!=children.end())
		return *childIt;
	else
		return 0;
	}

void RowColumn::setOrientation(RowColumn::Orientation newOrientation)
	{
	/* Set the orientation: */
	orientation=newOrientation;
	
	/* Update the number of rows and columns: */
	switch(orientation)
		{
		case VERTICAL:
			{
			columnWeights.clear();
			columnWeights.insert(columnWeights.end(),numMinorWidgets,0.0f);
			rowWeights.clear();
			unsigned int numRows=(children.size()+numMinorWidgets-1)/numMinorWidgets;
			rowWeights.insert(rowWeights.end(),numRows,0.0f);
			break;
			}
		
		case HORIZONTAL:
			{
			rowWeights.clear();
			rowWeights.insert(rowWeights.end(),numMinorWidgets,0.0f);
			columnWeights.clear();
			unsigned int numColumns=(children.size()+numMinorWidgets-1)/numMinorWidgets;
			columnWeights.insert(columnWeights.end(),numColumns,0.0f);
			break;
			}
		}
	
	if(isManaged)
		{
		/* Try to resize the widget to accomodate the new setting: */
		parent->requestResize(this,calcNaturalSize());
		}
	}

void RowColumn::setPacking(RowColumn::Packing newPacking)
	{
	/* Set the packing strategy: */
	packing=newPacking;
	
	if(isManaged)
		{
		/* Try to resize the widget to accomodate the new setting: */
		parent->requestResize(this,calcNaturalSize());
		}
	}

void RowColumn::setAlignment(const Alignment& newAlignment)
	{
	/* Set the alignment: */
	alignment=newAlignment;
	
	if(isManaged)
		{
		/* Try to resize the widget to accomodate the new setting: */
		parent->requestResize(this,calcNaturalSize());
		}
	}

void RowColumn::setNumMinorWidgets(GLsizei newNumMinorWidgets)
	{
	/* Set the number of minor widgets: */
	numMinorWidgets=newNumMinorWidgets;
	
	/* Update the number of rows and columns: */
	switch(orientation)
		{
		case VERTICAL:
			{
			columnWeights.clear();
			columnWeights.insert(columnWeights.end(),numMinorWidgets,0.0f);
			rowWeights.clear();
			unsigned int numRows=(children.size()+numMinorWidgets-1)/numMinorWidgets;
			rowWeights.insert(rowWeights.end(),numRows,0.0f);
			break;
			}
		
		case HORIZONTAL:
			{
			rowWeights.clear();
			rowWeights.insert(rowWeights.end(),numMinorWidgets,0.0f);
			columnWeights.clear();
			unsigned int numColumns=(children.size()+numMinorWidgets-1)/numMinorWidgets;
			columnWeights.insert(columnWeights.end(),numColumns,0.0f);
			break;
			}
		}
	
	if(isManaged)
		{
		/* Try to resize the widget to accomodate the new setting: */
		parent->requestResize(this,calcNaturalSize());
		}
	}

void RowColumn::setNextChildIndex(GLint newNextChildIndex)
	{
	/* Set the insertion index: */
	if(newNextChildIndex>GLint(children.size()))
		newNextChildIndex=GLint(children.size());
	nextChildIndex=newNextChildIndex;
	}

void RowColumn::setMarginWidth(GLfloat newMarginWidth)
	{
	/* Set the margin width: */
	marginWidth=newMarginWidth;
	
	if(isManaged)
		{
		/* Try to resize the widget to accomodate the new setting: */
		parent->requestResize(this,calcNaturalSize());
		}
	}

void RowColumn::setSpacing(GLfloat newSpacing)
	{
	/* Set the spacing: */
	spacing=newSpacing;
	
	if(isManaged)
		{
		/* Try to resize the widget to accomodate the new setting: */
		parent->requestResize(this,calcNaturalSize());
		}
	}

void RowColumn::setColumnWeight(int columnIndex,GLfloat weight)
	{
	if(columnIndex>=0&&columnIndex<int(columnWeights.size()))
		columnWeights[columnIndex]=weight;
	}

void RowColumn::setRowWeight(int rowIndex,GLfloat weight)
	{
	if(rowIndex>=0&&rowIndex<int(rowWeights.size()))
		rowWeights[rowIndex]=weight;
	}

GLint RowColumn::getChildIndex(const Widget* child) const
	{
	GLint result;
	
	/* Find the child's index in the list: */
	WidgetList::const_iterator chIt;
	for(result=0,chIt=children.begin();chIt!=children.end()&&*chIt!=child;++result,++chIt)
		;
	if(chIt==children.end())
		result=-1;
	
	return result;
	}

GLint RowColumn::getChildColumn(const Widget* child) const
	{
	/* Find the child's index in the list: */
	GLint index;
	WidgetList::const_iterator chIt;
	for(index=0,chIt=children.begin();chIt!=children.end()&&*chIt!=child;++index,++chIt)
		;
	if(chIt==children.end())
		return -1;
	
	/* Calculate the column index: */
	GLint result=-1; // Just to make compiler happy
	switch(orientation)
		{
		case VERTICAL:
			result=index%numMinorWidgets;
			break;
		
		case HORIZONTAL:
			result=index/numMinorWidgets;
			break;
		}
	return result;
	}

GLint RowColumn::getChildRow(const Widget* child) const
	{
	/* Find the child's index in the list: */
	GLint index;
	WidgetList::const_iterator chIt;
	for(index=0,chIt=children.begin();chIt!=children.end()&&*chIt!=child;++index,++chIt)
		;
	if(chIt==children.end())
		return -1;
	
	/* Calculate the row index: */
	GLint result=-1; // Just to make compiler happy
	switch(orientation)
		{
		case VERTICAL:
			result=index/numMinorWidgets;
			break;
		
		case HORIZONTAL:
			result=index%numMinorWidgets;
			break;
		}
	return result;
	}

Widget* RowColumn::getChild(GLint childIndex)
	{
	if(childIndex>=0&&childIndex<GLint(children.size()))
		return children[childIndex];
	else
		return 0;
	}

void RowColumn::removeWidgets(GLint majorDirectionIndex)
	{
	/* Do nothing if the index is out of bounds: */
	if(majorDirectionIndex<0||majorDirectionIndex>=(GLint(children.size())+numMinorWidgets-1)/numMinorWidgets)
		return;
	
	/* Remember the size of the removed widgets: */
	int dimension=orientation==VERTICAL?1:0;
	GLfloat removedSize=0.0f;
	
	/* Remove an entire row or column worth of widgets: */
	int firstIndex=majorDirectionIndex*numMinorWidgets;
	int lastIndex=firstIndex+numMinorWidgets;
	if(lastIndex>int(children.size()))
		lastIndex=int(children.size());
	for(int i=firstIndex;i<lastIndex;++i)
		{
		/* Keep track of the widget's size change: */
		if(removedSize<children[i]->getExterior().size[dimension])
			removedSize=children[i]->getExterior().size[dimension];
		
		/* Unmanage and delete the child: */
		deleteChild(children[i]);
		}
	children.erase(children.begin()+firstIndex,children.begin()+lastIndex);
	
	/* Update the grid descriptor arrays: */
	switch(orientation)
		{
		case VERTICAL:
			rowWeights.erase(rowWeights.begin()+majorDirectionIndex);
			break;
		
		case HORIZONTAL:
			columnWeights.erase(columnWeights.begin()+majorDirectionIndex);
			break;
		}
	
	/* Update the child insertion position: */
	if(nextChildIndex>=firstIndex+numMinorWidgets)
		nextChildIndex-=numMinorWidgets;
	else if(nextChildIndex>firstIndex)
		nextChildIndex=firstIndex;
	
	if(isManaged)
		{
		/* Try to resize the widget to account for the removed child: */
		Vector oldSize=getInterior().size;
		oldSize[dimension]-=removedSize;
		if(!children.empty())
			oldSize[dimension]-=spacing;
		parent->requestResize(this,calcExteriorSize(oldSize));
		}
	}

}
