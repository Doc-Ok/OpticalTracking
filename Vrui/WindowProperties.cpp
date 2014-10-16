/***********************************************************************
WindowProperties - Helper structure to fine-tune visual properties of
Vrui rendering windows.
Copyright (c) 2012 Oliver Kreylos

This file is part of the Virtual Reality User Interface Library (Vrui).

The Virtual Reality User Interface Library is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The Virtual Reality User Interface Library is distributed in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Virtual Reality User Interface Library; if not, write to the
Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#include <Vrui/WindowProperties.h>

namespace Vrui {

/*********************************
Methods of class WindowProperties:
*********************************/

WindowProperties::WindowProperties(void)
	:depthBufferSize(16),numAuxBuffers(0),stencilBufferSize(0)
	{
	for(int i=0;i<3;++i)
		colorBufferSize[i]=8;
	colorBufferSize[3]=0;
	for(int i=0;i<4;++i)
		accumBufferSize[i]=0;
	}

void WindowProperties::setColorBufferSize(int rgbSize,int alphaSize)
	{
	for(int i=0;i<3;++i)
		colorBufferSize[i]=rgbSize;
	colorBufferSize[3]=alphaSize;
	}

void WindowProperties::setAccumBufferSize(int rgbSize,int alphaSize)
	{
	for(int i=0;i<3;++i)
		accumBufferSize[i]=rgbSize;
	accumBufferSize[3]=alphaSize;
	}

void WindowProperties::merge(const WindowProperties& other)
	{
	/* Take the maximum of all requested properties: */
	for(int i=0;i<4;++i)
		if(colorBufferSize[i]<other.colorBufferSize[i])
			colorBufferSize[i]=other.colorBufferSize[i];
	if(depthBufferSize<other.depthBufferSize)
		depthBufferSize=other.depthBufferSize;
	if(numAuxBuffers<other.numAuxBuffers)
		numAuxBuffers=other.numAuxBuffers;
	if(stencilBufferSize<other.stencilBufferSize)
		stencilBufferSize=other.stencilBufferSize;
	for(int i=0;i<4;++i)
		if(accumBufferSize[i]<other.accumBufferSize[i])
			accumBufferSize[i]=other.accumBufferSize[i];
	}

}
