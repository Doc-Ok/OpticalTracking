/***********************************************************************
WidgetStateHelper - Helper functions to read or write top-level widget
states from or to configuration files.
Copyright (c) 2010-2013 Oliver Kreylos

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

#include <GLMotif/WidgetStateHelper.h>

#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Geometry/OrthogonalTransformation.h>
#include <Geometry/GeometryValueCoders.h>
#include <GL/gl.h>
#include <GL/GLBox.h>
#include <GL/GLValueCoders.h>
#include <GLMotif/Widget.h>
#include <GLMotif/WidgetManager.h>

namespace GLMotif {

void writeTopLevelPosition(const GLMotif::Widget* topLevelWidget,Misc::ConfigurationFileSection& configFileSection)
	{
	/* Get the widget's root: */
	const Widget* root=topLevelWidget->getRoot();
	
	/* Write the root's exterior size: */
	configFileSection.storeValue<Box>("./widgetExterior",root->getExterior());
	
	/* Write the root's widget transformation: */
	configFileSection.storeValue<WidgetManager::Transformation>("./widgetTransform",root->getManager()->calcWidgetTransformation(root));
	}

void readTopLevelPosition(GLMotif::Widget* topLevelWidget,const Misc::ConfigurationFileSection& configFileSection,bool useDefaults)
	{
	/* Get the widget's root: */
	Widget* root=topLevelWidget->getRoot();
	
	/* Read the root's exterior size: */
	Box exterior;
	if(useDefaults)
		exterior=configFileSection.retrieveValue<Box>("./widgetExterior",root->getExterior());
	else
		exterior=configFileSection.retrieveValue<Box>("./widgetExterior");
	root->resize(exterior);
	
	/* Read the root's widget transformation: */
	WidgetManager::Transformation transform;
	if(useDefaults)
		transform=configFileSection.retrieveValue<WidgetManager::Transformation>("./widgetTransform",root->getManager()->calcWidgetTransformation(root));
	else
		transform=configFileSection.retrieveValue<WidgetManager::Transformation>("./widgetTransform");
	root->getManager()->setPrimaryWidgetTransformation(root,transform);
	}

}
