/***********************************************************************
TransparentObject - Base class for objects that require a second
rendering pass with alpha blending enabled.
Copyright (c) 2007 Oliver Kreylos

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

#include <Vrui/TransparentObject.h>

namespace Vrui {

/******************************************
Static elements of class TransparentObject:
******************************************/

TransparentObject* TransparentObject::head=0;
TransparentObject* TransparentObject::tail=0;

/**********************************
Methods of class TransparentObject:
**********************************/

TransparentObject::TransparentObject(void)
	{
	/* Add the object to the end of the list: */
	pred=tail;
	succ=0;
	if(tail!=0)
		tail->succ=this;
	else
		head=this;
	tail=this;
	}

TransparentObject::~TransparentObject(void)
	{
	/* Remove the object from the list: */
	if(pred!=0)
		pred->succ=succ;
	else
		head=succ;
	if(succ!=0)
		succ->pred=pred;
	else
		tail=pred;
	}

void TransparentObject::transparencyPass(GLContextData& contextData)
	{
	/* Call rendering method of all registered transparent objects: */
	for(const TransparentObject* toPtr=head;toPtr!=0;toPtr=toPtr->succ)
		toPtr->glRenderActionTransparent(contextData);
	}

}
