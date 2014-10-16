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

#include <GL/GLColorMap.h>

#include <math.h>
#include <string.h>
#include <Misc/ThrowStdErr.h>
#include <Misc/Endianness.h>
#include <IO/File.h>
#include <IO/OpenFile.h>

/****************
Helper functions:
****************/

namespace Misc {

template <>
inline
void
swapEndianness(
	GLColor<GLfloat,4>& color)
	{
	swapEndianness(color.getRgba(),4);
	}

template <>
inline
void
swapEndianness(
	GLColor<GLfloat,4>* colors,
	size_t numColors)
	{
	for(size_t i=0;i<numColors;++i)
		swapEndianness(colors[i].getRgba(),4);
	}

}

/***************************
Methods of class GLColorMap:
***************************/

void GLColorMap::setNumEntries(GLsizei newNumEntries)
	{
	/* Check if number actually changed: */
	if(numEntries!=newNumEntries)
		{
		/* Reallocate entry array: */
		delete[] entries;
		numEntries=newNumEntries;
		entries=new Color[numEntries];
		
		/* Recalculate mapping factors: */
		factor=GLdouble(numEntries-1)/(max-min);
		offset=min*factor;
		}
	}

void GLColorMap::copyMap(GLsizei newNumEntries,const GLColorMap::Color* newEntries,GLdouble newMin,GLdouble newMax)
	{
	/* Set mapping range: */
	min=newMin;
	max=newMax;
	
	/* Set number of entries: */
	setNumEntries(newNumEntries);
	
	/* Copy map entries: */
	memcpy(entries,newEntries,numEntries*sizeof(Color));
	}

GLColorMap::GLColorMap(GLenum type,GLfloat alphaMax,GLfloat alphaGamma,GLdouble sMin,GLdouble sMax)
	:numEntries(0),entries(0),min(sMin),max(sMax)
	{
	/* Create entry array: */
	setNumEntries(256);
	
	/* Create the palette colors: */
	GLenum colorType=type&(GREYSCALE|RAINBOW);
	if(colorType==GREYSCALE)
		{
		for(GLsizei i=0;i<256;++i)
			{
			entries[i][0]=GLfloat(i)/255.0f;
			entries[i][1]=GLfloat(i)/255.0f;
			entries[i][2]=GLfloat(i)/255.0f;
			}
		}
	else if(colorType==RAINBOW)
		{
		for(GLsizei i=0;i<256;++i)
			{
			/* Create rainbow colors: */
			GLdouble rad=GLdouble(i)*(2.0*M_PI/256.0);
			if(rad<=2.0*M_PI/3.0)
				entries[i][0]=cos(0.75*rad);
			else if(rad>=4.0*M_PI/3.0)
				entries[i][0]=cos(0.75*(2.0*M_PI-rad));
			else
				entries[i][0]=0.0;
			entries[i][1]=sin(0.75*rad);
			if(entries[i][1]<0.0)
				entries[i][1]=0.0;
			entries[i][2]=sin(0.75*(rad-2.0*M_PI/3.0));
			if(entries[i][2]<0.0)
				entries[i][2]=0.0;
			}
		}
	
	/* Create the palette opacities: */
	GLenum alphaType=type&(CONSTANT_ALPHA|RAMP_ALPHA);
	if(alphaType==CONSTANT_ALPHA)
		{
		for(GLsizei i=0;i<256;++i)
			entries[i][3]=alphaMax;
		}
	else if(alphaType==RAMP_ALPHA)
		{
		double ag=double(alphaGamma);
		for(GLsizei i=0;i<256;++i)
			entries[i][3]=alphaMax*pow(double(i)/255.0,ag);
		}
	}

GLColorMap::GLColorMap(GLsizei sNumEntries,const GLColorMap::Color* sEntries,GLdouble sMin,GLdouble sMax)
	:numEntries(0),entries(0),min(sMin),max(sMax)
	{
	/* Copy entry array: */
	copyMap(sNumEntries,sEntries,sMin,sMax);
	}

GLColorMap::GLColorMap(GLsizei numKeys,const Color* colors,const GLdouble* keys,GLsizei sNumEntries)
	:numEntries(0),entries(0)
	{
	/* Create the color map: */
	setColors(numKeys,colors,keys,sNumEntries);
	}

GLColorMap::GLColorMap(const char* fileName,GLdouble sMin,GLdouble sMax)
	:numEntries(0),entries(0),min(sMin),max(sMax)
	{
	/* Load color map file: */
	load(fileName);
	}

GLColorMap::GLColorMap(const GLColorMap& source)
	:numEntries(0),entries(0)
	{
	/* Copy entry array: */
	copyMap(source.numEntries,source.entries,source.min,source.max);
	}

GLColorMap& GLColorMap::operator=(const GLColorMap& source)
	{
	if(this!=&source)
		{
		/* Copy entry array: */
		copyMap(source.numEntries,source.entries,source.min,source.max);
		}
	
	return *this;
	}

GLColorMap::~GLColorMap(void)
	{
	delete[] entries;
	}

GLColorMap& GLColorMap::load(const char* fileName)
	{
	/* Create entry array: */
	setNumEntries(256);
	
	/* Load the color entries from file: */
	IO::FilePtr file(IO::openFile(fileName));
	file->setEndianness(Misc::BigEndian);
	file->read(entries,numEntries);
	
	return *this;
	}

GLColorMap& GLColorMap::setColors(GLsizei newNumEntries,const GLColorMap::Color* newEntries)
	{
	/* Copy entry array: */
	copyMap(newNumEntries,newEntries,min,max);
	
	return *this;
	}

GLColorMap& GLColorMap::setColors(GLsizei numKeys,const Color* colors,const GLdouble* keys,GLsizei newNumEntries)
	{
	/* Set the color map range: */
	min=keys[0];
	max=keys[numKeys-1];
	
	/* Create entry array: */
	setNumEntries(newNumEntries);
	
	/* Evaluate the color function: */
	for(GLsizei i=0;i<numEntries;++i)
		{
		/* Calculate the key value for this color map entry: */
		GLdouble val=GLdouble(i)*(max-min)/GLdouble(numEntries-1)+min;
		
		/* Find the piecewise linear segment of the color function containing the key value using binary search: */
		GLsizei l=0;
		GLsizei r=numKeys;
		while(r-l>1)
			{
			/* Enforce the invariant keys[l]<=val<keys[r]: */
			GLsizei m=(l+r)>>1;
			if(keys[m]<=val)
				l=m;
			else
				r=m;
			}
		
		/* Evaluate the linear segment: */
		if(r<numEntries)
			{
			/* Interpolate linearly: */
			GLfloat w=GLfloat((val-keys[l])/(keys[r]-keys[l]));
			for(int j=0;j<4;++j)
				entries[i][j]=colors[l][j]*(1.0f-w)+colors[r][j]*w;
			}
		else
			{
			/* There is nothing to the right of the last key, so no need to interpolate: */
			entries[i]=colors[numKeys-1];
			}
		}
	
	return *this;
	}

void GLColorMap::save(const char* fileName) const
	{
	/* We only save 256-entry maps! */
	if(numEntries!=256)
		Misc::throwStdErr("GLColorMap::save: Attempt to save color map with wrong number of entries");
	
	/* Write color entries to file: */
	IO::FilePtr file(IO::openFile(fileName,IO::File::WriteOnly));
	file->setEndianness(Misc::BigEndian);
	file->write(entries,numEntries);
	}

GLColorMap& GLColorMap::setScalarRange(GLdouble newMin,GLdouble newMax)
	{
	min=newMin;
	max=newMax;
	factor=GLdouble(numEntries-1)/(max-min);
	offset=min*factor;
	
	return *this;
	}

GLColorMap& GLColorMap::changeTransparency(GLfloat gamma)
	{
	/* Change the transparencies (not opacities!): */
	double dg=double(gamma);
	for(GLsizei i=0;i<numEntries;++i)
		entries[i][3]=GLfloat(1.0-pow(1.0-double(entries[i][3]),dg));
	
	return *this;
	}

GLColorMap& GLColorMap::premultiplyAlpha(void)
	{
	/* Premultiply each entry: */
	for(GLsizei i=0;i<numEntries;++i)
		for(GLsizei j=0;j<3;++j)
			entries[i][j]*=entries[i][3];
	
	return *this;
	}

GLColorMap::Color GLColorMap::operator()(GLdouble scalar) const
	{
	/* Check for out-of-bounds arguments: */
	if(scalar<=min)
		return entries[0];
	else if(scalar>=max)
		return entries[numEntries-1];
	
	/* Calculate the base map index: */
	scalar=scalar*factor-offset;
	GLsizei index=GLsizei(floor(scalar));
	if(index==numEntries-1)
		--index;
	scalar-=GLdouble(index);
	Color result;
	for(GLsizei i=0;i<4;++i)
		result[i]=GLfloat(entries[index][i]*(1.0-scalar)+entries[index+1][i]*scalar);
	
	return result;
	}
