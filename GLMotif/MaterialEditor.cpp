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

#include <GLMotif/MaterialEditor.h>

#include <GLMotif/StyleSheet.h>

namespace GLMotif {

/*******************************
Methods of class MaterialEditor:
*******************************/

void MaterialEditor::colorSelectorCallback(HSVColorSelector::ValueChangedCallbackData* cbData)
	{
	/* Update the material component managed by the color selector reporting the event: */
	if(cbData->colorSelector==ambient)
		material.ambient=cbData->newColor;
	else if(cbData->colorSelector==diffuse)
		material.diffuse=cbData->newColor;
	else if(cbData->colorSelector==specular)
		material.specular=cbData->newColor;
	else if(cbData->colorSelector==emissive)
		material.emission=cbData->newColor;
	
	/* Call the value changed callbacks: */
	ValueChangedCallbackData myCbData(this,material);
	valueChangedCallbacks.call(&myCbData);
	}

void MaterialEditor::sliderCallback(TextFieldSlider::ValueChangedCallbackData* cbData)
	{
	/* Update the material's shininess: */
	material.shininess=cbData->value;
	
	/* Call the value changed callbacks: */
	ValueChangedCallbackData myCbData(this,material);
	valueChangedCallbacks.call(&myCbData);
	}

MaterialEditor::MaterialEditor(const char* sName,Container* sParent,bool sManageChild)
	:RowColumn(sName,sParent,false),
	 material(GLMaterial::Color(0.8f,0.8f,0.8f),GLMaterial::Color(0.5f,0.5f,0.5f),16.0f),
	 ambient(0),diffuse(0),emissive(0),specular(0),shininess(0)
	{
	/* Create the composite widget layout: */
	setOrientation(VERTICAL);
	setPacking(PACK_TIGHT);
	setNumMinorWidgets(1);
	
	/* Create the child widgets: */
	RowColumn* row1=new RowColumn("Row1",this,false);
	row1->setOrientation(HORIZONTAL);
	row1->setPacking(PACK_TIGHT);
	row1->setNumMinorWidgets(2);
	
	ambient=new HSVColorSelector("AmbientColorSelector",row1);
	ambient->setCurrentColor(material.ambient);
	ambient->getValueChangedCallbacks().add(this,&MaterialEditor::colorSelectorCallback);
	
	new Label("AmbientLabel",row1,"Ambient");
	
	diffuse=new HSVColorSelector("DiffuseColorSelector",row1);
	diffuse->setCurrentColor(material.diffuse);
	diffuse->getValueChangedCallbacks().add(this,&MaterialEditor::colorSelectorCallback);
	
	new Label("DiffuseLabel",row1,"Diffuse");
	
	emissive=new HSVColorSelector("EmissiveColorSelector",row1);
	emissive->setCurrentColor(material.emission);
	emissive->getValueChangedCallbacks().add(this,&MaterialEditor::colorSelectorCallback);
	
	new Label("EmissiveLabel",row1,"Emissive");
	
	row1->manageChild();
	
	RowColumn* row2=new RowColumn("Row2",this,false);
	row2->setOrientation(HORIZONTAL);
	row2->setPacking(PACK_TIGHT);
	row2->setNumMinorWidgets(2);
	
	specular=new HSVColorSelector("SpecularColorSelector",row2);
	specular->setCurrentColor(material.specular);
	specular->getValueChangedCallbacks().add(this,&MaterialEditor::colorSelectorCallback);
	
	new Label("SpecularLabel",row2,"Specular");
	
	shininess=new TextFieldSlider("ShininessSlider",row2,4,getStyleSheet()->fontHeight*5.0f);
	shininess->getTextField()->setFieldWidth(3);
	shininess->getTextField()->setPrecision(0);
	shininess->getTextField()->setFloatFormat(TextField::FIXED);
	shininess->setSliderMapping(TextFieldSlider::LINEAR);
	shininess->setValueType(TextFieldSlider::FLOAT);
	shininess->setValueRange(0.0,128.0,1.0);
	shininess->setValue(material.shininess);
	shininess->getValueChangedCallbacks().add(this,&MaterialEditor::sliderCallback);
	
	new Label("ShininessLabel",row2,"Shininess");
	
	row2->manageChild();
	
	if(sManageChild)
		manageChild();
	}

void MaterialEditor::setMaterial(const GLMaterial& newMaterial)
	{
	/* Update the current material properties: */
	material=newMaterial;
	
	/* Update the component widgets: */
	ambient->setCurrentColor(material.ambient);
	diffuse->setCurrentColor(material.diffuse);
	emissive->setCurrentColor(material.emission);
	specular->setCurrentColor(material.specular);
	shininess->setValue(material.shininess);
	}

}
