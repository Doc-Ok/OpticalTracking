/***********************************************************************
MouseCursorFaker - Helper class to render a fake mouse cursor for cases
where a hardware cursor is not available, or not appropriate.
Copyright (c) 2009-2014 Oliver Kreylos

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

#ifndef VRUI_INTERNAL_MOUSECURSORFAKER_INCLUDED
#define VRUI_INTERNAL_MOUSECURSORFAKER_INCLUDED

#include <Geometry/ComponentArray.h>
#include <Geometry/Vector.h>
#include <Geometry/Box.h>
#include <GL/gl.h>
#include <GL/GLObject.h>
#include <Images/RGBAImage.h>
#include <Vrui/Geometry.h>
#include <Vrui/TransparentObject.h>

/* Forward declarations: */
namespace Vrui {
class InputDevice;
}

namespace Vrui {

class MouseCursorFaker:public GLObject,public Vrui::TransparentObject
	{
	/* Embedded classes: */
	private:
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		public:
		GLuint textureObjectId; // ID of the mouse cursor texture object
		
		/* Constructors and destructors: */
		DataItem(void);
		virtual ~DataItem(void);
		};
	
	/* Elements: */
	private:
	const InputDevice* device; // Pointer to input device followed by the fake mouse cursor
	Images::RGBAImage cursorImage; // Image containing the mouse cursor texture
	Geometry::Box<float,2> cursorTexCoordBox; // Texture coordinate box for the mouse cursor texture
	Size cursorSize; // Size of mouse cursor in physical coordinate units (depth ignored)
	Vector cursorHotspot; // Mouse cursor hotspot coordinates (depth ignored)
	
	/* Constructors and destructors: */
	public:
	MouseCursorFaker(const InputDevice* sDevice,const char* cursorImageFileName,unsigned int cursorNominalSize); // Creates a fake mouse cursor by reading a cursor of the given nominal size from the given cursor file
	
	/* Methods from GLObject: */
	virtual void initContext(GLContextData& contextData) const;
	
	/* Methods from TransparentObject: */
	virtual void glRenderActionTransparent(GLContextData& contextData) const;
	
	/* New methods: */
	const Size& getCursorSize(void) const // Returns the current size of the mouse cursor in physical coordinate units
		{
		return cursorSize;
		}
	const Vector& getCursorHotspot(void) const // Returns the cursor hotspot coordinates
		{
		return cursorHotspot;
		}
	void setCursorSize(const Size& newCursorSize); // Sets the cursor size in physical coordinates
	void setCursorHotspot(const Vector& newCursorHotspot); // Sets the cursor hotspot coordinates
	};

}

#endif
