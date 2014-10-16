/***********************************************************************
WidgetAlgorithms - Functions to perform algorithms on trees of widgets.
Copyright (c) 2004-2010 Oliver Kreylos

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

#ifndef GLMOTIF_WIDGETALGORITHMS_INCLUDED
#define GLMOTIF_WIDGETALGORITHMS_INCLUDED

/* Forward declarations: */
namespace GLMotif {
class Widget;
}

namespace GLMotif {

Widget* getPreviousWidget(Widget* widget); // Returns the predecessor of the given widget inside the same top-level widget in depth-first order
Widget* getNextWidget(Widget* widget); // Returns the successor of the given widget inside the same top-level widget in depth-first order
template <class FunctorParam>
void traverseWidgetTree(Widget* rootWidget,const FunctorParam& functor);
template <class FunctorParam>
void traverseWidgetTree(Widget* rootWidget,FunctorParam& functor);

}

#ifndef GLMOTIF_WIDGETALGORITHMS_IMPLEMENTATION
#include <GLMotif/WidgetAlgorithms.icpp>
#endif

#endif
