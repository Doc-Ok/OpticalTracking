/***********************************************************************
FieldTypes - Type declarations for node fields.
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

#ifndef SCENEGRAPH_FIELDTYPES_INCLUDED
#define SCENEGRAPH_FIELDTYPES_INCLUDED

#include <string>
#include <vector>
#include <Misc/Autopointer.h>
#include <GL/gl.h>
#include <GL/GLColor.h>
#include <SceneGraph/Geometry.h>

/* Forward declarations: */
namespace SceneGraph {
class Node;
}

namespace SceneGraph {

/**********************
Value type definitions:
**********************/

typedef double Time;
typedef GLColor<GLfloat,3> Color;

/*********************
Base class for fields:
*********************/

class Field // Base class for fields
	{
	};

/*****************************************
Templatized class for single-value fields:
*****************************************/

template <class ValueParam>
class SF:public Field
	{
	/* Embedded classes: */
	public:
	typedef ValueParam Value; // Field value type
	
	/* Elements: */
	private:
	Value value; // Field value
	
	/* Constructors and destructors: */
	public:
	SF(void) // Default constructor
		{
		}
	SF(const Value& sValue)
		:value(sValue)
		{
		}
	
	/* Methods: */
	const Value& getValue(void) const // Returns the field's value
		{
		return value;
		}
	void setValue(const Value& newValue) // Sets the field's value
		{
		value=newValue;
		}
	};

/****************************************
Templatized class for multi-value fields:
****************************************/

template <class ValueParam>
class MF:public Field
	{
	/* Embedded classes: */
	public:
	typedef ValueParam Value; // Field value type
	typedef std::vector<Value> ValueList; // Type for value lists
	
	/* Elements: */
	private:
	ValueList values; // List of field values
	
	/* Constructors and destructors: */
	public:
	MF(void) // Default constructor
		{
		}
	MF(const ValueParam& sValue) // Creates a single-valued field
		{
		values.push_back(sValue);
		}
	
	/* Methods: */
	const ValueList& getValues(void) const // Returns the field's value list
		{
		return values;
		}
	ValueList& getValues(void) // Ditto
		{
		return values;
		}
	size_t getNumValues(void) const // Returns the number of values in the field
		{
		return values.size();
		}
	const Value& getValue(size_t index) const // Returns the given value from the list
		{
		return values[index];
		}
	void setValue(const Value& newValue) // Sets the field to the single given value
		{
		values.clear();
		values.push_back(newValue);
		}
	void setValue(size_t index,const Value& newValue) // Sets one value in the list
		{
		values[index]=newValue;
		}
	void clearValues(void) // Removes all values from the list
		{
		values.clear();
		}
	void removeValue(size_t index) // Removes one value from the list
		{
		values.erase(values.begin()+index);
		}
	void removeValue(const Value& value) // Removes all instances of the given value from the list
		{
		for(typename ValueList::iterator vIt=values.begin();vIt!=values.end();)
			{
			if(*vIt==value)
				values.erase(vIt);
			else
				++vIt;
			}
		}
	void appendValue(const Value& newValue) // Appends a new value to the end of the list
		{
		values.push_back(newValue);
		}
	void insertValue(size_t index,const Value& newValue) // Inserts the given value before the current value at the given index
		{
		values.insert(values.begin()+index,newValue);
		}
	};

/**********************
Field type definitions:
**********************/

typedef SF<bool> SFBool;
typedef SF<std::string> SFString;
typedef SF<Time> SFTime;
typedef SF<int> SFInt;
typedef SF<Scalar> SFFloat;
typedef SF<Size> SFSize;
typedef SF<Point> SFPoint;
typedef SF<Vector> SFVector;
typedef SF<Rotation> SFRotation;
typedef SF<Color> SFColor;
typedef SF<TexCoord> SFTexCoord;
typedef SF<Misc::Autopointer<Node> > SFNode;
typedef MF<bool> MFBool;
typedef MF<std::string> MFString;
typedef MF<Time> MFTime;
typedef MF<int> MFInt;
typedef MF<Scalar> MFFloat;
typedef MF<Size> MFSize;
typedef MF<Point> MFPoint;
typedef MF<Vector> MFVector;
typedef MF<Rotation> MFRotation;
typedef MF<Color> MFColor;
typedef MF<TexCoord> MFTexCoord;
typedef MF<Misc::Autopointer<Node> > MFNode;

}

#endif
