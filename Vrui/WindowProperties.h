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

#ifndef VRUI_WINDOWPROPERTIES_INCLUDED
#define VRUI_WINDOWPROPERTIES_INCLUDED

namespace Vrui {

struct WindowProperties
	{
	/* Elements: */
	public:
	int colorBufferSize[4]; // Minimum required number of bits in each of the main color buffer's channels (R, G, B, Alpha)
	int depthBufferSize; // Minimum required number of bits in the depth buffer
	int numAuxBuffers; // Minimum required number of auxiliary buffers
	int stencilBufferSize; // Minimum required number of bits in the stencil buffer
	int accumBufferSize[4]; // Minimum required number of bits in each of the accumulation buffer's channels (R, G, B, Alpha)

	/* Constructors and destructors: */
	WindowProperties(void); // Creates default property set

	/* Methods: */
	void setColorBufferSize(int rgbSize,int alphaSize =0); // Sets the bit sizes of the main color buffer's channels
	void setAccumBufferSize(int rgbSize,int alphaSize =0); // Sets the bit sizes of the accumulation buffer's channels
	void merge(const WindowProperties& other); // Merges this property set with the given property set
	};

}

#endif
