/***********************************************************************
ScrolledImage - Compound widget containing an image, and a vertical and
horizontal scroll bar.
Copyright (c) 2011-2014 Oliver Kreylos

This file is part of the GLMotif Widget Library (GLMotif).

The GLMotif Widget Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GLMotif Widget Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the GLMotif Widget Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef GLMOTIF_SCROLLEDIMAGE_INCLUDED
#define GLMOTIF_SCROLLEDIMAGE_INCLUDED

#include <GLMotif/Types.h>
#include <GLMotif/Container.h>
#include <GLMotif/Image.h>
#include <GLMotif/ScrollBar.h>

namespace GLMotif {

class ScrolledImage:public Container
	{
	/* Elements: */
	private:
	Vector preferredSize; // The widget's preferred size not including the scroll bars
	Image* image; // Pointer to the image widget
	ScrollBar* horizontalScrollBar; // Pointer to the horizontal scroll bar
	ScrollBar* verticalScrollBar; // Pointer to the vertical scroll bar
	GLfloat zoomFactor; // Zoom factor between image's original resolution and display resolution
	
	/* Private methods: */
	void init(bool manageChild); // Initializes the widget
	void scrollBarCallback(ScrollBar::ValueChangedCallbackData* cbData); // Method called on horizontal or vertical scrolling
	
	/* Constructors and destructors: */
	public:
	ScrolledImage(const char* sName,Container* sParent,const Images::RGBImage& sImage,const GLfloat sResolution[2],bool manageChild =true);
	ScrolledImage(const char* sName,Container* sParent,const char* imageFileName,const GLfloat sResolution[2],bool manageChild =true);
	ScrolledImage(const char* sName,Container* sParent,Image* sImage,bool manageChild =true); // Creates a scrolled image shell around an existing unmanaged image widget; reparents given widget
	virtual ~ScrolledImage(void);
	
	/* Methods inherited from Widget: */
	virtual Vector calcNaturalSize(void) const;
	virtual ZRange calcZRange(void) const;
	virtual void resize(const Box& newExterior);
	virtual void draw(GLContextData& contextData) const;
	virtual bool findRecipient(Event& event);
	
	/* Methods inherited from Container: */
	virtual void addChild(Widget* newChild);
	virtual void removeChild(Widget* removeChild);
	virtual void requestResize(Widget* child,const Vector& newExteriorSize);
	virtual Widget* getFirstChild(void);
	virtual Widget* getNextChild(Widget* child);
	
	/* New methods: */
	void setPreferredSize(const Vector& newPreferredSize); // Sets the widget's preferred size not including the scroll bars
	const Image* getImage(void) const // Returns a pointer to the image widget
		{
		return image;
		}
	Image* getImage(void) // Ditto
		{
		return image;
		}
	GLfloat getZoomFactor(void) const // Returns the image's current zoom factor
		{
		return zoomFactor;
		}
	void setZoomFactor(GLfloat newZoomFactor); // Sets the image's zoom factor
	void setZoomFactor(GLfloat newZoomFactor,const Vector& fixedPoint); // Sets the image's zoom factor around the given point in widget coordinates
	};

}

#endif
