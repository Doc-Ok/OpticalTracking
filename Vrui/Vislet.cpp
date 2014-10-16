/***********************************************************************
Vislet - Abstract base class for application-independent visualization
plug-ins that can be loaded into Vrui applications on demand.
Copyright (c) 2006-2012 Oliver Kreylos

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

#include <Misc/ThrowStdErr.h>

#include <Vrui/Vislet.h>

namespace Vrui {

/******************************
Methods of class VisletFactory:
******************************/

VisletFactory::VisletFactory(const char* sClassName,VisletManager& visletManager)
	:Plugins::Factory(sClassName)
	{
	}

Vislet* VisletFactory::createVislet(int numVisletArguments,const char* const visletArguments[]) const
	{
	Misc::throwStdErr("Cannot create vislet of abstract class %s",getClassName());
	
	/* Dummy statement to make the compiler happy: */
	return 0;
	}

void VisletFactory::destroyVislet(Vislet*) const
	{
	Misc::throwStdErr("Cannot destroy vislet of abstract class %s",getClassName());
	}

/***********************
Methods of class Vislet:
***********************/

Vislet::Vislet(void)
	:active(false)
	{
	}

Vislet::~Vislet(void)
	{
	}

VisletFactory* Vislet::getFactory(void) const
	{
	Misc::throwStdErr("Abstract vislet class has no factory object");
	
	/* Dummy statement to make the compiler happy: */
	return 0;
	}

void Vislet::disable(void)
	{
	active=false;
	}

void Vislet::enable(void)
	{
	active=true;
	}

void Vislet::frame(void)
	{
	}

void Vislet::display(GLContextData&) const
	{
	}

void Vislet::sound(ALContextData&) const
	{
	}

}
