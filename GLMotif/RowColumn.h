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

#ifndef GLMOTIF_ROWCOLUMN_INCLUDED
#define GLMOTIF_ROWCOLUMN_INCLUDED

#include <vector>
#include <GLMotif/Alignment.h>
#include <GLMotif/Container.h>

namespace GLMotif {

class RowColumn:public Container
	{
	/* Embedded classes: */
	public:
	enum Orientation // Type to select the major orientation of children layout
		{
		VERTICAL,HORIZONTAL
		};
	
	enum Packing // Type to select the packing of children
		{
		PACK_TIGHT,PACK_GRID
		};
	
	protected:
	struct GridCell // Structure to store origin and size of grid cells
		{
		/* Elements: */
		public:
		GLfloat origin,size; // Origin and size of a grid cell
		
		/* Constructors and destructors: */
		GridCell(GLfloat sOrigin,GLfloat sSize)
			:origin(sOrigin),size(sSize)
			{
			}
		};
	
	typedef std::vector<Widget*> WidgetList; // Data type for list of child widgets
	
	/* Elements: */
	protected:
	Orientation orientation; // Major layout orientation of children
	Packing packing; // Packing strategy
	Alignment alignment; // Alignment of smaller grids inside larger frames
	GLsizei numMinorWidgets; // Number of widgets in minor layout orientation
	GLfloat marginWidth; // Width of margin around table
	GLfloat spacing; // Width of spacing between children
	std::vector<GLfloat> rowWeights,columnWeights; // Weights determining how extra space is distributed between the rows and columns in PACK_TIGHT packing strategy
	std::vector<GridCell> rows,columns; // Grid cell descriptors
	WidgetList children; // List of child widgets
	GLint nextChildIndex; // Index at which to insert the next child into the list
	
	/* Protected methods: */
	Vector calcGrid(std::vector<GLfloat>& columnWidths,std::vector<GLfloat>& rowHeights) const;
	
	/* Constructors and destructors: */
	public:
	RowColumn(const char* sName,Container* sParent,bool manageChild =true);
	virtual ~RowColumn(void);
	
	/* Methods inherited from Widget: */
	virtual Vector calcNaturalSize(void) const;
	virtual ZRange calcZRange(void) const;
	virtual void resize(const Box& newExterior);
	virtual void draw(GLContextData& contextData) const;
	virtual bool findRecipient(Event& event);
	
	/* Methods inherited from Container: */
	virtual void addChild(Widget* newChild);
	virtual void removeChild(Widget* removeChild);
	virtual void requestResize(Widget* child,const Vector& newExteriorSize);
	virtual Widget* getFirstChild(void);
	virtual Widget* getNextChild(Widget* child);
	
	/* New methods: */
	void setOrientation(Orientation newOrientation); // Changes the major layout direction
	void setPacking(Packing newPacking); // Changes packing strategy
	void setAlignment(const Alignment& newAlignment); // Changes the alignment of smaller grids inside larger frames
	void setNumMinorWidgets(GLsizei newNumMinorWidgets); // Changes the number of widgets in minor layout direction
	void setNextChildIndex(GLint newNextChildIndex); // Explicitly sets the index at which to insert the next child
	void setMarginWidth(GLfloat newMarginWidth); // Changes the margin width
	void setSpacing(GLfloat newSpacing); // Changes the spacing
	GLfloat getMarginWidth(void) const // Returns the margin width
		{
		return marginWidth;
		}
	GLfloat getSpacing(void) const // Returns the spacing
		{
		return spacing;
		}
	int getNumColumns(void) const // Returns the current number of columns in the grid
		{
		return columnWeights.size();
		}
	int getNumRows(void) const // Returns the current number of rows in the grid
		{
		return rowWeights.size();
		}
	void setColumnWeight(int columnIndex,GLfloat newColumnWeight); // Sets the expansion weight of the given column
	void setRowWeight(int rowIndex,GLfloat newRowWeight); // Sets the expansion weight of the given row
	virtual GLint getChildIndex(const Widget* child) const; // Returns the index of a child widget, -1 if not a child
	virtual GLint getChildColumn(const Widget* child) const; // Returns the index of the column containing the child widget, -1 if not a child
	virtual GLint getChildRow(const Widget* child) const; // Returns the index of the row containing the child widget, -1 if not a child
	virtual Widget* getChild(GLint childIndex); // Returns the child widget of the given index or 0 if index is invalid
	virtual void removeWidgets(GLint majorDirectionIndex); // Removes an entire row or column of widgets, if orientation is vertical or horizontal, respectively
	};

}

#endif
