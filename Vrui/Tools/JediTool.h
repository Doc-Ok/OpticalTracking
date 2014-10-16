/***********************************************************************
JediTool - Class for tools using light sabers to point out features in a
3D display.
Copyright (c) 2007-2014 Oliver Kreylos

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

#ifndef VRUI_JEDITOOL_INCLUDED
#define VRUI_JEDITOOL_INCLUDED

#include <string>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <GL/gl.h>
#include <GL/GLObject.h>
#include <Images/RGBImage.h>
#include <Vrui/Geometry.h>
#include <Vrui/TransparentObject.h>

#include <Vrui/PointingTool.h>

namespace Vrui {

class JediTool;

class JediToolFactory:public ToolFactory
	{
	friend class JediTool;
	
	/* Elements: */
	private:
	Scalar lightsaberLength; // Length of light saber billboard
	Scalar lightsaberWidth; // Width of light saber billboard
	Scalar baseOffset; // Amount by how much the light saber billboard is shifted towards the hilt
	std::string lightsaberImageFileName; // Name of image file containing light saber texture
	
	/* Constructors and destructors: */
	public:
	JediToolFactory(ToolManager& toolManager);
	virtual ~JediToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual const char* getButtonFunction(int buttonSlotIndex) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class JediTool:public PointingTool,public GLObject,public TransparentObject
	{
	friend class JediToolFactory;
	
	/* Embedded classes: */
	private:
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		public:
		GLuint textureObjectId; // ID of the light saber texture object
		
		/* Constructors and destructors: */
		DataItem(void)
			{
			glGenTextures(1,&textureObjectId);
			}
		virtual ~DataItem(void)
			{
			glDeleteTextures(1,&textureObjectId);
			}
		};
	
	/* Elements: */
	static JediToolFactory* factory; // Pointer to the factory object for this class
	Images::RGBImage lightsaberImage; // The light saber texture image
	
	/* Transient state: */
	bool active; // Flag if the light saber is active
	double activationTime; // Time at which the light saber was activated
	Point origin; // Origin point of the light saber blade
	Vector axis; // Current light saber blade axis vector
	Scalar length; // Current light saber blade length
	
	/* Constructors and destructors: */
	public:
	JediTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	
	/* Methods from Tool: */
	virtual const ToolFactory* getFactory(void) const;
	virtual void buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData);
	virtual void frame(void);
	
	/* Methods from GLObject: */
	virtual void initContext(GLContextData& contextData) const;
	
	/* Methods from TransparentObject: */
	virtual void glRenderActionTransparent(GLContextData& contextData) const;
	};

}

#endif
