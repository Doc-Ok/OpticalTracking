/***********************************************************************
GlyphRenderer - Class to quickly render several kinds of common glyphs.
Copyright (c) 2004-2013 Oliver Kreylos

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

#ifndef VRUI_GLYPHRENDERER_INCLUDED
#define VRUI_GLYPHRENDERER_INCLUDED

#include <string>
#include <GL/gl.h>
#include <GL/GLMaterial.h>
#include <GL/GLObject.h>
#include <GL/GLContextData.h>
#include <Vrui/Geometry.h>

/* Forward declarations: */
namespace Misc {
class ConfigurationFileSection;
}

namespace Vrui {

class Glyph // Class for reusable visualizations of positions and orientations
	{
	friend class GlyphRenderer;
	
	/* Embedded classes: */
	public:
	enum GlyphType // Enumerated type for glyph types
		{
		CONE=0,CUBE,SPHERE,CROSSBALL,BOX,CURSOR,GLYPHS_END
		};
	
	/* Elements: */
	private:
	bool enabled; // Flag if glyph is enabled
	GlyphType glyphType; // Type of the glyph
	GLMaterial glyphMaterial; // Material for rendering the glyph (not used for cursor glyphs)
	
	/* Private methods: */
	static void render(int glyphType,GLfloat glyphSize); // Renders glyph of given type and size "from scratch"
	
	/* Constructors and destructors: */
	public:
	Glyph(void); // Constructs disabled default glyph
	
	/* Methods: */
	bool isEnabled(void) const // Returns if the glyph is enabled
		{
		return enabled;
		}
	GlyphType getGlyphType(void) const // Returns the glyph type
		{
		return glyphType;
		}
	const GLMaterial& getGlyphMaterial(void) const // Returns the glyph material
		{
		return glyphMaterial;
		}
	GLMaterial& getGlyphMaterial(void) // Ditto
		{
		return glyphMaterial;
		}
	void enable(void); // Enables a glyph without changing glyph type or material
	void enable(GlyphType newGlyphType,const GLMaterial& newGlyphMaterial); // Enables a glyph
	void disable(void); // Disables a glyph
	void setGlyphType(GlyphType newGlyphType); // Changes the glyph's type
	void setGlyphType(const char* newGlyphType); // Changes the glyph's type based on the given literal string
	void setGlyphMaterial(const GLMaterial& newGlyphMaterial); // Changes the glyph's material
	void configure(const Misc::ConfigurationFileSection& configFileSection,const char* glyphTypeTagName,const char* glyphMaterialTagName); // Configures glyph by reading the two given tags from the given configuration file section
	};

class GlyphRenderer:public GLObject
	{
	/* Embedded classes: */
	public:
	struct DataItem:public GLObject::DataItem // Structure for OpenGL per-context data
		{
		friend class GlyphRenderer;
		
		/* Elements: */
		private:
		GLContextData& contextData; // Reference to context data structure containing this data item
		GLuint glyphDisplayLists; // Base ID for consecutive display lists to render glyphs
		GLuint cursorTextureObjectId; // ID of texture object containing cursor glyph texture
		
		/* Constructors and destructors: */
		DataItem(GLContextData& sContextData);
		public:
		virtual ~DataItem(void);
		};
	
	/* Elements: */
	private:
	GLfloat glyphSize; // Overall size of all glyphs
	std::string cursorImageFileName; // Name of file containing cursor image
	unsigned int cursorNominalSize; // Nominal size of cursor image
	
	/* Constructors and destructors: */
	public:
	GlyphRenderer(GLfloat sGlyphSize,std::string sCursorImageFileName,unsigned int sCursorNominalSize); // Initializes glyph renderer for given glyph size
	
	/* Methods: */
	virtual void initContext(GLContextData& contextData) const;
	GLfloat getGlyphSize(void) const
		{
		return glyphSize;
		}
	const DataItem* getContextDataItem(GLContextData& contextData) const // Returns pointer to the context data item for quicker rendering of many glyphs
		{
		/* Return pointer to context data item: */
		return contextData.retrieveDataItem<DataItem>(this);
		}
	void renderGlyph(const Glyph& glyph,const OGTransform& transformation,const DataItem* contextDataItem) const; // Renders glyph into current OpenGL context
	};

}

#endif
