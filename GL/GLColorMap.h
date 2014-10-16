/***********************************************************************
GLColorMap - Class to map from scalar values to RGBA colors.
Copyright (c) 1999-2014 Oliver Kreylos

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

#ifndef GLCOLORMAP_INCLUDED
#define GLCOLORMAP_INCLUDED

#include <GL/gl.h>
#include <GL/GLColor.h>

class GLColorMap
	{
	/* Embedded classes: */
	public:
	typedef GLColor<GLfloat,4> Color; // Type of color entries
	
	enum CreationTypes // Types for automatic palette generation
		{
		GREYSCALE=0x1,RAINBOW=0x2,
		CONSTANT_ALPHA=0x4,RAMP_ALPHA=0x8
		};
	
	/* Elements: */
	private:
	GLsizei numEntries; // Number of colors in the map
	Color* entries; // Array of RGBA entries
	GLdouble min,max; // The scalar value range
	GLdouble factor,offset; // The scaling factors to map data values to indices
	
	/* Private methods: */
	void setNumEntries(GLsizei newNumEntries); // Changes the color map's size
	void copyMap(GLsizei newNumEntries,const Color* newEntries,GLdouble newMin,GLdouble newMax); // Copies from another color map
	
	/* Constructors and destructors: */
	public:
	GLColorMap(void) // Creates an empty color map
		:numEntries(0),entries(0)
		{
		}
	GLColorMap(GLenum type,GLfloat alphaMax,GLfloat alphaGamma,GLdouble sMin,GLdouble sMax); // Creates a 256-entry standard color map
	GLColorMap(GLsizei sNumEntries,const Color* sEntries,GLdouble sMin,GLdouble sMax); // Creates a color map from a color array
	GLColorMap(GLsizei numKeys,const Color* colors,const GLdouble* keys,GLsizei sNumEntries =256); // Creates a color map from a piecewise linear color function
	GLColorMap(const char* fileName,GLdouble sMin,GLdouble sMax); // Loads a 256 entry palette from a file
	GLColorMap(const GLColorMap& source);
	GLColorMap& operator=(const GLColorMap& source);
	~GLColorMap(void);
	
	/* Methods: */
	GLColorMap& load(const char* fileName); // Loads a 256-entry color map from a file
	GLColorMap& setColors(GLsizei newNumEntries,const Color* newEntries); // Sets the color map array directly
	GLColorMap& setColors(GLsizei numKeys,const Color* colors,const GLdouble* keys,GLsizei sNumEntries =256); // Creates a color map from a piecewise linear color function
	void save(const char* fileName) const; // Saves a 256-entry color map to a file
	GLdouble getScalarRangeMin(void) const // Returns minimum of scalar value range
		{
		return min;
		}
	GLdouble getScalarRangeMax(void) const // Returns maximum of scalar value range
		{
		return max;
		}
	GLColorMap& setScalarRange(GLdouble newMin,GLdouble newMax); // Changes the scalar value range
	GLColorMap& changeTransparency(GLfloat gamma); // Applies a gamma function to the transparency values
	GLColorMap& premultiplyAlpha(void); // Converts the colors into premultiplied alpha format for easier compositing
	GLsizei getNumEntries(void) const // Returns the number of entries in the map
		{
		return numEntries;
		}
	const Color* getColors(void) const // Returns a pointer to the color entry array
		{
		return entries;
		}
	const Color& operator[](GLsizei index) const // Returns a color map entry
		{
		return entries[index];
		}
	Color operator()(GLdouble scalar) const; // Returns the color for a scalar value using linear interpolation
	};

#endif
