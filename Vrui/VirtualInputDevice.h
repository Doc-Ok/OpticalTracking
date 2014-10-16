/***********************************************************************
VirtualInputDevice - Helper class to manage ungrabbed virtual input
devices.
Copyright (c) 2005 Oliver Kreylos

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

#ifndef VRUI_VIRTUALINPUTDEVICE_INCLUDED
#define VRUI_VIRTUALINPUTDEVICE_INCLUDED

#include <Geometry/Vector.h>
#include <Vrui/Geometry.h>
#include <Vrui/GlyphRenderer.h>

/* Forward declarations: */
namespace Misc {
class ConfigurationFileSection;
}
class GLContextData;
namespace Vrui {
class InputDevice;
}

namespace Vrui {

class VirtualInputDevice
	{
	/* Elements: */
	private:
	GlyphRenderer* glyphRenderer; // Pointer to the glyph renderer used to render virtual input devices
	Vector buttonOffset; // Offset from virtual input device position to center of button panel
	Vector buttonPanelDirection; // Direction of button panel
	Scalar buttonSize; // Size of glyphs for virtual input device buttons
	Scalar buttonSpacing; // Spacing between button centers
	Glyph offButtonGlyph,onButtonGlyph; // Glyphs used to render buttons
	Glyph deviceGlyph; // Glyph used to render virtual input devices
	
	/* Constructors and destructors: */
	public:
	VirtualInputDevice(GlyphRenderer* sGlyphRenderer,const Misc::ConfigurationFileSection& configFileSection); // Creates object by reading settings from given configuration file section
	~VirtualInputDevice(void);
	
	/* Methods: */
	bool pick(const InputDevice* device,const Point& pos) const; // Returns true if the given position is inside the given virtual input device
	Scalar pick(const InputDevice* device,const Ray& ray) const; // Returns true if the given ray intersects the given virtual input device
	int pickButton(const InputDevice* device,const Point& pos) const; // Returns index of the button whose representation contains the given position (or -1 if no button)
	int pickButton(const InputDevice* device,const Ray& ray) const; // Returns index of the button whose representation is intersected by the given ray (or -1 if no button)
	void renderDevice(const InputDevice* device,bool navigational,const GlyphRenderer::DataItem* glyphRendererContextDataItem,GLContextData& contextData) const; // Renders the given virtual input device into the given OpenGL context
	};

}

#endif
