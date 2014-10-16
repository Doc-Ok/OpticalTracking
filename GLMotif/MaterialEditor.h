/***********************************************************************
MaterialEditor - Class for composite widgets to display and edit OpenGL
material properties.
Copyright (c) 2013 Oliver Kreylos

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

#ifndef GLMOTIF_MATERIALEDITOR_INCLUDED
#define GLMOTIF_MATERIALEDITOR_INCLUDED

#include <Misc/CallbackData.h>
#include <Misc/CallbackList.h>
#include <GL/gl.h>
#include <GL/GLMaterial.h>
#include <GLMotif/RowColumn.h>
#include <GLMotif/HSVColorSelector.h>
#include <GLMotif/TextFieldSlider.h>

namespace GLMotif {

class MaterialEditor:public RowColumn
	{
	/* Embedded classes: */
	public:
	class ValueChangedCallbackData:public Misc::CallbackData
		{
		/* Elements: */
		public:
		MaterialEditor* materialEditor; // Pointer to the material editor widget causing the event
		const GLMaterial& newMaterial; // The new material properties
		
		/* Constructors and destructors: */
		ValueChangedCallbackData(MaterialEditor* sMaterialEditor,const GLMaterial& sNewMaterial)
			:materialEditor(sMaterialEditor),newMaterial(sNewMaterial)
			{
			}
		};
	
	/* Elements: */
	private:
	GLMaterial material; // The currently displayed material properties
	HSVColorSelector* ambient; // Color selector for the ambient material color
	HSVColorSelector* diffuse; // Color selector for the diffuse material color
	HSVColorSelector* emissive; // Color selector for the emissive material color
	HSVColorSelector* specular; // Color selector for the specular material color
	TextFieldSlider* shininess; // Slider for specular shininess
	Misc::CallbackList valueChangedCallbacks; // List of callbacks to be called when the current material changes due to a user interaction
	
	/* Private methods: */
	private:
	void colorSelectorCallback(HSVColorSelector::ValueChangedCallbackData* cbData);
	void sliderCallback(TextFieldSlider::ValueChangedCallbackData* cbData);
	
	/* Constructors and destructors: */
	public:
	MaterialEditor(const char* sName,Container* sParent,bool sManageChild =true);
	
	/* New methods: */
	void setMaterial(const GLMaterial& newMaterial); // Sets the currently displayed material properties
	const GLMaterial& getMaterial(void) const // Returns the currently displayed material properties
		{
		return material;
		}
	Misc::CallbackList& getValueChangedCallbacks(void) // Returns list of value changed callbacks
		{
		return valueChangedCallbacks;
		}
	};

}

#endif
