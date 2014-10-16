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

#ifndef GLMOTIF_WIDGETSTATEHELPER_INCLUDED
#define GLMOTIF_WIDGETSTATEHELPER_INCLUDED

/* Forward declarations: */
namespace Misc {
class ConfigurationFileSection;
}
namespace GLMotif {
class Widget;
}

namespace GLMotif {

void writeTopLevelPosition(const GLMotif::Widget* topLevelWidget,Misc::ConfigurationFileSection& configFileSection); // Writes size, position, and orientation of given top-level widget to configuration file section
void readTopLevelPosition(GLMotif::Widget* topLevelWidget,const Misc::ConfigurationFileSection& configFileSection,bool useDefaults =true); // Reads size, position, and orientation of given top-level widget from configuration file section; uses current values as default if useDefaults is true; throws exception if values are missing otherwise

}

#endif
