/***********************************************************************
EditableGrid - Data structure to represent 3D grids with editable data
values and interactive isosurface extraction.
Copyright (c) 2006-2013 Oliver Kreylos

This file is part of the Virtual Clay Editing Package.

The Virtual Clay Editing Package is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The Virtual Clay Editing Package is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Virtual Clay Editing Package; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#include "EditableGrid.h"

#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <Misc/SizedTypes.h>
#include <IO/File.h>
#include <GL/gl.h>
#include <GL/GLContextData.h>
#include <GL/GLExtensionManager.h>
#include <GL/Extensions/GLARBVertexBufferObject.h>

/***************************************
Methods of class EditableGrid::DataItem:
***************************************/

EditableGrid::DataItem::DataItem(void)
	{
	/* Check if the vertex buffer object extension is supported: */
	if(GLARBVertexBufferObject::isSupported())
		{
		/* Initialize the vertex buffer object extension: */
		GLARBVertexBufferObject::initExtension();
		
		/* Create the vertex buffer objects: */
		glGenBuffersARB(5,isoFragmentVertexBufferObjectIds);
		}
	else
		{
		for(int i=0;i<5;++i)
			isoFragmentVertexBufferObjectIds[i]=0;
		}
	for(int i=0;i<5;++i)
		isoFragmentVersions[i]=0;
	}

EditableGrid::DataItem::~DataItem(void)
	{
	bool haveBufferObjects=false;
	for(int i=0;i<5;++i)
		haveBufferObjects=haveBufferObjects||isoFragmentVertexBufferObjectIds[i]!=0;
	if(haveBufferObjects)
		glDeleteBuffersARB(5,isoFragmentVertexBufferObjectIds);
	}

/*************************************
Static elements of class EditableGrid:
*************************************/

const int EditableGrid::edgeVertexIndices[12][2]=
	{
	{0,1},{2,3},{4,5},{6,7},
	{0,2},{1,3},{4,6},{5,7},
	{0,4},{1,5},{2,6},{3,7}
	};

const int EditableGrid::edgeMasks[256]=
	{
	0x0000, 0x0111, 0x0221, 0x0330, 0x0412, 0x0503, 0x0633, 0x0722,
	0x0822, 0x0933, 0x0a03, 0x0b12, 0x0c30, 0x0d21, 0x0e11, 0x0f00,
	0x0144, 0x0055, 0x0365, 0x0274, 0x0556, 0x0447, 0x0777, 0x0666,
	0x0966, 0x0877, 0x0b47, 0x0a56, 0x0d74, 0x0c65, 0x0f55, 0x0e44,
	0x0284, 0x0395, 0x00a5, 0x01b4, 0x0696, 0x0787, 0x04b7, 0x05a6,
	0x0aa6, 0x0bb7, 0x0887, 0x0996, 0x0eb4, 0x0fa5, 0x0c95, 0x0d84,
	0x03c0, 0x02d1, 0x01e1, 0x00f0, 0x07d2, 0x06c3, 0x05f3, 0x04e2,
	0x0be2, 0x0af3, 0x09c3, 0x08d2, 0x0ff0, 0x0ee1, 0x0dd1, 0x0cc0,
	0x0448, 0x0559, 0x0669, 0x0778, 0x005a, 0x014b, 0x027b, 0x036a,
	0x0c6a, 0x0d7b, 0x0e4b, 0x0f5a, 0x0878, 0x0969, 0x0a59, 0x0b48,
	0x050c, 0x041d, 0x072d, 0x063c, 0x011e, 0x000f, 0x033f, 0x022e,
	0x0d2e, 0x0c3f, 0x0f0f, 0x0e1e, 0x093c, 0x082d, 0x0b1d, 0x0a0c,
	0x06cc, 0x07dd, 0x04ed, 0x05fc, 0x02de, 0x03cf, 0x00ff, 0x01ee,
	0x0eee, 0x0fff, 0x0ccf, 0x0dde, 0x0afc, 0x0bed, 0x08dd, 0x09cc,
	0x0788, 0x0699, 0x05a9, 0x04b8, 0x039a, 0x028b, 0x01bb, 0x00aa,
	0x0faa, 0x0ebb, 0x0d8b, 0x0c9a, 0x0bb8, 0x0aa9, 0x0999, 0x0888,
	0x0888, 0x0999, 0x0aa9, 0x0bb8, 0x0c9a, 0x0d8b, 0x0ebb, 0x0faa,
	0x00aa, 0x01bb, 0x028b, 0x039a, 0x04b8, 0x05a9, 0x0699, 0x0788,
	0x09cc, 0x08dd, 0x0bed, 0x0afc, 0x0dde, 0x0ccf, 0x0fff, 0x0eee,
	0x01ee, 0x00ff, 0x03cf, 0x02de, 0x05fc, 0x04ed, 0x07dd, 0x06cc,
	0x0a0c, 0x0b1d, 0x082d, 0x093c, 0x0e1e, 0x0f0f, 0x0c3f, 0x0d2e,
	0x022e, 0x033f, 0x000f, 0x011e, 0x063c, 0x072d, 0x041d, 0x050c,
	0x0b48, 0x0a59, 0x0969, 0x0878, 0x0f5a, 0x0e4b, 0x0d7b, 0x0c6a,
	0x036a, 0x027b, 0x014b, 0x005a, 0x0778, 0x0669, 0x0559, 0x0448,
	0x0cc0, 0x0dd1, 0x0ee1, 0x0ff0, 0x08d2, 0x09c3, 0x0af3, 0x0be2,
	0x04e2, 0x05f3, 0x06c3, 0x07d2, 0x00f0, 0x01e1, 0x02d1, 0x03c0,
	0x0d84, 0x0c95, 0x0fa5, 0x0eb4, 0x0996, 0x0887, 0x0bb7, 0x0aa6,
	0x05a6, 0x04b7, 0x0787, 0x0696, 0x01b4, 0x00a5, 0x0395, 0x0284,
	0x0e44, 0x0f55, 0x0c65, 0x0d74, 0x0a56, 0x0b47, 0x0877, 0x0966,
	0x0666, 0x0777, 0x0447, 0x0556, 0x0274, 0x0365, 0x0055, 0x0144,
	0x0f00, 0x0e11, 0x0d21, 0x0c30, 0x0b12, 0x0a03, 0x0933, 0x0822,
	0x0722, 0x0633, 0x0503, 0x0412, 0x0330, 0x0221, 0x0111, 0x0000
	};

const int EditableGrid::fragmentNumTriangles[256]=
	{
	0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 2,
	1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 3,
	1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 3,
	2, 3, 3, 2, 3, 4, 4, 3, 3, 4, 4, 3, 4, 5, 5, 2,
	1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 3,
	2, 3, 3, 4, 3, 2, 4, 3, 3, 4, 4, 5, 4, 3, 5, 2,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 4,
	3, 4, 4, 3, 4, 3, 5, 2, 4, 5, 5, 4, 5, 4, 2, 1,
	1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 3,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 4,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 2, 3, 4, 5, 3, 2,
	3, 4, 4, 3, 4, 5, 5, 4, 4, 5, 3, 2, 5, 2, 4, 1,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 2, 3, 3, 2,
	3, 4, 4, 5, 4, 3, 5, 4, 4, 5, 5, 2, 3, 2, 4, 1,
	3, 4, 4, 5, 4, 5, 5, 2, 4, 5, 3, 4, 3, 4, 2, 1,
	2, 3, 3, 2, 3, 2, 4, 1, 3, 4, 2, 1, 2, 1, 1, 0
	};

const int EditableGrid::triangleEdgeIndices[256][16]=
	{
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 0,  4,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 0,  9,  5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 5,  4,  8,  9,  5,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 4,  1, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 0,  1, 10,  8,  0, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 5,  0,  9,  1, 10,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 5,  1, 10,  5, 10,  9,  9, 10,  8, -1, -1, -1, -1, -1, -1, -1},
	{ 5, 11,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 0,  4,  8,  5, 11,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 9, 11,  1,  0,  9,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 1,  4,  8,  1,  8, 11, 11,  8,  9, -1, -1, -1, -1, -1, -1, -1},
	{ 4,  5, 11, 10,  4, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 0,  5, 11,  0, 11,  8,  8, 11, 10, -1, -1, -1, -1, -1, -1, -1},
	{ 4,  0,  9,  4,  9, 10, 10,  9, 11, -1, -1, -1, -1, -1, -1, -1},
	{ 9, 11,  8, 11, 10,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 2,  8,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 2,  0,  4,  6,  2,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 0,  9,  5,  8,  6,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 2,  9,  5,  2,  5,  6,  6,  5,  4, -1, -1, -1, -1, -1, -1, -1},
	{ 8,  6,  2,  4,  1, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{10,  6,  2, 10,  2,  1,  1,  2,  0, -1, -1, -1, -1, -1, -1, -1},
	{ 9,  5,  0,  8,  6,  2,  1, 10,  4, -1, -1, -1, -1, -1, -1, -1},
	{ 2, 10,  6,  9, 10,  2,  9,  1, 10,  9,  5,  1, -1, -1, -1, -1},
	{ 5, 11,  1,  8,  6,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 4,  6,  2,  4,  2,  0,  5, 11,  1, -1, -1, -1, -1, -1, -1, -1},
	{ 9, 11,  1,  9,  1,  0,  8,  6,  2, -1, -1, -1, -1, -1, -1, -1},
	{ 1,  9, 11,  1,  6,  9,  1,  4,  6,  6,  2,  9, -1, -1, -1, -1},
	{ 4,  5, 11,  4, 11, 10,  6,  2,  8, -1, -1, -1, -1, -1, -1, -1},
	{ 5, 11, 10,  5, 10,  2,  5,  2,  0,  6,  2, 10, -1, -1, -1, -1},
	{ 2,  8,  6,  9, 10,  0,  9, 11, 10, 10,  4,  0, -1, -1, -1, -1},
	{ 2, 10,  6,  2,  9, 10,  9, 11, 10, -1, -1, -1, -1, -1, -1, -1},
	{ 9,  2,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 9,  2,  7,  0,  4,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 0,  2,  7,  5,  0,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 8,  2,  7,  8,  7,  4,  4,  7,  5, -1, -1, -1, -1, -1, -1, -1},
	{ 9,  2,  7,  1, 10,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 0,  1, 10,  0, 10,  8,  2,  7,  9, -1, -1, -1, -1, -1, -1, -1},
	{ 0,  2,  7,  0,  7,  5,  1, 10,  4, -1, -1, -1, -1, -1, -1, -1},
	{ 1,  7,  5,  1,  8,  7,  1, 10,  8,  2,  7,  8, -1, -1, -1, -1},
	{ 5, 11,  1,  9,  2,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 4,  8,  0,  5, 11,  1,  2,  7,  9, -1, -1, -1, -1, -1, -1, -1},
	{ 7, 11,  1,  7,  1,  2,  2,  1,  0, -1, -1, -1, -1, -1, -1, -1},
	{ 1,  7, 11,  4,  7,  1,  4,  2,  7,  4,  8,  2, -1, -1, -1, -1},
	{11, 10,  4, 11,  4,  5,  9,  2,  7, -1, -1, -1, -1, -1, -1, -1},
	{ 2,  7,  9,  0,  5,  8,  8,  5, 11,  8, 11, 10, -1, -1, -1, -1},
	{ 7,  0,  2,  7, 10,  0,  7, 11, 10, 10,  4,  0, -1, -1, -1, -1},
	{ 7,  8,  2,  7, 11,  8, 11, 10,  8, -1, -1, -1, -1, -1, -1, -1},
	{ 9,  8,  6,  7,  9,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 9,  0,  4,  9,  4,  7,  7,  4,  6, -1, -1, -1, -1, -1, -1, -1},
	{ 0,  8,  6,  0,  6,  5,  5,  6,  7, -1, -1, -1, -1, -1, -1, -1},
	{ 5,  4,  7,  4,  6,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 6,  7,  9,  6,  9,  8,  4,  1, 10, -1, -1, -1, -1, -1, -1, -1},
	{ 9,  6,  7,  9,  1,  6,  9,  0,  1,  1, 10,  6, -1, -1, -1, -1},
	{ 1, 10,  4,  0,  8,  5,  5,  8,  6,  5,  6,  7, -1, -1, -1, -1},
	{10,  5,  1, 10,  6,  5,  6,  7,  5, -1, -1, -1, -1, -1, -1, -1},
	{ 9,  8,  6,  9,  6,  7, 11,  1,  5, -1, -1, -1, -1, -1, -1, -1},
	{11,  1,  5,  9,  0,  7,  7,  0,  4,  7,  4,  6, -1, -1, -1, -1},
	{ 8,  1,  0,  8,  7,  1,  8,  6,  7, 11,  1,  7, -1, -1, -1, -1},
	{ 1,  7, 11,  1,  4,  7,  4,  6,  7, -1, -1, -1, -1, -1, -1, -1},
	{ 9,  8,  7,  8,  6,  7, 11,  4,  5, 11, 10,  4, -1, -1, -1, -1},
	{ 7,  0,  6,  7,  9,  0,  6,  0, 10,  5, 11,  0, 10,  0, 11, -1},
	{10,  0, 11, 10,  4,  0, 11,  0,  7,  8,  6,  0,  7,  0,  6, -1},
	{10,  7, 11,  6,  7, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 6, 10,  3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 4,  8,  0, 10,  3,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 0,  9,  5, 10,  3,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 8,  9,  5,  8,  5,  4, 10,  3,  6, -1, -1, -1, -1, -1, -1, -1},
	{ 6,  4,  1,  3,  6,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 6,  8,  0,  6,  0,  3,  3,  0,  1, -1, -1, -1, -1, -1, -1, -1},
	{ 1,  3,  6,  1,  6,  4,  0,  9,  5, -1, -1, -1, -1, -1, -1, -1},
	{ 5,  1,  3,  5,  3,  8,  5,  8,  9,  8,  3,  6, -1, -1, -1, -1},
	{11,  1,  5,  3,  6, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 5, 11,  1,  4,  8,  0,  3,  6, 10, -1, -1, -1, -1, -1, -1, -1},
	{ 1,  0,  9,  1,  9, 11,  3,  6, 10, -1, -1, -1, -1, -1, -1, -1},
	{ 3,  6, 10,  1,  4, 11, 11,  4,  8, 11,  8,  9, -1, -1, -1, -1},
	{11,  3,  6, 11,  6,  5,  5,  6,  4, -1, -1, -1, -1, -1, -1, -1},
	{11,  3,  6,  5, 11,  6,  5,  6,  8,  5,  8,  0, -1, -1, -1, -1},
	{ 0,  6,  4,  0, 11,  6,  0,  9, 11,  3,  6, 11, -1, -1, -1, -1},
	{ 6, 11,  3,  6,  8, 11,  8,  9, 11, -1, -1, -1, -1, -1, -1, -1},
	{ 3,  2,  8, 10,  3,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 4, 10,  3,  4,  3,  0,  0,  3,  2, -1, -1, -1, -1, -1, -1, -1},
	{ 8, 10,  3,  8,  3,  2,  9,  5,  0, -1, -1, -1, -1, -1, -1, -1},
	{ 9,  3,  2,  9,  4,  3,  9,  5,  4, 10,  3,  4, -1, -1, -1, -1},
	{ 8,  4,  1,  8,  1,  2,  2,  1,  3, -1, -1, -1, -1, -1, -1, -1},
	{ 0,  1,  2,  2,  1,  3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 5,  0,  9,  1,  2,  4,  1,  3,  2,  2,  8,  4, -1, -1, -1, -1},
	{ 5,  2,  9,  5,  1,  2,  1,  3,  2, -1, -1, -1, -1, -1, -1, -1},
	{ 3,  2,  8,  3,  8, 10,  1,  5, 11, -1, -1, -1, -1, -1, -1, -1},
	{ 5, 11,  1,  4, 10,  0,  0, 10,  3,  0,  3,  2, -1, -1, -1, -1},
	{ 2,  8, 10,  2, 10,  3,  0,  9,  1,  1,  9, 11, -1, -1, -1, -1},
	{11,  4,  9, 11,  1,  4,  9,  4,  2, 10,  3,  4,  2,  4,  3, -1},
	{ 8,  4,  5,  8,  5,  3,  8,  3,  2,  3,  5, 11, -1, -1, -1, -1},
	{11,  0,  5, 11,  3,  0,  3,  2,  0, -1, -1, -1, -1, -1, -1, -1},
	{ 2,  4,  3,  2,  8,  4,  3,  4, 11,  0,  9,  4, 11,  4,  9, -1},
	{11,  2,  9,  3,  2, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 2,  7,  9,  6, 10,  3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 0,  4,  8,  2,  7,  9, 10,  3,  6, -1, -1, -1, -1, -1, -1, -1},
	{ 7,  5,  0,  7,  0,  2,  6, 10,  3, -1, -1, -1, -1, -1, -1, -1},
	{10,  3,  6,  8,  2,  4,  4,  2,  7,  4,  7,  5, -1, -1, -1, -1},
	{ 6,  4,  1,  6,  1,  3,  7,  9,  2, -1, -1, -1, -1, -1, -1, -1},
	{ 9,  2,  7,  0,  3,  8,  0,  1,  3,  3,  6,  8, -1, -1, -1, -1},
	{ 4,  1,  3,  4,  3,  6,  5,  0,  7,  7,  0,  2, -1, -1, -1, -1},
	{ 3,  8,  1,  3,  6,  8,  1,  8,  5,  2,  7,  8,  5,  8,  7, -1},
	{ 9,  2,  7, 11,  1,  5,  6, 10,  3, -1, -1, -1, -1, -1, -1, -1},
	{ 3,  6, 10,  5, 11,  1,  0,  4,  8,  2,  7,  9, -1, -1, -1, -1},
	{ 6, 10,  3,  7, 11,  2,  2, 11,  1,  2,  1,  0, -1, -1, -1, -1},
	{ 4,  8,  2,  4,  2,  7,  4,  7,  1, 11,  1,  7, 10,  3,  6, -1},
	{ 9,  2,  7, 11,  3,  5,  5,  3,  6,  5,  6,  4, -1, -1, -1, -1},
	{ 5, 11,  3,  5,  3,  6,  5,  6,  0,  8,  0,  6,  9,  2,  7, -1},
	{ 2, 11,  0,  2,  7, 11,  0, 11,  4,  3,  6, 11,  4, 11,  6, -1},
	{ 6, 11,  3,  6,  8, 11,  7, 11,  2,  2, 11,  8, -1, -1, -1, -1},
	{ 3,  7,  9,  3,  9, 10, 10,  9,  8, -1, -1, -1, -1, -1, -1, -1},
	{ 4, 10,  3,  0,  4,  3,  0,  3,  7,  0,  7,  9, -1, -1, -1, -1},
	{ 0,  8, 10,  0, 10,  7,  0,  7,  5,  7, 10,  3, -1, -1, -1, -1},
	{ 3,  4, 10,  3,  7,  4,  7,  5,  4, -1, -1, -1, -1, -1, -1, -1},
	{ 7,  9,  8,  7,  8,  1,  7,  1,  3,  4,  1,  8, -1, -1, -1, -1},
	{ 9,  3,  7,  9,  0,  3,  0,  1,  3, -1, -1, -1, -1, -1, -1, -1},
	{ 5,  8,  7,  5,  0,  8,  7,  8,  3,  4,  1,  8,  3,  8,  1, -1},
	{ 5,  3,  7,  1,  3,  5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 5, 11,  1,  9, 10,  7,  9,  8, 10, 10,  3,  7, -1, -1, -1, -1},
	{ 0,  4, 10,  0, 10,  3,  0,  3,  9,  7,  9,  3,  5, 11,  1, -1},
	{10,  7,  8, 10,  3,  7,  8,  7,  0, 11,  1,  7,  0,  7,  1, -1},
	{ 3,  4, 10,  3,  7,  4,  1,  4, 11, 11,  4,  7, -1, -1, -1, -1},
	{ 5,  3,  4,  5, 11,  3,  4,  3,  8,  7,  9,  3,  8,  3,  9, -1},
	{11,  0,  5, 11,  3,  0,  9,  0,  7,  7,  0,  3, -1, -1, -1, -1},
	{ 0,  8,  4,  7, 11,  3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{11,  3,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{11,  7,  3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 0,  4,  8,  7,  3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 9,  5,  0,  7,  3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 5,  4,  8,  5,  8,  9,  7,  3, 11, -1, -1, -1, -1, -1, -1, -1},
	{ 1, 10,  4, 11,  7,  3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{10,  8,  0, 10,  0,  1, 11,  7,  3, -1, -1, -1, -1, -1, -1, -1},
	{ 0,  9,  5,  1, 10,  4,  7,  3, 11, -1, -1, -1, -1, -1, -1, -1},
	{ 7,  3, 11,  5,  1,  9,  9,  1, 10,  9, 10,  8, -1, -1, -1, -1},
	{ 5,  7,  3,  1,  5,  3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 5,  7,  3,  5,  3,  1,  4,  8,  0, -1, -1, -1, -1, -1, -1, -1},
	{ 9,  7,  3,  9,  3,  0,  0,  3,  1, -1, -1, -1, -1, -1, -1, -1},
	{ 7,  8,  9,  7,  1,  8,  7,  3,  1,  4,  8,  1, -1, -1, -1, -1},
	{ 3, 10,  4,  3,  4,  7,  7,  4,  5, -1, -1, -1, -1, -1, -1, -1},
	{ 0, 10,  8,  0,  7, 10,  0,  5,  7,  7,  3, 10, -1, -1, -1, -1},
	{ 4,  3, 10,  0,  3,  4,  0,  7,  3,  0,  9,  7, -1, -1, -1, -1},
	{ 3,  9,  7,  3, 10,  9, 10,  8,  9, -1, -1, -1, -1, -1, -1, -1},
	{ 7,  3, 11,  2,  8,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 2,  0,  4,  2,  4,  6,  3, 11,  7, -1, -1, -1, -1, -1, -1, -1},
	{ 5,  0,  9,  7,  3, 11,  8,  6,  2, -1, -1, -1, -1, -1, -1, -1},
	{11,  7,  3,  5,  6,  9,  5,  4,  6,  6,  2,  9, -1, -1, -1, -1},
	{ 4,  1, 10,  6,  2,  8, 11,  7,  3, -1, -1, -1, -1, -1, -1, -1},
	{ 7,  3, 11,  2,  1,  6,  2,  0,  1,  1, 10,  6, -1, -1, -1, -1},
	{ 0,  9,  5,  2,  8,  6,  1, 10,  4,  7,  3, 11, -1, -1, -1, -1},
	{ 9,  5,  1,  9,  1, 10,  9, 10,  2,  6,  2, 10,  7,  3, 11, -1},
	{ 3,  1,  5,  3,  5,  7,  2,  8,  6, -1, -1, -1, -1, -1, -1, -1},
	{ 5,  7,  1,  7,  3,  1,  4,  2,  0,  4,  6,  2, -1, -1, -1, -1},
	{ 8,  6,  2,  9,  7,  0,  0,  7,  3,  0,  3,  1, -1, -1, -1, -1},
	{ 6,  9,  4,  6,  2,  9,  4,  9,  1,  7,  3,  9,  1,  9,  3, -1},
	{ 8,  6,  2,  4,  7, 10,  4,  5,  7,  7,  3, 10, -1, -1, -1, -1},
	{ 7, 10,  5,  7,  3, 10,  5, 10,  0,  6,  2, 10,  0, 10,  2, -1},
	{ 0,  9,  7,  0,  7,  3,  0,  3,  4, 10,  4,  3,  8,  6,  2, -1},
	{ 3,  9,  7,  3, 10,  9,  2,  9,  6,  6,  9, 10, -1, -1, -1, -1},
	{11,  9,  2,  3, 11,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 2,  3, 11,  2, 11,  9,  0,  4,  8, -1, -1, -1, -1, -1, -1, -1},
	{11,  5,  0, 11,  0,  3,  3,  0,  2, -1, -1, -1, -1, -1, -1, -1},
	{ 8,  5,  4,  8,  3,  5,  8,  2,  3,  3, 11,  5, -1, -1, -1, -1},
	{11,  9,  2, 11,  2,  3, 10,  4,  1, -1, -1, -1, -1, -1, -1, -1},
	{ 0,  1,  8,  1, 10,  8,  2, 11,  9,  2,  3, 11, -1, -1, -1, -1},
	{ 4,  1, 10,  0,  3,  5,  0,  2,  3,  3, 11,  5, -1, -1, -1, -1},
	{ 3,  5,  2,  3, 11,  5,  2,  5,  8,  1, 10,  5,  8,  5, 10, -1},
	{ 5,  9,  2,  5,  2,  1,  1,  2,  3, -1, -1, -1, -1, -1, -1, -1},
	{ 4,  8,  0,  5,  9,  1,  1,  9,  2,  1,  2,  3, -1, -1, -1, -1},
	{ 0,  2,  1,  2,  3,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 8,  1,  4,  8,  2,  1,  2,  3,  1, -1, -1, -1, -1, -1, -1, -1},
	{ 9,  2,  3,  9,  3,  4,  9,  4,  5, 10,  4,  3, -1, -1, -1, -1},
	{ 8,  5, 10,  8,  0,  5, 10,  5,  3,  9,  2,  5,  3,  5,  2, -1},
	{ 4,  3, 10,  4,  0,  3,  0,  2,  3, -1, -1, -1, -1, -1, -1, -1},
	{ 3,  8,  2, 10,  8,  3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 6,  3, 11,  6, 11,  8,  8, 11,  9, -1, -1, -1, -1, -1, -1, -1},
	{ 0,  4,  6,  0,  6, 11,  0, 11,  9,  3, 11,  6, -1, -1, -1, -1},
	{11,  6,  3,  5,  6, 11,  5,  8,  6,  5,  0,  8, -1, -1, -1, -1},
	{11,  6,  3, 11,  5,  6,  5,  4,  6, -1, -1, -1, -1, -1, -1, -1},
	{ 1, 10,  4, 11,  8,  3, 11,  9,  8,  8,  6,  3, -1, -1, -1, -1},
	{ 1,  6,  0,  1, 10,  6,  0,  6,  9,  3, 11,  6,  9,  6, 11, -1},
	{ 5,  0,  8,  5,  8,  6,  5,  6, 11,  3, 11,  6,  1, 10,  4, -1},
	{10,  5,  1, 10,  6,  5, 11,  5,  3,  3,  5,  6, -1, -1, -1, -1},
	{ 5,  3,  1,  5,  8,  3,  5,  9,  8,  8,  6,  3, -1, -1, -1, -1},
	{ 1,  9,  3,  1,  5,  9,  3,  9,  6,  0,  4,  9,  6,  9,  4, -1},
	{ 6,  0,  8,  6,  3,  0,  3,  1,  0, -1, -1, -1, -1, -1, -1, -1},
	{ 6,  1,  4,  3,  1,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 8,  3,  9,  8,  6,  3,  9,  3,  5, 10,  4,  3,  5,  3,  4, -1},
	{ 0,  5,  9, 10,  6,  3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 6,  0,  8,  6,  3,  0,  4,  0, 10, 10,  0,  3, -1, -1, -1, -1},
	{ 6,  3, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{10, 11,  7,  6, 10,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{10, 11,  7, 10,  7,  6,  8,  0,  4, -1, -1, -1, -1, -1, -1, -1},
	{ 7,  6, 10,  7, 10, 11,  5,  0,  9, -1, -1, -1, -1, -1, -1, -1},
	{11,  7,  6, 11,  6, 10,  9,  5,  8,  8,  5,  4, -1, -1, -1, -1},
	{ 1, 11,  7,  1,  7,  4,  4,  7,  6, -1, -1, -1, -1, -1, -1, -1},
	{ 8,  0,  1,  8,  1,  7,  8,  7,  6, 11,  7,  1, -1, -1, -1, -1},
	{ 9,  5,  0,  7,  4, 11,  7,  6,  4,  4,  1, 11, -1, -1, -1, -1},
	{ 9,  1,  8,  9,  5,  1,  8,  1,  6, 11,  7,  1,  6,  1,  7, -1},
	{10,  1,  5, 10,  5,  6,  6,  5,  7, -1, -1, -1, -1, -1, -1, -1},
	{ 0,  4,  8,  5,  6,  1,  5,  7,  6,  6, 10,  1, -1, -1, -1, -1},
	{ 9,  7,  6,  9,  6,  1,  9,  1,  0,  1,  6, 10, -1, -1, -1, -1},
	{ 6,  1,  7,  6, 10,  1,  7,  1,  9,  4,  8,  1,  9,  1,  8, -1},
	{ 5,  7,  4,  4,  7,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 0,  6,  8,  0,  5,  6,  5,  7,  6, -1, -1, -1, -1, -1, -1, -1},
	{ 9,  4,  0,  9,  7,  4,  7,  6,  4, -1, -1, -1, -1, -1, -1, -1},
	{ 9,  6,  8,  7,  6,  9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 7,  2,  8,  7,  8, 11, 11,  8, 10, -1, -1, -1, -1, -1, -1, -1},
	{ 7,  2,  0,  7,  0, 10,  7, 10, 11, 10,  0,  4, -1, -1, -1, -1},
	{ 0,  9,  5,  8, 11,  2,  8, 10, 11, 11,  7,  2, -1, -1, -1, -1},
	{11,  2, 10, 11,  7,  2, 10,  2,  4,  9,  5,  2,  4,  2,  5, -1},
	{ 1, 11,  7,  4,  1,  7,  4,  7,  2,  4,  2,  8, -1, -1, -1, -1},
	{ 7,  1, 11,  7,  2,  1,  2,  0,  1, -1, -1, -1, -1, -1, -1, -1},
	{ 4,  1, 11,  4, 11,  7,  4,  7,  8,  2,  8,  7,  0,  9,  5, -1},
	{ 7,  1, 11,  7,  2,  1,  5,  1,  9,  9,  1,  2, -1, -1, -1, -1},
	{ 1,  5,  7,  1,  7,  8,  1,  8, 10,  2,  8,  7, -1, -1, -1, -1},
	{ 0, 10,  2,  0,  4, 10,  2, 10,  7,  1,  5, 10,  7, 10,  5, -1},
	{ 0,  7,  1,  0,  9,  7,  1,  7, 10,  2,  8,  7, 10,  7,  8, -1},
	{ 9,  7,  2,  1,  4, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 8,  7,  2,  8,  4,  7,  4,  5,  7, -1, -1, -1, -1, -1, -1, -1},
	{ 0,  7,  2,  5,  7,  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 8,  7,  2,  8,  4,  7,  9,  7,  0,  0,  7,  4, -1, -1, -1, -1},
	{ 9,  7,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 2,  6, 10,  2, 10,  9,  9, 10, 11, -1, -1, -1, -1, -1, -1, -1},
	{ 0,  4,  8,  2,  6,  9,  9,  6, 10,  9, 10, 11, -1, -1, -1, -1},
	{ 5, 10, 11,  5,  2, 10,  5,  0,  2,  6, 10,  2, -1, -1, -1, -1},
	{ 4,  2,  5,  4,  8,  2,  5,  2, 11,  6, 10,  2, 11,  2, 10, -1},
	{ 1, 11,  9,  1,  9,  6,  1,  6,  4,  6,  9,  2, -1, -1, -1, -1},
	{ 9,  6, 11,  9,  2,  6, 11,  6,  1,  8,  0,  6,  1,  6,  0, -1},
	{ 4, 11,  6,  4,  1, 11,  6, 11,  2,  5,  0, 11,  2, 11,  0, -1},
	{ 5,  1, 11,  8,  2,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 2,  6, 10,  9,  2, 10,  9, 10,  1,  9,  1,  5, -1, -1, -1, -1},
	{ 9,  2,  6,  9,  6, 10,  9, 10,  5,  1,  5, 10,  0,  4,  8, -1},
	{10,  2,  6, 10,  1,  2,  1,  0,  2, -1, -1, -1, -1, -1, -1, -1},
	{10,  2,  6, 10,  1,  2,  8,  2,  4,  4,  2,  1, -1, -1, -1, -1},
	{ 2,  5,  9,  2,  6,  5,  6,  4,  5, -1, -1, -1, -1, -1, -1, -1},
	{ 2,  5,  9,  2,  6,  5,  0,  5,  8,  8,  5,  6, -1, -1, -1, -1},
	{ 2,  4,  0,  6,  4,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 2,  6,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 9,  8, 11, 11,  8, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 4,  9,  0,  4, 10,  9, 10, 11,  9, -1, -1, -1, -1, -1, -1, -1},
	{ 0, 11,  5,  0,  8, 11,  8, 10, 11, -1, -1, -1, -1, -1, -1, -1},
	{ 4, 11,  5, 10, 11,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 1,  8,  4,  1, 11,  8, 11,  9,  8, -1, -1, -1, -1, -1, -1, -1},
	{ 9,  1, 11,  0,  1,  9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 1,  8,  4,  1, 11,  8,  0,  8,  5,  5,  8, 11, -1, -1, -1, -1},
	{ 5,  1, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 5, 10,  1,  5,  9, 10,  9,  8, 10, -1, -1, -1, -1, -1, -1, -1},
	{ 4,  9,  0,  4, 10,  9,  5,  9,  1,  1,  9, 10, -1, -1, -1, -1},
	{ 0, 10,  1,  8, 10,  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 4, 10,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 5,  8,  4,  9,  8,  5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 0,  5,  9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 0,  8,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}
	};

/*****************************
Methods of class EditableGrid:
*****************************/

EditableGrid::EditableGrid(const EditableGrid::Index& sNumVertices,const EditableGrid::Size& sCellSize)
	:numVertices(sNumVertices),
	 numCells(numVertices-Index(1,1,1)),
	 cellSize(sCellSize),
	 vertices(numVertices),
	 cells(numCells)
	{
	/* Initialize the vertex array: */
	for(VertexArray::iterator vIt=vertices.begin();vIt!=vertices.end();++vIt)
		{
		vIt->value=0.0f;
		vIt->gradient=VertexGradient::zero;
		}
	for(int i=0;i<3;++i)
		vertexStrides[i]=numVertices.calcIncrement(i);
	
	/* Initialize the cell array: */
	for(CellArray::iterator cIt=cells.begin();cIt!=cells.end();++cIt)
		{
		cIt->numTriangles=0;
		cIt->triangleOffset=0;
		}
	for(int vertexIndex=0;vertexIndex<8;++vertexIndex)
		{
		cellVertexOffsets[vertexIndex]=0;
		for(int i=0;i<3;++i)
			if(vertexIndex&(1<<i))
				cellVertexOffsets[vertexIndex]+=vertexStrides[i];
		}
	
	for(int i=0;i<3;++i)
		gradientScale[i]=0.5f/cellSize[i];
	
	/* Initialize the isosurface fragment cache: */
	for(int i=0;i<5;++i)
		isoFragmentVersions[i]=1;
	}

EditableGrid::~EditableGrid(void)
	{
	/* Nothing to do... */
	}

float EditableGrid::getValue(const EditableGrid::Point& p) const
	{
	/* Calculate the index of the cell containing the point and the point's offset inside the cell: */
	Index cell;
	float offset[3];
	for(int i=0;i<3;++i)
		{
		float pi=p[i]/cellSize[i];
		cell[i]=int(Math::floor(pi));
		offset[i]=pi-float(cell[i]);
		if(cell[i]<0)
			{
			cell[i]=0;
			offset[i]=0.0f;
			}
		else if(cell[i]>numCells[i]-1)
			{
			cell[i]=numCells[i]-1;
			offset[i]=1.0f;
			}
		}
	const Vertex* cellPtr=vertices.getAddress(cell);
	
	/* Interpolate the data value: */
	float v[4];
	v[0]=cellPtr[cellVertexOffsets[0]].value*(1.0f-offset[0])+cellPtr[cellVertexOffsets[1]].value*offset[0];
	v[1]=cellPtr[cellVertexOffsets[2]].value*(1.0f-offset[0])+cellPtr[cellVertexOffsets[3]].value*offset[0];
	v[2]=cellPtr[cellVertexOffsets[4]].value*(1.0f-offset[0])+cellPtr[cellVertexOffsets[5]].value*offset[0];
	v[3]=cellPtr[cellVertexOffsets[6]].value*(1.0f-offset[0])+cellPtr[cellVertexOffsets[7]].value*offset[0];
	
	v[0]=v[0]*(1.0f-offset[1])+v[1]*offset[1];
	v[2]=v[2]*(1.0f-offset[1])+v[3]*offset[1];
	
	return v[0]*(1.0f-offset[2])+v[2]*offset[2];
	}

void EditableGrid::invalidateVertices(const EditableGrid::Index& min,const EditableGrid::Index& max)
	{
	/* Recalculate all gradients inside the affected area: */
	Index gMin,gMax;
	for(int i=0;i<3;++i)
		{
		gMin[i]=min[i]>0?min[i]-1:0;
		gMax[i]=max[i]<numVertices[i]?max[i]+1:numVertices[i];
		}
	
	for(Index v=gMin;v[0]<gMax[0];v.preInc(gMin,gMax))
		{
		Vertex* vertex=vertices.getAddress(v);
		for(int i=0;i<3;++i)
			{
			if(v[i]==0)
				{
				const Vertex* left=vertex+vertexStrides[i];
				const Vertex* right=left+vertexStrides[i];
				vertex->gradient[i]=(-3.0f*vertex->value+4.0f*left->value-right->value)*gradientScale[i];
				}
			else if(v[i]==numVertices[i]-1)
				{
				const Vertex* right=vertex-vertexStrides[i];
				const Vertex* left=right-vertexStrides[i];
				vertex->gradient[i]=(left->value-4.0f*right->value+3.0f*vertex->value)*gradientScale[i];
				}
			else
				{
				const Vertex* left=vertex-vertexStrides[i];
				const Vertex* right=vertex+vertexStrides[i];
				vertex->gradient[i]=(right->value-left->value)*gradientScale[i];
				}
			}
		}
	
	/* Regenerate the isosurface fragments of all affected cells: */
	Index cMin,cMax;
	for(int i=0;i<3;++i)
		{
		cMin[i]=min[i]>1?min[i]-2:0;
		cMax[i]=max[i]<numCells[i]?max[i]+1:numCells[i];
		}
	
	for(Index c=cMin;c[0]<cMax[0];c.preInc(cMin,cMax))
		{
		/* Get a pointer to the cell's base vertex and calculate the base vertex' position: */
		const Vertex* baseVertex=vertices.getAddress(c);
		
		/* Determine the cell's marching cubes case index: */
		int caseIndex=0x0;
		for(int i=0;i<8;++i)
			if(baseVertex[cellVertexOffsets[i]].value>=0.5f)
				caseIndex|=1<<i;
		
		IsosurfaceVertex edgeVertices[12];
		if(fragmentNumTriangles[caseIndex]!=0)
			{
			/* Calculate the position of the cell's base vertex: */
			float basePoint[3];
			for(int i=0;i<3;++i)
				basePoint[i]=float(c[i])*cellSize[i];
			
			/* Calculate the edge intersection points and normal vectors: */
			IsosurfaceVertex* evPtr=edgeVertices;
			int cem=edgeMasks[caseIndex];
			for(int edge=0;edge<12;++edge,++evPtr)
				{
				if(cem&(1<<edge))
					{
					/* Calculate intersection point on the edge: */
					const Vertex* v0=&baseVertex[cellVertexOffsets[edgeVertexIndices[edge][0]]];
					const Vertex* v1=&baseVertex[cellVertexOffsets[edgeVertexIndices[edge][1]]];
					float w1=(0.5f-v0->value)/(v1->value-v0->value);
					for(int i=0;i<3;++i)
						{
						evPtr->position[i]=basePoint[i];
						if(edgeVertexIndices[edge][0]&(1<<i))
							evPtr->position[i]+=cellSize[i];
						}
					int edgeDim=edge>>2;
					evPtr->position[edgeDim]+=cellSize[edgeDim]*w1;
					
					// float mag2=0.0f;
					for(int i=0;i<3;++i)
						{
						evPtr->normal[i]=v0->gradient[i]*(w1-1.0f)-v1->gradient[i]*w1;
						// mag2+=Math::sqr(evPtr->normal[i]);
						}
					#if 0
					mag2=Math::sqrt(mag2);
					for(int i=0;i<3;++i)
						evPtr->normal[i]/=mag2;
					#endif
					}
				}
			}
		
		/* Prepare a slot to store the resulting isosurface fragment: */
		if(fragmentNumTriangles[caseIndex]!=cells(c).numTriangles)
			{
			unsigned int cellIndex=cells.calcLinearIndex(c);
			
			if(cells(c).numTriangles!=0)
				{
				/* Delete the old fragment from its list: */
				std::vector<unsigned int>& ifo=isoFragmentOwners[cells(c).numTriangles-1];
				unsigned int lastFragmentIndex=ifo.size()-1;
				cells.getArray()[ifo[lastFragmentIndex]].triangleOffset=cells(c).triangleOffset;
				ifo[cells(c).triangleOffset]=ifo[lastFragmentIndex];
				ifo.pop_back();
				std::vector<IsosurfaceVertex>& iff=isoFragments[cells(c).numTriangles-1];
				for(int i=0;i<cells(c).numTriangles*3;++i)
					iff[cells(c).triangleOffset*cells(c).numTriangles*3+i]=iff[lastFragmentIndex*cells(c).numTriangles*3+i];
				for(int i=0;i<cells(c).numTriangles*3;++i)
					iff.pop_back();
				}
			
			cells(c).numTriangles=fragmentNumTriangles[caseIndex];
			
			if(cells(c).numTriangles!=0)
				{
				/* Add the new fragment to its list: */
				std::vector<unsigned int>& ifo=isoFragmentOwners[cells(c).numTriangles-1];
				cells(c).triangleOffset=ifo.size();
				ifo.push_back(cellIndex);
				std::vector<IsosurfaceVertex>& iff=isoFragments[cells(c).numTriangles-1];
				for(int i=0;i<cells(c).numTriangles*3;++i)
					iff.push_back(IsosurfaceVertex());
				}
			}
		
		if(cells(c).numTriangles!=0)
			{
			IsosurfaceVertex* isvPtr=&isoFragments[cells(c).numTriangles-1][cells(c).triangleOffset*cells(c).numTriangles*3];
			
			/* Store the resulting fragment in the isosurface: */
			for(const int* ctei=triangleEdgeIndices[caseIndex];*ctei>=0;ctei+=3)
				{
				for(int i=0;i<3;++i,++isvPtr)
					*isvPtr=edgeVertices[ctei[i]];
				}
			}
		}
	
	/* Invalidate the isosurface fragment cache: */
	for(int i=0;i<5;++i)
		++isoFragmentVersions[i];
	}

void EditableGrid::initContext(GLContextData& contextData) const
	{
	/* Create a data item: */
	DataItem* dataItem=new DataItem();
	contextData.addDataItem(this,dataItem);
	}

void EditableGrid::glRenderAction(GLContextData& contextData) const
	{
	/* Retrieve the data item: */
	DataItem* dataItem=contextData.retrieveDataItem<DataItem>(this);
	
	/* Update and render all isofragment caches: */
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	for(int listIndex=0;listIndex<5;++listIndex)
		{
		size_t numVertices=isoFragments[listIndex].size();
		if(numVertices!=0)
			{
			glBindBufferARB(GL_ARRAY_BUFFER_ARB,dataItem->isoFragmentVertexBufferObjectIds[listIndex]);
			
			if(dataItem->isoFragmentVersions[listIndex]!=isoFragmentVersions[listIndex])
				{
				const IsosurfaceVertex* vertices=&isoFragments[listIndex][0];
				glBufferDataARB(GL_ARRAY_BUFFER_ARB,numVertices*sizeof(IsosurfaceVertex),vertices,GL_DYNAMIC_DRAW_ARB);
				dataItem->isoFragmentVersions[listIndex]=isoFragmentVersions[listIndex];
				}
			
			glInterleavedArrays(GL_N3F_V3F,0,0);
			glDrawArrays(GL_TRIANGLES,0,numVertices);
			}
		}
	
	glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	}

namespace {

/****************
Helper functions:
****************/

inline void writeLine(IO::File& file,const char* formatString,...)
	{
	/* Print the line into a buffer: */
	char text[2048]; // Buffer for strings - hopefully long enough...
	va_list ap;
	va_start(ap,formatString);
	vsnprintf(text,sizeof(text),formatString,ap);
	va_end(ap);
	
	/* Write the buffer to the binary file: */
	file.write(text,strlen(text));
	}

}

void EditableGrid::exportSurface(IO::File& file) const
	{
	/*********************************************************************
	Create a list of isosurface vertices and store vertex indices with
	each edge in the grid:
	*********************************************************************/
	
	std::vector<IsosurfaceVertex> isosurfaceVertices; // List of isosurface vertices
	unsigned int isosurfaceVertexIndex=0;
	
	/* Create a hash table to associate isosurface vertex indices with grid edge indices: */
	Misc::HashTable<ptrdiff_t,unsigned int> edgeVertexMap(101);
	
	/* Process all edges in x, y, and z directions: */
	for(int edgeDirection=0;edgeDirection<3;++edgeDirection)
		{
		Index max=numVertices;
		--max[edgeDirection];
		ptrdiff_t stride=vertices.getIncrement(edgeDirection);
		for(Index ci(0);ci[0]<max[0];ci.preInc(max))
			{
			const Vertex* edgeBase=vertices.getAddress(ci);
			if((edgeBase[0].value>=0.5f&&edgeBase[stride].value<0.5f)||(edgeBase[0].value<0.5f&&edgeBase[stride].value>=0.5f))
				{
				/* Calculate an isosurface vertex: */
				IsosurfaceVertex ev;
				
				/* Calculate the interpolation weight for the edge: */
				float w1=(0.5f-edgeBase[0].value)/(edgeBase[stride].value-edgeBase[0].value);
				
				/* Calculate the vertex position along the edge: */
				for(int i=0;i<3;++i)
					ev.position[i]=float(ci[i])*cellSize[i];
				ev.position[edgeDirection]+=cellSize[edgeDirection]*w1;
				
				/* Calculate the normalized edge normal vector by interpolating between the edge vertices' gradients: */
				float normalLen=0.0f;
				for(int i=0;i<3;++i)
					{
					ev.normal[i]=edgeBase[0].gradient[i]*(w1-1.0)-edgeBase[stride].gradient[i]*w1;
					normalLen+=Math::sqr(ev.normal[i]);
					}
				normalLen=Math::sqrt(normalLen);
				for(int i=0;i<3;++i)
					ev.normal[i]/=normalLen;
				
				/* Store the vertex: */
				isosurfaceVertices.push_back(ev);
				edgeVertexMap[vertices.calcLinearIndex(ci)*3+edgeDirection]=isosurfaceVertexIndex;
				++isosurfaceVertexIndex;
				}
			}
		}
	
	/*********************************************************************
	Create triangles by processing each grid cell and retrieving
	isosurface vertex indices from the grid edges:
	*********************************************************************/
	
	std::vector<unsigned int> vertexIndices;
	unsigned int numFaces=0;
	
	for(Index ci(0);ci[0]<numCells[0];ci.preInc(numCells))
		{
		/* Get a pointer to the cell's base vertex and calculate the base vertex' position: */
		ptrdiff_t cellIndex=vertices.calcLinearIndex(ci);
		const Vertex* cellBase=vertices.getArray()+cellIndex;
		
		/* Determine the cell's marching cubes case index: */
		int caseIndex=0x0;
		for(int i=0;i<8;++i)
			if(cellBase[cellVertexOffsets[i]].value>=0.5f)
				caseIndex|=1<<i;
		
		unsigned int edgeIsosurfaceVertexIndices[12];
		if(fragmentNumTriangles[caseIndex]!=0)
			{
			/* Retrieve the indices of the edge intersection points: */
			int cem=edgeMasks[caseIndex];
			for(int edge=0;edge<12;++edge)
				{
				if(cem&(1<<edge))
					{
					/* Calculate the edge index: */
					ptrdiff_t edgeIndex=(cellIndex+cellVertexOffsets[edgeVertexIndices[edge][0]])*3+edge/4;
					
					/* Retrieve the edge's isosurface vertex: */
					edgeIsosurfaceVertexIndices[edge]=edgeVertexMap[edgeIndex].getDest();
					}
				}
			
			/* Create the triangles for the cell's isosurface case: */
			for(int fragment=0;fragment<fragmentNumTriangles[caseIndex];++fragment)
				{
				for(int i=0;i<3;++i)
					vertexIndices.push_back(edgeIsosurfaceVertexIndices[triangleEdgeIndices[caseIndex][fragment*3+i]]);
				++numFaces;
				}
			}
		}
	
	/*********************************************************************
	Write the mesh file:
	*********************************************************************/
	
	/* Write a little-endian binary PLY file: */
	file.setEndianness(Misc::LittleEndian);
	
	/* Write the mesh file header: */
	writeLine(file,"ply\n");
	writeLine(file,"format binary_little_endian 1.0\n");
	writeLine(file,"element vertex %u\n",isosurfaceVertexIndex);
	writeLine(file,"property float32 x\n");
	writeLine(file,"property float32 y\n");
	writeLine(file,"property float32 z\n");
	writeLine(file,"property float32 nx\n");
	writeLine(file,"property float32 ny\n");
	writeLine(file,"property float32 nz\n");
	writeLine(file,"element face %u\n",numFaces);
	writeLine(file,"property list uint8 uint32 vertex_indices\n");
	writeLine(file,"end_header\n");
	
	/* Write the vertices: */
	for(std::vector<IsosurfaceVertex>::iterator ivIt=isosurfaceVertices.begin();ivIt!=isosurfaceVertices.end();++ivIt)
		{
		for(int i=0;i<3;++i)
			file.write<Misc::Float32>(ivIt->position[i]);
		for(int i=0;i<3;++i)
			file.write<Misc::Float32>(ivIt->normal[i]);
		}
	
	/* Write the face vertex indices: */
	std::vector<unsigned int>::iterator viIt=vertexIndices.begin();
	for(unsigned int faceIndex=0;faceIndex<numFaces;++faceIndex)
		{
		file.write<Misc::UInt8>(3U);
		for(int i=0;i<3;++i,++viIt)
			file.write<Misc::UInt32>(*viIt);
		}
	}
