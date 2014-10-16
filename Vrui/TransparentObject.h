/***********************************************************************
TransparentObject - Base class for objects that require a second
rendering pass with alpha blending enabled.
Copyright (c) 2007 Oliver Kreylos

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

#ifndef VRUI_TRANSPARENTOBJECT_INCLUDED
#define VRUI_TRANSPARENTOBJECT_INCLUDED

/* Forward declarations: */
class GLContextData;

namespace Vrui {

class TransparentObject
	{
	/* Elements: */
	private:
	static TransparentObject* head; // Head of the list of transparent objects
	static TransparentObject* tail; // Tail of the list of transparent objects
	TransparentObject* pred; // Pointer to predecessor in the list
	TransparentObject* succ; // Pointer to successor in the list
	
	/* Constructors and destructors: */
	public:
	TransparentObject(void); // Adds the newly created object to Vrui's transparent rendering pass
	virtual ~TransparentObject(void); // Removes the newly created object from Vrui's transparent rendering pass
	
	/* Methods: */
	virtual void glRenderActionTransparent(GLContextData& contextData) const =0; // Rendering method
	static bool needRenderPass(void) // Returns true if there are any registered transparent objects
		{
		return head!=0;
		}
	static void transparencyPass(GLContextData& contextData); // Calls the transparent rendering methods of all transparent objects; does not change OpenGL state
	};

}

#endif
