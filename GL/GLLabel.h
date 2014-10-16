/***********************************************************************
GLLabel - Class to render 3D text strings using texture-based fonts.
Copyright (c) 2010-2012 Oliver Kreylos

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

#ifndef GLLABEL_INCLUDED
#define GLLABEL_INCLUDED

#include <vector>
#include <GL/gl.h>
#include <GL/TLSHelper.h>
#include <GL/GLColor.h>
#include <GL/GLBox.h>
#include <GL/GLString.h>
#include <GL/GLObject.h>

/* Forward declarations: */
class GLFont;

class GLLabel:public GLString,public GLObject
	{
	/* Embedded classes: */
	public:
	typedef GLColor<GLfloat,4> Color; // Type for colors
	typedef GLBox<GLfloat,3> Box; // Type for model-space boxes
	
	class DeferredRenderer // Class to gather GLLabel objects during a rendering pass and draw them en-bloc at the end of the pass
		{
		/* Elements: */
		private:
		static GL_THREAD_LOCAL(DeferredRenderer*) currentDeferredRenderer; // Pointer to the currently active deferred renderer
		GLContextData& contextData; // Reference to the OpenGL context data object
		DeferredRenderer* previousDeferredRenderer; // Pointer to the deferred renderer that was suspended when this one was installed
		std::vector<const GLLabel*> gatheredLabels; // List of gathered GLLabel objects
		
		/* Constructors and destructors: */
		public:
		DeferredRenderer(GLContextData& sContextData); // Creates a deferred renderer and installs it in the current OpenGL context
		~DeferredRenderer(void); // Destroys the deferred renderer and uninstalls it after rendering gathered labels
		
		/* Methods: */
		void draw(void); // Draws all gathered GLLabel objects and clears the list
		static bool addLabel(const GLLabel* label); // Adds a GLLabel object to the deferred renderer's list; returns false if label needs to be drawn immediately
		};
	
	private:
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		public:
		GLuint textureObjectId;
		unsigned int version; // Version number of string currently in texture object

		/* Constructors and destructors: */
		DataItem(void)
			:version(0)
			{
			glGenTextures(1,&textureObjectId);
			}
		~DataItem(void)
			{
			glDeleteTextures(1,&textureObjectId);
			}
		};
	
	friend class DeferredRenderer;
	
	/* Elements: */
	const GLFont* font; // Font associated with this label
	Color background; // String's background color
	Color foreground; // String's foreground color
	unsigned int version; // Monotonically increasing version number of string
	Box labelBox; // Position of label in model space
	
	/* Constructors and destructors: */
	public:
	GLLabel(void) // Dummy constructor
		:font(0),version(1)
		{
		}
	GLLabel(const char* sString,const GLFont& sFont); // Creates label from C-style string and font
	GLLabel(const char* sStringBegin,const char* sStringEnd,const GLFont& sFont); // Creates label from character array and font
	GLLabel(const GLString& sString,const GLFont& sFont); // Creates label from GL string and font
	GLLabel(const GLLabel& source); // Copy constructor
	GLLabel& operator=(const GLLabel& source); // Assignment operator
	
	/* Methods from GLString: */
	void setString(const char* newString,const GLFont& newFont); // Overrides base class method
	void setString(const char* newStringBegin,const char* newStringEnd,const GLFont& newFont); // Overrides base class method
	void adoptString(char* newString,const GLFont& font); // Overrides base class method
	void adoptString(GLsizei newLength,char* newString,const GLFont& font); // Overrides base class method
	void setFont(const GLFont& newFont); // Overrides base class method
	
	/* Methods from GLObject: */
	virtual void initContext(GLContextData& contextData) const;
	
	/* New methods: */
	const GLFont* getFont(void) const // Returns the label's font
		{
		return font;
		}
	const Color& getBackground(void) const // Returns the background color
		{
		return background;
		}
	const Color& getForeground(void) const // Returns the foreground color
		{
		return foreground;
		}
	void invalidate(void) // Invalidates the currently cached version of the label string
		{
		++version;
		}
	Box::Vector calcNaturalSize(void) const; // Returns the size of the label box as dictated by the font
	const Box::Vector& getLabelSize(void) const // Returns the size of the label box
		{
		return labelBox.size;
		}
	const Box& getLabelBox(void) const // Returns the label box
		{
		return labelBox;
		}
	void setString(const char* newString); // Replaces the string
	void setString(const char* newStringBegin,const char* newStringEnd); // Ditto
	void setString(const GLString& newString); // Ditto
	void adoptString(char* newString); // Takes ownership of the given new[]-allocated string
	void adoptString(GLsizei newLength,char* newString); // Ditto, if new string length is already known
	template <class InputColorType>
	void setBackground(const InputColorType& newBackground) // Sets the background color
		{
		/* Set the background color: */
		background=newBackground;
		
		/* Increment the version number: */
		++version;
		}
	template <class InputColorType>
	void setForeground(const InputColorType& newForeground) // Sets the foreground color
		{
		/* Set the foreground color: */
		foreground=newForeground;
		
		/* Increment the version number: */
		++version;
		}
	void resetBox(void); // Resets the label box to the default defined by the label's font
	void setOrigin(const Box::Vector& newOrigin); // Moves the label's origin to the given model-space position
	void clipBox(const Box& clipBox); // Clips the label to the given box and adjusts texture coordinates accordingly
	GLint calcCharacterIndex(GLfloat modelPos) const; // Returns the position of the string's character at the given model-space position
	GLfloat calcCharacterPos(GLint characterPos) const; // Returns the model space position of the right edge of the given character
	void draw(GLContextData& contextData) const; // Draws the label at the current model-space position and size
	void draw(GLsizei selectionStart,GLsizei selectionEnd,const Color& selectionBackgroundColor,const Color& selectionForegroundColor,GLContextData& contextData) const; // Ditto, with additional selection range and selection colors
	};

#endif
