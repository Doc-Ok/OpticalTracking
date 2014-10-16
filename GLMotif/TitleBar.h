/***********************************************************************
TitleBar - Class for title bars used to move popup windows in 3D space.
Mostly a wrapper class around label; moving functionality must be
mplemented by the surrounding toolkit in collaboration with a widget
manager.
Copyright (c) 2005 Oliver Kreylos

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

#ifndef GLMOTIF_TITLEBAR_INCLUDED
#define GLMOTIF_TITLEBAR_INCLUDED

#include <GLMotif/Label.h>
#include <GLMotif/Draggable.h>

namespace GLMotif {

class TitleBar:public Label,public Draggable
	{
	/* Constructors and destructors: */
	public:
	TitleBar(const char* sName,Container* sParent,const char* sLabel,const GLFont* sFont,bool manageChild =true); // Deprecated
	TitleBar(const char* sName,Container* sParent,const char* sLabel,bool manageChild =true);
	};

}

#endif
